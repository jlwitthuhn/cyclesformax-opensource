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
 * @brief Defines functions to build param objects used internally by Cycles.
 */

#include <vector>

#include <boost/optional.hpp>

#include "util_pass.h"

class CyclesRenderParams;
class Int2;

namespace ccl {
	class BufferParams;
	class SceneParams;
	class SessionParams;
}

ccl::BufferParams get_buffer_params(Int2 buffer_size, std::vector<RenderPassInfo> pass_info = std::vector<RenderPassInfo>{});
boost::optional<ccl::SessionParams> get_session_params(const CyclesRenderParams& rend_params);
ccl::SceneParams get_scene_params();
