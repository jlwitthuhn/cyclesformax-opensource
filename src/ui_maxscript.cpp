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
 
#include "ui_maxscript.h"

#include <cfloat>
#include <climits>

#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/strings.h>

#include "const_classid.h"
#include "plugin_urender_cycles.h"
#include "rend_params.h"

extern CyclesRenderParams gui_render_params;

static bool globals_registered = false;

template <typename T, T COUNT>
Value* set_enum(Value* const new_val, T& ref)
{
	const int int_val = new_val->to_int();
	if (int_val >= 0 && int_val < static_cast<int>(COUNT)) {
		ref = static_cast<T>(int_val);
	}
	return Integer::intern(static_cast<int>(ref));
}

Value* set_bool(Value* const new_val, bool& ref)
{
	const int int_val = new_val->to_int();
	ref = (int_val != 0);
	return Integer::intern(static_cast<int>(ref));
}

Value* set_int(Value* const new_val, int& ref, const int min = INT_MIN, const int max = INT_MAX)
{
	const int int_val = new_val->to_int();
	if (int_val >= min && int_val <= max) {
		ref = int_val;
	}
	return Integer::intern(ref);
}

Value* set_float(Value* const new_val, float& ref, const float min = FLT_MIN, const float max = FLT_MAX)
{
	const float float_val = new_val->to_float();
	if (float_val >= min && float_val <= max) {
		ref = float_val;
	}
	return Float::intern(ref);
}

////
// renderDevice
////

static Value* get_render_device()
{
	return Integer::intern(static_cast<int>(gui_render_params.render_device));
}

static Value* set_render_device(Value* const val)
{
	return set_enum<RenderDevice, RenderDevice::DEVICE_COUNT>(val, gui_render_params.render_device);
}

////
// cudaDevice
////

static Value* get_cuda_device()
{
	return new String(gui_render_params.cuda_device.c_str());
}

static Value* set_cuda_device(Value* const val)
{
	const wchar_t* const str_val = val->to_string();
	gui_render_params.cuda_device = std::wstring(str_val);
	return val;
}

////
// cudaWithCPU
////

static Value* get_cuda_with_cpu()
{
	return Integer::intern(static_cast<int>(gui_render_params.cuda_with_cpu));
}

static Value* set_cuda_with_cpu(Value* const val)
{
	return set_bool(val, gui_render_params.cuda_with_cpu);
}

////
// optixDevice
////

static Value* get_optix_device()
{
	return new String(gui_render_params.optix_device.c_str());
}

static Value* set_optix_device(Value* const val)
{
	const wchar_t* const str_val = val->to_string();
	gui_render_params.optix_device = std::wstring(str_val);
	return val;
}

////
// previewSamples
////

static Value* get_preview_samples()
{
	return Integer::intern(gui_render_params.mtl_preview_samples);
}

static Value* set_preview_samples(Value* const val)
{
	return set_int(val, gui_render_params.mtl_preview_samples, 1);
}

////
// samples
////

static Value* get_samples()
{
	return Integer::intern(gui_render_params.samples);
}

static Value* set_samples(Value* const val)
{
	return set_int(val, gui_render_params.samples, 1);
}

////
// useAdaptiveSampling
////

static Value* get_use_adaptive_sampling()
{
	return Integer::intern(static_cast<int>(gui_render_params.use_adaptive_sampling));
}

static Value* set_use_adaptive_sampling(Value* const val)
{
	return set_bool(val, gui_render_params.use_adaptive_sampling);
}

////
// adaptiveThreshold
////

static Value* get_adaptive_threshold()
{
	return Float::intern(gui_render_params.adaptive_threshold);
}

static Value* set_adaptive_threshold(Value* const val)
{
	return set_float(val, gui_render_params.adaptive_threshold, 0.0f, 1.0f);
}

////
// adaptiveMinimumSamples
////

static Value* get_adaptive_minimum_samples()
{
	return Integer::intern(gui_render_params.adaptive_min_samples);
}

static Value* set_adaptive_minimum_samples(Value* const val)
{
	return set_int(val, gui_render_params.adaptive_min_samples, 0);
}

////
// sampleClampDirect
////

static Value* get_sample_clamp_direct()
{
	return Float::intern(gui_render_params.clamp_direct);
}

