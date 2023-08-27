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
 
#pragma once

/**
  * @file
  * @brief Defines functions used to translate a mesh.
  */

#include <functional>
#include <memory>
#include <vector>

#include <maxtypes.h>
#include <object.h>

#include "trans_output.h"

namespace ccl {
	class Mesh;
	class Object;
	class Shader;
}

class INode;
class IParticleObjectExt;
class MaxMultiShaderHelper;
class MaxShaderManager;
class Mesh;
class Mtl;

/**
  * @brief Returns a MeshGeometryObj equivalent to the geometry of the given node.
  */
std::shared_ptr<MeshGeometryObj> get_mesh_geometry(INode* node, View& view, TimeValue t, const std::vector<int>& mblur_sample_ticks, std::function<void()> ui_callback = nullptr);

/**
  * @brief Returns a MeshGeometryObj equivalent to the geometry of the given max Mesh.
  */
std::shared_ptr<MeshGeometryObj> get_mesh_geometry(Mesh* mesh, TimeValue t, const std::vector<int>& mblur_sample_ticks, int mtl_id_override = -1, std::function<void()> ui_callback = nullptr);

/**
 * @brief Returns a CyclesGeomObject equivalent to the given node. This will not have any attached geometry.
 */
CyclesGeomObject get_geom_object(TimeValue t, INode* node, const std::vector<int>& mblur_sample_ticks);

/**
 * @brief Returns a CyclesGeomObject equivalent to the particle index. This will not have any attached geometry.
 */
CyclesGeomObject get_geom_object(TimeValue t, INode* node, IParticleObjectExt* particle_ext, int particle_index, const std::vector<int>& mblur_sample_ticks);


/**
 * @brief Returns a ccl::Mesh* equivalent to a given MeshGeometryObj
 */
ccl::Mesh* get_ccl_mesh(
	std::shared_ptr<MeshGeometryObj> mesh_geometry,
	const MaxMultiShaderHelper& ms_helper,
	std::function<void()> ui_callback = nullptr
);

/**
 * @brief Returns a ccl::Object* equivalent to a given input
 */
ccl::Object* get_ccl_object(
	CyclesGeomObject geom_object,
	ccl::Mesh* ccl_mesh
);
