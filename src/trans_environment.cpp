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
 
#include "trans_environment.h"

#include <RenderingAPI/Renderer/IEnvironmentContainer.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "util_translate_environment.h"

using MaxSDK::RenderingAPI::IEnvironmentContainer;
using MaxSDK::RenderingAPI::IRenderSessionContext;
using MaxSDK::RenderingAPI::TranslationResult;
using MaxSDK::RenderingAPI::Translator;
using MaxSDK::RenderingAPI::TranslatorGraphNode;

CyclesEnvironmentTranslator::CyclesEnvironmentTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	BaseTranslator_ReferenceTarget(key.get_environment(), true, translator_graph_node),
	Translator(translator_graph_node),
	logger(global_log_manager.new_logger(L"CyclesEnvironmentTranslator", false, true))
{
	*logger << LogCtl::SEPARATOR;
}

CyclesEnvironmentTranslator::~CyclesEnvironmentTranslator()
{

}

bool CyclesEnvironmentTranslator::GetEnvParams(CyclesEnvironmentParams& env_params) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	env_params = GetOutput_SimpleValue<CyclesEnvironmentParams>(0, env_params);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesEnvironmentTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesEnvironmentTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
#else
static_assert(false);
#endif
{
	*logger << "Translate called..." << LogCtl::WRITE_LINE;

	IRenderSessionContext& session_context = GetRenderSessionContext();

	const CyclesEnvironmentParams env_params = get_environment_params(session_context, translation_time);

	SetOutput_SimpleValue<CyclesEnvironmentParams>(0, env_params);

	new_validity = session_context.GetEnvironment()->Validity(translation_time);

	return TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesEnvironmentTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesEnvironmentTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesEnvironmentTranslator::CheckValidity(const TimeValue /*t*/, const Interval& /*previous_validity*/) const
{
	return NEVER;
}

MSTR CyclesEnvironmentTranslator::GetTimingCategory() const
{
	return L"Environment";
}

CyclesEnvironmentTranslator::TranslatorKey::TranslatorKey(IEnvironmentContainer* const environment) :
	MaxSDK::RenderingAPI::TranslatorKey(compute_hash(environment))
{
	env_ptr = environment;
}

IEnvironmentContainer* CyclesEnvironmentTranslator::TranslatorKey::get_environment() const
{
	return env_ptr;
}

bool CyclesEnvironmentTranslator::TranslatorKey::operator==(const MaxSDK::RenderingAPI::TranslatorKey& rhs) const
{
	const TranslatorKey* const cast_key = dynamic_cast<const TranslatorKey*>(&rhs);

	if (cast_key == nullptr) {
		return false;
	}

	return (env_ptr == cast_key->env_ptr);
}

std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey> CyclesEnvironmentTranslator::TranslatorKey::CreateClone() const
{
	return std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey>(new TranslatorKey(*this));
}

std::unique_ptr<Translator> CyclesEnvironmentTranslator::TranslatorKey::AllocateNewTranslator(
	TranslatorGraphNode& translator_graph_node,
	const IRenderSessionContext& /*render_session_context*/,
	const TimeValue /*initial_time*/
	) const
{
	return std::unique_ptr<Translator>(new CyclesEnvironmentTranslator(*this, translator_graph_node));
}

size_t CyclesEnvironmentTranslator::TranslatorKey::compute_hash(IEnvironmentContainer* const environment)
{
	return std::hash<IEnvironmentContainer*>()(environment);
}
