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
 
#include "rend_offline_translation_man.h"

#include <sstream>

#include <render/light.h>
#include <render/mesh.h>
#include <render/object.h>

#include <inode.h>
#include <IParticleObjectExt.h>
#include <Rendering/Renderer.h>
#include <RenderingAPI/Renderer/ICameraContainer.h>
#include <RenderingAPI/Renderer/ISceneContainer.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/IRenderingLogger.h>
#include <RenderingAPI/Renderer/IRenderingProcess.h>
#include <units.h>

#include "const_classid.h"
#include "cycles_image.h"
#include "extern_tyflow.h"
#include "rend_logger_ext.h"
#include "rend_params.h"
#include "rend_shader_manager.h"
#include "util_cycles_params.h"
#include "util_matrix_max.h"
#include "util_sphere_mesh.h"
#include "util_translate_environment.h"
#include "util_translate_geometry.h"
#include "util_translate_light.h"

namespace ccl {
	class Mesh;
}

static ccl::float3 get_float3_from_colorref(const COLORREF input)
{
	ccl::float3 output;
	output.x = GetRValue(input) / 255.0f;
	output.y = GetGValue(input) / 255.0f;
	output.z = GetBValue(input) / 255.0f;

	return output;
}

static std::string bad_from_wstring(const std::wstring input)
{
	std::stringstream result_stream;

	for (const wchar_t this_char : input) {
		if (this_char < 256) {
			result_stream << static_cast<char>(this_char);
		}
		else {
			result_stream << '?';
		}
	}

	return result_stream.str();
}

static std::string get_asset_name(INode* const node)
{
	INode* current_node{ node };
	while (current_node->GetParentNode() != nullptr && current_node->GetParentNode()->IsRootNode() == 0) {
		current_node = current_node->GetParentNode();
	}
	return bad_from_wstring(current_node->GetName());
}

