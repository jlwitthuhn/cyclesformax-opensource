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
 
#include "rend_offline_frame_man.h"

#include <algorithm>
#include <set>
#include <thread>
#include <vector>

#include <render/background.h>
#include <render/camera.h>

#include <Rendering/RendProgressCallback.h>
#include <RenderingAPI/Renderer/ICameraContainer.h>
#include <RenderingAPI/Renderer/IRenderingLogger.h>
#include <RenderingAPI/Renderer/IRenderingProcess.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "cache_baked_texmap.h"
#include "cycles_session.h"
#include "max_rend_framebuffer_reader.h"
#include "plugin_re_simple.h"
#include "rend_params.h"
#include "rend_update_timer.h"
#include "util_cycles_device.h"
#include "util_cycles_film.h"
#include "util_cycles_integrator.h"
#include "util_cycles_params.h"
#include "util_cycles_status.h"
#include "util_pass.h"
#include "util_stereo.h"
#include "util_translate_camera.h"

using MaxSDK::RenderingAPI::IRenderingProcess;

// Amount of time to sleep between status checks in the main loop
const std::chrono::milliseconds STATUS_LOOP_DELAY{ 25 };

static bool allow_render_passes(StereoscopyType stereo_type)
{
	static std::set<StereoscopyType> allowed_types;
	if (allowed_types.size() == 0) {
		allowed_types.insert(StereoscopyType::NONE);
		allowed_types.insert(StereoscopyType::LEFT_EYE);
		allowed_types.insert(StereoscopyType::RIGHT_EYE);
	}
	return (allowed_types.count(stereo_type) == 1);
}

enum class CryptomatteType {
	OBJECT,
	MATERIAL,
	ASSET,
	NONE
};

static CryptomatteType get_cryptomatte_type(CyclesRenderElement* const render_element)
{
	const RenderPassType type{ render_element->GetRenderPassType() };
	if (type == RenderPassType::CRYPTOMATTE_OBJ) {
		return CryptomatteType::OBJECT;
	}
	if (type == RenderPassType::CRYPTOMATTE_MTL) {
		return CryptomatteType::MATERIAL;
	}
	if (type == RenderPassType::CRYPTOMATTE_ASSET) {
		return CryptomatteType::ASSET;
	}
	else {
		return CryptomatteType::NONE;
	}
}

