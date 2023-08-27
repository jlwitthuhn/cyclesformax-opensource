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
 
#include "util_cycles_device.h"

#include <sstream>

#include <boost/algorithm/string.hpp>

#include <device/device.h>

static std::wstring wstring_from_string(std::string input) {
	std::wstringstream stream;
	for (char c : input) {
		stream << static_cast<wchar_t>(c);
	}
	return stream.str();
}

static bool device_types_match(const RenderDevice device_type, const ccl::DeviceType ccl_device_type)
{
	if (device_type == RenderDevice::CUDA && ccl_device_type == ccl::DeviceType::DEVICE_CUDA) {
		return true;
	}
	if (device_type == RenderDevice::OPTIX && ccl_device_type == ccl::DeviceType::DEVICE_OPTIX) {
		return true;
	}
	return false;
}

RenderDeviceSelector::RenderDeviceSelector(const RenderDevice device, const std::wstring device_list_string) :
	device_type(device)
{
	std::wstring trimmed_input = device_list_string;
	boost::trim(trimmed_input);

	if (trimmed_input == L"ALL") {
		use_all = true;
	}
	else if (trimmed_input == L"SINGLE") {
		use_single = true;
	}

	std::vector<std::wstring> device_vector;
	boost::split(device_vector, device_list_string, boost::is_any_of(","), boost::token_compress_on);
	for (std::wstring device_id : device_vector) {
		boost::trim(device_id);
		selected_devices.insert(device_id);
	}
}

bool RenderDeviceSelector::use_device(const ccl::DeviceInfo& device_info)
{
	if (device_types_match(device_type, device_info.type) == false) {
		return false;
	}
	return use_device_id(wstring_from_string(device_info.id));
}

bool RenderDeviceSelector::use_device_id(const std::wstring& device_id)
{
	if (use_all) {
		return true;
	}
	if (use_single) {
		if (single_used) {
			return false;
		}
		else {
			single_used = true;
			return true;
		}
	}
	if (selected_devices.count(device_id) == 1) {
		return true;
	}
	return false;
}

CyclesDeviceDesc::CyclesDeviceDesc(std::wstring id, std::wstring desc)
{
	this->id = id;
	this->desc = desc;
}

ccl::DeviceInfo get_cycles_device_info(
	const RenderDevice rend_device,
	const std::wstring cuda_device_list,
	const std::wstring optix_device_list,
	const int threads,
	const bool cuda_with_cpu,
	const bool debug_multi_cuda
	)
{
	constexpr ccl::uint MASK = static_cast<ccl::uint>(ccl::DeviceTypeMask::DEVICE_MASK_ALL);
	ccl::vector<ccl::DeviceInfo> devices = ccl::Device::available_devices(MASK, debug_multi_cuda);

	if (rend_device == RenderDevice::CPU) {
		for (ccl::DeviceInfo& device_info : devices) {
			if (device_info.type == ccl::DEVICE_CPU) {
				return device_info;
			}
		}
	}
	else if (rend_device == RenderDevice::CUDA) {
		RenderDeviceSelector selector(RenderDevice::CUDA, cuda_device_list);

		ccl::vector<ccl::DeviceInfo> selected_devices;
		for (const ccl::DeviceInfo& device_info : devices) {
			if (device_info.type == ccl::DEVICE_CUDA) {
				if (selector.use_device(device_info)) {
					selected_devices.push_back(device_info);
				}
			}
		}
		if (cuda_with_cpu) {
			for (ccl::DeviceInfo& device_info : devices) {
				if (device_info.type == ccl::DEVICE_CPU) {
					selected_devices.push_back(device_info);
				}
			}
		}
		if (selected_devices.size() == 1) {
			return selected_devices[0];
		}
		else if (selected_devices.size() > 1) {
			return ccl::Device::get_multi_device(selected_devices, threads, true);
		}
	}
	else if (rend_device == RenderDevice::OPTIX) {
		RenderDeviceSelector selector(RenderDevice::OPTIX, optix_device_list);
		ccl::vector<ccl::DeviceInfo> selected_devices;
		for (const ccl::DeviceInfo& device_info : devices) {
			if (device_info.type == ccl::DEVICE_OPTIX) {
				if (selector.use_device(device_info)) {
					selected_devices.push_back(device_info);
				}
			}
		}
		if (selected_devices.size() == 1) {
			return selected_devices[0];
		}
		else if (selected_devices.size() > 1) {
			return ccl::Device::get_multi_device(selected_devices, threads, true);
		}
	}

	ccl::DeviceInfo error_result;
	error_result.type = ccl::DeviceType::DEVICE_NONE;

	return error_result;
}

std::vector<CyclesDeviceDesc> get_cuda_device_descs(const bool debug_multi_cuda)
{
	std::vector<CyclesDeviceDesc> result;

	constexpr ccl::uint MASK = static_cast<ccl::uint>(ccl::DeviceTypeMask::DEVICE_MASK_ALL);
	ccl::vector<ccl::DeviceInfo> devices = ccl::Device::available_devices(MASK, debug_multi_cuda);
	for (const ccl::DeviceInfo& device_info : devices) {
		if (device_info.type == ccl::DeviceType::DEVICE_CUDA) {
			std::wstring device_id = wstring_from_string(device_info.id);
			std::wstring device_desc = wstring_from_string(device_info.description);
			std::wstring desc_line = device_id + L" - " + device_desc;
			result.push_back(CyclesDeviceDesc(device_id, desc_line));
		}
	}

	return result;
}

std::vector<CyclesDeviceDesc> get_optix_device_descs(const bool debug_multi_cuda)
{
	std::vector<CyclesDeviceDesc> result;

	constexpr ccl::uint MASK = static_cast<ccl::uint>(ccl::DeviceTypeMask::DEVICE_MASK_ALL);
	ccl::vector<ccl::DeviceInfo> devices = ccl::Device::available_devices(MASK, debug_multi_cuda);
	for (const ccl::DeviceInfo& device_info : devices) {
		if (device_info.type == ccl::DeviceType::DEVICE_OPTIX) {
			std::wstring device_id = wstring_from_string(device_info.id);
			std::wstring device_desc = wstring_from_string(device_info.description);
			std::wstring desc_line = device_id + L" - " + device_desc;
			result.push_back(CyclesDeviceDesc(device_id, desc_line));
		}
	}

	return result;
}
