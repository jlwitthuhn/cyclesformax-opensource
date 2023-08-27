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
 
#include "trans_mtl_list.h"

#include <inode.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/ISceneContainer.h>

#include "trans_mtl_node.h"

using MaxSDK::RenderingAPI::ISceneContainer;
using MaxSDK::RenderingAPI::ITranslationProgress;
using MaxSDK::RenderingAPI::TranslationResult;

CyclesMaterialListTranslator::CyclesMaterialListTranslator(const TranslatorKey& /*key*/, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) : MaxSDK::RenderingAPI::Translator(translator_graph_node)
{
	GetRenderSessionContext().GetScene().RegisterChangeNotifier(*this);
}

CyclesMaterialListTranslator::~CyclesMaterialListTranslator()
{
	GetRenderSessionContext().GetScene().UnregisterChangeNotifier(*this);
}

bool CyclesMaterialListTranslator::GetMaterialProperties(MaterialProperties& mtl_properties) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	mtl_properties = GetOutput_SimpleValue<MaterialProperties>(0, mtl_properties);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesMaterialListTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesMaterialListTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress)
#else
static_assert(false);
#endif
{
	const ISceneContainer& scene_container = GetRenderSessionContext().GetScene();
	const std::vector<INode*> geom_nodes = scene_container.GetGeometricNodes(translation_time, new_validity);

	MaterialProperties result;
	result.last_update_time = std::chrono::steady_clock::now();

	new_validity = FOREVER;
	for (INode* const node : geom_nodes) {
		Mtl* const mtl = node->GetMtl();
		if (mtl == nullptr) {
			continue;
		}

		TranslationResult child_trans_result;
		const CyclesMtlNodeTranslator* const mtl_node_translator = AcquireChildTranslator<CyclesMtlNodeTranslator>(CyclesMtlNodeTranslator::TranslatorKey(mtl), translation_time, translation_progress, child_trans_result);

		if (mtl_node_translator == nullptr) {
			continue;
		}

		NodeMaterialProperties mtl_properties;
		bool success = mtl_node_translator->GetNodeMaterialProperties(mtl_properties);

		if (success) {
			if (mtl_properties.last_update_time > result.last_update_time) {
				result.last_update_time = mtl_properties.last_update_time;
			}

			new_validity &= mtl_properties.validity;
		}
	}

	SetOutput_SimpleValue<MaterialProperties>(0, result);

	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesMaterialListTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesMaterialListTranslator::PreTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesMaterialListTranslator::PostTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesMaterialListTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesMaterialListTranslator::CheckValidity(const TimeValue /*t*/, const Interval& previous_validity) const
{
	return previous_validity;
}

MSTR CyclesMaterialListTranslator::GetTimingCategory() const
{
	return L"MaterialList";
}

void CyclesMaterialListTranslator::NotifySceneNodesChanged()
{
	Invalidate();
}

void CyclesMaterialListTranslator::NotifySceneBoundingBoxChanged()
{
	// Do nothing
}
