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
 * @brief Defines classes and functions to assist with render device selection.
 */

#include <set>
#include <string>
#include <vector>

#include "util_enums.h"

namespace ccl {
	class DeviceInfo;
}

/**
 * @brief Class to determine if a given render device has been selected based on user config. CUDA and OptiX only.
 */
class RenderDeviceSelector {
public:
	RenderDeviceSelector(RenderDevice device, std::wstring device_list_string);

	bool use_device(const ccl::DeviceInfo& device_info);
	bool use_device_id(const std::wstring& device_id);

	bool _use_single() const { return use_single; }

private:
	bool use_all{ false };
	bool use_single{ false };
	bool single_used{ false };
	RenderDevice device_type;
	std::set<std::wstring> selected_devices;
};

/**
 * @brief Class to contain both a Cycles device ID and human-readable description.
 */
class CyclesDeviceDesc {
public:
	CyclesDeviceDesc(std::wstring id, std::wstring desc);

	std::wstring id;
	std::wstring desc;
};

/**
 * @brief Returns a ccl::DeviceInfo based on user-configurable device selection parameters.
 */
ccl::DeviceInfo get_cycles_device_info(
	RenderDevice rend_device,
	std::wstring cuda_device_list,
	std::wstring optix_device_list,
	int threads,
	bool cuda_with_cpu,
	bool debug_multi_cuda
);

std::vector<CyclesDeviceDesc> get_cuda_device_descs(bool debug_multi_cuda);
std::vector<CyclesDeviceDesc> get_optix_device_descs(bool debug_multi_cuda);
