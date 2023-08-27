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
 
#include "max_rend_interactive_session.h"

#include <render/background.h>
#include <render/camera.h>
#include <render/mesh.h>
#include <render/object.h>
#include <render/scene.h>

#include <RenderingAPI/Renderer/IRenderSettingsContainer.h>
#include <RenderingAPI/Renderer/IRenderingProcess.h>
#include <RenderingAPI/Translator/ITranslationManager.h>

#include "cache_baked_texmap.h"
#include "cycles_session.h"
#include "rend_logger_ext.h"
#include "rend_shader_manager.h"
#include "trans_scene.h"
#include "util_cycles_integrator.h"
#include "util_cycles_params.h"
#include "util_cycles_status.h"
#include "util_translate_camera.h"
#include "util_translate_environment.h"
#include "util_translate_geometry.h"
#include "util_translate_light.h"

// ActiveShade will always use no stereoscopy
static constexpr StereoscopyType STEREO_TYPE = StereoscopyType::NONE;

static int get_processor_count()
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	if (sys_info.dwNumberOfProcessors < 2) {
		return 2;
	}

	return sys_info.dwNumberOfProcessors;
}

CyclesInteractiveRenderSession::CyclesInteractiveRenderSession(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, const CyclesRenderParams& rend_params) :
	session_context{ session_context },
	rend_params{ rend_params },
	resolutions{ 640, 480, STEREO_TYPE },
	texmap_cache{ std::make_unique<BakedTexmapCache>(session_context, rend_params.texmap_bake_width, rend_params.texmap_bake_height) },
	logger{ global_log_manager.new_logger(L"CyclesInteractiveRenderSession") }
{
	*logger << LogCtl::SEPARATOR;

	this->rend_params.use_progressive_refine = true;
}

CyclesInteractiveRenderSession::~CyclesInteractiveRenderSession()
{
	if (scene_translator != nullptr) {
		session_context.GetTranslationManager().ReleaseSceneTranslator(*scene_translator);
		scene_translator = nullptr;
	}
}

bool CyclesInteractiveRenderSession::InitiateInteractiveSession(const TimeValue t)
{
	*logger << "InitiateInteractiveSession called, translating scene..." << LogCtl::WRITE_LINE;

	bool scene_updated = false;
	TranslateOrUpdateScene(t, scene_updated);

	state = SessionState::READY_TO_TRANSLATE;

	*logger << "InitiateInteractiveSession ended" << LogCtl::WRITE_LINE;

	return true;
}

