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
 
#include "cycles_mikkt_mesh.h"

#include <render/mesh.h>

#include "extern_mikktspace.h"
#include "rend_logger.h"

struct CyclesMeshTangentContext {
	ccl::Mesh* mesh;
	ccl::float3* vertex_normals;
	ccl::float3* tex_coords;
	ccl::float3* uv_tangents;
	float* uv_tangent_signs;
};

static int get_num_faces(const SMikkTSpaceContext* const pContext)
{
	CyclesMeshTangentContext* const context = static_cast<CyclesMeshTangentContext*>(pContext->m_pUserData);
	ccl::Mesh* const mesh = context->mesh;

	const int num_faces = static_cast<int>(mesh->num_triangles());

	return num_faces;
}

static int get_num_vertices_of_face(const SMikkTSpaceContext* const /*pContext*/, const int /*iFace*/)
{
	// All faces are triangles
	return 3;
}

static void get_position(const SMikkTSpaceContext* const pContext, float fvPosOut[], const int iFace, const int iVert)
{
	CyclesMeshTangentContext* const context = static_cast<CyclesMeshTangentContext*>(pContext->m_pUserData);
	ccl::Mesh* const mesh = context->mesh;

	const ccl::Mesh::Triangle triangle = mesh->get_triangle(iFace);
	const ccl::float3 position = mesh->get_verts()[triangle.v[iVert]];
	
	fvPosOut[0] = position.x;
	fvPosOut[1] = position.y;
	fvPosOut[2] = position.z;
}

static void get_normal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
	CyclesMeshTangentContext* const context = static_cast<CyclesMeshTangentContext*>(pContext->m_pUserData);
	ccl::Mesh* const mesh = context->mesh;
	ccl::float3* const vertex_normals = context->vertex_normals;

	const ccl::Mesh::Triangle triangle = mesh->get_triangle(iFace);
	const ccl::float3 normal = vertex_normals[triangle.v[iVert]];
	
	fvNormOut[0] = normal.x;
	fvNormOut[1] = normal.y;
	fvNormOut[2] = normal.z;
}

static void get_tex_coord(const SMikkTSpaceContext* const pContext, float fvTexcOut[], const int iFace, const int iVert)
{
	CyclesMeshTangentContext* const context = static_cast<CyclesMeshTangentContext*>(pContext->m_pUserData);
	ccl::float3* const tex_coords = context->tex_coords;

	const ccl::float3 tex_coord = tex_coords[3 * iFace + iVert];

	fvTexcOut[0] = tex_coord.x;
	fvTexcOut[1] = tex_coord.y;
}

static void set_tspace_basic(const SMikkTSpaceContext* const pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{
	CyclesMeshTangentContext* const context = static_cast<CyclesMeshTangentContext*>(pContext->m_pUserData);
	ccl::float3* const uv_tangents = context->uv_tangents;
	float* const uv_tangent_signs = context->uv_tangent_signs;
	
	uv_tangents[3 * iFace + iVert].x = fvTangent[0];
	uv_tangents[3 * iFace + iVert].y = fvTangent[1];
	uv_tangents[3 * iFace + iVert].z = fvTangent[2];

	uv_tangent_signs[3 * iFace + iVert] = fSign;
}

void add_ccl_mesh_tangents(ccl::Mesh* const mesh)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"MikkTangents", false, true);

	if (mesh == nullptr) {
		*logger << "mesh is nullptr, returning early" << LogCtl::WRITE_LINE;
		return;
	}

	*logger << "Checking for tangent attribute" << LogCtl::WRITE_LINE;

	if (mesh->attributes.find(ccl::AttributeStandard::ATTR_STD_UV_TANGENT) != nullptr) {
		// Tangent attribute already exists, return without doing anything
		*logger << "tangents already exist, returning early" << LogCtl::WRITE_LINE;
		return;
	}

	*logger << "All required information exists, setting up context..." << LogCtl::WRITE_LINE;

	if (mesh->attributes.find(ccl::AttributeStandard::ATTR_STD_FACE_NORMAL) == nullptr ||
		mesh->attributes.find(ccl::AttributeStandard::ATTR_STD_VERTEX_NORMAL) == nullptr ||
		mesh->attributes.find(ccl::AttributeStandard::ATTR_STD_UV) == nullptr ) {
		// We have insufficient information to calculate tangents without these attributes
		*logger << "unable to calculate tangents, returning early" << LogCtl::WRITE_LINE;
		return;
	}
	
	*logger << "All required information exists, setting up context..." << LogCtl::WRITE_LINE;

	SMikkTSpaceInterface mikkt_space_interface;
	mikkt_space_interface.m_getNumFaces = get_num_faces;
	mikkt_space_interface.m_getNumVerticesOfFace = get_num_vertices_of_face;
	mikkt_space_interface.m_getPosition = get_position;
	mikkt_space_interface.m_getNormal = get_normal;
	mikkt_space_interface.m_getTexCoord = get_tex_coord;
	mikkt_space_interface.m_setTSpaceBasic = set_tspace_basic;

	mikkt_space_interface.m_setTSpace = nullptr;

	CyclesMeshTangentContext mesh_context;
	mesh_context.mesh = mesh;
	mesh_context.vertex_normals = mesh->attributes.find(ccl::AttributeStandard::ATTR_STD_VERTEX_NORMAL)->data_float3();
	mesh_context.tex_coords = mesh->attributes.find(ccl::AttributeStandard::ATTR_STD_UV)->data_float3();
	
	ccl::Attribute* const attribute_uv_tangent = mesh->attributes.add(ccl::AttributeStandard::ATTR_STD_UV_TANGENT);
	mesh_context.uv_tangents = attribute_uv_tangent->data_float3();

	ccl::Attribute* const attribute_uv_tangent_sign = mesh->attributes.add(ccl::AttributeStandard::ATTR_STD_UV_TANGENT_SIGN);
	mesh_context.uv_tangent_signs = attribute_uv_tangent_sign->data_float();

	SMikkTSpaceContext mikkt_space_context;
	mikkt_space_context.m_pInterface = &mikkt_space_interface;
	mikkt_space_context.m_pUserData = static_cast<void*>(&mesh_context);
	
	*logger << "Context setup complete, evaluating..." << LogCtl::WRITE_LINE;

	genTangSpaceDefault(&mikkt_space_context);

	*logger << "Tangent space calculation complete" << LogCtl::WRITE_LINE;
}
