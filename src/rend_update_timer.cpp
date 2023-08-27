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
 
#include "rend_update_timer.h"

BufferUpdateTimer::BufferUpdateTimer()
{
	reset();
}

void BufferUpdateTimer::reset()
{
	begin = std::chrono::steady_clock::now();
	last_update = std::chrono::steady_clock::now();
}

bool BufferUpdateTimer::should_update()
{
	const std::chrono::milliseconds update_delay{ 450 };

	const std::chrono::steady_clock::time_point now{ std::chrono::steady_clock::now() };
	const std::chrono::seconds seconds_since_begin{ std::chrono::duration_cast<std::chrono::seconds>(now - begin) };
	const std::chrono::milliseconds ms_since_last_update{ std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update) };

	bool result{ false };
	if (seconds_since_begin.count() < 20) {
		if (ms_since_last_update > update_delay) {
			result = true;
		}
	}
	else if (seconds_since_begin.count() < 60) {
		if (ms_since_last_update > update_delay * 3) {
			result = true;
		}
	}
	else if (seconds_since_begin.count() < 120) {
		if (ms_since_last_update > update_delay * 6) {
			result = true;
		}
	}
	else {
		if (ms_since_last_update > update_delay * 10) {
			result = true;
		}
	}

	if (result) {
		last_update = std::chrono::steady_clock::now();
	}
	return result;
}