static Value* set_sample_clamp_direct(Value* const val)
{
	return set_float(val, gui_render_params.clamp_direct, 0.0f);
}

////
// useClampDirect
////

static Value* get_use_clamp_direct()
{
	return Integer::intern(static_cast<int>(gui_render_params.use_clamp_direct));
}

static Value* set_use_clamp_direct(Value* const val)
{
	return set_bool(val, gui_render_params.use_clamp_direct);
}

////
// sampleClampIndirect
////

static Value* get_sample_clamp_indirect()
{
	return Float::intern(gui_render_params.clamp_indirect);
}

static Value* set_sample_clamp_indirect(Value* const val)
{
	return set_float(val, gui_render_params.clamp_indirect, 0.0f);
}

////
// useClampIndirect
////

static Value* get_use_clamp_indirect()
{
	return Integer::intern(static_cast<int>(gui_render_params.use_clamp_indirect));
}

static Value* set_use_clamp_indirect(Value* const val)
{
	return set_bool(val, gui_render_params.use_clamp_indirect);
}


////
// randSeed
////

static Value* get_rand_seed()
{
	return Integer::intern(gui_render_params.rand_seed);
}

static Value* set_rand_seed(Value* const val)
{
	const int int_val = val->to_int();
	gui_render_params.rand_seed = int_val;
	return Integer::intern(gui_render_params.rand_seed);
}

////
// sampleVolStepRate
////

static Value* get_vol_step_rate()
{
	return Float::intern(gui_render_params.vol_step_rate);
}

static Value* set_vol_step_rate(Value* const val)
{
	return set_float(val, gui_render_params.vol_step_rate, 0.001f);
}

////
// sampleVolMaxSteps
////

static Value* get_vol_max_steps()
{
	return Integer::intern(gui_render_params.vol_max_steps);
}

static Value* set_vol_max_steps(Value* const val)
{
	return set_int(val, gui_render_params.vol_max_steps, 1);
}

////
// skyIntensity
////

static Value* get_sky_intensity()
{
	return Float::intern(gui_render_params.bg_intensity);
}

static Value* set_sky_intensity(Value* const val)
{
	return set_float(val, gui_render_params.bg_intensity, 0.0f);
}

////
// misMapSize
////

static Value* get_mis_map_size()
{
	return Integer::intern(gui_render_params.mis_map_size);
}

static Value* set_mis_map_size(Value* const val)
{
	return set_int(val, gui_render_params.mis_map_size, 1);
}

////
// pointLightSize
////

static Value* get_point_light_size()
{
	return Float::intern(gui_render_params.point_light_size);
}

static Value* set_point_light_size(Value* const val)
{
	return set_float(val, gui_render_params.point_light_size, 0.0f);
}

////
// texmapBakeWidth
////

static Value* get_texmap_bake_width()
{
	return Integer::intern(gui_render_params.texmap_bake_width);
}

static Value* set_texmap_bake_width(Value* const val)
{
	return set_int(val, gui_render_params.texmap_bake_width, 1);
}

////
// texmapBakeHeight
////

static Value* get_texmap_bake_height()
{
	return Integer::intern(gui_render_params.texmap_bake_height);
}

static Value* set_texmap_bake_height(Value* const val)
{
	return set_int(val, gui_render_params.texmap_bake_height, 1);
}

////
// deformBlurSamples
////

static Value* get_deform_blur_samples()
{
	return Integer::intern(gui_render_params.deform_blur_samples);
}

static Value* set_deform_blur_samples(Value* const val)
{
	return set_int(val, gui_render_params.deform_blur_samples, 1);
}

////
// lightpathMaxBounce
////

static Value* get_lp_max_bounce()
{
	return Integer::intern(gui_render_params.lp_max_bounce);
}

static Value* set_lp_max_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_max_bounce, 1);
}

////
// lightpathMinBounce
////

static Value* get_lp_min_bounce()
{
	return Integer::intern(gui_render_params.lp_min_bounce);
}

static Value* set_lp_min_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_min_bounce, 1);
}

////
// lightpathMaxDiffuseBounce
////

static Value* get_lp_diffuse_bounce()
{
	return Integer::intern(gui_render_params.lp_diffuse_bounce);
}

static Value* set_lp_diffuse_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_diffuse_bounce, 1);
}