static std::vector<RenderPassInfo> get_render_pass_info(CyclesRenderParams& rend_params, MaxSDK::RenderingAPI::IRenderSessionContext& session_context)
{
	std::vector<RenderPassInfo> result;
	// Always include combined pass from default RenderPassInfo constructor
	result.push_back(RenderPassInfo{});

	if (allow_render_passes(rend_params.stereo_type) == false) {
		// stereo rendering does not support render passes (yet)
		return result;
	}

	std::vector<CyclesRenderElement*> cryptomatte_object_elements;
	std::vector<CyclesRenderElement*> cryptomatte_material_elements;
	std::vector<CyclesRenderElement*> cryptomatte_asset_elements;

	for (IRenderElement* const this_render_element : session_context.GetRenderElements()) {
		if (this_render_element->IsEnabled() == FALSE) {
			continue;
		}
		CyclesRenderElement* const cycles_element{ dynamic_cast<CyclesRenderElement*>(this_render_element) };
		if (cycles_element == nullptr) {
			continue;
		}

		// Filter out cryptomatte passes to be handled later
		const CryptomatteType type{ get_cryptomatte_type(cycles_element) };
		if (type == CryptomatteType::OBJECT) {
			cryptomatte_object_elements.push_back(cycles_element);
		}
		else if (type == CryptomatteType::MATERIAL) {
			cryptomatte_material_elements.push_back(cycles_element);
		}
		else if (type == CryptomatteType::ASSET) {
			cryptomatte_asset_elements.push_back(cycles_element);
		}
		else {
			const RenderPassInfo this_info{ cycles_element };
			if (this_info.type != RenderPassType::INVALID) {
				result.push_back(this_info);
			}
		}
	}

	const size_t max_size{ std::max(cryptomatte_object_elements.size(), std::max(cryptomatte_material_elements.size(), cryptomatte_asset_elements.size())) };

	// Objects
	for (size_t i = 0; i < max_size; i++) {
		if (i < cryptomatte_object_elements.size()) {
			// Create a pass with a real render element
			CyclesRenderElement* const this_element{ cryptomatte_object_elements[i] };
			const RenderPassInfo this_info{ this_element, i };
			if (this_info.type != RenderPassType::INVALID) {
				result.push_back(this_info);
			}
		}
		else {
			// Create a dummy pass with no attached render element
			const RenderPassInfo this_info{ RenderPassType::CRYPTOMATTE_OBJ, ccl::PassType::PASS_CRYPTOMATTE, 4, "CryptoObject", i };
			result.push_back(this_info);
		}
	}

	// Materials
	for (size_t i = 0; i < max_size; i++) {
		if (i < cryptomatte_material_elements.size()) {
			// Create a pass with a real render element
			CyclesRenderElement* const this_element = cryptomatte_material_elements[i];
			const RenderPassInfo this_info{ this_element, i };
			if (this_info.type != RenderPassType::INVALID) {
				result.push_back(this_info);
			}
		}
		else {
			// Create a dummy pass with no attached render element
			const RenderPassInfo this_info{ RenderPassType::CRYPTOMATTE_MTL, ccl::PassType::PASS_CRYPTOMATTE, 4, "CryptoMaterial", i };
			result.push_back(this_info);
		}
	}

	// Assets
	for (size_t i = 0; i < max_size; i++) {
		if (i < cryptomatte_asset_elements.size()) {
			// Create a pass with a real render element
			CyclesRenderElement* const this_element = cryptomatte_asset_elements[i];
			const RenderPassInfo this_info{ this_element, i };
			if (this_info.type != RenderPassType::INVALID) {
				result.push_back(this_info);
			}
		}
		else {
			// Create a dummy pass with no attached render element
			const RenderPassInfo this_info{ RenderPassType::CRYPTOMATTE_ASSET, ccl::PassType::PASS_CRYPTOMATTE, 4, "CryptoAsset", i };
			result.push_back(this_info);
		}
	}

	if (rend_params.use_adaptive_sampling) {
		// Create a dummy pass with no attached render element
		const RenderPassInfo aux_info{ RenderPassType::ADAPTIVE_AUX, ccl::PassType::PASS_ADAPTIVE_AUX_BUFFER, 4, "AdaptiveAuxBuffer", 0 };
		result.push_back(aux_info);
		const RenderPassInfo count_info{ RenderPassType::SAMPLE_COUNT, ccl::PassType::PASS_SAMPLE_COUNT, 1, "SampleCount", 0 };
		result.push_back(count_info);
	}

	return result;
}

struct CryptoMatteInfo {
	size_t depth{ 1 };
	ccl::CryptomatteType type{ ccl::CryptomatteType::CRYPT_NONE };
};

static CryptoMatteInfo get_cryptomatte_info(const std::vector<RenderPassInfo>& passes)
{
	size_t count_object{ 0 };
	size_t count_material{ 0 };
	size_t count_asset{ 0 };

	for (const RenderPassInfo& this_pass : passes) {
		if (this_pass.ccl_type == ccl::PassType::PASS_CRYPTOMATTE) {
			if (this_pass.name.find("Object") != std::string::npos) {
				count_object++;
			}
			else if (this_pass.name.find("Material") != std::string::npos) {
				count_material++;
			}
			else if (this_pass.name.find("Asset") != std::string::npos) {
				count_asset++;
			}
		}
	}

	CryptoMatteInfo result;

	const size_t max_layers = std::max(count_object, std::max(count_material, count_asset));
	result.depth = max_layers;

	if (count_object > 0) {
		result.type = static_cast<ccl::CryptomatteType>(result.type | ccl::CryptomatteType::CRYPT_OBJECT);
	}
	if (count_material > 0) {
		result.type = static_cast<ccl::CryptomatteType>(result.type | ccl::CryptomatteType::CRYPT_MATERIAL);
	}
	if (count_asset > 0) {
		result.type = static_cast<ccl::CryptomatteType>(result.type | ccl::CryptomatteType::CRYPT_ASSET);
	}

	return result;
}

