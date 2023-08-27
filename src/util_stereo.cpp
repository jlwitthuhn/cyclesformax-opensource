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
 
#include "util_stereo.h"

static bool should_copy_r_channel(const AccumulationBufferType type, const int render_index)
{
	if (type == AccumulationBufferType::NONE) {
		return false;
	}
	if (type == AccumulationBufferType::SINGLE || type == AccumulationBufferType::LEFT_RIGHT || type == AccumulationBufferType::TOP_BOTTOM) {
		return true;
	}

	if (render_index % 2 == 0) {
		return (type == AccumulationBufferType::ANAGLYPH_RC || type == AccumulationBufferType::ANAGLYPH_YB);
	}
	else {
		return (type == AccumulationBufferType::ANAGLYPH_GM);
	}
}

static bool should_copy_g_channel(const AccumulationBufferType type, const int render_index)
{
	if (type == AccumulationBufferType::NONE) {
		return false;
	}
	if (type == AccumulationBufferType::SINGLE || type == AccumulationBufferType::LEFT_RIGHT || type == AccumulationBufferType::TOP_BOTTOM) {
		return true;
	}

	if (render_index % 2 == 0) {
		return (type == AccumulationBufferType::ANAGLYPH_GM || type == AccumulationBufferType::ANAGLYPH_YB);
	}
	else {
		return (type == AccumulationBufferType::ANAGLYPH_RC);
	}
}

static bool should_copy_b_channel(const AccumulationBufferType type, const int render_index)
{
	if (type == AccumulationBufferType::NONE) {
		return false;
	}
	if (type == AccumulationBufferType::SINGLE || type == AccumulationBufferType::LEFT_RIGHT || type == AccumulationBufferType::TOP_BOTTOM) {
		return true;
	}

	if (render_index % 2 == 0) {
		return false;
	}
	else {
		return true;
	}
}

static bool should_copy_a_channel(const AccumulationBufferType type)
{
	return (type == AccumulationBufferType::SINGLE || type == AccumulationBufferType::LEFT_RIGHT || type == AccumulationBufferType::TOP_BOTTOM);
}

ColorChannelFlags::ColorChannelFlags(const bool r, const bool g, const bool b, const bool a) :
	copy_r(r),
	copy_g(g),
	copy_b(b),
	copy_a(a)
{

}

ColorChannelFlags::ColorChannelFlags(const AccumulationBufferType type, const int render_index) :
	copy_r(should_copy_r_channel(type, render_index)),
	copy_g(should_copy_g_channel(type, render_index)),
	copy_b(should_copy_b_channel(type, render_index)),
	copy_a(should_copy_a_channel(type))
{

}

int get_render_count_for_stereo_type(const StereoscopyType stereo_type)
{
	if (stereo_type == StereoscopyType::ANAGLYPH ||
		stereo_type == StereoscopyType::SPLIT_LEFT_RIGHT ||
		stereo_type == StereoscopyType::SPLIT_TOP_BOTTOM )
	{
		return 2;
	}

	return 1;
}

bool is_buffer_type_anaglyph(const AccumulationBufferType type)
{
	return (type == AccumulationBufferType::ANAGLYPH_RC || type == AccumulationBufferType::ANAGLYPH_GM || type == AccumulationBufferType::ANAGLYPH_YB);
}
