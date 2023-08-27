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
 * @brief Defines the translator class CyclesTexmapTranslator.
 */

#include <chrono>

#include <RenderingAPI/Translator/BaseTranslators/BaseTranslator_ReferenceTarget.h>
#include <RenderingAPI/Translator/TranslatorKey.h>

class TexmapProperties {
public:
	std::chrono::steady_clock::time_point last_update_time;
	Interval validity = FOREVER;

	bool operator==(const TexmapProperties& other) const;
	bool operator!=(const TexmapProperties& other) const;
};

class CyclesTexmapTranslator : public MaxSDK::RenderingAPI::BaseTranslator_ReferenceTarget
{
public:
	class TranslatorKey;

	CyclesTexmapTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node);
	~CyclesTexmapTranslator();

	bool GetTexmapProperties(TexmapProperties& tex_properties) const;

#if PLUGIN_SDK_VERSION >= 2018
	virtual MaxSDK::RenderingAPI::TranslationResult Translate(const TimeValue t, Interval& validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress, KeyframeList& keyframes_needed) override;
	virtual MaxSDK::RenderingAPI::TranslationResult TranslateKeyframe(const TimeValue t, const TimeValue keyframe_t, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress) override;
#elif PLUGIN_SDK_VERSION == 2017
	virtual MaxSDK::RenderingAPI::TranslationResult Translate(const TimeValue t, Interval& validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress) override;
	virtual MaxSDK::RenderingAPI::TranslationResult TranslateKeyframe(const TimeValue t, const TimeValue keyframe_t, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress);
#else
	static_assert(false);
#endif
	virtual void AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics &stats) const override;
	virtual Interval CheckValidity(const TimeValue t, const Interval &previous_validity) const override;
	virtual MSTR GetTimingCategory() const override;

protected:
	virtual void NotificationCallback_NotifyEvent(const MaxSDK::RenderingAPI::IGenericEvent &genericEvent, void *userData) override;

private:
	Texmap* tex_ptr;
	std::chrono::steady_clock::time_point last_update_time;
};

class CyclesTexmapTranslator::TranslatorKey : public MaxSDK::RenderingAPI::TranslatorKey
{
public:
	TranslatorKey(Texmap* tex);

	Texmap* get_tex() const;

	// from MaxSDK::RenderingAPI::TranslatorKey
	virtual bool operator==(const MaxSDK::RenderingAPI::TranslatorKey& rhs) const override;
	virtual std::unique_ptr<const MaxSDK::RenderingAPI::TranslatorKey> CreateClone() const override;
	virtual std::unique_ptr<MaxSDK::RenderingAPI::Translator> AllocateNewTranslator(MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node, const MaxSDK::RenderingAPI::IRenderSessionContext& render_session_context, const TimeValue initial_time) const override;

private:
	static size_t compute_hash(Texmap* tex);

	Texmap* tex_ptr;
};
