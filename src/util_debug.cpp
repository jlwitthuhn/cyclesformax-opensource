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
 
#include "util_debug.h"

#include <chrono>

#include <Windows.h>

#include "rend_logger.h"

extern bool enable_plugin_debug;

void debug_spin(wchar_t* const label, const int milliseconds)
{
	const std::chrono::milliseconds spin_duration(milliseconds);

	if (enable_plugin_debug == false) {
		// Debug mode is off, no need to spin
		return;
	}
	const std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(label, true);

	*logger << "Spin begin..." << LogCtl::WRITE_LINE;
	while (true) {
		const auto waited = std::chrono::steady_clock::now() - time_begin;
		if (waited > spin_duration) {
			break;
		}
		Sleep(0);
	}
	*logger << "Spin complete" << LogCtl::WRITE_LINE;
}
