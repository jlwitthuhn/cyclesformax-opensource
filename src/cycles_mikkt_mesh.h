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
 * @brief Defines a function to populate ccl::Mesh with tangent data.
 */

namespace ccl {
	class Mesh;
};

/**
 * @brief For a given ccl::Mesh, populate the ATTR_STD_UV_TANGENT attribute.
 *
 * For a given ccl::Mesh, add and populate the ATTR_STD_UV_TANGENT attribute.
 * Requires that vertex/face normals already exist on the given mesh.
 */
void add_ccl_mesh_tangents(ccl::Mesh* mesh);