OfflineFrameManager::OfflineFrameManager(
	MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
	BakedTexmapCache& texmap_cache,
	CyclesRenderParams& rend_params
	) :
	translation_manager{
		std::make_unique<OfflineTranslationManager>(
			session_context,
			texmap_cache,
			rend_params
		)
	},
	texmap_cache{ texmap_cache },
	rend_params{ rend_params },
	session_context{ session_context },
	logger{ global_log_manager.new_logger(L"OfflineFrameManager") }
{
	*logger << LogCtl::SEPARATOR;

	// Check if we can find render device here. If not, log a message
	if (get_cycles_device_info(
		rend_params.render_device,
		rend_params.cuda_device,
		rend_params.optix_device,
		0,
		false,
		rend_params.debug_multi_cuda).type == ccl::DeviceType::DEVICE_NONE
		)
	{
		session_context.GetLogger().LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Error, L"Failed to find selected render device.");
	}
}


// TODO: This function should ideally just call TranslationManager::translate()
// Everything else should be moved to the translation manager or the frame manager constructor
void OfflineFrameManager::translate()
{
	*logger << "translate begin..." << LogCtl::WRITE_LINE;

	// Find the resolution of our render buffers
	boost::optional<CyclesCameraParams> camera_params{ get_camera_params(session_context, rend_params.frame_t, rend_params.stereo_type) };
	if (camera_params.has_value() == false) {
		frame_errored = true;
		return;
	}
	rend_params.region = camera_params->region;
	*logger << "Buffer resolution: " << rend_params.region << LogCtl::WRITE_LINE;
	const RenderResolutions resolutions{ camera_params->final_resolution.x(), camera_params->final_resolution.y(), rend_params.stereo_type };

	// Get and log render pass info
	const std::vector<RenderPassInfo> render_pass_info{ get_render_pass_info(rend_params, session_context) };
	const CryptoMatteInfo cryptomatte_info = get_cryptomatte_info(render_pass_info);
	ccl::BufferParams buffer_params{ get_buffer_params(camera_params->region.size(), render_pass_info) };
	*logger << "render passes: " << LogCtl::WRITE_LINE;
	for (const RenderPassInfo& this_pass : render_pass_info) {
		*logger << this_pass.name << LogCtl::WRITE_LINE;
	}

	// Device selection may fail here, error the frame in this case
	boost::optional<ccl::SessionParams> opt_session_params{ get_session_params(rend_params) };
	if (opt_session_params.has_value() == false) {
		frame_errored = true;
		return;
	}
	const ccl::SessionParams session_params{ *opt_session_params };

	*logger << "Creating CyclesSession..." << LogCtl::WRITE_LINE;

	session = std::make_unique<CyclesSession>(*opt_session_params, rend_params, resolutions, render_pass_info);
	session->reset_and_cache(buffer_params, session_params.samples);

	translation_manager->init(session->device);
	translation_manager->scene->background->set_transparent(rend_params.use_transparent_sky);

	assert(translation_manager->scene->integrator != nullptr);
	apply_integrator_params(*(translation_manager->scene->integrator), rend_params, *camera_params);
	assert(translation_manager->scene->film != nullptr);
	apply_film_params(*(translation_manager->scene->film), rend_params);

	*logger << "cryptomatte depth: " << cryptomatte_info.depth << LogCtl::WRITE_LINE;
	*logger << "cryptomatte passes: " << cryptomatte_info.type << LogCtl::WRITE_LINE;

	translation_manager->scene->film->set_cryptomatte_depth(static_cast<int>(cryptomatte_info.depth)) ;
	translation_manager->scene->film->set_cryptomatte_passes(cryptomatte_info.type);

	translation_manager->scene->film->tag_passes_update(translation_manager->scene, buffer_params.passes);

	session->scene = translation_manager->scene;

	// Skip all translation if frame is errored
	if (!frame_errored) {
		*logger << "Calling setup_camera..." << LogCtl::WRITE_LINE;
		if (setup_camera()) {

			*logger << "Calculating motion blur..." << LogCtl::WRITE_LINE;
			std::vector<int> mblur_sample_ticks;
			const int mblur_ticks_offset{ camera_params->get_mblur_full_offset() };
			if (mblur_ticks_offset > 0) {
				const int blur_samples{ rend_params.deform_blur_samples };
				for (int i = 1; i <= blur_samples; ++i) {
					const int ticks{ (i * mblur_ticks_offset) / blur_samples };
					mblur_sample_ticks.push_back(ticks);
					mblur_sample_ticks.push_back(-1 * ticks);
				}
				std::sort(mblur_sample_ticks.begin(), mblur_sample_ticks.end());
			}

			*logger << "Calling copy_scene..." << LogCtl::WRITE_LINE;
			translation_manager->copy_scene(mblur_sample_ticks);
		}
		else {
			frame_errored = true;
		}
	}

	*logger << "translate end" << LogCtl::WRITE_LINE;
}

