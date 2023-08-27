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
 * @brief Defines the class CyclesStatus.
 */

#include <string>

/**
 * @brief Class to describe to current render status.
 */
class CyclesStatus {
public:
	CyclesStatus(std::string status, std::string substat, int requested_samples);

	int work_done;
	int work_total;
	int samples_rendered;
	bool complete;
	bool render_in_progress;
	bool errored;
	std::wstring max_render_status_message;
	std::wstring error_message;
};
