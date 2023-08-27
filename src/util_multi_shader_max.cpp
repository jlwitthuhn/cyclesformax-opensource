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
 
#include "util_multi_shader_max.h"

#include <cassert>

MaxMultiShaderHelper::MaxMultiShaderHelper(ccl::Shader* const default_shader_in, const int size_in)
{
	assert(default_shader_in != nullptr);

	size = size_in;

	default_shader = default_shader_in;

	default_mesh_index = static_cast<int>(mesh_shader_vector.size());
	mesh_shader_vector.push_back(default_shader);
}

void MaxMultiShaderHelper::add_shader(const int submat_index, ccl::Shader* const shader)
{
	if (shader == nullptr) {
		return;
	}

	shaders_by_submat[submat_index] = shader;
	mesh_index_by_submat[submat_index] = static_cast<int>(mesh_shader_vector.size());

	mesh_shader_vector.push_back(shader);
}

int MaxMultiShaderHelper::get_mesh_index(const int submat_index) const
{
	const int index_in_range = submat_index % size;

	if (mesh_index_by_submat.count(index_in_range) == 0) {
		return default_mesh_index;
	}

	return mesh_index_by_submat.at(index_in_range);
}
