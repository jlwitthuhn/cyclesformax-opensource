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
 * @brief Defines classes to support managing Cycles render passes.
 */

#include <cstddef>
#include <string>
#include <vector>

#include <kernel/kernel_types.h>
#include <render/film.h>
#include <util/util_vector.h>

#include "util_enums.h"

class CyclesRenderElement;

/**
 * @brief Class to describe a single render pass.
 */
class RenderPassInfo {
public:
	RenderPassInfo();
	RenderPassInfo(CyclesRenderElement* render_element);
	RenderPassInfo(CyclesRenderElement* render_element, size_t suffix);
	RenderPassInfo(RenderPassType type, ccl::PassType ccl_type, int channels, std::string name, size_t suffix);

	RenderPassType type;
	ccl::PassType ccl_type;
	int channels;
	std::string name;
	CyclesRenderElement* render_element;
};

/**
 * @brief Converts render pass info from internal format to an array of ccl::Pass.
 */
ccl::vector<ccl::Pass> get_ccl_pass_vector(const std::vector<RenderPassInfo>& pass_info_vector);