bool CyclesInteractiveRenderSession::UpdateInteractiveSession(const TimeValue t, bool &doneRendering)
{
	*logger << "UpdateInteractiveSession called..."<< LogCtl::WRITE_LINE;

	doneRendering = false;

	if (state == SessionState::READY_TO_TRANSLATE) {
		*logger << "State: ready to translate" << LogCtl::WRITE_LINE;

		bool scene_updated = false;
		TranslateOrUpdateScene(t, scene_updated);
		first_fb_update_done = false;

		state = SessionState::RETRANSLATE;
	}
	else if (state == SessionState::RETRANSLATE) {
		*logger << "State: retranslating" << LogCtl::WRITE_LINE;

		bool scene_updated = false;
		TranslateOrUpdateScene(t, scene_updated);
		state = SessionState::PREPARE_FOR_BUILD;
	}
	else if (state == SessionState::PREPARE_FOR_BUILD) {
		*logger << "State: prepare for build" << LogCtl::WRITE_LINE;

		DestroyCyclesSession();

		state = SessionState::BUILD_STAGE_1;
	}
	else if (state == SessionState::BUILD_STAGE_1) {
		// This state handles building the session except for geometry
		*logger << "State: build stage 1" << LogCtl::WRITE_LINE;

		CyclesSceneDescriptor scene_desc;
		scene_translator->GetSceneDescriptor(scene_desc);
		bool success = BuildCyclesSession(scene_desc);

		if (success) {
			state = SessionState::BUILD_STAGE_2;
		}
		else {
			state = SessionState::ABORTING;
		}
	}
	else if (state == SessionState::BUILD_STAGE_2) {
		// This state exclusively handles building geometry
		*logger << "State: build stage 2" << LogCtl::WRITE_LINE;

		CyclesSceneDescriptor scene_desc;
		scene_translator->GetSceneDescriptor(scene_desc);
		BuildGeometry(scene_desc.scene_geometry);

		state = SessionState::BUILD_STAGE_3;
	}
	else if (state == SessionState::BUILD_STAGE_3) {
		// This state exclusively handles setting a backplate
		*logger << "State: build stage 3" << LogCtl::WRITE_LINE;

		const auto backplate = texmap_cache->get_backplate_bitmap(resolutions.render_res());
		cycles_session->set_backplate_bitmap(backplate);

		state = SessionState::READY_TO_BAKE;
	}
	else if (state == SessionState::READY_TO_BAKE) {
		*logger << "State: ready to bake" << LogCtl::WRITE_LINE;

		texmap_cache->queue_dirty_texmaps();
		texmap_cache->bake_texmaps_iteration();

		state = SessionState::TEX_BAKING;
	}
	else if (state == SessionState::TEX_BAKING) {
		*logger << "State: baking" << LogCtl::WRITE_LINE;

		bool baking_done = texmap_cache->bake_texmaps_iteration();
		if (baking_done) {
			session_context.CallRenderEnd(t);
			cycles_session->start(0);
			state = SessionState::RENDERING;
		}
	}
	else if (state == SessionState::RENDERING) {
		*logger << "State: rendering" << LogCtl::WRITE_LINE;

		if (first_fb_update_done && session_context.GetTranslationManager().DoesSceneNeedUpdate(*scene_translator, t)) {
			*logger << "update needed, destroying session" << LogCtl::WRITE_LINE;

			cycles_session->progress.set_cancel(std::string("Scene changed"));
			state = SessionState::ABORTING;
		}
		else {
			std::string status;
			std::string substat;

			cycles_session->progress.get_status(status, substat);

			*logger << "status: " << status.c_str() << LogCtl::WRITE_LINE;
			*logger << "substat: " << substat.c_str() << LogCtl::WRITE_LINE;

			const CyclesStatus cycles_status(status, substat, rend_params.samples);

			if (cycles_status.complete) {
				state = SessionState::COMPLETE;
				doneRendering = true;
			}

			const std::chrono::milliseconds ms_since_last_copy = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_fb_update);
			const bool ready_for_fb_copy = (ms_since_last_copy.count() > 100); // 10 FPS max
			const bool do_time_based_copy = ready_for_fb_copy && cycles_status.render_in_progress && cycles_status.samples_rendered > 1;

			if (doneRendering || do_time_based_copy) {
				*logger << "Copying display buffer..." << LogCtl::WRITE_LINE;

				cycles_session->copy_accum_buffer(session_context, false);
				last_fb_update = std::chrono::steady_clock::now();
				first_fb_update_done = true;
			}
			else {
				*logger << "Doing nothing" << LogCtl::WRITE_LINE;
			}
		}
	}
	else if (state == SessionState::ABORTING) {
		*logger << "State: aborting" << LogCtl::WRITE_LINE;

		DestroyCyclesSession();
		state = SessionState::READY_TO_TRANSLATE;
	}
	else if (state == SessionState::COMPLETE) {
		*logger << "State: complete" << LogCtl::WRITE_LINE;

		if (session_context.GetTranslationManager().DoesSceneNeedUpdate(*scene_translator, t)) {
			cycles_session->progress.set_cancel(std::string("Scene changed"));
			DestroyCyclesSession();
			state = SessionState::READY_TO_TRANSLATE;
		}
		doneRendering = true;
	}

	*logger << "UpdateInteractiveSession complete" << LogCtl::WRITE_LINE;

	return true;
}

