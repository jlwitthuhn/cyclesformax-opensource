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
 
#include "trans_scene.h"

#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "trans_camera.h"
#include "trans_environment.h"
#include "trans_geom_list.h"
#include "trans_light_list.h"
#include "trans_mtl_list.h"
#include "trans_texmap_list.h"

CyclesSceneTranslator::CyclesSceneTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	MaxSDK::RenderingAPI::Translator(translator_graph_node),
	logger(global_log_manager.new_logger(L"CyclesSceneTranslator", false, true))
{
	UNREFERENCED_PARAMETER(key);

	*logger << LogCtl::SEPARATOR;
}

CyclesSceneTranslator::~CyclesSceneTranslator()
{

}

bool CyclesSceneTranslator::GetSceneDescriptor(CyclesSceneDescriptor& scene_desc) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	scene_desc = GetOutput_SimpleValue<CyclesSceneDescriptor>(0, scene_desc);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesSceneTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesSceneTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& translation_progress)
#else
static_assert(false);
#endif
{
	*logger << "Translate called..." << LogCtl::WRITE_LINE;

	MaxSDK::RenderingAPI::TranslationResult result;

	*logger << "Creating environment translator..." << LogCtl::WRITE_LINE;

	const CyclesEnvironmentTranslator* const env_translator = AcquireChildTranslator<CyclesEnvironmentTranslator>(CyclesEnvironmentTranslator::TranslatorKey(GetRenderSessionContext().GetEnvironment()), translation_time, translation_progress, result);
	if (env_translator == nullptr) {
		return result;
	}
	
	*logger << "Creating camera translator..." << LogCtl::WRITE_LINE;

	const CyclesCameraTranslator* const camera_translator = AcquireChildTranslator<CyclesCameraTranslator>(CyclesCameraTranslator::TranslatorKey(), translation_time, translation_progress, result);
	if (camera_translator == nullptr) {
		return result;
	}

	*logger << "Creating geometry translator..." << LogCtl::WRITE_LINE;

	const CyclesGeometryListTranslator* const geom_translator = AcquireChildTranslator<CyclesGeometryListTranslator>(CyclesGeometryListTranslator::TranslatorKey(), translation_time, translation_progress, result);
	if (geom_translator == nullptr) {
		return result;
	}

	*logger << "Creating light translator..." << LogCtl::WRITE_LINE;

	const CyclesLightListTranslator* const light_translator = AcquireChildTranslator<CyclesLightListTranslator>(CyclesLightListTranslator::TranslatorKey(), translation_time, translation_progress, result);
	if (light_translator == nullptr) {
		return result;
	}

	*logger << "Creating materials translator..." << LogCtl::WRITE_LINE;

	const CyclesMaterialListTranslator* const mat_translator = AcquireChildTranslator<CyclesMaterialListTranslator>(CyclesMaterialListTranslator::TranslatorKey(), translation_time, translation_progress, result);
	if (mat_translator == nullptr) {
		return result;
	}

	*logger << "Creating texmaps translator..." << LogCtl::WRITE_LINE;

	const CyclesTexmapListTranslator* const tex_translator = AcquireChildTranslator<CyclesTexmapListTranslator>(CyclesTexmapListTranslator::TranslatorKey(), translation_time, translation_progress, result);
	if (tex_translator == nullptr) {
		return result;
	}

	CyclesSceneDescriptor scene_desc;
	camera_translator->GetCameraParams(scene_desc.camera_params);
	env_translator->GetEnvParams(scene_desc.env_params);
	geom_translator->GetSceneGeometry(scene_desc.scene_geometry);
	light_translator->GetSceneLights(scene_desc.scene_lights);
	mat_translator->GetMaterialProperties(scene_desc.mtl_properties);
	tex_translator->GetTexmapUpdateTimes(scene_desc.texmap_times);

	*logger << "Last mtl update: " << scene_desc.mtl_properties.last_update_time.time_since_epoch().count() << LogCtl::WRITE_LINE;

	SetOutput_SimpleValue<CyclesSceneDescriptor>(0, scene_desc);

	new_validity = FOREVER;

	*logger << "Translate complete" << LogCtl::WRITE_LINE;

	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesSceneTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesSceneTranslator::PreTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesSceneTranslator::PostTranslate(const TimeValue /*translationTime*/, Interval& /*validity*/)
{
	// Do nothing
}

void CyclesSceneTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesSceneTranslator::CheckValidity(const TimeValue /*t*/, const Interval& previous_validity) const
{
	return previous_validity;
}

MSTR CyclesSceneTranslator::GetTimingCategory() const
{
	return L"Scene";
}
