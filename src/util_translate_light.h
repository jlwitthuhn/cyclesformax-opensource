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
 * @brief Defines functions used to translate a light.
 */

#include <memory>

#include <maxtypes.h>

#include "trans_output.h"

class INode;
class MaxShaderManager;
class Object;

namespace ccl {
	class Scene;
}

/**
 * @brief Returns a CyclesLightParams representing the given node.
 */
CyclesLightParams get_light_params(INode* node, Object* object, TimeValue t);

/**
 * @brief Adds a light to a ccl::Scene
 */
void add_light_to_scene(ccl::Scene* scene, const std::unique_ptr<MaxShaderManager>& shader_manager, CyclesLightParams light_params, float point_light_size);
