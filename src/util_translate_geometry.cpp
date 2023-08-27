/* 
 * This file is part of Cycles for Max. (c) Jeffrey Witthuhn
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
#include "util_translate_geometry.h"

#include <list>
#include <random>
#include <set>

#include <render/mesh.h>
#include <render/object.h>
#include <render/scene.h>
#include <RenderingAPI/Translator/Helpers/IMeshFlattener.h>

#include <inode.h>
#include <IParticleObjectExt.h>
#include <mesh.h>
#include <modstack.h>

#include "const_classid.h"
#include "cycles_mikkt_mesh.h"
#include "plugin_mod_properties.h"
#include "rend_logger.h"
#include "rend_shader_manager.h"
#include "util_debug.h"
#include "util_matrix_max.h"

#include <thread>

// This is used to keep the max interface responsive
// The UI will refresh every ITERATIONS_PER_UI_UPDATE items of the same type
// It will also refresh on the first item of each type
constexpr int ITERATIONS_PER_UI_UPDATE = 180000;
#define MAYBE_UI_CALLBACK(x) if (ui_callback != nullptr && (x % ITERATIONS_PER_UI_UPDATE) == 0) ui_callback();

static bool is_node_shadow_catcher(INode* const node, const TimeValue t)
{
	if (node->GetObjectRef()->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		IDerivedObject* const derived_object = dynamic_cast<IDerivedObject*>(node->GetObjectRef());
		for (int i = 0; i < derived_object->NumModifiers(); i++) {
			Modifier* const this_modifier = derived_object->GetModifier(i);
			if (this_modifier != nullptr && this_modifier->ClassID() == CYCLES_MOD_PROPERTIES_CLASS) {
				CyclesPropertiesMod* const cycles_properties = dynamic_cast<CyclesPropertiesMod*>(this_modifier);
				return cycles_properties->GetIsShadowCatcher(t);
			}
		}
	}
	return false;
}

std::shared_ptr<MeshGeometryObj> get_mesh_geometry(INode* const node, View& view, const TimeValue t, const std::vector<int>& mblur_sample_ticks, const std::function<void()> ui_callback)
{
	using MaxSDK::RenderingAPI::TranslationHelpers::IMeshFlattener;

	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilGeomGet", false, true);
	*logger << LogCtl::SEPARATOR;

	const std::shared_ptr<MeshGeometryObj> result = std::make_shared<MeshGeometryObj>();

	// Fill in non-motion fields of output
	Interval mesh_valid = FOREVER;
	{
		const std::unique_ptr<IMeshFlattener> mesh_flattener = IMeshFlattener::AllocateInstance(*node, view, t, mesh_valid);

		// At the end of this function, if no real uvw data has been added, the uvw vector is emptied
		bool mesh_has_uvw_map = false;
		for (size_t submesh_index = 0; submesh_index < mesh_flattener->GetNumSubMeshes(); submesh_index++) {
			if (ui_callback != nullptr) {
				ui_callback();
			}

			const size_t loop_start_vert_count = result->verts.size();
			const size_t loop_start_normal_count = result->normals.size();
			const size_t loop_start_face_count = result->faces.size();
			const size_t loop_start_uvw_vert_count = result->uvw_verts.size();

			MtlID mtl_id;
			std::vector<IPoint3> face_vec;
			std::vector<Point3> vertex_vec;
			std::vector<Point3> normal_vec;
			std::vector<IMeshFlattener::TextureCoordChannel> tex_coord_vec;
			mesh_flattener->GetSubMesh(submesh_index, mtl_id, face_vec, vertex_vec, normal_vec, tex_coord_vec);

			*logger << "Found " << tex_coord_vec.size() << " texture coordinate channels" << LogCtl::WRITE_LINE;

			assert(vertex_vec.size() == normal_vec.size());

			result->verts.resize(loop_start_vert_count + vertex_vec.size());
			for (size_t i = 0; i < vertex_vec.size(); i++) {
				Point3& src_vert = vertex_vec[i];
				ccl::float3& dest_vert = result->verts[loop_start_vert_count + i];
				dest_vert.x = src_vert.x;
				dest_vert.y = src_vert.y;
				dest_vert.z = src_vert.z;
				MAYBE_UI_CALLBACK(i)
			}

			result->normals.resize(loop_start_normal_count + normal_vec.size());
			for (size_t i = 0; i < normal_vec.size(); i++) {
				Point3& src_norm = normal_vec[i];
				ccl::float3& dest_norm = result->normals[loop_start_normal_count + i];
				dest_norm.x = src_norm.x;
				dest_norm.y = src_norm.y;
				dest_norm.z = src_norm.z;
				MAYBE_UI_CALLBACK(i)
			}

			result->faces.reserve(loop_start_face_count + face_vec.size());
			for (size_t i = 0; i < face_vec.size(); i++) {
				IPoint3& src_face = face_vec[i];
				const int v0 = static_cast<int>(loop_start_vert_count) + src_face.x;
				const int v1 = static_cast<int>(loop_start_vert_count) + src_face.y;
				const int v2 = static_cast<int>(loop_start_vert_count) + src_face.z;
				TriangleFace face(v0, v1, v2, mtl_id, true);
				result->faces.push_back(face);
				MAYBE_UI_CALLBACK(i)
			}

			const int CORNERS_PER_FACE = 3;
			const size_t total_corners = face_vec.size() * CORNERS_PER_FACE;
			result->uvw_verts.reserve(loop_start_uvw_vert_count + total_corners);
			bool submesh_tex_coords_copied = false;
			for (IMeshFlattener::TextureCoordChannel& tex_coord_channel : tex_coord_vec) {
				*logger << "Copying texmap channel " << tex_coord_channel.channel_id << LogCtl::WRITE_LINE;

				if (tex_coord_channel.channel_id == 0) {
					// Ignore channel 0, this is not actually a uv map
					continue;
				}
				if (tex_coord_channel.coords.size() != vertex_vec.size()) {
					// UVs either don't exist or are malformed, ignore
					continue;
				}

				// Tex coords map 1:1 with position vertices
				// The output type requires they be mapped to face corners instead
				// Here we walk through the list of corners and grab the appropriate uv coord for each
				for (size_t i = 0; i < face_vec.size(); i++) {
					MAYBE_UI_CALLBACK(i)
					const IPoint3 this_face = face_vec[i];
					const Point3 uv0 = tex_coord_channel.coords[this_face.x];
					const Point3 uv1 = tex_coord_channel.coords[this_face.y];
					const Point3 uv2 = tex_coord_channel.coords[this_face.z];
					const ccl::float3 ccl_uv0 = ccl::make_float3(uv0.x, uv0.y, uv0.z);
					result->uvw_verts.push_back(ccl_uv0);
					const ccl::float3 ccl_uv1 = ccl::make_float3(uv1.x, uv1.y, uv1.z);
					result->uvw_verts.push_back(ccl_uv1);
					const ccl::float3 ccl_uv2 = ccl::make_float3(uv2.x, uv2.y, uv2.z);
					result->uvw_verts.push_back(ccl_uv2);
					if (result->normals.empty() == false) {
						// Convert from UV tangents to XYZ tangent with sign
						const Point3 tu0_max = tex_coord_channel.tangentsU[this_face.x];
						const Point3 tu1_max = tex_coord_channel.tangentsU[this_face.y];
						const Point3 tu2_max = tex_coord_channel.tangentsU[this_face.z];
						const Point3 tv0_max = tex_coord_channel.tangentsV[this_face.x];
						const Point3 tv1_max = tex_coord_channel.tangentsV[this_face.y];
						const Point3 tv2_max = tex_coord_channel.tangentsV[this_face.z];

						const ccl::float3 tu0 = ccl::make_float3(tu0_max.x, tu0_max.y, tu0_max.z);
						const ccl::float3 tu1 = ccl::make_float3(tu1_max.x, tu1_max.y, tu1_max.z);
						const ccl::float3 tu2 = ccl::make_float3(tu2_max.x, tu2_max.y, tu2_max.z);
						const ccl::float3 tv0 = ccl::make_float3(tv0_max.x, tv0_max.y, tv0_max.z);
						const ccl::float3 tv1 = ccl::make_float3(tv1_max.x, tv1_max.y, tv1_max.z);
						const ccl::float3 tv2 = ccl::make_float3(tv2_max.x, tv2_max.y, tv2_max.z);

						const ccl::float3 n0 = result->normals[this_face.x];
						const ccl::float3 n1 = result->normals[this_face.y];
						const ccl::float3 n2 = result->normals[this_face.z];

						const ccl::float3 txyz0 = ccl::normalize((tu0 - n0) * ccl::dot(tu0, n0));
						const ccl::float3 txyz1 = ccl::normalize((tu1 - n1) * ccl::dot(tu1, n1));
						const ccl::float3 txyz2 = ccl::normalize((tu2 - n2) * ccl::dot(tu2, n2));

						const float sign0 = (ccl::dot(ccl::cross(n0, tu0), tv0) < 0.0f) ? -1.0f : 1.0f;
						const float sign1 = (ccl::dot(ccl::cross(n1, tu1), tv1) < 0.0f) ? -1.0f : 1.0f;
						const float sign2 = (ccl::dot(ccl::cross(n2, tu2), tv2) < 0.0f) ? -1.0f : 1.0f;

						result->uvw_tangents.push_back(tu0);
						result->uvw_tangents.push_back(tu1);
						result->uvw_tangents.push_back(tu2);

						result->uvw_tangent_signs.push_back(sign0);
						result->uvw_tangent_signs.push_back(sign1);
						result->uvw_tangent_signs.push_back(sign2);
					}
				}

				result->uv_channels_present.push_back(tex_coord_channel.channel_id);

				submesh_tex_coords_copied = true;
				break;
			}
			mesh_has_uvw_map = mesh_has_uvw_map || submesh_tex_coords_copied;

			// If no real data was copied, add dummy (0, 0) uvw points
			if (submesh_tex_coords_copied == false) {
				for (size_t i = 0; i < face_vec.size(); i++) {
					ccl::float3 zero = ccl::make_float3(0.0f);
					result->uvw_verts.push_back(zero);
					result->uvw_verts.push_back(zero);
					result->uvw_verts.push_back(zero);
					MAYBE_UI_CALLBACK(i)
				}
			}

			assert(result->faces.size() * 3 == result->uvw_verts.size());
		}

		if (mesh_has_uvw_map == false) {
			result->uvw_verts.clear();
		}
	}

	*logger << "current tick: " << t << LogCtl::WRITE_LINE;
	*logger << "mesh_valid: " << mesh_valid.Start() << "-" << mesh_valid.End() << LogCtl::WRITE_LINE;

	// Get motion stuff
	const TimeValue mblur_ticks_offset = mblur_sample_ticks.size() > 0 ? mblur_sample_ticks[mblur_sample_ticks.size() - 1] : 0;
	const TimeValue t_pre = (mblur_ticks_offset > t) ? 0 : t - mblur_ticks_offset;
	const TimeValue t_post = t + mblur_ticks_offset;
	Interval motion_interval;
	motion_interval.Set(t_pre, t_post);

	// If the mesh validity does not cover the entire motion interval, we need to store motion data
	const bool populate_motion_vectors = mblur_ticks_offset > 0 && (mesh_valid.InInterval(motion_interval) == false);
	if (populate_motion_vectors) {
		*logger << "getting mesh deform motion blur data" << LogCtl::WRITE_LINE;

		// This will be set to false if we find an inconsistency in vertex count between frames
		bool topology_consistent = true;

		std::set<MtlID> mtl_ids_present;

		for (int this_offset : mblur_sample_ticks) {
			TimeValue this_t = t + this_offset;
			if (this_t < 0) {
				this_t = 0;
			}

			std::vector<ccl::float3> this_tick_verts;
			std::vector<ccl::float3> this_tick_normals;

			Interval mesh_valid = FOREVER;
			std::unique_ptr<IMeshFlattener> mesh_flattener = IMeshFlattener::AllocateInstance(*node, view, this_t, mesh_valid);


			for (size_t submesh_index = 0; submesh_index < mesh_flattener->GetNumSubMeshes(); submesh_index++) {
				MtlID mtl_id;
				std::vector<IPoint3> face_vec;
				std::vector<Point3> vertex_vec;
				std::vector<Point3> normal_vec;
				std::vector<IMeshFlattener::TextureCoordChannel> texture_vec;
				mesh_flattener->GetSubMesh(submesh_index, mtl_id, face_vec, vertex_vec, normal_vec, texture_vec);

				for (size_t i = 0; i < vertex_vec.size(); i++) {
					Point3 src_pos = vertex_vec[i];
					this_tick_verts.push_back(ccl::make_float3(src_pos.x, src_pos.y, src_pos.z));
				}
				for (size_t i = 0; i < normal_vec.size(); i++) {
					Point3 src_norm = normal_vec[i];
					this_tick_normals.push_back(ccl::make_float3(src_norm.x, src_norm.y, src_norm.z));
				}

				mtl_ids_present.insert(mtl_id);
			}

			const bool pos_consistent = result->verts.size() == this_tick_verts.size();
			const bool norm_consistent = result->normals.size() == this_tick_normals.size();

			topology_consistent = topology_consistent && pos_consistent && norm_consistent;

			result->motion_verts.push_back(this_tick_verts);
			result->motion_normals.push_back(this_tick_normals);
		}

		result->mtl_ids_present = std::vector<MtlID>(mtl_ids_present.begin(), mtl_ids_present.end());

		if (topology_consistent) {
			result->use_mesh_motion_blur = true;
			assert(result->verts.size() == result->verts_post.size() * 2);
			assert(result->normals.size() == result->normals_post.size() * 2);
		}
		else {
			result->motion_verts.clear();
			result->motion_normals.clear();
		}

		*logger << "motion blur data calculation complete" << LogCtl::WRITE_LINE;
	}

	*logger << "complete" << LogCtl::WRITE_LINE;

	return result;
}

class SmoothGroupVert {
public:
	SmoothGroupVert(const DWORD smooth_group, const DWORD new_vert) : smooth_group{ smooth_group }, new_vert{ new_vert } {}

	DWORD smooth_group;
	DWORD new_vert;
};

class MultiVertex {
public:
	MultiVertex(unsigned int vert_index) : vert_index{ vert_index } {}

	int get_vert_index(DWORD smooth_group);
	void add_smooth_vert(DWORD smooth_group, DWORD vertex);
	void collapse_list();

	typedef std::list<SmoothGroupVert>::iterator list_iter;

	std::list<SmoothGroupVert> smooth_verts;
	unsigned int vert_index;
};

int MultiVertex::get_vert_index(DWORD smooth_group)
{
	for (SmoothGroupVert& this_vert : smooth_verts) {
		if (smooth_group & this_vert.smooth_group) {
			return this_vert.new_vert;
		}
	}

	// Use default vert if no smooth groups match
	return vert_index;
}

void MultiVertex::add_smooth_vert(DWORD smooth_group, DWORD vertex)
{
	if (smooth_group == 0) {
		return;
	}

	for (SmoothGroupVert& this_vert : smooth_verts) {
		if (smooth_group & this_vert.smooth_group) {
			this_vert.smooth_group |= smooth_group;
			return;
		}
	}

	smooth_verts.push_back(SmoothGroupVert{ smooth_group, vertex });
}

void MultiVertex::collapse_list()
{
	// After all vertex information has been added, we must look over the list and merge compatible entries
	// This operation is O(n^2) but n is guaranteed to be sufficiently small that this is not a problem (<= 32)
	for (list_iter iter = smooth_verts.begin(); iter != smooth_verts.end(); ++iter) {
		// For each element in the list, compare it to each subsequent element in the list and see if a merge is possible
		list_iter inner_iter = smooth_verts.begin();
		while (inner_iter != smooth_verts.end()) {
			if (inner_iter == iter) { // Don't compare against self
				++inner_iter;
				continue;
			}

			if (iter->smooth_group & inner_iter->smooth_group) {
				iter->smooth_group |= inner_iter->smooth_group;
				smooth_verts.erase(inner_iter++);
			}
			else {
				++inner_iter;
			}
		}
	}
}

std::shared_ptr<MeshGeometryObj> get_mesh_geometry(Mesh* mesh, const TimeValue /*t*/, const std::vector<int>& /*mblur_sample_ticks*/, const int mtl_id_override, const std::function<void()> ui_callback)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilGeomGet2", false, true);
	*logger << LogCtl::SEPARATOR;

	const std::shared_ptr<MeshGeometryObj> result = std::make_shared<MeshGeometryObj>();

	std::vector<MultiVertex> multi_verts;
	multi_verts.resize(mesh->numVerts, MultiVertex(0));
	for (int i = 0; i < multi_verts.size(); ++i) {
		multi_verts[i] = MultiVertex(i);
		MAYBE_UI_CALLBACK(i)
	}

	// Calculate which verts need to be duplicated for smoothing groups
	for (int i = 0; i < mesh->numFaces; ++i) {
		// Face type always represents a triangle
		Face* face = mesh->faces + i;
		// For each of the three verts, add this smooth group
		for (int j = 0; j < 3; ++j) {
			multi_verts[face->v[j]].add_smooth_vert(face->smGroup, face->v[j]);
		}
		MAYBE_UI_CALLBACK(i)
	}

	// total_verts will be updated as we add new verts
	int total_verts = mesh->numVerts;
	{
		int i = 0;
		for (MultiVertex& multi_vert : multi_verts) {
			// Collapse
			multi_vert.collapse_list();

			// Populate new vert indices
			MultiVertex::list_iter normal_iter = multi_vert.smooth_verts.begin();
			// Skip this vert if the normal list is empty
			if (normal_iter == multi_vert.smooth_verts.end()) {
				continue;
			}
			++normal_iter;
			// Starting with the second element, populate new_vert (first one uses original vert)
			while (normal_iter != multi_vert.smooth_verts.end()) {
				normal_iter->new_vert = total_verts;
				++total_verts;
				++normal_iter;
			}

			MAYBE_UI_CALLBACK(i++)
		}
	}

	result->verts.resize(total_verts, ccl::make_float3(0.0f, 0.0f, 0.0f));
	
	// Copy original verts into result
	for (int i = 0; i < mesh->numVerts; ++i) {
		const Point3& this_vert = mesh->verts[i];
		result->verts[i] = ccl::make_float3(this_vert.x, this_vert.y, this_vert.z);

		MAYBE_UI_CALLBACK(i)
	}

	// Duplicate verts needed for smoothing
	{
		int i = 0;
		for (MultiVertex& multi_vert : multi_verts) {
			if (multi_vert.smooth_verts.size() < 2) {
				continue;
			}

			for (SmoothGroupVert& smooth_vert : multi_vert.smooth_verts) {
				if (smooth_vert.new_vert == multi_vert.vert_index) {
					continue;
				}
				result->verts[smooth_vert.new_vert] = result->verts[multi_vert.vert_index];
			}
			MAYBE_UI_CALLBACK(i++)
		}
	}

	// Copy faces, mesh is already triangles
	result->faces.reserve(mesh->numFaces);
	for (int i = 0; i < mesh->numFaces; ++i) {
		Face* const face = mesh->faces + i;

		// Original verts for this triangle
		const int orig_v0 = face->v[0];
		const int orig_v1 = face->v[1];
		const int orig_v2 = face->v[2];

		// Altered verts for the cycles triangle
		const int v0 = multi_verts[orig_v0].get_vert_index(face->smGroup);
		const int v1 = multi_verts[orig_v1].get_vert_index(face->smGroup);
		const int v2 = multi_verts[orig_v2].get_vert_index(face->smGroup);

		// Get material
		int mtl_index;
		if (mtl_id_override >= 0) {
			mtl_index = mtl_id_override;
		}
		else {
			mtl_index = mesh->getFaceMtlIndex(i);
		}

		result->faces.push_back(TriangleFace(v0, v1, v2, mtl_index, face->smGroup != 0));

		MAYBE_UI_CALLBACK(i)
	}
	
	// If texture verts exist, copy them
	if (mesh->numTVerts > 0) {
		result->uvw_verts.reserve(3 * mesh->numFaces);
		for (int i = 0; i < mesh->numFaces; ++i) {
			UVVert* tverts = mesh->tVerts;
			TVFace* face = mesh->tvFace + i;

			DWORD v0 = face->t[0];
			DWORD v1 = face->t[1];
			DWORD v2 = face->t[2];

			result->uvw_verts.push_back(ccl::make_float3(tverts[v0].x, tverts[v0].y, tverts[v0].z));
			result->uvw_verts.push_back(ccl::make_float3(tverts[v1].x, tverts[v1].y, tverts[v1].z));
			result->uvw_verts.push_back(ccl::make_float3(tverts[v2].x, tverts[v2].y, tverts[v2].z));

			MAYBE_UI_CALLBACK(i)
		}
	}

	return result;
}