void OfflineFrameManager::run_frame()
{
	if (frame_errored) {
		session_context.GetLogger().LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Error, L"Failed to set up render job, outputting red.");

		const IPoint2 max_resolution{ session_context.GetCamera().GetResolution() };
		const Int2 resolution{ Int2(max_resolution.x, max_resolution.y) };
		ConstantColorFrameBufferReader color_reader{ BMM_Color_fl(1.0f, 0.0f, 0.0f, 1.0f), resolution };
		session_context.GetMainFrameBufferProcessor().ProcessFrameBuffer(false, rend_params.frame_t, color_reader);
		session_context.UpdateBitmapDisplay();
	}
	else {
		run_frame_internal();
	}
}

void OfflineFrameManager::end_render()
{
	*logger << "end_render begin" << LogCtl::WRITE_LINE;
	stop_requested.store(true);
	translation_manager->end_render();
}

void OfflineFrameManager::run_frame_internal()
{
	*logger << "run_frame_internal begin..." << LogCtl::WRITE_LINE;

	int cameras_to_render{ get_render_count_for_stereo_type(rend_params.stereo_type) };

	int cameras_rendered{ 0 };

	// Skip starting the session of the user has already cancelled the render
	if (session_context.GetRenderingProcess().HasAbortBeenRequested()) {
		return;
	}

	*logger << "Maybe creating backplate..." << LogCtl::WRITE_LINE;
	const std::shared_ptr<BackplateBitmap> backplate_ptr{ texmap_cache.get_backplate_bitmap(session->get_render_resolution()) };
	session->set_backplate_bitmap(backplate_ptr);

	while (cameras_rendered < cameras_to_render) {
		*logger << "++++++++ RENDER LOOP BEGIN ++++++++" << LogCtl::WRITE_LINE;
		*logger << "camera: " << cameras_rendered << LogCtl::WRITE_LINE;

		setup_stereo_camera(cameras_rendered);
		*logger << "stereo updated" << LogCtl::WRITE_LINE;

		session->reset_with_cache();
		session->progress.reset();

		session->start(cameras_rendered);

		*logger << "setting status..." << LogCtl::WRITE_LINE;
		session_context.GetRenderingProcess().SetRenderingProgressTitle(L"Starting render session...");

		*logger << "stereo_eye: " << session->scene->camera->get_stereo_eye() << LogCtl::WRITE_LINE;
		*logger << "beginning loop..." << LogCtl::WRITE_LINE;

		render_status_loop();

		*logger << "loop complete" << LogCtl::WRITE_LINE;

		if (frame_was_cancelled) {
			break;
		}

		session_context.GetRenderingProcess().SetRenderingProgressTitle(L"Finished rendering frame, copying image buffer...");

		// Wait for the session to end cleanly as we may want to fiddle with the camera and then re-render for stereoscopy
		wait_for_session_end();

		session->copy_accum_buffer(session_context, true);

		cameras_rendered++;
	}

	wait_for_session_end();

	*logger << "run_frame_internal end" << LogCtl::WRITE_LINE;
}


// Cancels the active session and waits until it has completed running
void OfflineFrameManager::wait_for_session_end()
{
	session->end_session_thread();

	while (session->is_session_running()) {
		*logger << "waiting on session thread..." << LogCtl::WRITE_LINE;

		session_context.GetRenderingProcess().SetRenderingProgressTitle(L"Waiting for render session to end...");
		std::this_thread::sleep_for(STATUS_LOOP_DELAY);
	}

	// Cycles internally cleans up after the thread only when wait() is explicitly called
	session->wait();
}