OfflineTranslationManager::OfflineTranslationManager(
	MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
	BakedTexmapCache& texmap_cache,
	CyclesRenderParams& rend_params
	) :
	session_context(session_context),
	rend_params(rend_params),
	frame_t(rend_params.frame_t),
	texmap_cache(texmap_cache),
	logger(global_log_manager.new_logger(L"OfflineTranslationManager"))
{
	*logger << LogCtl::SEPARATOR;

	*logger << "Constructor complete" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::init(ccl::Device* const device)
{
	*logger << "init called..." << LogCtl::WRITE_LINE;

	ccl::SceneParams scene_params = get_scene_params();
	scene = new ccl::Scene(scene_params, device);

	if (scene == nullptr) {
		*logger << "Failed to create scene, aborting..." << LogCtl::WRITE_LINE;
		return;
	}

	*logger << "Creating shader manager..." << LogCtl::WRITE_LINE;

	shader_manager = std::make_unique<MaxShaderManager>(texmap_cache, scene, frame_t);
	shader_manager->set_mis_map_size(rend_params.mis_map_size);
}

void OfflineTranslationManager::copy_scene(const std::vector<int>& mblur_sample_ticks)
{
	*logger << "copy_scene called..." << LogCtl::WRITE_LINE;


	if (scene == nullptr) {
		*logger << "scene pointer is null, aborting copy..." << LogCtl::WRITE_LINE;
		return;
	}

	if (rend_params.in_mtl_edit) {
		add_mtl_preview_lights_new();
	}

	CyclesEnvironmentParams env_params = get_environment_params(session_context, frame_t);
	apply_environment_params(env_params, rend_params, shader_manager);

	Interval geom_nodes_valid = FOREVER;
	const std::vector<INode*> geom_nodes = session_context.GetScene().GetGeometricNodes(frame_t, geom_nodes_valid);

	*logger << "Found " << geom_nodes.size() << " geom nodes" << LogCtl::WRITE_LINE;

	for (INode* const geom_node : geom_nodes) {
		if (should_stop()) {
			*logger << "Ending geom node translation early"<< LogCtl::WRITE_LINE;
			break;
		}
		add_node_to_scene(geom_node, mblur_sample_ticks);
	}

	Interval light_nodes_valid = FOREVER;
	const std::vector<INode*> light_nodes = session_context.GetScene().GetLightNodes(frame_t, light_nodes_valid);

	*logger << "Found " << light_nodes.size() << " light nodes" << LogCtl::WRITE_LINE;

	for (INode* const light_node : light_nodes) {
		if (should_stop()) {
			*logger << "Ending light node translation early" << LogCtl::WRITE_LINE;
			break;
		}
		add_node_to_scene(light_node, mblur_sample_ticks);
	}

	*logger << "Unique geometry count: " << scene->geometry.size() << LogCtl::WRITE_LINE;
	*logger << "Logging shader stats..." << LogCtl::WRITE_LINE;
	shader_manager->log_shader_stats();
}

void OfflineTranslationManager::end_render()
{
	*logger << "end_render called..." << LogCtl::WRITE_LINE;
	stop_requested.store(true);
}

void OfflineTranslationManager::add_node_to_scene(INode* const node, const std::vector<int>& mblur_sample_ticks)
{
	*logger << "Processing node: " << node->GetName() << LogCtl::WRITE_LINE;

	std::wstring prefix(L"Translating object: ");
	session_context.GetRenderingProcess().SetRenderingProgressTitle((prefix + node->GetName()).c_str());
	refresh_ui();

	session_context.CallRenderBegin(*node, frame_t);

	if (!rend_params.std_render_hidden && (node->IsHidden(0, 1) || node->IsObjectHidden())) {
		*logger << "Object is hidden, skipping" << LogCtl::WRITE_LINE;
		return;
	}

	ObjectState os = node->EvalWorldState(frame_t);
	*logger << os.obj->ClassName() << ": " << os.obj->ClassID() << LogCtl::WRITE_LINE;

	if (os.obj != nullptr && os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID) {
		IParticleObjectExt* const particle_interface = static_cast<IParticleObjectExt*>(os.obj->GetInterface(PARTICLEOBJECTEXT_INTERFACE));
		tyParticleInterface* const ty_interface = static_cast<tyParticleInterface*>(os.obj->GetInterface(TYPARTICLE_INTERFACE));
		if (particle_interface && os.obj->IsParticleSystem()) {
			if (os.obj->ClassID() != PFLOW_PARTICLE_GROUP) {
				*logger << "Processing as IParticleExt..." << LogCtl::WRITE_LINE;
				process_particle_system(node, particle_interface, mblur_sample_ticks);
			}
			else {
				*logger << "Class is PFLOW_PARTICLE_GROUP, skipping" << LogCtl::WRITE_LINE;
				return;
			}
		}
		else if (os.obj->ClassID() == TYFLOW_BASE && ty_interface) {
			*logger << "Processing as TyFlow..." << LogCtl::WRITE_LINE;
			process_particle_system_ty(node, ty_interface, mblur_sample_ticks);
		}
		else {
			*logger << "Processing as generic GeomObject..." << LogCtl::WRITE_LINE;
			process_geom_object(node, static_cast<GeomObject*>(os.obj), mblur_sample_ticks);
		}
	}
	else if (os.obj != nullptr && os.obj->SuperClassID() == LIGHT_CLASS_ID) {
		*logger << "Processing as Light..." << LogCtl::WRITE_LINE;
		process_light_object(node, os.obj);
	}
	else if (os.obj != nullptr) {
		*logger << "Unknown SuperClassID: " << os.obj->SuperClassID() << LogCtl::WRITE_LINE;
	}

	*logger << "Node complete" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::add_mtl_preview_lights_new()
{
	*logger << "Adding lights" << LogCtl::WRITE_LINE;

	std::vector<DefaultLight> default_lights = session_context.GetDefaultLights();
	for (const DefaultLight& this_light : default_lights) {
		const Point3 light_pos = this_light.tm.GetTrans();
		const Color light_color = this_light.ls.color;

		ccl::Light* const this_ccl_light = new ccl::Light();

		const float LIGHT_DISTANCE_FACTOR = 135.0f;

		this_ccl_light->set_light_type(ccl::LightType::LIGHT_DISTANT);
		this_ccl_light->set_co(ccl::make_float3(0.0f, 0.0f, 0.0f));
		this_ccl_light->set_dir(ccl::make_float3(-1.0f * light_pos.x, -1.0f * light_pos.y, -1.0f * light_pos.z));
		this_ccl_light->set_size(0.0f);

		this_ccl_light->set_tfm(ccl::transform_identity());
		this_ccl_light->set_cast_shadow(true);
		this_ccl_light->set_use_mis(true);
		this_ccl_light->set_use_diffuse(true);
		this_ccl_light->set_use_glossy(true);
		this_ccl_light->set_use_transmission(true);
		this_ccl_light->set_use_scatter(true);
		this_ccl_light->set_is_enabled(true);

		const ccl::float3 ccl_light_color = ccl::make_float3(light_color.r, light_color.g, light_color.b);

		const int shader_index = shader_manager->get_light_shader(ccl_light_color, this_light.ls.intens * 2.8f);
		this_ccl_light->set_shader(scene->shaders[shader_index]);

		scene->lights.push_back(this_ccl_light);
		this_ccl_light->tag_update(scene);
		*logger << "loop iteration complete" << LogCtl::WRITE_LINE;
	}
	*logger << "Done adding lights" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::add_mtl_preview_lights()
{
	std::vector<DefaultLight> default_lights = session_context.GetDefaultLights();
	bool first_iteration = true;
	for (const DefaultLight& this_light : default_lights) {
		const Point3 light_pos = this_light.tm.GetTrans();
		float light_scale = 14000.0f;

		*logger << "adding preview light with scale: " << light_scale << LogCtl::WRITE_LINE;

		EmissionShaderDescriptor em_desc;
		em_desc.color = ccl::make_float3(1.0f, 1.0f, 1.0f);

		if (first_iteration) {
			light_scale = 15000.0f;
			em_desc.strength = 130.0f;
		}
		else {
			light_scale = 10000.0f;
			em_desc.strength = 75.f;
		}

		const int shader_index = shader_manager->get_emission_shader(em_desc);

		ccl::Mesh* const mesh = get_sphere_mesh(10, 20, scene->shaders[shader_index]);

		scene->geometry.push_back(mesh);

		ccl::Object* const object = new ccl::Object();
		object->set_geometry(mesh);

		// XYZ axis for this transform in camera space:
		// Positive x is right
		// Positive y is away from camera
		// Positive z is up
		object->set_tfm(ccl::transform_translate(light_pos.x, light_pos.y, light_pos.z) *
			ccl::transform_scale(light_scale, light_scale, light_scale));

		scene->objects.push_back(object);

		first_iteration = false;
	}
}

void OfflineTranslationManager::process_geom_object(INode* const node, GeomObject* const obj, const std::vector<int>& mblur_sample_ticks)
{
	*logger << "Processing geom object..." << LogCtl::WRITE_LINE;

	Interval view_valid{ FOREVER };
	View& cam_view{ const_cast<View&>(session_context.GetCamera().GetView(frame_t, view_valid)) };

	refresh_ui();

	// If user has requested an abort while this mesh was translating, skip adding it to the scene
	if (should_stop()) {
		return;
	}

	const ccl::float3 wire_color{ get_float3_from_colorref(node->GetWireColor()) };
	ccl::Shader* const default_shader{ scene->shaders[shader_manager->get_simple_color_shader(wire_color)] };

	Mtl* const node_mtl{ node->GetMtl() };

	*logger << "Getting mesh..." << LogCtl::WRITE_LINE;

	const MeshDescriptor this_desc{ obj, node_mtl };
	ccl::Mesh* this_mesh{ nullptr };

	const bool cached_mesh_exists = (mesh_cache.count(this_desc) == 1);
	if (cached_mesh_exists) {
		*logger << "Using cached mesh" << LogCtl::WRITE_LINE;
		this_mesh = mesh_cache[this_desc];
	}
	else {
		*logger << "Making new mesh..." << LogCtl::WRITE_LINE;
		const std::shared_ptr<MeshGeometryObj> mesh_geom = get_mesh_geometry(node, cam_view, frame_t, mblur_sample_ticks, std::bind(&OfflineTranslationManager::refresh_ui, this));
		MaxMultiShaderHelper ms_helper(default_shader);
		if (node_mtl != nullptr) {
			ms_helper = shader_manager->get_mtl_multishader(node_mtl);
		}
		ccl::Mesh* const new_mesh = get_ccl_mesh(mesh_geom, ms_helper, std::bind(&OfflineTranslationManager::refresh_ui, this));
		scene->geometry.push_back(new_mesh);
		mesh_cache[this_desc] = new_mesh;
		this_mesh = new_mesh;
		*logger << "verts: " << mesh_geom->verts.size() << LogCtl::WRITE_LINE;
	}


	*logger << "Getting object properties..." << LogCtl::WRITE_LINE;
	const CyclesGeomObject geom_object{ get_geom_object(frame_t, node, mblur_sample_ticks) };

	ccl::Object* const new_object{ get_ccl_object(geom_object, this_mesh) };
	new_object->name = bad_from_wstring(node->GetName());
	new_object->set_asset_name(ccl::ustring(get_asset_name(node)));
	scene->objects.push_back(new_object);

	*logger << "Added as " << new_object->name.c_str() << ", " << new_object->get_asset_name().c_str() << LogCtl::WRITE_LINE;

	refresh_ui();

	*logger << "Done with geom object" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::process_light_object(INode* const node, Object* const obj)
{
	*logger << "Processing light object..." << LogCtl::WRITE_LINE;

	const CyclesLightParams light_params = get_light_params(node, obj, frame_t);
	if (light_params.active) {
		if (light_params.errored) {
			*logger << "Light is errored, writing warning to render log" << LogCtl::WRITE_LINE;
			*logger << LogLevel::WARN << light_params.error_string.c_str() << LogCtl::WRITE_LINE;
			session_context.GetLogger().LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Error, light_params.error_string.c_str());
		}
		else {
			add_light_to_scene(scene, shader_manager, light_params, rend_params.point_light_size);
			*logger << "Added" << LogCtl::WRITE_LINE;
		}
	}
	else {
		*logger << "Light is inactive, skipping" << LogCtl::WRITE_LINE;
	}

	*logger << "Done with light object" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::process_particle_system(
	INode* const node,
	IParticleObjectExt* const particle_ext,
	const std::vector<int>& mblur_sample_ticks
	)
{
	*logger << "Processing particle system..." << LogCtl::WRITE_LINE;

	Mtl* const node_mtl = node->GetMtl();

	const ccl::float3 wire_color{ get_float3_from_colorref(node->GetWireColor()) };
	ccl::Shader* const default_shader{ scene->shaders[shader_manager->get_simple_color_shader(wire_color)] };

	particle_ext->UpdateParticles(node, frame_t);

	const int particle_count{ particle_ext->NumParticles() };

	for (int i{ 0 }; i < particle_count; i++) {
		if (particle_ext->GetParticleAgeByIndex(i) < 0) {
			// Particle has not spawned yet
			continue;
		}

		ccl::Mesh* ccl_mesh{ nullptr };

		Mesh* const this_mesh{ particle_ext->GetParticleShapeByIndex(i) };

		RawMeshDescriptor mesh_desc{ this_mesh, node_mtl, -1 };
		if (raw_mesh_cache.count(mesh_desc)) {
			*logger << "Using cached mesh" << LogCtl::WRITE_LINE;
			ccl_mesh = raw_mesh_cache[mesh_desc];
		}
		else {
			*logger << "Making new mesh..." << LogCtl::WRITE_LINE;
			const std::shared_ptr<MeshGeometryObj> mesh_geom = get_mesh_geometry(this_mesh, frame_t, mblur_sample_ticks, -1, std::bind(&OfflineTranslationManager::refresh_ui, this));
			MaxMultiShaderHelper ms_helper(default_shader);
			if (node_mtl != nullptr) {
				ms_helper = shader_manager->get_mtl_multishader(node_mtl);
			}
			ccl::Mesh* const new_mesh = get_ccl_mesh(mesh_geom, ms_helper, std::bind(&OfflineTranslationManager::refresh_ui, this));
			scene->geometry.push_back(new_mesh);
			raw_mesh_cache[mesh_desc] = new_mesh;
			ccl_mesh = new_mesh;
			*logger << "verts: " << mesh_geom->verts.size() << LogCtl::WRITE_LINE;
		}

		*logger << "Getting object properties..." << LogCtl::WRITE_LINE;
		const CyclesGeomObject geom_object{ get_geom_object(frame_t, node, particle_ext, i, mblur_sample_ticks) };

		ccl::Object* const new_object{ get_ccl_object(geom_object, ccl_mesh) };
		new_object->name = bad_from_wstring(node->GetName());
		new_object->set_asset_name(ccl::ustring(get_asset_name(node)));
		scene->objects.push_back(new_object);

		*logger << "Added as " << new_object->name.c_str() << ", " << new_object->get_asset_name().c_str() << LogCtl::WRITE_LINE;
	}

	refresh_ui();
	
	*logger << "Done with particle system" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::process_particle_system_ty(
	INode* const node,
	tyParticleInterface* const ty_ext,
	const std::vector<int>& mblur_sample_ticks)
{
	*logger << "Processing as tyFlow system..." << LogCtl::WRITE_LINE;

	Mtl* const node_mtl = node->GetMtl();

	const ccl::float3 wire_color{ get_float3_from_colorref(node->GetWireColor()) };
	ccl::Shader* const default_shader{ scene->shaders[shader_manager->get_simple_color_shader(wire_color)] };

	ty_ext->UpdateTyParticles(node, frame_t, tyParticleObjectExt2::plugin_arnold);

	std::vector<tyInstanceInfo> instances = ty_ext->CollectInstances(node, frame_t, frame_t, tyParticleObjectExt2::plugin_arnold);
	for (const tyInstanceInfo& instance_info : instances) {
		ccl::Mesh* ccl_mesh = nullptr;

		*logger << "found " << instance_info.instances.size() << " instances for this mesh" << LogCtl::WRITE_LINE;
		for (const tyInstance& this_instance : instance_info.instances) {
			Mtl* const this_mtl = this_instance.materialOverride ? this_instance.materialOverride : node_mtl;
			RawMeshDescriptor mesh_desc{ instance_info.mesh, this_mtl, this_instance.matIDOverride };
			if (raw_mesh_cache.count(mesh_desc)) {
				*logger << "Using cached mesh" << LogCtl::WRITE_LINE;
				ccl_mesh = raw_mesh_cache[mesh_desc];
			}
			else {
				*logger << "Making new mesh..." << LogCtl::WRITE_LINE;
				const std::shared_ptr<MeshGeometryObj> mesh_geom = get_mesh_geometry(instance_info.mesh, frame_t, mblur_sample_ticks, this_instance.matIDOverride, std::bind(&OfflineTranslationManager::refresh_ui, this));
				MaxMultiShaderHelper ms_helper(default_shader);
				if (this_mtl != nullptr) {
					ms_helper = shader_manager->get_mtl_multishader(this_mtl);
				}
				ccl::Mesh* const new_mesh = get_ccl_mesh(mesh_geom, ms_helper, std::bind(&OfflineTranslationManager::refresh_ui, this));
				scene->geometry.push_back(new_mesh);
				raw_mesh_cache[mesh_desc] = new_mesh;
				ccl_mesh = new_mesh;
				*logger << "verts: " << mesh_geom->verts.size() << LogCtl::WRITE_LINE;
			}

			*logger << "Getting object properties..." << LogCtl::WRITE_LINE;

			// Assemble a GeomObject ourselves for this special case
			CyclesGeomObject geom_object;

			// Set up random value based on INode handle
			// This is used for the "random" output of the object info node
			{
				std::mt19937 rng;
				const unsigned long seed = node->GetHandle() + static_cast<unsigned long>(this_instance.ID);
				rng.seed(seed);
				geom_object.random_id = rng();
			}

			if (mblur_sample_ticks.size() > 0) {
				geom_object.use_object_motion_blur = true;
				geom_object.tfm = cycles_transform_from_max_matrix(this_instance.tm0);

				const float pre_scale = static_cast<float>(mblur_sample_ticks[0]) / static_cast<float>(GetTicksPerFrame());
				Matrix3 pre_transform = this_instance.tm0;
				pre_transform.SetTrans(pre_transform.GetTrans() + this_instance.vel * pre_scale);

				const float post_scale = static_cast<float>(mblur_sample_ticks[mblur_sample_ticks.size() - 1]) / static_cast<float>(GetTicksPerFrame());
				Matrix3 post_transform = this_instance.tm0;
				post_transform.SetTrans(post_transform.GetTrans() + this_instance.vel * post_scale);

				geom_object.tfm_pre = cycles_transform_from_max_matrix(pre_transform);
				geom_object.tfm_post = cycles_transform_from_max_matrix(post_transform);
			}
			else {
				geom_object.tfm = cycles_transform_from_max_matrix(this_instance.tm0);
			}

			geom_object.is_shadow_catcher = false;
			geom_object.visible_to_camera = node->GetPrimaryVisibility();

			ccl::Object* const new_object{ get_ccl_object(geom_object, ccl_mesh) };
			new_object->name = bad_from_wstring(node->GetName());
			new_object->set_asset_name(ccl::ustring(get_asset_name(node)));
			scene->objects.push_back(new_object);

			*logger << "Added as " << new_object->name.c_str() << ", " << new_object->get_asset_name().c_str() << LogCtl::WRITE_LINE;
		}
	}

	refresh_ui();

	*logger << "Done with tyFlow system" << LogCtl::WRITE_LINE;
}

void OfflineTranslationManager::refresh_ui()
{
	session_context.GetRenderingProcess().SetInfiniteProgress(1, MaxSDK::RenderingAPI::IRenderingProcess::ProgressType::Translation);
}

bool OfflineTranslationManager::should_stop()
{
	return (session_context.GetRenderingProcess().HasAbortBeenRequested() || stop_requested);
}

bool OfflineTranslationManager::MeshDescriptor::operator<(const MeshDescriptor& other) const
{
	if (geom_object < other.geom_object) {
		return true;
	}
	else if (other.geom_object < geom_object) {
		return false;
	}

	if (mtl < other.mtl) {
		return true;
	}
	else if (other.mtl < mtl) {
		return false;
	}

	return false;
}

bool OfflineTranslationManager::RawMeshDescriptor::operator<(const RawMeshDescriptor& other) const
{
	if (mesh < other.mesh) {
		return true;
	}
	else if (other.mesh < mesh) {
		return false;
	}

	if (mtl < other.mtl) {
		return true;
	}
	else if (other.mtl < mtl) {
		return false;
	}

	if (shader_index_override < other.shader_index_override) {
		return true;
	}
	else if (other.shader_index_override < shader_index_override) {
		return false;
	}

	return false;
}