CyclesGeomObject get_geom_object(const TimeValue t, INode* const node, const std::vector<int>& mblur_sample_ticks)
{
	CyclesGeomObject result;

	// Set up random value based on INode handle
	// This is used for the "random" output of the object info node
	{
		std::mt19937 rng;
		const unsigned long seed = node->GetHandle();
		rng.seed(seed);
		result.random_id = rng();
	}

	const TimeValue mblur_ticks_offset = mblur_sample_ticks.size() > 0 ? mblur_sample_ticks[mblur_sample_ticks.size() - 1] : 0;
	const TimeValue t_pre = (mblur_ticks_offset > t) ? 0 : t - mblur_ticks_offset;
	const TimeValue t_post = t + mblur_ticks_offset;

	result.tfm = cycles_transform_from_max_matrix(node->GetObjTMAfterWSM(t));
	if (mblur_ticks_offset > 0) {
		const TimeValue t_pre = (mblur_ticks_offset > t) ? 0 : t - mblur_ticks_offset;
		result.tfm_pre = cycles_transform_from_max_matrix(node->GetObjTMAfterWSM(t_pre));
		result.tfm_post = cycles_transform_from_max_matrix(node->GetObjTMAfterWSM(t + mblur_ticks_offset));
		result.use_object_motion_blur = true;
	}

	if (node->IsHidden()) {
		result.hidden = true;
	}

	result.is_shadow_catcher = is_node_shadow_catcher(node, t);
	result.visible_to_camera = node->GetPrimaryVisibility();

	return result;
}