////
// lightpathMaxGlossyBounce
////

static Value* get_lp_glossy_bounce()
{
	return Integer::intern(gui_render_params.lp_glossy_bounce);
}

static Value* set_lp_glossy_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_glossy_bounce, 1);
}

////
// lightpathMaxTransmissionBounce
////

static Value* get_lp_transmission_bounce()
{
	return Integer::intern(gui_render_params.lp_transmission_bounce);
}

static Value* set_lp_transmission_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_transmission_bounce, 1);
}

////
// lightpathMaxTransparentBounce
////

static Value* get_lp_transparent_bounce()
{
	return Integer::intern(gui_render_params.lp_transparent_bounce);
}

static Value* set_lp_transparent_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_transparent_bounce, 1);
}

////
// lightpathMaxVolumeBounce
////

static Value* get_lp_volume_bounce()
{
	return Integer::intern(gui_render_params.lp_volume_bounce);
}

static Value* set_lp_volume_bounce(Value* const val)
{
	return set_int(val, gui_render_params.lp_volume_bounce, 1);
}

////
// filmFilterType
////

static Value* get_film_filter_type()
{
	return Integer::intern(static_cast<int>(gui_render_params.filter_type));
}

static Value* set_film_filter_type(Value* const val)
{
	return set_enum<FilmFilterType, FilmFilterType::COUNT>(val, gui_render_params.filter_type);
}

////
// filmFilterSize
////

static Value* get_film_filter_size()
{
	return Float::intern(gui_render_params.filter_size);
}

static Value* set_film_filter_size(Value* const val)
{
	return set_float(val, gui_render_params.filter_size, 0.01f);
}

////
// filmTransparentSky
////

static Value* get_film_transparent_sky()
{
	return Integer::intern(static_cast<int>(gui_render_params.use_transparent_sky));
}

static Value* set_film_transparent_sky(Value* const val)
{
	return set_bool(val, gui_render_params.use_transparent_sky);
}

////
// filmExposure
////

static Value* get_film_exposure()
{
	return Float::intern(gui_render_params.exposure_multiplier);
}

static Value* set_film_exposure(Value* const val)
{
	return set_float(val, gui_render_params.exposure_multiplier, 0.0f);
}

/////
// renderThreads
/////

static Value* get_render_threads()
{
	return Integer::intern(gui_render_params.cpu_threads);
}

static Value* set_render_threads(Value* const val)
{
	return set_int(val, gui_render_params.cpu_threads, 0);
}

////
// tileWidthCpu
////

static Value* get_tile_width()
{
	return Integer::intern(gui_render_params.tile_width);
}

static Value* set_tile_width(Value* const val)
{
	const int int_val = val->to_int();
	if (int_val <= 0) {
		return Integer::intern(gui_render_params.tile_width);
	}

	gui_render_params.tile_width = int_val;
	return val;
}

////
// tileHeightCpu
////

static Value* get_tile_height()
{
	return Integer::intern(gui_render_params.tile_height);
}

static Value* set_tile_height(Value* const val)
{
	const int int_val = val->to_int();
	if (int_val <= 0) {
		return Integer::intern(gui_render_params.tile_height);
	}

	gui_render_params.tile_height = int_val;
	return val;
}

////
// useProgressiveRefine
////

static Value* get_progressive_refine()
{
	return Integer::intern(static_cast<int>(gui_render_params.use_progressive_refine));
}

static Value* set_progressive_refine(Value* const val)
{
	return set_bool(val, gui_render_params.use_progressive_refine);
}

////
// stereoMode
////

static Value* get_stereo_mode()
{
	return Integer::intern(static_cast<int>(gui_render_params.stereo_type));
}

static Value* set_stereo_mode(Value* const val)
{
	const int int_val = val->to_int();
	if (int_val < 0 || int_val >= static_cast<int>(StereoscopyType::TYPE_COUNT)) {
		return Integer::intern(static_cast<int>(gui_render_params.stereo_type));
	}

	gui_render_params.stereo_type = static_cast<StereoscopyType>(int_val);
	return val;
}

////
// stereoInterocularDistance
////

static Value* get_interocular_dist()
{
	return Float::intern(gui_render_params.interocular_distance);
}

