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
 * @brief Defines CyclesOfflineRenderSession to enable offline renders.
 */

#include <memory>

#include <RenderingAPI/Renderer/IOfflineRenderSession.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "rend_logger.h"
#include "rend_params.h"

class BakedTexmapCache;
class OfflineFrameManager;

 /**
 * @brief Implementation of IOfflineRenderSession, which is used by Max to perform offline renders.
 */

class CyclesOfflineRenderSession : public MaxSDK::RenderingAPI::IOfflineRenderSession {
public:
	CyclesOfflineRenderSession(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, const CyclesRenderParams& rend_params);
	virtual ~CyclesOfflineRenderSession() override;

	// Functions called by 3ds Max
	virtual bool TranslateScene(const TimeValue t) override;
	virtual bool RenderOfflineFrame(const TimeValue t) override;
	virtual bool StoreRenderElementResult(const TimeValue t, IRenderElement& render_element, MaxSDK::RenderingAPI::IFrameBufferProcessor& frame_buffer_processor) override;
	virtual void StopRendering() override;
	virtual void PauseRendering() override;
	virtual void ResumeRendering() override;

private:
	const std::unique_ptr<BakedTexmapCache> texmap_cache;

	MaxSDK::RenderingAPI::IRenderSessionContext& session_context;
	CyclesRenderParams rend_params;

	std::unique_ptr<OfflineFrameManager> frame_manager;

	const std::unique_ptr<LoggerInterface> logger;
};
