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
 
#include "trans_texmap_single.h"

#include <Materials/Texmap.h>

using MaxSDK::RenderingAPI::IGenericEvent;
using MaxSDK::RenderingAPI::IRenderSessionContext;
using MaxSDK::RenderingAPI::Translator;
using MaxSDK::RenderingAPI::TranslationResult;
using MaxSDK::RenderingAPI::TranslatorGraphNode;


bool TexmapProperties::operator==(const TexmapProperties& other) const
{
	return last_update_time == other.last_update_time && validity == other.validity;
}

bool TexmapProperties::operator!=(const TexmapProperties& other) const
{
	return !operator==(other);
}

CyclesTexmapTranslator::CyclesTexmapTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	BaseTranslator_ReferenceTarget(key.get_tex(), true, translator_graph_node),
	Translator(translator_graph_node)
{
	tex_ptr = key.get_tex();
	last_update_time = std::chrono::steady_clock::now();
}

CyclesTexmapTranslator::~CyclesTexmapTranslator()
{

}

bool CyclesTexmapTranslator::GetTexmapProperties(TexmapProperties& tex_properties) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	tex_properties = GetOutput_SimpleValue<TexmapProperties>(0, tex_properties);
	return true;
}


#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesTexmapTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesTexmapTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
#else
static_assert(false);
#endif
{
	TexmapProperties result;
	result.last_update_time = last_update_time;

	new_validity = tex_ptr->Validity(translation_time);
	result.validity = new_validity;

	SetOutput_SimpleValue<TexmapProperties>(0, result);

	return TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesTexmapTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesTexmapTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesTexmapTranslator::CheckValidity(const TimeValue /*t*/, const Interval& /*previous_validity*/) const
{
	return NEVER;
}

MSTR CyclesTexmapTranslator::GetTimingCategory() const
{
	return L"SingleTexmap";
}

void CyclesTexmapTranslator::NotificationCallback_NotifyEvent(const IGenericEvent& genericEvent, void* const userData)
{
	MaxSDK::RenderingAPI::BaseTranslator_ReferenceTarget::NotificationCallback_NotifyEvent(genericEvent, userData);
	last_update_time = std::chrono::steady_clock::now();
}

CyclesTexmapTranslator::TranslatorKey::TranslatorKey(Texmap* tex) : MaxSDK::RenderingAPI::TranslatorKey(compute_hash(tex))
{
	tex_ptr = tex;
}

Texmap* CyclesTexmapTranslator::TranslatorKey::get_tex() const
{
	return tex_ptr;
}

bool CyclesTexmapTranslator::TranslatorKey::operator==(const MaxSDK::RenderingAPI::TranslatorKey& rhs) const
{
	const TranslatorKey* const cast_key = dynamic_cast<const TranslatorKey*>(&rhs);

	if (cast_key == nullptr) {
		return false;
	}

	return (tex_ptr == cast_key->tex_ptr);
}

std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey> CyclesTexmapTranslator::TranslatorKey::CreateClone() const
{
	return std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey>(new TranslatorKey(*this));
}

std::unique_ptr<Translator> CyclesTexmapTranslator::TranslatorKey::AllocateNewTranslator(
	TranslatorGraphNode& translator_graph_node,
	const IRenderSessionContext& /*render_session_context*/,
	const TimeValue /*initial_time*/
	) const
{
	return std::unique_ptr<Translator>(new CyclesTexmapTranslator(*this, translator_graph_node));
}

size_t CyclesTexmapTranslator::TranslatorKey::compute_hash(Texmap* const tex)
{
	return std::hash<Texmap*>()(tex);
}