void CyclesInteractiveRenderSession::TerminateInteractiveSession()
{
	*logger << "TerminateInteractiveSession called..." << LogCtl::WRITE_LINE;

	if (cycles_session != nullptr) {
		DestroyCyclesSession();
	}
}

bool CyclesInteractiveRenderSession::IsInteractiveSessionUpToDate(const TimeValue t)
{
	*logger << "IsInteractiveSessionUpToDate called..." << LogCtl::WRITE_LINE;

	if (scene_translator == nullptr) {
		return false;
	}

	return !session_context.GetTranslationManager().DoesSceneNeedUpdate(*scene_translator, t);
}

bool CyclesInteractiveRenderSession::BuildCyclesSession(CyclesSceneDescriptor& scene_desc)
{
	*logger << "BuildCyclesSession called..." << LogCtl::WRITE_LINE;
	*logger << "scene_desc meshes: " << scene_desc.scene_geometry.geom_objects.size() << LogCtl::WRITE_LINE;
	*logger << "scene_desc mtl updated tick: " << scene_desc.mtl_properties.last_update_time.time_since_epoch().count() << LogCtl::WRITE_LINE;

	if (cycles_session != nullptr) {
		DestroyCyclesSession();
	}

	*logger << "Old session destroyed" << LogCtl::WRITE_LINE;

	texmap_cache->set_texmap_times(scene_desc.texmap_times.update_times);

	rend_params.region = scene_desc.camera_params.region;

	// With at least 3 cores, leave one alone so max is more responsive
	int threads = get_processor_count();
	if (threads >= 3) {
		threads--;
	}

	// We don't need to get the session_width/height because activeshade will always use the final resolution
	const ccl::BufferParams buffer_params = get_buffer_params(scene_desc.camera_params.final_resolution);

	const boost::optional<ccl::SessionParams> opt_session_params{ get_session_params(rend_params) };
	if (opt_session_params.has_value() == false) {
		return false;
	}
	ccl::SessionParams session_params = *opt_session_params;

	session_params.progressive_update_timeout = 0.08f;

	*logger << "Creating session" << LogCtl::WRITE_LINE;

	// Create session and fill in appropriate parameters
	std::vector<RenderPassInfo> default_pass_vec;
	default_pass_vec.push_back(RenderPassInfo());

	resolutions = RenderResolutions(
		scene_desc.camera_params.final_resolution.x(),
		scene_desc.camera_params.final_resolution.y(),
		STEREO_TYPE);

	cycles_session = std::make_unique<CyclesSession>(session_params, rend_params, resolutions, default_pass_vec);
	cycles_session->reset_and_cache(buffer_params, session_params.samples);

	const ccl::SceneParams scene_params = get_scene_params();

	*logger << "Creating scene" << LogCtl::WRITE_LINE;

	ccl::Scene* cycles_scene = new ccl::Scene(scene_params, cycles_session->device);

	cycles_scene->background->set_transparent(rend_params.use_transparent_sky);

	*logger << "Setting film exposure to: " << rend_params.exposure_multiplier << LogCtl::WRITE_LINE;

	cycles_scene->film->set_exposure(rend_params.exposure_multiplier);

	assert(cycles_scene->integrator != nullptr);
	apply_integrator_params(*(cycles_scene->integrator), rend_params, scene_desc.camera_params);

	cycles_session->scene = cycles_scene;

	*logger << "Creating shader manager" << LogCtl::WRITE_LINE;

	shader_manager = std::make_unique<MaxShaderManager>(*texmap_cache, cycles_scene, rend_params.frame_t);
	shader_manager->set_mis_map_size(rend_params.mis_map_size);

	apply_camera_params(scene_desc.camera_params, rend_params, *(cycles_session->scene->camera));
	cycles_session->scene->camera->update(cycles_session->scene);

	apply_environment_params(scene_desc.env_params, rend_params, shader_manager);

	*logger << "Completed camera and env setup" << LogCtl::WRITE_LINE;

	// Copy lights
	for (CyclesLightParams light_params : scene_desc.scene_lights.lights) {
		add_light_to_scene(cycles_scene, shader_manager, light_params, rend_params.point_light_size);
	}

	*logger << "Completed adding lights" << LogCtl::WRITE_LINE;

	cycles_session->reset_with_cache();

	return true;
}