CyclesGeomObject get_geom_object(const TimeValue t, INode* const node, IParticleObjectExt* const particle_ext, const int particle_index, const std::vector<int>& /*mblur_sample_ticks*/)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"TempGetGeomObject", false, false);
	*logger << LogCtl::SEPARATOR;

	CyclesGeomObject result;

	// Set up random value based on INode handle
	// This is used for the "random" output of the object info node
	{
		std::mt19937 rng;
		const unsigned long seed = node->GetHandle() + static_cast<unsigned long>(particle_index);
		rng.seed(seed);
		result.random_id = rng();
	}

	const Matrix3 part_tfm = *(particle_ext->GetParticleTMByIndex(particle_index));

	result.tfm = cycles_transform_from_max_matrix(part_tfm);
	result.tfm_pre = result.tfm;
	result.tfm_post = result.tfm;

	result.is_shadow_catcher = is_node_shadow_catcher(node, t);
	result.visible_to_camera = node->GetPrimaryVisibility();

	return result;
}

static void mesh_thread_func(ccl::Mesh* const mesh, volatile bool* const complete)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilGeomThread", false, true);
	*logger << LogCtl::SEPARATOR;

	mesh->add_face_normals();
	mesh->add_vertex_normals();
	add_ccl_mesh_tangents(mesh);

	*logger << "mesh_thread_func complete" << LogCtl::WRITE_LINE;

	if (complete) {
		*complete = true;
	}
}

