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
 
#include "max_rend_framebuffer_reader.h"

#include <OpenEXR/half.h>
#include <Rendering/ToneOperator.h>

constexpr size_t BACKPLATE_CHANNELS = 3;

AccumulationBuffer::AccumulationBuffer(const int width, const int height, const std::vector<RenderPassInfo>& render_pass_info_vec) :
	width(width), height(height)
{
	constexpr int COMBINED_PASS_CHANNELS = 4;
	pass_offsets["Combined"] = 0;

	int total_channels = COMBINED_PASS_CHANNELS; // Start with just combined pass
	for (const RenderPassInfo this_info : render_pass_info_vec) {
		if (this_info.channels == 0) {
			continue;
		}
		if (pass_offsets.count(this_info.name) == 0) {
			pass_offsets[this_info.name] = total_channels;
			total_channels += this_info.channels;
		}
	}

	const size_t buffer_size = num_pixels() * total_channels;
	data = new float[buffer_size];
	
	// Initialize all passes
	// To 0.0f for one-channel
	// To (0.0f, 0.0f, 0.0f, 1.0f) for four-channel
	for (const RenderPassInfo this_info : render_pass_info_vec) {
		float* const this_pass_buffer = get_pass_buffer(this_info.name);
		if (this_info.channels == 1) {
			for (int i = 0; i < num_pixels() * this_info.channels; i++) {
				this_pass_buffer[i] = 0.0f;
			}
		}
		else if (this_info.channels == 4) {
			for (int i = 0; i < num_pixels() * this_info.channels; i++) {
				if (i % 4 == 3) {
					this_pass_buffer[i] = 1.0f;
				}
				else {
					this_pass_buffer[i] = 0.0f;
				}
			}
		}
	}
}

AccumulationBuffer::~AccumulationBuffer()
{
	if (data != nullptr) {
		delete data;
		data = nullptr;
	}
}

size_t AccumulationBuffer::num_pixels() const
{
	return static_cast<size_t>(get_width()) * get_height();
}

int AccumulationBuffer::get_width() const
{
	return width;
}

int AccumulationBuffer::get_height() const
{
	return height;
}

float* AccumulationBuffer::get_pass_buffer(const std::string name) const
{
	if (pass_offsets.count(name) == 1) {
		return data + num_pixels() * pass_offsets.at(name);
	}
	// Default to combined pass
	return data;
}

ConstantColorFrameBufferReader::ConstantColorFrameBufferReader(const BMM_Color_fl color, const Int2 resolution) :
	color(color),
	resolution(resolution)
{

}

bool ConstantColorFrameBufferReader::GetPixelLine(const unsigned int /*y*/, const unsigned int /*x_start*/, const unsigned int num_pixels, BMM_Color_fl* const target_pixels)
{
	for (unsigned int i = 0; i < num_pixels; ++i) {
		target_pixels[i] = color;
	}
	return true;
}

IPoint2 ConstantColorFrameBufferReader::GetResolution() const
{
	return IPoint2(resolution.x(), resolution.y());
}

Box2 ConstantColorFrameBufferReader::GetRegion() const
{
	return Box2(IPoint2(0.0, 0.0), GetResolution());
}

AccumulationBufferReader::AccumulationBufferReader(AccumulationBuffer* const buffer_in, ToneOperator* const tone_operator, const IntRect region) :
	buffer(buffer_in),
	tone_operator(tone_operator),
	region(region)
{

}

bool AccumulationBufferReader::GetPixelLine(const unsigned int y, const unsigned int x_start, const unsigned int num_pixels, BMM_Color_fl* const target_pixels)
{
	const int width = buffer->get_width();
	const int height = buffer->get_height();
	const unsigned int start_index = (height - 1 - y) * width + x_start;

	float* const rgba_buffer = buffer->get_pass_buffer("Combined");
	for (unsigned int i = 0; i < num_pixels; ++i) {
		const float y_pos = y;
		const float x_pos = x_start + i;

		float source_pixel[4];

		source_pixel[0] = rgba_buffer[4 * (start_index + i) + 0];
		source_pixel[1] = rgba_buffer[4 * (start_index + i) + 1];
		source_pixel[2] = rgba_buffer[4 * (start_index + i) + 2];
		source_pixel[3] = rgba_buffer[4 * (start_index + i) + 3];

		if (backplate) {
			//float* const backplate_pixel = backplate->pixels + BACKPLATE_CHANNELS * ((start_index + i) % backplate->num_pixels());
			float* const backplate_pixel = get_backplate_pixel(x_start + i, height - 1 - y);

			float composite_pixel[4];
			if (tone_operator != nullptr && tone_operator->GetProcessBackground()) {
				// Apply tone operator to the composited value
				const float alpha = source_pixel[3];
				composite_pixel[0] = source_pixel[0] + (1.0f - alpha) * backplate_pixel[0];
				composite_pixel[1] = source_pixel[1] + (1.0f - alpha) * backplate_pixel[1];
				composite_pixel[2] = source_pixel[2] + (1.0f - alpha) * backplate_pixel[2];
				tone_operator->ScaledToRGB(composite_pixel, Point2(x_pos, y_pos));
			}
			else {
				// Apply tone operator only to the rendered value, not the backplate
				if (tone_operator) {
					tone_operator->ScaledToRGB(source_pixel, Point2(x_pos, y_pos));
				}
				const float alpha = source_pixel[3];
				composite_pixel[0] = source_pixel[0] + (1.0f - alpha) * backplate_pixel[0];
				composite_pixel[1] = source_pixel[1] + (1.0f - alpha) * backplate_pixel[1];
				composite_pixel[2] = source_pixel[2] + (1.0f - alpha) * backplate_pixel[2];
			}

			target_pixels[i].r = composite_pixel[0];
			target_pixels[i].g = composite_pixel[1];
			target_pixels[i].b = composite_pixel[2];
			target_pixels[i].a = 1.0f;
		}
		else {
			if (tone_operator) {
				tone_operator->ScaledToRGB(source_pixel, Point2(x_pos, y_pos));
			}

			target_pixels[i].r = source_pixel[0];
			target_pixels[i].g = source_pixel[1];
			target_pixels[i].b = source_pixel[2];
			target_pixels[i].a = source_pixel[3];
		}
	}
	return true;
}

IPoint2 AccumulationBufferReader::GetResolution() const
{
	return IPoint2(buffer->get_width(), buffer->get_height());
}

Box2 AccumulationBufferReader::GetRegion() const
{
	const Int2 point_begin = region.begin();
	const Int2 point_end = region.end();

	const IPoint2 max_point_begin(point_begin.x(), point_begin.y());
	const IPoint2 max_point_end(point_end.x() - 1, point_end.y() - 1);

	return Box2(max_point_begin, max_point_end);
}

void AccumulationBufferReader::set_backplate(std::shared_ptr<BackplateBitmap> bitmap)
{
	backplate = bitmap;
}

float* AccumulationBufferReader::get_backplate_pixel(const unsigned int x_in, const unsigned int y_in)
{
	size_t x = x_in;
	if (x >= backplate->resolution.x()) {
		x = x % backplate->resolution.x();
	}
	size_t y = y_in;
	if (y >= backplate->resolution.y()) {
		y = y % backplate->resolution.y();
	}
	return backplate->pixels + ((y * backplate->resolution.x()) + x) * BACKPLATE_CHANNELS;
}
