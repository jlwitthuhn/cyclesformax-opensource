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
 * @brief Defines class CyclesRenderParams.
 */

#include <string>

#include <ioapi.h>

#include "util_enums.h"
#include "util_simple_types.h"

/**
 * @brief Class responsible for holding, loading, and saving render parameters.
 */
class CyclesRenderParams {
public:

	//////
	// Cycles options
	//////

	// Uncategorized
	RenderDevice render_device = RenderDevice::CPU;
	bool cuda_with_cpu = false;
	std::wstring cuda_device = L"SINGLE";
	std::wstring optix_device = L"SINGLE";
	int mtl_preview_samples = 32;

	// Sampling
	int samples = 100;
	float clamp_direct = 10.0f;
	bool use_clamp_direct = false;
	float clamp_indirect = 10.0f;
	bool use_clamp_indirect = true;
	int rand_seed = 1234;
	bool animate_rand_seed = true;
	int vol_max_steps = 1024;
	float vol_step_rate = 1.0;
	bool use_adaptive_sampling = false;
	float adaptive_threshold = 0.0f;
	int adaptive_min_samples = 0;

	// Translation
	float bg_intensity = 1.0f;
	int mis_map_size = 2048;
	float point_light_size = 0.394f;
	int texmap_bake_width = 512;
	int texmap_bake_height = 512;
	int deform_blur_samples = 1;

	// Light path
	int lp_max_bounce = 7;
	int lp_min_bounce = 2;
	int lp_diffuse_bounce = 3;
	int lp_glossy_bounce = 6;
	int lp_transmission_bounce = 8;
	int lp_transparent_bounce = 8;
	int lp_volume_bounce = 4;

	// Film
	bool use_transparent_sky = false;
	float exposure_multiplier = 1.0f;
	FilmFilterType filter_type = FilmFilterType::BLACKMAN_HARRIS;
	float filter_size = 1.5f;

	// Performance
	int tile_width = 80;
	int tile_height = 80;
	bool use_progressive_refine = true;
	int cpu_threads = 0;

	// Stereoscopy
	StereoscopyType stereo_type = StereoscopyType::NONE;
	float interocular_distance = 1.6f;
	float convergence_distance = 48.0f;
	bool stereo_swap_eyes = false;
	AnaglyphType anaglyph_type = AnaglyphType::RED_CYAN;

	// Render elements
	float mist_near = 1.0f;
	float mist_depth = 100.0f;
	float mist_exponent = 2.0f;
	
	// Debug
	bool diagnostic_log = false;
	bool debug_multi_cuda = false;

	//////
	// Standard options
	//////

	// These options are configurable for all plugin renderers
	bool std_render_hidden;

	//////
	// Calculated params
	//////
	IntRect region;

	TimeValue frame_t;
	bool in_mtl_edit = false;

	CyclesRenderParams();

	void SetDefaults();
	void SetMtlEditParams();

	IOResult LoadChunks(ILoad& iload);
	IOResult SaveChunks(ISave& isave) const;

private:

	//////
	// Cycles option chunk IDs
	// If anything is changed, the compatibility level must be raised
	// Adding will not break compatibility
	//////
	// Compatibility level changes:
	//
	// Ignore the following potentially corrupt values when loading level 1
	//  - TEXMAP_BAKE_WIDTH_CHUNK
	//  - TEXMAP_BAKE_HEIGHT_CHUNK
	//  - MIS_MAP_SIZE_CHUNK
	//  - PASS_MIST_NEAR_CHUNK
	//  - PASS_MIST_DEPTH_CHUNK
	//  - PASS_MIST_EXPONENT_CHUNK
	static const int PARAMS_COMPATIBILITY_LEVEL = 2;
	static const USHORT COMPATIBILITY_LEVEL_CHUNK = 37;

	static const USHORT RENDER_DEVICE_CHUNK = 1001;
	static const USHORT MTL_PREVIEW_SAMPLES_CHUNK = 1003;
	static const USHORT RAND_SEED_CHUNK = 1004;
	static const USHORT CUDA_DEVICE_256_CHUNK = 1006;
	static const USHORT CUDA_WITH_CPU_CHUNK = 1007;
	static const USHORT OPTIX_DEVICE_256_CHUNK = 1008;

