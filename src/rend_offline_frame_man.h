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
 * @brief Defines renderer class OfflineFrameManager.
 */

#include <atomic>
#include <memory>

#include "cycles_session.h"
#include "rend_logger.h"
#include "rend_offline_translation_man.h"

class BakedTexmapCache;

/**
 * @brief This class is responsible for encapsulating all data needed to render a frame, including a CyclesSession.
 */
class OfflineFrameManager {
public:
	
	OfflineFrameManager(
		MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
		BakedTexmapCache& texmap_cache,
		CyclesRenderParams& rend_params
	);

	void translate();

	void run_frame();
	void end_render();

private:
	const std::unique_ptr<OfflineTranslationManager> translation_manager;

	BakedTexmapCache& texmap_cache;

	MaxSDK::RenderingAPI::IRenderSessionContext& session_context;
	CyclesRenderParams& rend_params;

	std::unique_ptr<CyclesSession> session;

	std::atomic<bool> stop_requested{ false };

	bool frame_errored{ false };
	bool frame_was_cancelled{ false };

	void run_frame_internal();

	void wait_for_session_end();

	void render_status_loop();

	bool setup_camera();

	void setup_stereo_camera(int render_pass_index);

	const std::unique_ptr<LoggerInterface> logger;
};
