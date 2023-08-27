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
 * @brief Defines classes and functions needed by the cycles image loading API.
 */

#include <memory>
#include <string>

#include <render/image.h>

/**
 * @brief Class that is used to store a sampled image to be loaded into Cycles.
 */
class SampledImage
{
public:
	size_t width{ 0 };
	size_t height{ 0 };

	std::string filename;

	std::shared_ptr<unsigned char> char_pixels;
	std::shared_ptr<float> float_pixels;
};

/**
 * @brief Class that is used to load (2d) textures into Cycles.
 */
class CyclesPluginImageLoader : public ccl::ImageLoader
{
public:
	CyclesPluginImageLoader(SampledImage sampled_image) : sampled_image{ sampled_image } {}

	virtual bool load_metadata(const ccl::ImageDeviceFeatures &features, ccl::ImageMetaData &metadata) override;
	virtual bool load_pixels(const ccl::ImageMetaData &metadata, void* pixels, const size_t pixels_size, const bool associate_alpha) override;

	virtual std::string name() const override;
	virtual bool equals(const ImageLoader &other) const override;

private:
	SampledImage sampled_image;
};
