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
 
#include "util_cycles_params.h"

#include <render/buffers.h>
#include <render/session.h>

#include "rend_params.h"
#include "util_cycles_device.h"

ccl::BufferParams get_buffer_params(const Int2 buffer_size, const std::vector<RenderPassInfo> pass_info)
{
	ccl::BufferParams buffer_params;

	buffer_params.width = buffer_size.x();
	buffer_params.height = buffer_size.y();

	buffer_params.full_width = buffer_size.x();
	buffer_params.full_height = buffer_size.y();

	if (pass_info.size() == 0) {
		const std::vector<RenderPassInfo> default_pass_info{ RenderPassInfo() };
		buffer_params.passes = get_ccl_pass_vector(default_pass_info);
	}
	else {
		buffer_params.passes = get_ccl_pass_vector(pass_info);
	}

	return buffer_params;
}

boost::optional<ccl::SessionParams> get_session_params(const CyclesRenderParams& rend_params)
{
	ccl::SessionParams session_params;

	session_params.background = true;
	session_params.threads = rend_params.cpu_threads;
	session_params.tile_order = ccl::TileOrder::TILE_HILBERT_SPIRAL;
	session_params.display_buffer_linear = true;

	session_params.samples = rend_params.samples;

	session_params.tile_size = ccl::make_int2(rend_params.tile_width, rend_params.tile_height);

	if (rend_params.use_progressive_refine) {
		session_params.progressive = true;
		session_params.progressive_refine = true;
		session_params.progressive_update_timeout = 0.35f;
	}
	else {
		session_params.progressive = false;
		session_params.progressive_refine = false;
	}

	ccl::DeviceInfo device_info{
		get_cycles_device_info(
			rend_params.render_device,
			rend_params.cuda_device,
			rend_params.optix_device,
			rend_params.cpu_threads,
			rend_params.cuda_with_cpu,
			rend_params.debug_multi_cuda
		)
	};

	if (device_info.type != ccl::DeviceType::DEVICE_NONE) {
		session_params.device = device_info;
	}
	else {
		return boost::none;
	}

	session_params.adaptive_sampling = rend_params.use_adaptive_sampling;

	return session_params;
}

ccl::SceneParams get_scene_params()
{
	ccl::SceneParams scene_params;

	scene_params.bvh_type = ccl::SceneParams::BVH_STATIC;
	scene_params.shadingsystem = ccl::SHADINGSYSTEM_SVM;

	return scene_params;
}
