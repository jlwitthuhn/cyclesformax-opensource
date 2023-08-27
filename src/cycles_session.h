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
 * @brief Defines CyclesSession and supporting classes. 
 */

#include <memory>
#include <mutex>
#include <vector>

#include <render/session.h>

#include "rend_logger.h"
#include "util_enums.h"
#include "util_pass.h"
#include "util_resolution.h"
#include "util_simple_types.h"

namespace MaxSDK {
	namespace RenderingAPI {
		class IRenderSessionContext;
	}
}

class ColorAssigner;
class CyclesRenderParams;
class AccumulationBuffer;

/**
 * @brief Class that extends ccl::Session with some extra helper functions.
 */
class CyclesSession : public ccl::Session {
public:
	CyclesSession(const ccl::SessionParams& params, const CyclesRenderParams& rend_params, RenderResolutions resolutions, const std::vector<RenderPassInfo>& render_pass_info_vec);
	virtual ~CyclesSession();

	virtual void start(int render_index);

	void copy_accum_buffer(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, bool all_passes);

	void end_session_thread();
	bool is_session_running() const;

	Int2 get_render_resolution() const;

	void set_backplate_bitmap(std::shared_ptr<BackplateBitmap> bitmap);

	// These two are used in stereoscopic renders so everything can easily be reset
	// They are not called for activeshade renders
	void reset_and_cache(const ccl::BufferParams& params, int samples);
	void reset_with_cache();

	void update_render_tile(ccl::RenderTile& rtile, bool highlight);
	void write_render_tile(ccl::RenderTile& rtile);

private:
	const CyclesRenderParams& rend_params;
	const RenderResolutions resolutions;

	const std::vector<RenderPassInfo> render_pass_info_vec;

	std::shared_ptr<BackplateBitmap> backplate_bitmap;

	AccumulationBufferType accumulation_buffer_type = AccumulationBufferType::NONE;
	std::unique_ptr<AccumulationBuffer> accumulation_buffer;

	std::unique_ptr<ColorAssigner> color_assigner;

	int render_index = 0;

	ccl::BufferParams cached_buffer_params;
	int cached_sample_count = 1;

	std::mutex thread_logger_mutex;
	const std::unique_ptr<LoggerInterface> thread_logger;

	const std::unique_ptr<LoggerInterface> logger;

	void init_accumulation_buffer();

	void copy_rtile_to_accum(ccl::RenderTile& rtile, bool highlight_this_tile);
	void copy_passes_from_accum();

	void thread_log(const std::wstring& message);
	void thread_log(const ccl::RenderTile& rtile);
};