static Value* set_interocular_dist(Value* const val)
{
	const float float_val = val->to_float();
	if (float_val < 0.0f) {
		return Float::intern(gui_render_params.interocular_distance);
	}

	gui_render_params.interocular_distance = float_val;
	return val;
}

////
// stereoConvergenceDistance
////

static Value* get_stereo_converge_dist()
{
	return Float::intern(gui_render_params.convergence_distance);
}

static Value* set_stereo_converge_dist(Value* const val)
{
	const float float_val = val->to_float();
	if (float_val < 0.0f) {
		return Float::intern(gui_render_params.convergence_distance);
	}

	gui_render_params.convergence_distance = float_val;
	return val;
}

////
// stereoSwapEyes
////

static Value* get_stereo_swap_eyes()
{
	return Integer::intern(static_cast<int>(gui_render_params.stereo_swap_eyes));
}

static Value* set_stereo_swap_eyes(Value* const val)
{
	return set_bool(val, gui_render_params.stereo_swap_eyes);
}

////
// stereoAnaglyphMode
////

static Value* get_stereo_anaglyph_mode()
{
	return Integer::intern(static_cast<int>(gui_render_params.anaglyph_type));
}

static Value* set_stereo_anaglyph_mode(Value* const val)
{
	const int int_val = val->to_int();
	if (int_val < 0 || int_val >= static_cast<int>(AnaglyphType::TYPE_COUNT)) {
		return Integer::intern(static_cast<int>(gui_render_params.anaglyph_type));
	}

	gui_render_params.anaglyph_type = static_cast<AnaglyphType>(int_val);
	return val;
}

/////
// passMistNear
/////

static Value* get_pass_mist_near()
{
	return Float::intern(gui_render_params.mist_near);
}

static Value* set_pass_mist_near(Value* const val)
{
	const float float_val = val->to_float();
	if (float_val <= 0.0f) {
		return Float::intern(gui_render_params.mist_near);
	}

	gui_render_params.mist_near = float_val;
	return val;
}

/////
// passMistDepth
/////

static Value* get_pass_mist_depth()
{
	return Float::intern(gui_render_params.mist_depth);
}

static Value* set_pass_mist_depth(Value* const val)
{
	const float float_val = val->to_float();
	if (float_val <= 0.0f) {
		return Float::intern(gui_render_params.mist_depth);
	}

	gui_render_params.mist_depth = float_val;
	return val;
}

/////
// passMistExponent
/////

static Value* get_pass_mist_exp()
{
	return Float::intern(gui_render_params.mist_exponent);
}

static Value* set_pass_mist_exp(Value* const val)
{
	const float float_val = val->to_float();
	if (float_val <= 0.0f) {
		return Float::intern(gui_render_params.mist_exponent);
	}

	gui_render_params.mist_exponent = float_val;
	return val;
}

