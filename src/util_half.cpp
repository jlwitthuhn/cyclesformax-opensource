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
 
#include <OpenEXR/half.h>

#include "util_half.h"

ccl::float4 float4_from_half_array(const pluginHalf* const half_array)
{
	ccl::float4 result;

	half r_half, g_half, b_half, a_half;
	r_half.setBits(half_array[0]);
	g_half.setBits(half_array[1]);
	b_half.setBits(half_array[2]);
	a_half.setBits(half_array[3]);

	result.x = static_cast<float>(r_half);
	result.y = static_cast<float>(g_half);
	result.z = static_cast<float>(b_half);
	result.w = static_cast<float>(a_half);

	return result;
}
