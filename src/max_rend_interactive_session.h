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
 * @brief Defines CyclesInteractiveRenderSession to enable ActiveShade renders.
 */

#include <chrono>
#include <memory>

#include <RenderingAPI/Renderer/IInteractiveRenderSession.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "rend_logger.h"
#include "rend_params.h"
#include "trans_output.h"
#include "util_resolution.h"

class BakedTexmapCache;
class CyclesSceneTranslator;
class CyclesSession;
class MaxShaderManager;

/**
 * @brief Enum to track what state the CyclesInteractiveRenderSession is in.
 */
enum class SessionState {
	READY_TO_TRANSLATE,
	RETRANSLATE,
	PREPARE_FOR_BUILD,
	BUILD_STAGE_1,
	BUILD_STAGE_2,
	BUILD_STAGE_3,
	READY_TO_BAKE,
	TEX_BAKING,
	RENDERING,
	ABORTING,
	COMPLETE
};

/**
 * @brief Implementation of IInteractiveRenderSession, which is used by Max to perform ActiveShade renders.
 *
 * This class is implemented as a state machine that performs some task and then transitions to the next state when
 * UpdateInteractiveSession is called by Max.
 */
class CyclesInteractiveRenderSession : public MaxSDK::RenderingAPI::IInteractiveRenderSession {
public:
	CyclesInteractiveRenderSession(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, const CyclesRenderParams& rend_params);
	~CyclesInteractiveRenderSession();

	virtual bool InitiateInteractiveSession(const TimeValue t) override;
	virtual bool UpdateInteractiveSession(const TimeValue t, bool &doneRendering) override;
	virtual void TerminateInteractiveSession() override;
	virtual bool IsInteractiveSessionUpToDate(const TimeValue t) override;

private:
	MaxSDK::RenderingAPI::IRenderSessionContext& session_context;
	CyclesRenderParams rend_params;
	CyclesSceneTranslator* scene_translator = nullptr;
	RenderResolutions resolutions;

	std::unique_ptr<BakedTexmapCache> texmap_cache;
	std::unique_ptr<MaxShaderManager> shader_manager;

	std::unique_ptr<CyclesSession> cycles_session;

	SessionState state = SessionState::READY_TO_TRANSLATE;

	std::chrono::steady_clock::time_point last_fb_update;
	bool first_fb_update_done = false;

	bool BuildCyclesSession(CyclesSceneDescriptor& scene_desc);
	void BuildGeometry(CyclesSceneGeometryList& geometry_list);
	void DestroyCyclesSession();

	bool TranslateOrUpdateScene(const TimeValue t, bool& scene_updated);

	const std::unique_ptr<LoggerInterface> logger;
};
