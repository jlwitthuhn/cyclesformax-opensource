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
 * @brief Defines the class ColorAssigner.
 */

#include <map>
#include <mutex>
#include <random>
#include <vector>

#include <util/util_types.h>

#include "util_simple_types.h"

/**
 * @brief Class to generate random colors for tile borders in a thread-safe manner.
 */
class ColorAssigner {
public:
	ColorAssigner();

	ccl::float3 get_color(int x, int y);

private:
	std::minstd_rand rng;
	std::mutex assigned_colors_mutex;

	std::vector<ccl::float3> available_colors;
	std::map<Int2, ccl::float3> assigned_colors;
};
