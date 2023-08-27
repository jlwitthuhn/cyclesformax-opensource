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
 
#include "max_rend_offline_session.h"

#include <Rendering/Renderer.h>
#include <RenderingAPI/Renderer/IRenderSettingsContainer.h>
#include <RenderingAPI/Renderer/IRenderingLogger.h>

#include "cache_baked_texmap.h"
#include "rend_offline_frame_man.h"
#include "util_cycles_device_dump.h"

using MaxSDK::RenderingAPI::IRenderSessionContext;

CyclesOfflineRenderSession::CyclesOfflineRenderSession(IRenderSessionContext& session_context, const CyclesRenderParams& rend_params) :
	texmap_cache{ std::make_unique<BakedTexmapCache>(session_context, rend_params.texmap_bake_width, rend_params.texmap_bake_height) },
	session_context{ session_context },
	rend_params{ rend_params },
	logger{ global_log_manager.new_logger(L"CyclesOfflineRenderSession") }
{
	*logger << LogCtl::SEPARATOR;
}

CyclesOfflineRenderSession::~CyclesOfflineRenderSession()
{
	*logger << "Destructor called" << LogCtl::WRITE_LINE;
}

bool CyclesOfflineRenderSession::TranslateScene(const TimeValue t)
{
	*logger << "TranslateScene called..." << LogCtl::WRITE_LINE;

	if (rend_params.diagnostic_log) {
		MaxSDK::RenderingAPI::IRenderingLogger& logger = session_context.GetLogger();
		logger.LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, L"Logging diagnostic information begin...");
		dump_cycles_devices(&logger);
		logger.LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Warning, L"Diagnostic logging complete");
	}

	frame_manager = std::unique_ptr<OfflineFrameManager>{};

	rend_params.in_mtl_edit = session_context.GetRenderSettings().GetIsMEditRender();
	rend_params.std_render_hidden = session_context.GetRenderSettings().GetRenderHiddenObjects();

	rend_params.frame_t = t;

	rend_params.SetMtlEditParams();

	*logger << "Translating frame..." << LogCtl::WRITE_LINE;

	texmap_cache->new_frame(rend_params.frame_t);
	frame_manager = std::make_unique<OfflineFrameManager>(session_context, *texmap_cache, rend_params);
	frame_manager->translate();

	texmap_cache->bake_all_texmaps();

	session_context.CallRenderEnd(rend_params.frame_t);

	*logger << "TranslateScene complete" << LogCtl::WRITE_LINE;

	return true;
}

bool CyclesOfflineRenderSession::RenderOfflineFrame(const TimeValue)
{
	// We ignore the input TimeValue here, only the value called with TranslateScene matters

	*logger << "RenderOfflineFrame called..." << LogCtl::WRITE_LINE;

	if (frame_manager) {
		frame_manager->run_frame();
	}
	else {
		*logger << "frame_manager was not set, doing nothing..." << LogCtl::WRITE_LINE;
	}

	*logger << "RenderOfflineFrame complete" << LogCtl::WRITE_LINE;

	return true;
}

bool CyclesOfflineRenderSession::StoreRenderElementResult(
	const TimeValue /*t*/,
	IRenderElement& /*render_element*/,
	MaxSDK::RenderingAPI::IFrameBufferProcessor& /*frame_buffer_processor*/
	)
{
	return true;
}

void CyclesOfflineRenderSession::StopRendering()
{
	*logger << "StopRendering called..." << LogCtl::WRITE_LINE;

	if (frame_manager) {
		frame_manager->end_render();
	}
}

void CyclesOfflineRenderSession::PauseRendering()
{

}

void CyclesOfflineRenderSession::ResumeRendering()
{

}
