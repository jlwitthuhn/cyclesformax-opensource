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
 
#include "trans_texmap_list.h"

#include <inode.h>
#include <Materials/Mtl.h>
#include <Materials/Texmap.h>
#include <RenderingAPI/Renderer/IEnvironmentContainer.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/ISceneContainer.h>

#include <set>

#include "trans_texmap_single.h"

using MaxSDK::RenderingAPI::ISceneContainer;
using MaxSDK::RenderingAPI::ITranslationProgress;
using MaxSDK::RenderingAPI::TranslationResult;
using MaxSDK::RenderingAPI::Translator;
using MaxSDK::RenderingAPI::TranslatorGraphNode;
using MaxSDK::RenderingAPI::TranslatorStatistics;

static void add_sub_texmaps_to_set(Texmap* texmap, std::set<Texmap*>& texmap_set)
{
	if (texmap == nullptr) {
		return;
	}

	texmap_set.insert(texmap);

	for (int i = 0; i < texmap->NumSubTexmaps(); ++i) {
		Texmap* this_texmap = texmap->GetSubTexmap(i);

		if (this_texmap == nullptr) {
			continue;
		}

		texmap_set.insert(this_texmap);
		add_sub_texmaps_to_set(this_texmap, texmap_set);
	}
}

static void add_texmaps_to_set(Mtl* mtl, std::set<Texmap*>& texmap_set)
{
	if (mtl == nullptr) {
		return;
	}

	for (int i = 0; i < mtl->NumSubTexmaps(); ++i) {
		Texmap* this_texmap = mtl->GetSubTexmap(i);

		if (this_texmap == nullptr) {
			continue;
		}

		add_sub_texmaps_to_set(this_texmap, texmap_set);
	}
}

CyclesTexmapListTranslator::CyclesTexmapListTranslator(const TranslatorKey& key, TranslatorGraphNode& translator_graph_node) : Translator(translator_graph_node)
{
	UNREFERENCED_PARAMETER(key);
}

CyclesTexmapListTranslator::~CyclesTexmapListTranslator()
{

}

bool CyclesTexmapListTranslator::GetTexmapUpdateTimes(TexmapUpdateTimes& tex_properties) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	tex_properties = GetOutput_SimpleValue<TexmapUpdateTimes>(0, tex_properties);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesTexmapListTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesTexmapListTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress)
#else
static_assert(false);
#endif
{
	std::set<Texmap*> all_texmaps;

	const ISceneContainer& scene_container = GetRenderSessionContext().GetScene();
	const std::vector<INode*> geom_nodes = scene_container.GetGeometricNodes(translation_time, new_validity);

	for (INode* const node : geom_nodes) {
		Mtl* const mtl = node->GetMtl();

		if (mtl == nullptr) {
			continue;
		}

		add_texmaps_to_set(mtl, all_texmaps);
	}

	if (GetRenderSessionContext().GetEnvironment()->GetEnvironmentTexture() != nullptr) {
		add_sub_texmaps_to_set(GetRenderSessionContext().GetEnvironment()->GetEnvironmentTexture(), all_texmaps);
	}

	TexmapUpdateTimes result;

	for (Texmap* const texmap : all_texmaps) {
		TranslationResult child_trans_result;
		const CyclesTexmapTranslator* const texmap_translator = AcquireChildTranslator<CyclesTexmapTranslator>(CyclesTexmapTranslator::TranslatorKey(texmap), translation_time, translation_progress, child_trans_result);

		if (texmap_translator == nullptr) {
			continue;
		}

		TexmapProperties texmap_props;
		texmap_translator->GetTexmapProperties(texmap_props);

		result.update_times[texmap] = texmap_props.last_update_time;
	}

	SetOutput_SimpleValue<TexmapUpdateTimes>(0, result);

	return TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesTexmapListTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesTexmapListTranslator::PreTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesTexmapListTranslator::PostTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesTexmapListTranslator::AccumulateStatistics(TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesTexmapListTranslator::CheckValidity(const TimeValue /*t*/, const Interval& previous_validity) const
{
	return previous_validity;
}

MSTR CyclesTexmapListTranslator::GetTimingCategory() const
{
	return L"TexmapList";
}
