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
 
#include "util_cycles_status.h"

#include <boost/regex.hpp>

CyclesStatus::CyclesStatus(const std::string status, const std::string substat, const int requested_samples)
{
	work_done = 0;
	work_total = 1;
	complete = false;
	render_in_progress = false;
	errored = false;

	samples_rendered = requested_samples;

	boost::regex sample_pattern("Path Tracing Sample (\\d+)\\/(\\d+)");
	boost::regex tile_pattern("Path Tracing Tile (\\d+)\\/(\\d+)");
	boost::regex tile_sample_pattern("Path Tracing Tile (\\d+)\\/(\\d+), Sample (\\d+)\\/(\\d+)");

	boost::smatch match;
	if (boost::regex_match(status, match, sample_pattern)) {
		work_done = std::stoi(match[1]);
		work_total = std::stoi(match[2]);
		samples_rendered = std::stoi(match[1]);
		render_in_progress = true;
		max_render_status_message = L"Rendering...";
	}
	else if (boost::regex_match(status, match, tile_pattern)) {
		work_done = std::stoi(match[1]);
		work_total = std::stoi(match[2]) + 1;
		render_in_progress = true;
		max_render_status_message = L"Rendering...";
	}
	else if (boost::regex_match(status, match, tile_sample_pattern)) {
		const int tiles_started = std::stoi(match[1]);
		const int tiles_total = std::stoi(match[2]);
		const int samples_done = std::stoi(match[3]);
		const int samples_total = std::stoi(match[4]);
		work_done = tiles_started;
		work_total = tiles_total;
		if (tiles_started == tiles_total && samples_done == samples_total) {
			complete = true;
		}
		render_in_progress = true;
		max_render_status_message = L"Rendering...";
	}
	else {
		work_done = 100;
		work_total = 100;
		max_render_status_message = std::wstring(status.begin(), status.end());
	}

	if (status == "Finished" || status == "Done") {
		complete = true;
	}

	// If the status is "Cancel", an error may be stored in the substat
	if (status == "Cancel") {
		errored = true;
		if (substat.find("error") != std::string::npos) {
			error_message = std::wstring(substat.begin(), substat.end());
		}
	}
}
