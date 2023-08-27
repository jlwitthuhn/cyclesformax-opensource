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
 
#include "util_resolution.h"

static Int2 get_output_res(const int final_width, const int final_height)
{
	return Int2(final_width, final_height);
}

static Int2 get_render_res(const int final_width, const int final_height, const StereoscopyType stereo_type)
{
	if (stereo_type == StereoscopyType::SPLIT_LEFT_RIGHT) {
		return Int2(final_width / 2, final_height);
	}
	else if (stereo_type == StereoscopyType::SPLIT_TOP_BOTTOM) {
		return Int2(final_width, final_height / 2);
	}
	else {
		return Int2(final_width, final_height);
	}
}

RenderResolutions::RenderResolutions(const int final_width, const int final_height, const StereoscopyType stereo_type) :
	_output_res(get_output_res(final_width, final_height)),
	_render_res(get_render_res(final_width, final_height, stereo_type))
{

}