ccl::Mesh* get_ccl_mesh(
	const std::shared_ptr<MeshGeometryObj> mesh_geometry,
	const MaxMultiShaderHelper& ms_helper,
	const std::function<void()> ui_callback)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilGeomGetCclMesh", false, true);

	ccl::Mesh* const ccl_mesh = new ccl::Mesh();
	ccl::AttributeSet& attributes = ccl_mesh->attributes;

	*logger << "Copying verts..." << LogCtl::WRITE_LINE;
	*logger << "vert count: " << mesh_geometry->verts.size() << LogCtl::WRITE_LINE;

	// Copy verts
	ccl::array<ccl::float3>& verts = ccl_mesh->get_verts();
	verts.resize(mesh_geometry->verts.size());
	for (int i = 0; i < mesh_geometry->verts.size(); ++i) {
		verts[i] = mesh_geometry->verts[i];
		MAYBE_UI_CALLBACK(i)
	}

	*logger << "ccl vert count: " << ccl_mesh->get_verts().size() << LogCtl::WRITE_LINE;

	*logger << "Copying normals..." << LogCtl::WRITE_LINE;

	// Copy Normals
	if (mesh_geometry->normals.size() != 0) {
		ccl::Attribute* attr_normal = attributes.add(ccl::ATTR_STD_VERTEX_NORMAL);
		ccl::float3* normal_buffer = attr_normal->data_float3();
		for (size_t i = 0; i < mesh_geometry->normals.size(); i++) {
			normal_buffer[i] = mesh_geometry->normals[i];
			MAYBE_UI_CALLBACK(i)
		}
	}

	*logger << "Copying faces..." << LogCtl::WRITE_LINE;
	*logger << "face count: " << mesh_geometry->faces.size() << LogCtl::WRITE_LINE;

	// Copy faces
	{
		int i = 0;
		for (TriangleFace tri_face : mesh_geometry->faces) {
			const int shader_mesh_index = ms_helper.get_mesh_index(tri_face.shader_mtl_index);
			ccl_mesh->add_triangle(tri_face.v0, tri_face.v1, tri_face.v2, shader_mesh_index, tri_face.smooth);
			MAYBE_UI_CALLBACK(i++)
		}
	}

	*logger << "Copying uv verts..." << LogCtl::WRITE_LINE;

	// Copy uv verts if they exist
	if (mesh_geometry->uvw_verts.size() > 0) {
		ccl::ustring name = ccl::ustring("UVMap");
		ccl::Attribute* const attr = ccl_mesh->attributes.add(ccl::AttributeStandard::ATTR_STD_UV, name);
		ccl::float2* uv_data = attr->data_float2();

		// Verify that the attribute is as big as we need
		const bool invalid_attribute_buffer = (mesh_geometry->uvw_verts.size() * sizeof(ccl::float2) != attr->buffer.size());
		if (invalid_attribute_buffer) {
			*logger << LogLevel::ERR << "invalid attribute size" << sizeof(ccl::float2) << LogCtl::WRITE_LINE;
		}

		{
			int i = 0;
			for (const ccl::float3 this_tvert : mesh_geometry->uvw_verts) {
				uv_data[0].x = this_tvert.x;
				uv_data[0].y = this_tvert.y;
				++uv_data;
				MAYBE_UI_CALLBACK(i++)
			}
		}
	}

	*logger << "Copying tangents..." << LogCtl::WRITE_LINE;

	// Copy tangents
	if (mesh_geometry->uvw_verts.size() > 0) {
		ccl::Attribute* const attribute_uv_tangent = ccl_mesh->attributes.add(ccl::AttributeStandard::ATTR_STD_UV_TANGENT);
		ccl::float3* const uv_tangent_ptr = attribute_uv_tangent->data_float3();
		ccl::Attribute* const attribute_uv_tangent_sign = ccl_mesh->attributes.add(ccl::AttributeStandard::ATTR_STD_UV_TANGENT_SIGN);
		float* const uv_tangent_sign_ptr = attribute_uv_tangent_sign->data_float();
		for (auto i = 0; i < mesh_geometry->uvw_verts.size(); i++) {
			uv_tangent_ptr[i] = mesh_geometry->uvw_tangents[i];
			uv_tangent_sign_ptr[i] = mesh_geometry->uvw_tangent_signs[i];
		}
	}
	else {
		*logger << "skipped" << LogCtl::WRITE_LINE;
	}

	*logger << "Copying motion data..." << LogCtl::WRITE_LINE;

	// Copy motion, if it exists
	if (mesh_geometry->use_mesh_motion_blur) {
		const size_t vert_count = mesh_geometry->verts.size();
		ccl_mesh->set_use_motion_blur(true);
		ccl_mesh->set_motion_steps(static_cast<ccl::uint>(mesh_geometry->motion_verts.size()) + 1);
		ccl::Attribute* attr_motion = attributes.add(ccl::ATTR_STD_MOTION_VERTEX_POSITION);
		ccl::Attribute* attr_motion_normal = attributes.add(ccl::ATTR_STD_MOTION_VERTEX_NORMAL);
		ccl::float3* pos_motion_data = attr_motion->data_float3();
		ccl::float3* norm_motion_data = attr_motion_normal->data_float3();
		for (size_t i = 0; i < mesh_geometry->motion_verts.size(); i++) {
			const std::vector<ccl::float3>& this_verts = mesh_geometry->motion_verts[i];
			const std::vector<ccl::float3>& this_normals = mesh_geometry->motion_normals[i];
			const size_t data_offset = i * vert_count;
			for (size_t j = 0; j < vert_count; j++) {
				pos_motion_data[data_offset + j] = this_verts[j];
				norm_motion_data[data_offset + j] = this_normals[j];
			}
		}
	}
	else {
		*logger << "skipped" << LogCtl::WRITE_LINE;
	}

	*logger << "Copying shader pointers to mesh" << LogCtl::WRITE_LINE;

	// Copy shader pointers into mesh object
	for (ccl::Shader* const shader : ms_helper.mesh_shader_vector) {
		ccl::array<ccl::Node*>& shaders = ccl_mesh->get_used_shaders();
		shaders.push_back_slow(shader);
		ccl_mesh->set_used_shaders(shaders);
	}

	*logger << "Beginning to calculate tangents" << LogCtl::WRITE_LINE;

	// Calculate tangents and normals in a separate thread so we can keep updating the UI from here

	if (ui_callback) {
		try {
			volatile bool thread_complete = false;
			std::thread mesh_compute_thread(mesh_thread_func, ccl_mesh, &thread_complete);
			*logger << "Mesh thread created" << LogCtl::WRITE_LINE;

			while (thread_complete == false) {
				if (ui_callback != nullptr) {
					ui_callback();
				}
				Sleep(0);
			}
			mesh_compute_thread.join();
		}
		catch (...) {
			*logger << "An exception happened" << LogCtl::WRITE_LINE;
		}
	}
	else {
		mesh_thread_func(ccl_mesh, nullptr);
	}

	*logger << "Tangents calculated" << LogCtl::WRITE_LINE;

	return ccl_mesh;
}

ccl::Object* get_ccl_object(
	const CyclesGeomObject geom_object,
	ccl::Mesh* const ccl_mesh )
{
	ccl::Object* const result = new ccl::Object();
	result->set_random_id(geom_object.random_id);
	result->set_geometry(ccl_mesh);
	result->set_tfm(geom_object.tfm);
	result->set_is_shadow_catcher(geom_object.is_shadow_catcher);

	result->set_visibility(ccl::PATH_RAY_ALL_VISIBILITY);
	if (geom_object.visible_to_camera == false) {
		result->set_visibility(result->get_visibility() & (~ccl::PATH_RAY_CAMERA));
	}

	if (geom_object.use_object_motion_blur) {
		// TODO: Make this use an arbitrary number of samples
		ccl::array<ccl::Transform>& motion = result->get_motion();
		motion.resize(3);
		motion[0] = geom_object.tfm_pre;
		motion[1] = geom_object.tfm;
		motion[2] = geom_object.tfm_post;
	}

	return result;
}
