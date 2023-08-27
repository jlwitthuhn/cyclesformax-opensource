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
 * @brief Defines the class MaxMultiShaderHelper.
 */

#include <map>
#include <vector>

namespace ccl {
	class Shader;
}

/**
 * @brief Class to assist with converting a Max multi/sub-object material.
 */
class MaxMultiShaderHelper {
public:
	MaxMultiShaderHelper(ccl::Shader* default_shader_in, int size_in = 1);

	void add_shader(int submat_index, ccl::Shader* shader);
	int get_mesh_index(int submat_index) const;

	std::vector<ccl::Shader*> mesh_shader_vector;

private:
	int size;

	ccl::Shader* default_shader = nullptr;
	int default_mesh_index = 0;

	std::map<int, ccl::Shader*> shaders_by_submat;
	std::map<int, int> mesh_index_by_submat;
};
