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
 
#include "cycles_session.h"

#include <render/background.h>
#include <render/scene.h>

#include <pbbitmap.h>
#include <Rendering/ToneOperator.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/IRenderSettingsContainer.h>

#include "max_rend_framebuffer_reader.h"
#include "plugin_re_simple.h"
#include "rend_params.h"
#include "util_color_assign.h"
#include "util_stereo.h"

static void copy_combined_tile_pixels_to_accum(
	float* const src_buffer, float* const dest_buffer,
	const RenderResolutions resolutions,
	const size_t x_in, const size_t y_in,
	const size_t w, const size_t h,
	const AccumulationBufferType buffer_type,
	const int render_index,
	const IntRect region)
{
	constexpr size_t COMBINED_CHANNELS = 4;
	const size_t src_row_stride = COMBINED_CHANNELS * w;
	const size_t dest_row_stride = COMBINED_CHANNELS * resolutions.output_res().x();

	const ColorChannelFlags channels(buffer_type, render_index);
	const bool copy_all_channels = channels.copy_r && channels.copy_g && channels.copy_b && channels.copy_a;

	const size_t x = x_in + region.begin().x();
	const size_t y = y_in + resolutions.render_res().y() - region.end().y();

	float* src_begin = src_buffer;
	// Choose our dest pointer based on buffer type
	float* dest_begin = nullptr;
	if (buffer_type == AccumulationBufferType::SINGLE ||
		buffer_type == AccumulationBufferType::ANAGLYPH_RC || buffer_type == AccumulationBufferType::ANAGLYPH_GM || buffer_type == AccumulationBufferType::ANAGLYPH_YB) {
		dest_begin = dest_buffer + COMBINED_CHANNELS * (static_cast<size_t>(resolutions.output_res().x()) * y + x);
	}
	else if (buffer_type == AccumulationBufferType::LEFT_RIGHT) {
		dest_begin = dest_buffer + COMBINED_CHANNELS * (static_cast<size_t>(resolutions.output_res().x()) * y + x);
		if (render_index % 2 == 1) {
			dest_begin += COMBINED_CHANNELS * resolutions.render_res().x();
		}
	}
	else if (buffer_type == AccumulationBufferType::TOP_BOTTOM) {
		dest_begin = dest_buffer + COMBINED_CHANNELS * (static_cast<size_t>(resolutions.output_res().x()) * y + x);
		if (render_index % 2 == 0) {
			dest_begin += COMBINED_CHANNELS * resolutions.render_res().x() * resolutions.render_res().y();
		}
	}
	else {
		// If we don't recognize the buffer type, abort here
		return;
	}

	// Copy rows
	for (size_t i = 0; i < h; i++) {

		if (copy_all_channels) {
			std::memcpy(dest_begin, src_begin, COMBINED_CHANNELS * w * sizeof(float));
		}
		else {
			for (size_t j = 0; j < w; j++) {
				const size_t src_offset = j * COMBINED_CHANNELS;
				const size_t dest_offset = j * COMBINED_CHANNELS;
				float* const src_pixel = src_begin + src_offset;
				float* const dest_pixel = dest_begin + dest_offset;
				if (channels.copy_r) {
					dest_pixel[0] = src_pixel[0];
				}
				if (channels.copy_g) {
					dest_pixel[1] = src_pixel[1];
				}
				if (channels.copy_b) {
					dest_pixel[2] = src_pixel[2];
				}
				if (channels.copy_a) {
					dest_pixel[3] = src_pixel[3];
				}
			}
		}
		src_begin += src_row_stride;
		dest_begin += dest_row_stride;
	}
}