void OfflineFrameManager::render_status_loop()
{
	bool render_incomplete{ true };
	BufferUpdateTimer update_timer;

	*logger << "loop begin..." << LogCtl::WRITE_LINE;

	while (render_incomplete) {
		std::string status;
		std::string substat;

		session->progress.get_status(status, substat);

		*logger << "stat: " << status.c_str() << " sub: " << substat.c_str() << LogCtl::WRITE_LINE;

		const CyclesStatus cycles_status{ status, substat, rend_params.samples };
		if (cycles_status.complete) {
			render_incomplete = false;
		}

		// Check if render has errored out
		if (cycles_status.errored) {
			session_context.GetLogger().LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Error, L"Internal render error, aborting");
			session_context.GetLogger().LogMessage(MaxSDK::RenderingAPI::IRenderingLogger::MessageType::Error, cycles_status.error_message.c_str());
			break;
		}

		session_context.GetRenderingProcess().SetRenderingProgress(cycles_status.work_done, cycles_status.work_total, IRenderingProcess::ProgressType::Rendering);
		if (session_context.GetRenderingProcess().HasAbortBeenRequested()) {
			stop_requested = true;
		}

		if (update_timer.should_update()) {
			*logger << "copying frame..." << LogCtl::WRITE_LINE;
			session->copy_accum_buffer(session_context, true);
		}

		// Check if render has been cancelled
		if (stop_requested) {
			render_incomplete = false;
			frame_was_cancelled = true;
			session_context.GetRenderingProcess().SetRenderingProgressTitle(L"Aborting...");
			session->progress.set_cancel(std::string("Aborted"));
		}
		else if (cycles_status.max_render_status_message.size() > 0) {
			const wchar_t* const message_c_str = cycles_status.max_render_status_message.c_str();
			MaxSDK::RenderingAPI::IRenderingProcess& process = session_context.GetRenderingProcess();
			process.SetRenderingProgressTitle(message_c_str);
		}

		*logger << "sleeping..." << LogCtl::WRITE_LINE;

		std::this_thread::sleep_for(STATUS_LOOP_DELAY);
	}

	*logger << "loop end" << LogCtl::WRITE_LINE;
}


bool OfflineFrameManager::setup_camera()
{
	boost::optional<CyclesCameraParams> camera_params = get_camera_params(session_context, rend_params.frame_t, rend_params.stereo_type);
	if (camera_params) {
		apply_camera_params(*camera_params, rend_params, *(session->scene->camera));
		session->scene->camera->update(session->scene);
		return true;
	}
	else {
		return false;
	}
}

void OfflineFrameManager::setup_stereo_camera(int render_pass_index)
{
	if (rend_params.stereo_type == StereoscopyType::NONE) {
		// Do nothing
		return;
	}
	else if (rend_params.stereo_type == StereoscopyType::LEFT_EYE) {
		session->scene->camera->set_stereo_eye(ccl::Camera::StereoEye::STEREO_LEFT);
	}
	else if (rend_params.stereo_type == StereoscopyType::RIGHT_EYE) {
		session->scene->camera->set_stereo_eye(ccl::Camera::StereoEye::STEREO_RIGHT);
	}
	else if (
		rend_params.stereo_type == StereoscopyType::ANAGLYPH ||
		rend_params.stereo_type == StereoscopyType::SPLIT_LEFT_RIGHT ||
		rend_params.stereo_type == StereoscopyType::SPLIT_TOP_BOTTOM )
	{
		if (render_pass_index % 2 == 0) {
			session->scene->camera->set_stereo_eye(ccl::Camera::StereoEye::STEREO_LEFT);
		}
		else {
			session->scene->camera->set_stereo_eye(ccl::Camera::StereoEye::STEREO_RIGHT);
		}
	}

	// Swap if necessary
	if (rend_params.stereo_swap_eyes) {
		if (session->scene->camera->get_stereo_eye() == ccl::Camera::StereoEye::STEREO_LEFT) {
			session->scene->camera->set_stereo_eye(ccl::Camera::StereoEye::STEREO_RIGHT);
		}
		else if (session->scene->camera->get_stereo_eye() == ccl::Camera::StereoEye::STEREO_RIGHT) {
			session->scene->camera->set_stereo_eye(ccl::Camera::StereoEye::STEREO_LEFT);
		}
	}

	session->scene->camera->need_device_update = true;
}