void CyclesInteractiveRenderSession::BuildGeometry(CyclesSceneGeometryList& geometry_list)
{
	*logger << "CyclesInteractiveRenderSession::BuildGeometry called..." << LogCtl::WRITE_LINE;

	// Copy meshes into the scene
	for (CyclesGeomObject& geom_object : geometry_list.geom_objects) {
		*logger << "Copying mesh..." << LogCtl::WRITE_LINE;
		if (geom_object.mesh_geometry.get() == nullptr) {
			*logger << "No mesh" << LogCtl::WRITE_LINE;
			continue;
		}
		// TODO: This should use caching similar to how it is done in rend_scene_manager.cpp
		// Until this is changed, instancing will not work in activeshade
		const int default_shader_index = shader_manager->get_simple_color_shader(geom_object.wire_color);
		const auto default_shader = shader_manager->get_ccl_shader(default_shader_index);
		MaxMultiShaderHelper ms_helper(default_shader);
		if (geom_object.mtl != nullptr) {
			session_context.CallRenderBegin(*(geom_object.mtl), rend_params.frame_t);
			ms_helper = shader_manager->get_mtl_multishader(geom_object.mtl);
		}
		ccl::Mesh* const ccl_mesh = get_ccl_mesh(geom_object.mesh_geometry, ms_helper);
		cycles_session->scene->geometry.push_back(ccl_mesh);
		ccl::Object* const new_object = get_ccl_object(geom_object, ccl_mesh);
		cycles_session->scene->objects.push_back(new_object);
		*logger << "Copy complete" << LogCtl::WRITE_LINE;
	}

	// Shader manager is no longer needed, dispose of it here
	shader_manager.reset(nullptr);

	*logger << "CyclesInteractiveRenderSession::BuildGeometry complete" << LogCtl::WRITE_LINE;
}

void CyclesInteractiveRenderSession::DestroyCyclesSession()
{
	*logger << "CyclesInteractiveRenderSession::DestroyCyclesSession called..." << LogCtl::WRITE_LINE;

	if (cycles_session) {
		cycles_session = std::unique_ptr<CyclesSession>();
	}

	*logger << "CyclesInteractiveRenderSession::DestroyCyclesSession complete" << LogCtl::WRITE_LINE;
}

bool CyclesInteractiveRenderSession::TranslateOrUpdateScene(const TimeValue t, bool& scene_updated)
{
	*logger << "TranslateOrUpdateScene called..." << LogCtl::WRITE_LINE;
	scene_updated = false;

	MaxSDK::RenderingAPI::ITranslationManager& translation_manager = session_context.GetTranslationManager();
	MaxSDK::RenderingAPI::TranslationResult result;

	rend_params.frame_t = t;
	rend_params.std_render_hidden = session_context.GetRenderSettings().GetRenderHiddenObjects();
	rend_params.use_progressive_refine = true;

	session_context.GetRenderingProcess().TranslationStarted();
	if (scene_translator == nullptr) {
		*logger << "creating root translator..." << LogCtl::WRITE_LINE;
		texmap_cache->new_frame(t);
		scene_translator = translation_manager.TranslateScene<CyclesSceneTranslator>(CyclesSceneTranslator::TranslatorKey(), t, result);
		scene_updated = true;
	}
	else if (translation_manager.DoesSceneNeedUpdate(*scene_translator, t)) {
		*logger << "updating translators..." << LogCtl::WRITE_LINE;
		texmap_cache->new_frame(t);
		translation_manager.UpdateScene(*scene_translator, t);
		scene_updated = true;
	}
	else {
		*logger << "Nothing to do here" << LogCtl::WRITE_LINE;
	}
	session_context.GetRenderingProcess().TranslationFinished();

	*logger << "TranslateOrUpdateScene end" << LogCtl::WRITE_LINE;

	return true;
}
