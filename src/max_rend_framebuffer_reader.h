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
 * @brief Defines classes that can load Cycles render buffer data into the Max framebuffer.
 */

#include <map>
#include <memory>
#include <vector>

#include <RenderingAPI/Renderer/IFrameBufferProcessor.h>

#include "util_pass.h"
#include "util_simple_types.h"

class ToneOperator;

/**
 * @brief Class to store a full frame buffer created from multiple Cycles render tiles.
 */
class AccumulationBuffer {
public:
	AccumulationBuffer(int width, int height, const std::vector<RenderPassInfo>& render_pass_info_vec);
	~AccumulationBuffer();

	size_t num_pixels() const;

	int get_width() const;
	int get_height() const;

	float* get_pass_buffer(std::string name) const;

private:
	const int width;
	const int height;

	float* data = nullptr;

	std::map<std::string, int> pass_offsets;
};

/**
 * @brief Class to set the Max framebuffer to a constant color.
 */
class ConstantColorFrameBufferReader : public MaxSDK::RenderingAPI::IFrameBufferProcessor::IFrameBufferReader {
public:
	ConstantColorFrameBufferReader(BMM_Color_fl color, Int2 resolution);

	virtual bool GetPixelLine(const unsigned int y, const unsigned int x_start, const unsigned int num_pixels, BMM_Color_fl* const target_pixels) override;
	virtual IPoint2 GetResolution() const override;
	virtual Box2 GetRegion() const override;

private:
	const BMM_Color_fl color;
	const Int2 resolution;
};

/**
* @brief Class to set the Max framebuffer from an AccumulationBuffer.
*/
class AccumulationBufferReader : public MaxSDK::RenderingAPI::IFrameBufferProcessor::IFrameBufferReader {
public:
	AccumulationBufferReader(AccumulationBuffer* buffer_in, ToneOperator* tone_operator, IntRect region);

	virtual bool GetPixelLine(const unsigned int y, const unsigned int x_start, const unsigned int num_pixels, BMM_Color_fl* const target_pixels) override;
	virtual IPoint2 GetResolution() const override;
	virtual Box2 GetRegion() const override;

	void set_backplate(std::shared_ptr<BackplateBitmap> bitmap);

private:
	AccumulationBuffer* const buffer;
	ToneOperator* const tone_operator;
	const IntRect region;

	std::shared_ptr<BackplateBitmap> backplate;

	float* get_backplate_pixel(unsigned int x, unsigned int y);
};
