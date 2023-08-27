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
 
#include "trans_mtl_node.h"

#include <Materials/Mtl.h>

using MaxSDK::RenderingAPI::IGenericEvent;
using MaxSDK::RenderingAPI::IRenderSessionContext;
using MaxSDK::RenderingAPI::TranslationResult;
using MaxSDK::RenderingAPI::Translator;
using MaxSDK::RenderingAPI::TranslatorGraphNode;

NodeMaterialProperties::NodeMaterialProperties()
{
	last_update_time = std::chrono::steady_clock::now();
}

bool NodeMaterialProperties::operator==(const NodeMaterialProperties& other) const
{
	return last_update_time == other.last_update_time && validity == other.validity;
}

bool NodeMaterialProperties::operator!=(const NodeMaterialProperties& other) const
{
	return !operator==(other);
}

CyclesMtlNodeTranslator::CyclesMtlNodeTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	BaseTranslator_ReferenceTarget(key.get_mtl(), true, translator_graph_node),
	Translator(translator_graph_node)
{
	mtl_ptr = key.get_mtl();
	last_update_time = std::chrono::steady_clock::now();
}

CyclesMtlNodeTranslator::~CyclesMtlNodeTranslator()
{

}

bool CyclesMtlNodeTranslator::GetNodeMaterialProperties(NodeMaterialProperties& mtl_properties) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	mtl_properties = GetOutput_SimpleValue<NodeMaterialProperties>(0, mtl_properties);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesMtlNodeTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesMtlNodeTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
#else
static_assert(false);
#endif
{
	NodeMaterialProperties result;
	result.last_update_time = last_update_time;

	new_validity = mtl_ptr->Validity(translation_time);
	result.validity = new_validity;

	SetOutput_SimpleValue<NodeMaterialProperties>(0, result);

	return TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesMtlNodeTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesMtlNodeTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesMtlNodeTranslator::CheckValidity(const TimeValue /*t*/, const Interval& /*previous_validity*/) const
{
	return NEVER;
}

MSTR CyclesMtlNodeTranslator::GetTimingCategory() const
{
	return L"MtlNode";
}

void CyclesMtlNodeTranslator::NotificationCallback_NotifyEvent(const IGenericEvent& genericEvent, void* const userData)
{
	MaxSDK::RenderingAPI::BaseTranslator_ReferenceTarget::NotificationCallback_NotifyEvent(genericEvent, userData);
	last_update_time = std::chrono::steady_clock::now();
}

CyclesMtlNodeTranslator::TranslatorKey::TranslatorKey(Mtl* const mtl) : MaxSDK::RenderingAPI::TranslatorKey(compute_hash(mtl))
{
	mtl_ptr = mtl;
}

Mtl* CyclesMtlNodeTranslator::TranslatorKey::get_mtl() const
{
	return mtl_ptr;
}

bool CyclesMtlNodeTranslator::TranslatorKey::operator==(const MaxSDK::RenderingAPI::TranslatorKey& rhs) const
{
	const TranslatorKey* const cast_key = dynamic_cast<const TranslatorKey*>(&rhs);

	if (cast_key == nullptr) {
		return false;
	}

	return (mtl_ptr == cast_key->mtl_ptr);
}

std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey> CyclesMtlNodeTranslator::TranslatorKey::CreateClone() const
{
	return std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey>(new TranslatorKey(*this));
}

std::unique_ptr<Translator> CyclesMtlNodeTranslator::TranslatorKey::AllocateNewTranslator(TranslatorGraphNode& translator_graph_node, const IRenderSessionContext& /*render_session_context*/, const TimeValue /*initial_time*/) const
{
	return std::unique_ptr<Translator>(new CyclesMtlNodeTranslator(*this, translator_graph_node));
}

size_t CyclesMtlNodeTranslator::TranslatorKey::compute_hash(Mtl* const mtl)
{
	return std::hash<Mtl*>()(mtl);
}