	static const USHORT SAMPLES_CHUNK = 2001;
	static const USHORT CLAMP_DIRECT_CHUNK = 2002;
	static const USHORT CLAMP_INDIRECT_CHUNK = 2003;
	static const USHORT ANIMATE_RAND_SEED_CHUNK = 2004;
	static const USHORT VOL_MAX_STEPS_CHUNK = 2006;
	static const USHORT USE_CLAMP_DIRECT_CHUNK = 2007;
	static const USHORT USE_CLAMP_INDIRECT_CHUNK = 2008;
	static const USHORT VOL_STEP_RATE_CHUNK = 2009;
	static const USHORT USE_ADAPTIVE_SAMPLING_CHUNK = 2010;
	static const USHORT ADAPTIVE_SAMPLING_THRESHOLD_CHUNK = 2011;
	static const USHORT ADAPTIVE_SAMPLING_MINIMUM_CHUNK = 2012;

	static const USHORT BG_INTENSITY_CHUNK = 1002;
	static const USHORT MIS_MAP_SIZE_CHUNK = 7003;
	static const USHORT POINT_LIGHT_SIZE_CHUNK = 7004;
	static const USHORT TEXMAP_BAKE_WIDTH_CHUNK = 7001;
	static const USHORT TEXMAP_BAKE_HEIGHT_CHUNK = 7002;
	static const USHORT DEFORM_BLUR_SAMPLES_CHUNK = 7005;

	static const USHORT TRANSPARENT_SKY_CHUNK = 3001;
	static const USHORT EXPOSURE_CHUNK = 3002;
	static const USHORT FILM_FILTER_TYPE_CHUNK = 3003;
	static const USHORT FILM_FILTER_SIZE_CHUNK = 3004;

	static const USHORT TILE_WIDTH_CHUNK = 4001;
	static const USHORT TILE_HEIGHT_CHUNK = 4002;
	static const USHORT USE_PROGRESSIVE_REFINE_CHUNK = 4003;
	static const USHORT PERF_CPU_THREADS_CHUNK = 4008;

	static const USHORT LP_MAX_BOUNCE_CHUNK = 5001;
	static const USHORT LP_MIN_BOUNCE_CHUNK = 5002;
	static const USHORT LP_DIFFUSE_BOUNCE_CHUNK = 5003;
	static const USHORT LP_GLOSSY_BOUNCE_CHUNK = 5004;
	static const USHORT LP_TRANSMISSION_BOUNCE_CHUNK = 5005;
	static const USHORT LP_VOLUME_BOUNCE_CHUNK = 5006;
	static const USHORT LP_TRANSPARENT_BOUNCE_CHUNK = 5007;

	static const USHORT STEREO_TYPE_CHUNK = 6001;
	static const USHORT STEREO_INTEROCULAR_DIST_CHUNK = 6002;
	static const USHORT STEREO_CONVERGENCE_DIST_CHUNK = 6003;
	static const USHORT STEREO_SWAP_EYES_CHUNK = 6004;
	static const USHORT STEREO_ANAGLYPH_TYPE_CHUNK = 6005;

	static const USHORT PASS_MIST_NEAR_CHUNK = 8001;
	static const USHORT PASS_MIST_DEPTH_CHUNK = 8002;
	static const USHORT PASS_MIST_EXPONENT_CHUNK = 8003;

	// Old parameters that are now read-only for compatibility
	static const USHORT CUDA_DEVICE_032_CHUNK = 1005;

	// Deprecated parameters, these must not be reused before a compatibility level bump
	static const USHORT _VOL_STEP_SIZE_CHUNK = 2005;
	static const USHORT _TILE_WIDTH_CPU_CHUNK = 4004;
	static const USHORT _TILE_HEIGHT_CPU_CHUNK = 4005;
	static const USHORT _TILE_WIDTH_GPU_CHUNK = 4006;
	static const USHORT _TILE_HEIGHT_GPU_CHUNK = 4007;
};