void register_maxscript_globals()
{
	if (globals_registered) {
		return;
	}

	define_struct_global(L"renderDevice", L"cyclesRender", get_render_device, set_render_device);
	define_struct_global(L"cudaDevice", L"cyclesRender", get_cuda_device, set_cuda_device);
	define_struct_global(L"cudaWithCPU", L"cyclesRender", get_cuda_with_cpu, set_cuda_with_cpu);
	define_struct_global(L"optixDevice", L"cyclesRender", get_optix_device, set_optix_device);
	define_struct_global(L"previewSamples", L"cyclesRender", get_preview_samples, set_preview_samples);

	define_struct_global(L"samples", L"cyclesRender", get_samples, set_samples);
	define_struct_global(L"useAdaptiveSampling", L"cyclesRender", get_use_adaptive_sampling, set_use_adaptive_sampling);
	define_struct_global(L"adaptiveThreshold", L"cyclesRender", get_adaptive_threshold, set_adaptive_threshold);
	define_struct_global(L"adaptiveMinimumSamples", L"cyclesRender", get_adaptive_minimum_samples, set_adaptive_minimum_samples);
	define_struct_global(L"sampleClampDirect", L"cyclesRender", get_sample_clamp_direct, set_sample_clamp_direct);
	define_struct_global(L"useClampDirect", L"cyclesRender", get_use_clamp_direct, set_use_clamp_direct);
	define_struct_global(L"sampleClampIndirect", L"cyclesRender", get_sample_clamp_indirect, set_sample_clamp_indirect);
	define_struct_global(L"useClampIndirect", L"cyclesRender", get_use_clamp_indirect, set_use_clamp_indirect);
	define_struct_global(L"randSeed", L"cyclesRender", get_rand_seed, set_rand_seed);
	define_struct_global(L"sampleVolStepRate", L"cyclesRender", get_vol_step_rate, set_vol_step_rate);
	define_struct_global(L"sampleVolMaxSteps", L"cyclesRender", get_vol_max_steps, set_vol_max_steps);

	define_struct_global(L"misMapSize", L"cyclesRender", get_mis_map_size, set_mis_map_size);
	define_struct_global(L"skyIntensity", L"cyclesRender", get_sky_intensity, set_sky_intensity);
	define_struct_global(L"pointLightSize", L"cyclesRender", get_point_light_size, set_point_light_size);
	define_struct_global(L"texmapBakeWidth", L"cyclesRender", get_texmap_bake_width, set_texmap_bake_width);
	define_struct_global(L"texmapBakeHeight", L"cyclesRender", get_texmap_bake_height, set_texmap_bake_height);
	define_struct_global(L"deformBlurSamples", L"cyclesRender", get_deform_blur_samples, set_deform_blur_samples);

	define_struct_global(L"lightpathMaxBounce", L"cyclesRender", get_lp_max_bounce, set_lp_max_bounce);
	define_struct_global(L"lightpathMinBounce", L"cyclesRender", get_lp_min_bounce, set_lp_min_bounce);
	define_struct_global(L"lightpathMaxDiffuseBounce", L"cyclesRender", get_lp_diffuse_bounce, set_lp_diffuse_bounce);
	define_struct_global(L"lightpathMaxGlossyBounce", L"cyclesRender", get_lp_glossy_bounce, set_lp_glossy_bounce);
	define_struct_global(L"lightpathMaxTransmissionBounce", L"cyclesRender", get_lp_transmission_bounce, set_lp_transmission_bounce);
	define_struct_global(L"lightpathMaxTransparentBounce", L"cyclesRender", get_lp_transparent_bounce, set_lp_transparent_bounce);
	define_struct_global(L"lightpathMaxVolumeBounce", L"cyclesRender", get_lp_volume_bounce, set_lp_volume_bounce);

	define_struct_global(L"filmFilterType", L"cyclesRender", get_film_filter_type, set_film_filter_type);
	define_struct_global(L"filmFilterSize", L"cyclesRender", get_film_filter_size, set_film_filter_size);
	define_struct_global(L"filmTransparentSky", L"cyclesRender", get_film_transparent_sky, set_film_transparent_sky);
	define_struct_global(L"filmExposure", L"cyclesRender", get_film_exposure, set_film_exposure);
	
	define_struct_global(L"renderThreads", L"cyclesRender", get_render_threads, set_render_threads);
	define_struct_global(L"tileWidth", L"cyclesRender", get_tile_width, set_tile_width);
	define_struct_global(L"tileHeight", L"cyclesRender", get_tile_height, set_tile_height);
	define_struct_global(L"useProgressiveRefine", L"cyclesRender", get_progressive_refine, set_progressive_refine);

	define_struct_global(L"stereoMode", L"cyclesRender", get_stereo_mode, set_stereo_mode);
	define_struct_global(L"stereoInterocularDistance", L"cyclesRender", get_interocular_dist, set_interocular_dist);
	define_struct_global(L"stereoConvergenceDistance", L"cyclesRender", get_stereo_converge_dist, set_stereo_converge_dist);
	define_struct_global(L"stereoSwapEyes", L"cyclesRender", get_stereo_swap_eyes, set_stereo_swap_eyes);
	define_struct_global(L"stereoAnaglyphMode", L"cyclesRender", get_stereo_anaglyph_mode, set_stereo_anaglyph_mode);

	define_struct_global(L"passMistNear", L"cyclesRender", get_pass_mist_near, set_pass_mist_near);
	define_struct_global(L"passMistDepth", L"cyclesRender", get_pass_mist_depth, set_pass_mist_depth);
	define_struct_global(L"passMistExponent", L"cyclesRender", get_pass_mist_exp, set_pass_mist_exp);

	globals_registered = true;
}
