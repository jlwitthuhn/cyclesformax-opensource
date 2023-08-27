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
 
#include "trans_light_list.h"

#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/ISceneContainer.h>

#include "trans_light_node.h"

using MaxSDK::RenderingAPI::ISceneContainer;
using MaxSDK::RenderingAPI::TranslationResult;

CyclesLightListTranslator::CyclesLightListTranslator(const TranslatorKey& /*key*/, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node)
	: MaxSDK::RenderingAPI::Translator(translator_graph_node)
{
	GetRenderSessionContext().GetScene().RegisterChangeNotifier(*this);
}

CyclesLightListTranslator::~CyclesLightListTranslator()
{
	GetRenderSessionContext().GetScene().UnregisterChangeNotifier(*this);
}

bool CyclesLightListTranslator::GetSceneLights(CyclesSceneLightList& scene_lights) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	scene_lights = GetOutput_SimpleValue<CyclesSceneLightList>(0, scene_lights);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesLightListTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesLightListTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress)
#else
static_assert(false);
#endif
{
	const ISceneContainer& scene_container = GetRenderSessionContext().GetScene();
	const std::vector<INode*> light_nodes = scene_container.GetLightNodes(translation_time, new_validity);

	CyclesSceneLightList result;
	for (INode* const node : light_nodes) {
		if (node == nullptr) {
			continue;
		}

		TranslationResult child_trans_result;
		const CyclesLightNodeTranslator* light_node_translator = AcquireChildTranslator<CyclesLightNodeTranslator>(CyclesLightNodeTranslator::TranslatorKey(node), translation_time, translation_progress, child_trans_result);

		if (light_node_translator == nullptr) {
			continue;
		}

		CyclesLightParams this_light_params;
		bool success = light_node_translator->GetLightParams(this_light_params);

		if (success) {
			result.lights.push_back(this_light_params);
		}
	}

	SetOutput_SimpleValue<CyclesSceneLightList>(0, result);

	new_validity = FOREVER;
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesLightListTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesLightListTranslator::PreTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesLightListTranslator::PostTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesLightListTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesLightListTranslator::CheckValidity(const TimeValue /*t*/, const Interval& previous_validity) const
{
	return previous_validity;
}

MSTR CyclesLightListTranslator::GetTimingCategory() const
{
	return L"LightList";
}

void CyclesLightListTranslator::NotifySceneNodesChanged()
{
	Invalidate();
}

void CyclesLightListTranslator::NotifySceneBoundingBoxChanged()
{
	// Do nothing
}
