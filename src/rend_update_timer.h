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
 * @brief Defines the class BufferUpdateTimer.
 */

#include <chrono>

/**
 * @brief Class to determine when the Max output framebuffer should be updated.
 *
 * The period bewteen updates increases as the render goes on to reduce copying overhead
 * in long-running renders.
 */
class BufferUpdateTimer {
public:
	BufferUpdateTimer();

	void reset();

	bool should_update();

private:
	std::chrono::milliseconds sleep_delay;
	std::chrono::milliseconds initial_update_delay;

	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point last_update;
};