static void copy_data_tile_pixels_to_accum(
	float* const src_buffer, float* const dest_buffer,
	const RenderResolutions resolutions,
	const size_t x_in, const size_t y_in,
	const size_t w, const size_t h,
	const size_t channels,
	const AccumulationBufferType buffer_type,
	const int render_index,
	const IntRect region)
{
	if (channels == 0) {
		return;
	}
	const size_t src_row_stride = channels * w;
	const size_t dest_row_stride = channels * resolutions.output_res().x();

	const size_t x = x_in + region.begin().x();
	const size_t y = y_in + resolutions.render_res().y() - region.end().y();

	float* src_begin = src_buffer;
	// Choose our dest pointer based on buffer type
	float* dest_begin = nullptr;
	if (buffer_type == AccumulationBufferType::SINGLE ||
		buffer_type == AccumulationBufferType::ANAGLYPH_RC || buffer_type == AccumulationBufferType::ANAGLYPH_GM || buffer_type == AccumulationBufferType::ANAGLYPH_YB) {
		dest_begin = dest_buffer + channels * (resolutions.output_res().x() * y + x);
	}
	else if (buffer_type == AccumulationBufferType::LEFT_RIGHT) {
		dest_begin = dest_buffer + channels * (resolutions.output_res().x() * y + x);
		if (render_index % 2 == 1) {
			dest_begin += channels * resolutions.render_res().x();
		}
	}
	else if (buffer_type == AccumulationBufferType::TOP_BOTTOM) {
		dest_begin = dest_buffer + channels * (resolutions.output_res().x() * y + x);
		if (render_index % 2 == 0) {
			dest_begin += channels * resolutions.render_res().x() * resolutions.render_res().y();
		}
	}
	else {
		// If we don't recognize the buffer type, abort here
		return;
	}

	// Copy rows
	for (size_t i = 0; i < h; i++) {
		std::memcpy(dest_begin, src_begin, channels * w * sizeof(float));
		src_begin += src_row_stride;
		dest_begin += dest_row_stride;
	}
}

static AccumulationBufferType get_accumulation_buffer_type(const CyclesRenderParams& rend_params)
{
	if (rend_params.stereo_type == StereoscopyType::ANAGLYPH) {
		if (rend_params.anaglyph_type == AnaglyphType::RED_CYAN) {
			return AccumulationBufferType::ANAGLYPH_RC;
		}
		else if (rend_params.anaglyph_type == AnaglyphType::GREEN_MAGENTA) {
			return AccumulationBufferType::ANAGLYPH_GM;
		}
		else if (rend_params.anaglyph_type == AnaglyphType::YELLOW_BLUE) {
			return AccumulationBufferType::ANAGLYPH_YB;
		}
	}

	if (rend_params.stereo_type == StereoscopyType::SPLIT_LEFT_RIGHT) {
		return AccumulationBufferType::LEFT_RIGHT;
	}

	if (rend_params.stereo_type == StereoscopyType::SPLIT_TOP_BOTTOM) {
		return AccumulationBufferType::TOP_BOTTOM;
	}

	return AccumulationBufferType::SINGLE;
}

CyclesSession::CyclesSession(
	const ccl::SessionParams& params,
	const CyclesRenderParams& rend_params,
	const RenderResolutions resolutions,
	const std::vector<RenderPassInfo>& render_pass_info_vec
) :
	ccl::Session(params),
	rend_params(rend_params),
	resolutions(resolutions),
	render_pass_info_vec(render_pass_info_vec),
	//thread_logger(global_log_manager.new_logger(L"CyclesSessionThread")),
	thread_logger(std::make_unique<LoggerInterface>()),
	logger(global_log_manager.new_logger(L"CyclesSession"))
{
	*logger << LogCtl::SEPARATOR;

	update_render_tile_cb = std::bind(&CyclesSession::update_render_tile, this, std::placeholders::_1, std::placeholders::_2);
	write_render_tile_cb = std::bind(&CyclesSession::write_render_tile, this, std::placeholders::_1);

	accumulation_buffer_type = get_accumulation_buffer_type(rend_params);
	color_assigner = std::make_unique<ColorAssigner>();

	*logger << "Constructor complete" << LogCtl::WRITE_LINE;
}

