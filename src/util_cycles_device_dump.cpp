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
 
#include "util_cycles_device_dump.h"

#include <sstream>
#include <string>

#include <device/device.h>

#include <RenderingAPI/Renderer/IRenderingLogger.h>

static void log_device_info(ccl::DeviceInfo& device_info, MaxSDK::RenderingAPI::IRenderingLogger* const logger, const size_t indentation)
{
	std::wstring line_begin(indentation * 2, L'-');

	std::wstring type(L"UNKNOWN");
	if (device_info.type == ccl::DeviceType::DEVICE_NONE) {
		type = std::wstring(L"DEVICE_NONE");
	}
	else if (device_info.type == ccl::DeviceType::DEVICE_CPU) {
		type = std::wstring(L"DEVICE_CPU");
	}
	else if (device_info.type == ccl::DeviceType::DEVICE_OPENCL) {
		type = std::wstring(L"DEVICE_OPENCL");
	}
	else if (device_info.type == ccl::DeviceType::DEVICE_CUDA) {
		type = std::wstring(L"DEVICE_CUDA");
	}
	else if (device_info.type == ccl::DeviceType::DEVICE_NETWORK) {
		type = std::wstring(L"DEVICE_NETWORK");
	}
	else if (device_info.type == ccl::DeviceType::DEVICE_MULTI) {
		type = std::wstring(L"DEVICE_MULTI");
	}
	std::wstring type_line = line_begin + std::wstring(L"type: ") + type;

	std::wstring desc_line = line_begin + L"description: ";
	std::wstringstream desc_stream;
	for (char this_char : device_info.description) {
		desc_stream << static_cast<wchar_t>(this_char);
	}
	desc_line += desc_stream.str();

	std::wstring id_line = line_begin + L"id: ";
	std::wstringstream id_stream;
	for (char this_char : device_info.id) {
		id_stream << static_cast<wchar_t>(this_char);
	}
	id_line += id_stream.str();

	std::wstring num_line = line_begin + L"num: ";
	num_line += std::to_wstring(device_info.num);

	std::wstring display_device_line = line_begin + L"display_device: ";
	if (device_info.display_device) {
		display_device_line += L"true";
	}
	else {
		display_device_line += L"false";
	}

	std::wstring use_split_kernel_line = line_begin + L"use_split_kernel: ";
	if (device_info.use_split_kernel) {
		use_split_kernel_line += L"true";
	}
	else {
		use_split_kernel_line += L"false";
	}

	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, type_line.c_str());
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, desc_line.c_str());
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, id_line.c_str());
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, num_line.c_str());
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, display_device_line.c_str());
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, use_split_kernel_line.c_str());

	if (device_info.type == ccl::DeviceType::DEVICE_MULTI) {
		std::wstring subdevice_line = line_begin + L"subdevices: ";
		logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, subdevice_line.c_str());
		for (ccl::DeviceInfo& subdevice_info : device_info.multi_devices) {
			logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, L"-----");
			log_device_info(subdevice_info, logger, indentation + 1);
		}
	}
}

void dump_cycles_devices(MaxSDK::RenderingAPI::IRenderingLogger* const logger)
{
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, L"Available cycles render devices:");
	logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, L"----------");

	ccl::vector<ccl::DeviceInfo> devices = ccl::Device::available_devices();

	for (ccl::DeviceInfo& device_info : devices) {
		log_device_info(device_info, logger, 1);
		logger->LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, L"----------");
	}
}
