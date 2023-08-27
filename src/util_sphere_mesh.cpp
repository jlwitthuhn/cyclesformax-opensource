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
 
#include "util_sphere_mesh.h"

#include <cmath>

#include <render/mesh.h>

// Finds the z coordinate of the top of a given layer
static float find_z_coord(const size_t layers, const size_t current_layer)
{
	float fraction = static_cast<float>(current_layer) / static_cast<float>(layers);
	return cosf(M_PI_F * fraction);
}

static void fill_circle_coords(ccl::float3* const buffer, const float z_coord, const size_t verts)
{
	// z_coord^2 + radius^2 = 1
	// radius = root(1-z_coord^2)
	float radius = sqrtf(1.0f - z_coord * z_coord);

	for (size_t i = 0; i < verts; i++) {
		float fraction = static_cast<float>(i) / static_cast<float>(verts);
		
		buffer[i].x = cosf(M_2PI_F * fraction) * radius;
		buffer[i].y = sinf(M_2PI_F * fraction) * radius;
		buffer[i].z = z_coord;
	}
}

ccl::Mesh* get_sphere_mesh(const int layers, const int ring_verts, ccl::Shader* const shader)
{
	if (layers < 4) {
		return nullptr;
	}

	ccl::Mesh* const result = new ccl::Mesh();

	ccl::array<ccl::Node*> used_shaders;
	used_shaders.push_back_slow(shader);
	result->set_used_shaders(used_shaders);
	
	ccl::array<ccl::float3>& verts = result->get_verts();
	verts.resize(2 + (layers - static_cast<size_t>(1)) * ring_verts); // +2 is top and bottom vert

	// First two verts are top and bottom
	verts[0].x = 0.0f;
	verts[0].y = 0.0f;
	verts[0].z = 1.0f;
	verts[1].x = 0.0f;
	verts[1].y = 0.0f;
	verts[1].z = -1.0f;

	// Fill in verts for each layer
	ccl::float3* const vert_data = verts.data();
	ccl::float3* const ring_vert_data = vert_data + 2;
	for (size_t i = 1; i < layers; i++) {
		float z_coord = find_z_coord(layers, i);
		fill_circle_coords(ring_vert_data + (i - 1) * ring_verts, z_coord, ring_verts); // (i - 1) because we have no data for ring 0
	}

	// Add triangles for top layer
	for (int i = 0; i < ring_verts; i++) {
		// Add in CCW pattern
		int v0 = 0; // Top vert
		int v1 = 2 + i;
		int v2 = 2 + ((i + 1) % ring_verts);
		result->add_triangle(v0, v1, v2, 0, false);
	}

	// Add triangle pairs for inner layers
	for (int rows_written = 1; rows_written < (layers - 1); rows_written++) {
		for (int i = 0; i < ring_verts; i++) {
			// Find the quad made of 4 verticies
			int v0_top = 2 + (rows_written - 1) * ring_verts + i;
			int v1_top = 2 + (rows_written - 1) * ring_verts + ((i + 1) % ring_verts);
			int v0_bot = 2 + (rows_written) * ring_verts + i;
			int v1_bot = 2 + (rows_written) * ring_verts + ((i + 1) % ring_verts);
			result->add_triangle(v1_top, v0_top, v0_bot, 0, false);
			result->add_triangle(v0_bot, v1_bot, v1_top, 0, false);
		}
	}

	// Add triangles for bottom layer
	for (int i = 0; i < ring_verts; i++) {
		int v0 = 1; // Bottom vert
		int v1 = 2 + (layers - 2) * ring_verts + i;
		int v2 = 2 + (layers - 2) * ring_verts + ((i + 1) % ring_verts);
		result->add_triangle(v0, v2, v1, 0, false);
	}

	return result;
}