CyclesSession::~CyclesSession()
{
	*logger << "destructor called..." << LogCtl::WRITE_LINE;
	
	// This block copied from ccl::Session
	// The thread must end before accumulation_buffer is deleted
	if(session_thread) {
		// Wait for session thread to end
		progress.set_cancel("Exiting");

		gpu_need_display_buffer_update = false;
		gpu_need_display_buffer_update_cond.notify_all();

		{
			ccl::thread_scoped_lock pause_lock(pause_mutex);
			pause = false;
		}
		pause_cond.notify_all();

		wait();
	}

	*logger << "destructor complete" << LogCtl::WRITE_LINE;
}

void CyclesSession::start(const int render_index)
{
	*logger << "start called..." << LogCtl::WRITE_LINE;

	this->render_index = render_index;
	if (accumulation_buffer == nullptr) {
		init_accumulation_buffer();
	}

	// If a compositing backplate is set, force transparent sky on
	if (backplate_bitmap) {
		scene->background->set_transparent(true);
		scene->background->tag_update(scene);
	}

	ccl::Session::start();

	*logger << "start complete" << LogCtl::WRITE_LINE;
}

void CyclesSession::copy_accum_buffer(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, const bool all_passes)
{
	*logger << "copy_accum_buffer called..." << LogCtl::WRITE_LINE;

	ToneOperator* buffer_tone_operator = nullptr;
	{
		ToneOperator* const current_tone_operator = session_context.GetRenderSettings().GetToneOperator();
		if (current_tone_operator != nullptr && current_tone_operator->Active(0)) {
			buffer_tone_operator = current_tone_operator;
		}
	}
	IntRect adjusted_region = rend_params.region;
	if (accumulation_buffer_type == AccumulationBufferType::TOP_BOTTOM && render_index % 2 == 1) {
		const Int2 adjustment = Int2(0, resolutions.render_res().y());
		adjusted_region = adjusted_region.move(adjustment);
	}
	else if (accumulation_buffer_type == AccumulationBufferType::LEFT_RIGHT && render_index % 2 == 1) {
		const Int2 adjustment = Int2(resolutions.render_res().x(), 0);
		adjusted_region = adjusted_region.move(adjustment);
	}
	AccumulationBufferReader buffer_reader(accumulation_buffer.get(), buffer_tone_operator, adjusted_region);
	if (rend_params.use_transparent_sky == false) {
		// We only want to comp in a backplate if the sky is visible
		buffer_reader.set_backplate(backplate_bitmap);
	}
	session_context.GetMainFrameBufferProcessor().ProcessFrameBuffer(false, 0, buffer_reader);
	session_context.UpdateBitmapDisplay();
	if (all_passes) {
		copy_passes_from_accum();
	}

	*logger << "copy_accum_buffer complete" << LogCtl::WRITE_LINE;
}

void CyclesSession::end_session_thread()
{
	progress.set_cancel("Terminating render session...");
	{
		ccl::thread_scoped_lock pause_lock(pause_mutex);
		pause = false;
	}
	pause_cond.notify_all();
}

bool CyclesSession::is_session_running() const
{
	return session_running;
}

Int2 CyclesSession::get_render_resolution() const
{
	return resolutions.render_res();
}

void CyclesSession::set_backplate_bitmap(std::shared_ptr<BackplateBitmap> bitmap)
{
	backplate_bitmap = bitmap;
}

void CyclesSession::reset_and_cache(const ccl::BufferParams& params, int samples)
{
	cached_buffer_params = params;
	cached_sample_count = samples;

	ccl::Session::reset(cached_buffer_params, samples);
}

void CyclesSession::reset_with_cache()
{
	ccl::Session::reset(cached_buffer_params, cached_sample_count);
}

void CyclesSession::update_render_tile(ccl::RenderTile& rtile, bool)
{
	const bool highlight_tile{ rend_params.use_progressive_refine == false };
	copy_rtile_to_accum(rtile, highlight_tile);
}

