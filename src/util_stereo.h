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
 * @brief Defines classes and functions to support stereoscopic rendering.
 */

#include <util/util_types.h>

#include "util_enums.h"


/**
 * @brief Class to track which channels should be copied from a render buffer in stereo renders.
 */
class ColorChannelFlags {
public:
	ColorChannelFlags(bool r, bool g, bool b, bool a);
	ColorChannelFlags(AccumulationBufferType type, int render_index);

	const bool copy_r;
	const bool copy_g;
	const bool copy_b;
	const bool copy_a;
};

/**
 * @brief Returns the total number of renders that must be done for a given stereo render type.
 */
int get_render_count_for_stereo_type(StereoscopyType stereo_type);

/**
 * @brief Returns true if the given type is used for anaglyph stereo renders.
 */
bool is_buffer_type_anaglyph(AccumulationBufferType type);
