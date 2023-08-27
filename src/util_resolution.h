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

#include "util_enums.h"
#include "util_simple_types.h"

#include <util/util_types.h>

/**
 * @brief Helper class to store various resolutions related to the current render.
 */
class RenderResolutions {
public:
	RenderResolutions(int final_width, int final_height, StereoscopyType stereo_type);

	inline Int2 output_res() const { return _output_res; }
	inline Int2 render_res() const { return _render_res; }

private:
	// Resolution of the Max frame buffer to write the result into
	Int2 _output_res;
	// Resolution of one cycles render, ignoring selected region
	Int2 _render_res;
};