void CyclesSession::write_render_tile(ccl::RenderTile& rtile)
{
	copy_rtile_to_accum(rtile, false);
}

void CyclesSession::init_accumulation_buffer()
{
	const int width{ resolutions.output_res().x() };
	const int height{ resolutions.output_res().y() };

	*logger << "Creating accum buffer: " << width << ' ' << height << LogCtl::WRITE_LINE;

	if (accumulation_buffer_type != AccumulationBufferType::NONE) {
		accumulation_buffer = std::make_unique<AccumulationBuffer>(width, height, render_pass_info_vec);
	}
}

void CyclesSession::copy_rtile_to_accum(ccl::RenderTile& rtile, const bool highlight_this_tile)
{
	// Do not use unsafe logging here, use thread_log only
	thread_log(L"copy_rtile_to_accum called");
	thread_log(rtile);

	ccl::BufferParams& params{ rtile.buffers->params };
	const float exposure{ scene->film->get_exposure() };

	thread_log(L"Getting pixels...");

	rtile.buffers->copy_from_device();

	// Cast some members of rtile to size_t to avoid accidentally working with ints later
	const size_t tile_width = rtile.w;
	const size_t tile_height = rtile.h;

	// Create temporary storage
	constexpr size_t MAX_CHANNELS = 4;
	std::vector<float> pixels(MAX_CHANNELS * params.width * params.height);

	// Loop through all available passes and copy from tile to accumulation buffer
	for (const RenderPassInfo& this_pass_info : render_pass_info_vec) {
		int sample = rtile.sample;
		const int range_start_sample = tile_manager.range_start_sample;
		if (range_start_sample != -1) {
			sample -= range_start_sample;
		}

		thread_log(L"Loading temp buffer");

		rtile.buffers->get_pass_rect(this_pass_info.name, exposure, sample, this_pass_info.channels, &pixels[0]);

		if (highlight_this_tile && this_pass_info.type == RenderPassType::COMBINED) {
			thread_log(L"Highlighting");
			const ccl::float3 border_color = color_assigner->get_color(rtile.x, rtile.y);

			// Horizontal lines
			{
				const size_t last_row_offset = tile_width * (tile_height - 1) * this_pass_info.channels;
				for (size_t i = 0; i < tile_width; i++) {
					const size_t first_row_index = i * this_pass_info.channels;
					// Top row
					pixels[first_row_index] = border_color.x;
					pixels[first_row_index + 1] = border_color.y;
					pixels[first_row_index + 2] = border_color.z;
					pixels[first_row_index + 3] = 1.0f;
					// Bottom row
					pixels[last_row_offset + first_row_index] = border_color.x;
					pixels[last_row_offset + first_row_index + 1] = border_color.y;
					pixels[last_row_offset + first_row_index + 2] = border_color.z;
					pixels[last_row_offset + first_row_index + 3] = 1.0f;
				}
			}

			// Vertical lines
			{
				const size_t last_col_offset = (tile_width - 1) * this_pass_info.channels;
				for (size_t i = 0; i < tile_height; i++) {
					const size_t first_col_index = i * tile_width * this_pass_info.channels;
					// Left column
					pixels[first_col_index] = border_color.x;
					pixels[first_col_index + 1] = border_color.y;
					pixels[first_col_index + 2] = border_color.z;
					pixels[first_col_index + 3] = 1.0f;
					// Right column
					pixels[last_col_offset + first_col_index] = border_color.x;
					pixels[last_col_offset + first_col_index + 1] = border_color.y;
					pixels[last_col_offset + first_col_index + 2] = border_color.z;
					pixels[last_col_offset + first_col_index + 3] = 1.0f;
				}
			}
		}

		if (this_pass_info.type == RenderPassType::COMBINED) {
			thread_log(L"copying as combined");
			copy_combined_tile_pixels_to_accum(
				pixels.data(), accumulation_buffer->get_pass_buffer("Combined"),
				resolutions,
				rtile.x, rtile.y,
				tile_width, tile_height,
				accumulation_buffer_type,
				render_index,
				rend_params.region
			);
			thread_log(L"done");
		}
		else {
			thread_log(L"copying as other");
			copy_data_tile_pixels_to_accum(
				pixels.data(), accumulation_buffer->get_pass_buffer(this_pass_info.name),
				resolutions,
				rtile.x, rtile.y,
				tile_width, tile_height,
				static_cast<size_t>(this_pass_info.channels),
				accumulation_buffer_type,
				render_index,
				rend_params.region
			);
			thread_log(L"done");
		}
	}

	thread_log(L"copy_rtile_to_accum complete");
}


