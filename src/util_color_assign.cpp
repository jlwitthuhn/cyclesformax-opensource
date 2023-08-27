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
 
#include "util_color_assign.h"

ColorAssigner::ColorAssigner()
{
	constexpr float HI_VAL = 0.97f;
	constexpr float ME_VAL = 0.41f;
	constexpr float LO_VAL = 0.12f;

	available_colors.push_back(ccl::make_float3(HI_VAL, LO_VAL, LO_VAL));
	available_colors.push_back(ccl::make_float3(LO_VAL, HI_VAL, LO_VAL));
	available_colors.push_back(ccl::make_float3(LO_VAL, LO_VAL, HI_VAL));

	available_colors.push_back(ccl::make_float3(HI_VAL, HI_VAL, LO_VAL));
	available_colors.push_back(ccl::make_float3(LO_VAL, HI_VAL, HI_VAL));
	available_colors.push_back(ccl::make_float3(HI_VAL, LO_VAL, HI_VAL));

	available_colors.push_back(ccl::make_float3(HI_VAL, ME_VAL, LO_VAL));
	available_colors.push_back(ccl::make_float3(HI_VAL, LO_VAL, ME_VAL));
	available_colors.push_back(ccl::make_float3(ME_VAL, HI_VAL, LO_VAL));
	available_colors.push_back(ccl::make_float3(LO_VAL, HI_VAL, ME_VAL));
	available_colors.push_back(ccl::make_float3(ME_VAL, LO_VAL, HI_VAL));
	available_colors.push_back(ccl::make_float3(LO_VAL, ME_VAL, HI_VAL));
}

ccl::float3 ColorAssigner::get_color(const int x, const int y)
{
	const std::lock_guard<std::mutex> lock(assigned_colors_mutex);
	const Int2 this_pair(x, y);

	if (assigned_colors.count(this_pair) == 1) {
		return assigned_colors[this_pair];
	}

	const size_t next_color_index = rng() % available_colors.size();
	assigned_colors[this_pair] = available_colors[next_color_index];
	return assigned_colors[this_pair];
}
