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
 
#include "trans_geom_list.h"

#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/ISceneContainer.h>

#include "trans_geom_node.h"

using MaxSDK::RenderingAPI::ISceneContainer;
using MaxSDK::RenderingAPI::ITranslationProgress;
using MaxSDK::RenderingAPI::TranslationResult;

CyclesGeometryListTranslator::CyclesGeometryListTranslator(const TranslatorKey& /*key*/, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	MaxSDK::RenderingAPI::Translator(translator_graph_node),
	logger(global_log_manager.new_logger(L"CyclesGeomListTranslator", false, true))
{
	*logger << LogCtl::SEPARATOR;
	GetRenderSessionContext().GetScene().RegisterChangeNotifier(*this);
}

CyclesGeometryListTranslator::~CyclesGeometryListTranslator()
{
	GetRenderSessionContext().GetScene().UnregisterChangeNotifier(*this);
}

bool CyclesGeometryListTranslator::GetSceneGeometry(CyclesSceneGeometryList& scene_geometry) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	scene_geometry = GetOutput_SimpleValue<CyclesSceneGeometryList>(0, scene_geometry);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesGeometryListTranslator::Translate(const TimeValue translation_time, Interval &new_validity, MaxSDK::RenderingAPI::ITranslationProgress &translation_progress, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesGeometryListTranslator::Translate(const TimeValue translation_time, Interval &new_validity, MaxSDK::RenderingAPI::ITranslationProgress &translation_progress)
#else
static_assert(false);
#endif
{
	*logger << "Translate called..." << LogCtl::WRITE_LINE;

	const ISceneContainer& scene_container = GetRenderSessionContext().GetScene();
	const std::vector<INode*> geom_nodes = scene_container.GetGeometricNodes(translation_time, new_validity);

	CyclesSceneGeometryList result;

	*logger << "Beginning to iterate through nodes..." << LogCtl::WRITE_LINE;

	for (INode* const node : geom_nodes) {
		if (node == nullptr) {
			continue;
		}

		TranslationResult child_trans_result;
		const CyclesGeomNodeTranslator* geom_node_translator =
			AcquireChildTranslator<CyclesGeomNodeTranslator>(
				CyclesGeomNodeTranslator::TranslatorKey(*node),
				translation_time, translation_progress,
				child_trans_result
				);
	
		if (geom_node_translator == nullptr) {
			continue;
		}

		CyclesGeomObject this_geom_object;
		bool success = geom_node_translator->GetGeomObject(this_geom_object);

		if (success) {
			result.geom_objects.push_back(this_geom_object);
		}
	}

	*logger << "All nodes complete" << LogCtl::WRITE_LINE;

	SetOutput_SimpleValue<CyclesSceneGeometryList>(0, result);

	new_validity = FOREVER;
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesGeometryListTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesGeometryListTranslator::PreTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesGeometryListTranslator::PostTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesGeometryListTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesGeometryListTranslator::CheckValidity(const TimeValue /*t*/, const Interval& previous_validity) const
{
	return previous_validity;
}

MSTR CyclesGeometryListTranslator::GetTimingCategory() const
{
	return L"GeometryList";
}

void CyclesGeometryListTranslator::NotifySceneNodesChanged()
{
	Invalidate();
}

void CyclesGeometryListTranslator::NotifySceneBoundingBoxChanged()
{
	// Do nothing
}
