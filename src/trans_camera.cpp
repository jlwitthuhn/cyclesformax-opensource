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
 
#include "trans_camera.h"

#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "util_translate_camera.h"

CyclesCameraTranslator::CyclesCameraTranslator(const TranslatorKey& /*key*/, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	MaxSDK::RenderingAPI::Translator(translator_graph_node),
	logger(global_log_manager.new_logger(L"CyclesCameraTranslator", false, true))
{
	*logger << LogCtl::SEPARATOR;

	GetRenderSessionContext().GetCamera().RegisterChangeNotifier(*this);
}

CyclesCameraTranslator::~CyclesCameraTranslator()
{
	GetRenderSessionContext().GetCamera().UnregisterChangeNotifier(*this);
}

bool CyclesCameraTranslator::GetCameraParams(CyclesCameraParams& camera_params) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	camera_params = GetOutput_SimpleValue<CyclesCameraParams>(0, camera_params);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesCameraTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesCameraTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
#else
static_assert(false);
#endif
{
	*logger << "Translate called..." << LogCtl::WRITE_LINE;

	MaxSDK::RenderingAPI::IRenderSessionContext& session_context = GetRenderSessionContext();

	const boost::optional<CyclesCameraParams> camera_params = get_camera_params(session_context, translation_time, StereoscopyType::NONE);

	if (camera_params.has_value() == false) {
		return MaxSDK::RenderingAPI::TranslationResult::Failure;
	}

	SetOutput_SimpleValue<CyclesCameraParams>(0, *camera_params);

	new_validity = Interval(translation_time, translation_time);

	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesCameraTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesCameraTranslator::PreTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesCameraTranslator::PostTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesCameraTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesCameraTranslator::CheckValidity(const TimeValue /*t*/, const Interval& previous_validity) const
{
	return previous_validity;
}

MSTR CyclesCameraTranslator::GetTimingCategory() const
{
	return L"Camera";
}

void CyclesCameraTranslator::NotifyCameraChanged()
{
	*logger << "NotifyCameraChanged called..." << LogCtl::WRITE_LINE;
	Invalidate();
}
