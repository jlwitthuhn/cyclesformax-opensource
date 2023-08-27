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
 
#include "trans_light_node.h"

#include <inode.h>
#include <object.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "util_translate_light.h"

using MaxSDK::RenderingAPI::IRenderSessionContext;
using MaxSDK::RenderingAPI::TranslationResult;
using MaxSDK::RenderingAPI::Translator;
using MaxSDK::RenderingAPI::TranslatorGraphNode;

CyclesLightNodeTranslator::CyclesLightNodeTranslator(const TranslatorKey& key, TranslatorGraphNode& translator_graph_node) :
	BaseTranslator_ReferenceTarget(key.get_node(), true, translator_graph_node),
	Translator(translator_graph_node)
{
	node_ptr = key.get_node();
}

CyclesLightNodeTranslator::~CyclesLightNodeTranslator()
{

}

bool CyclesLightNodeTranslator::GetLightParams(CyclesLightParams& light_params) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	light_params = GetOutput_SimpleValue<CyclesLightParams>(0, light_params);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesLightNodeTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesLightNodeTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
#else
static_assert(false);
#endif
{
	GetRenderSessionContext().CallRenderBegin(*node_ptr, translation_time);

	ObjectState os = node_ptr->EvalWorldState(translation_time);

	if (os.obj == nullptr || os.obj->SuperClassID() != LIGHT_CLASS_ID) {
		ResetAllOutputs();
		return TranslationResult::Success;
	}

	const CyclesLightParams result = get_light_params(node_ptr, os.obj, translation_time);

	SetOutput_SimpleValue<CyclesLightParams>(0, result);
	new_validity = os.obj->ObjectValidity(translation_time);

	return TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesLightNodeTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesLightNodeTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesLightNodeTranslator::CheckValidity(const TimeValue /*t*/, const Interval& /*previous_validity*/) const
{
	return NEVER;
}

MSTR CyclesLightNodeTranslator::GetTimingCategory() const
{
	return L"LightNode";
}

CyclesLightNodeTranslator::TranslatorKey::TranslatorKey(INode* const node) :
	MaxSDK::RenderingAPI::TranslatorKey(compute_hash(node))
{
	node_ptr = node;
}

INode* CyclesLightNodeTranslator::TranslatorKey::get_node() const
{
	return node_ptr;
}

bool CyclesLightNodeTranslator::TranslatorKey::operator==(const MaxSDK::RenderingAPI::TranslatorKey& rhs) const
{
	const TranslatorKey* cast_key = dynamic_cast<const TranslatorKey*>(&rhs);

	if (cast_key == nullptr) {
		return false;
	}

	return (node_ptr == cast_key->node_ptr);
}

std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey> CyclesLightNodeTranslator::TranslatorKey::CreateClone() const
{
	return std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey>(new TranslatorKey(*this));
}

std::unique_ptr<Translator> CyclesLightNodeTranslator::TranslatorKey::AllocateNewTranslator(TranslatorGraphNode& translator_graph_node, const IRenderSessionContext& /*render_session_context*/, const TimeValue /*initial_time*/) const
{
	return std::unique_ptr<Translator>(new CyclesLightNodeTranslator(*this, translator_graph_node));
}

size_t CyclesLightNodeTranslator::TranslatorKey::compute_hash(INode* node)
{
	return std::hash<INode*>()(node);
}
