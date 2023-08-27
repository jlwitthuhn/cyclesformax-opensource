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
 * @brief Defines the translator class CyclesSceneTranslator.
 */

#include <memory>

#include <RenderingAPI/Translator/GenericTranslatorKeys.h>
#include <RenderingAPI/Translator/Translator.h>

#include "rend_logger.h"
#include "trans_output.h"

class CyclesSceneTranslator : public virtual MaxSDK::RenderingAPI::Translator
{
public:
	typedef MaxSDK::RenderingAPI::GenericTranslatorKey_Empty<CyclesSceneTranslator> TranslatorKey;

	CyclesSceneTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node);
	~CyclesSceneTranslator();

	bool GetSceneDescriptor(CyclesSceneDescriptor& scene_desc) const;

#if PLUGIN_SDK_VERSION >= 2018
	virtual MaxSDK::RenderingAPI::TranslationResult Translate(const TimeValue t, Interval& validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress, KeyframeList& keyframes_needed) override;
	virtual MaxSDK::RenderingAPI::TranslationResult TranslateKeyframe(const TimeValue t, const TimeValue keyframe_t, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress) override;
#elif PLUGIN_SDK_VERSION == 2017
	virtual MaxSDK::RenderingAPI::TranslationResult Translate(const TimeValue t, Interval& validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress) override;
	virtual MaxSDK::RenderingAPI::TranslationResult TranslateKeyframe(const TimeValue t, const TimeValue keyframe_t, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress);
#else
	static_assert(false);
#endif
	virtual void PreTranslate(const TimeValue translationTime, Interval &validity) override;
	virtual void PostTranslate(const TimeValue translationTime, Interval &validity) override;
	virtual void AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics &stats) const override;
	virtual Interval CheckValidity(const TimeValue t, const Interval &previous_validity) const override;
	virtual MSTR GetTimingCategory() const override;

private:
	const std::unique_ptr<LoggerInterface> logger;
};