void CyclesSession::copy_passes_from_accum()
{
	for (const RenderPassInfo& this_pass_info : render_pass_info_vec) {
		if (this_pass_info.type == RenderPassType::COMBINED || this_pass_info.render_element == nullptr) {
			continue;
		}

		PBBitmap* dest_pb_bitmap = nullptr;
		this_pass_info.render_element->GetPBBitmap(dest_pb_bitmap);
		if (dest_pb_bitmap == nullptr) {
			continue;
		}
		Bitmap* dest_bitmap = dest_pb_bitmap->bm;
		if (dest_bitmap == nullptr) {
			continue;
		}

		float* const source_buffer = accumulation_buffer->get_pass_buffer(this_pass_info.name);

		BMM_Color_fl* const temp_line_buffer = new BMM_Color_fl[resolutions.output_res().x()];

		// Iterate through and copy each row to temp buffer, then to re bitmap
		const IntRect region = rend_params.region;
		for (size_t i = 0; i < resolutions.output_res().y(); i++) {
			const int output_y = (resolutions.output_res().y() - 1) - static_cast<int>(i);
			if (output_y < region.begin().y() || output_y >= region.end().y()) {
				// Skip lines not present in this region
				continue;
			}

			const size_t source_row_offset = i * resolutions.output_res().x() * this_pass_info.channels;
			float* const source_row = source_buffer + source_row_offset;
			// Iterate through individual pixels
			for (size_t j = 0; j < resolutions.output_res().x(); j++) {
				if (this_pass_info.channels == 1) {
					temp_line_buffer[j].r = source_row[j];
					temp_line_buffer[j].g = source_row[j];
					temp_line_buffer[j].b = source_row[j];
					temp_line_buffer[j].a = 1.0f;
				}
				else if (this_pass_info.channels == 4) {
					temp_line_buffer[j].r = source_row[j * this_pass_info.channels];
					temp_line_buffer[j].g = source_row[j * this_pass_info.channels + 1];
					temp_line_buffer[j].b = source_row[j * this_pass_info.channels + 2];
					temp_line_buffer[j].a = source_row[j * this_pass_info.channels + 3];
				}
			}
			dest_bitmap->PutPixels(region.begin().x(), output_y, region.size().x(), temp_line_buffer + region.begin().x());
		}
		delete[] temp_line_buffer;
	}
}

void CyclesSession::thread_log(const std::wstring& message)
{
	if (thread_logger->enabled()) {
		std::lock_guard<std::mutex> lock(thread_logger_mutex);
		*thread_logger << message.c_str() << LogCtl::WRITE_LINE;
	}
}

void CyclesSession::thread_log(const ccl::RenderTile& rtile)
{
	if (thread_logger->enabled()) {
		std::lock_guard<std::mutex> lock(thread_logger_mutex);
		*thread_logger << "rtile--" << LogCtl::WRITE_LINE;
		*thread_logger << "x, y: " << rtile.x << ", " << rtile.y << LogCtl::WRITE_LINE;
		*thread_logger << "w, h: " << rtile.w << ", " << rtile.h << LogCtl::WRITE_LINE;
	}
}
