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
 
#include "cycles_image.h"

#include <cassert>

static constexpr size_t DEPTH = 1;
static constexpr int CHANNNELS_USED = 4;

bool CyclesPluginImageLoader::load_metadata(const ccl::ImageDeviceFeatures&, ccl::ImageMetaData& metadata)
{
	metadata.depth = DEPTH;
	metadata.channels = CHANNNELS_USED;

	metadata.width = sampled_image.width;
	metadata.height = sampled_image.height;

	if (sampled_image.float_pixels != nullptr) {
		metadata.type = ccl::ImageDataType::IMAGE_DATA_TYPE_FLOAT4;
	}
	else {
		metadata.type = ccl::ImageDataType::IMAGE_DATA_TYPE_BYTE4;
	}

	return true;
}

bool CyclesPluginImageLoader::load_pixels(const ccl::ImageMetaData& metadata, void* const pixels, const size_t pixels_size, bool)
{
	(pixels_size); // This is so msvc doesn't complain about an unused variable in release builds
	if (metadata.type == ccl::ImageDataType::IMAGE_DATA_TYPE_BYTE4) {
		assert(data.char_pixels.get() != nullptr);
		assert(data.float_pixels.get() == nullptr);
		assert(pixels_size = metadata.width * metadata.height * metadata.channels);
		std::memcpy(pixels, sampled_image.char_pixels.get(), metadata.width * metadata.height * metadata.channels);
		return true;
	}
	else if (metadata.type == ccl::ImageDataType::IMAGE_DATA_TYPE_FLOAT4) {
		assert(data.float_pixels.get() != nullptr);
		assert(data.char_pixels.get() == nullptr);
		assert(pixels_size = metadata.width * metadata.height * metadata.channels * sizeof(float));
		std::memcpy(pixels, sampled_image.float_pixels.get(), metadata.width * metadata.height * metadata.channels * sizeof(float));
		return true;
	}

	return false;
}

std::string CyclesPluginImageLoader::name() const
{
	return sampled_image.filename;
}

bool CyclesPluginImageLoader::equals(const ImageLoader& other) const
{
	const CyclesPluginImageLoader* const ptr{ dynamic_cast<const CyclesPluginImageLoader*>(&other) };
	return (
		sampled_image.char_pixels == ptr->sampled_image.char_pixels &&
		sampled_image.float_pixels == ptr->sampled_image.float_pixels &&
		sampled_image.width == ptr->sampled_image.width &&
		sampled_image.height == ptr->sampled_image.height &&
		sampled_image.filename == ptr->sampled_image.filename
	);
}
