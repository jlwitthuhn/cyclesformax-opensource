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
 
#include "rend_params.h"

#include <array>
#include <map>
#include <vector>

#include <boost/optional.hpp>

#include "util_cycles_device.h"

static_assert(sizeof(wchar_t) == 2, "VS 2015 compatible type sizes required");
static_assert(sizeof(int) == 4, "VS 2015 compatible type sizes required");
static_assert(sizeof(float) == 4, "VS 2015 compatible type sizes required");
static_assert(sizeof(bool) == 1, "VS 2015 compatible type sizes required");

template <typename T>
static boost::optional<T> get_chunk_value(const std::map<USHORT, std::vector<char>>& chunk_map, const USHORT chunk_id)
{
	if (chunk_map.count(chunk_id)) {
		const auto& chunk = chunk_map.at(chunk_id);
		if (chunk.size() == sizeof(T)) {
			return *(reinterpret_cast<const T*>(chunk.data()));
		}
	}
	return boost::optional<T>();
}

template <typename T>
static void load_chunk_value(const std::map<USHORT, std::vector<char>>& chunk_map, const USHORT chunk_id, T& target)
{
	if (const auto value = get_chunk_value<T>(chunk_map, chunk_id)) {
		target = static_cast<T>(value.get());
	}
}

template <typename T>
static void load_chunk_value_enum(const std::map<USHORT, std::vector<char>>& chunk_map, const USHORT chunk_id, T& target)
{
	int as_int = static_cast<int>(target);
	load_chunk_value<int>(chunk_map, chunk_id, as_int);
	target = static_cast<T>(as_int);
}

CyclesRenderParams::CyclesRenderParams()
{
	std_render_hidden = false;

	in_mtl_edit = false;

	frame_t = 0;
}

void CyclesRenderParams::SetDefaults()
{
	CyclesRenderParams default_params;

	render_device = default_params.render_device;
	cuda_with_cpu = default_params.cuda_with_cpu;
	cuda_device = default_params.cuda_device;
	optix_device = default_params.optix_device;
	mtl_preview_samples = default_params.mtl_preview_samples;

	samples = default_params.samples;
	clamp_direct = default_params.clamp_direct;
	use_clamp_direct = default_params.use_clamp_direct;
	clamp_indirect = default_params.clamp_indirect;
	use_clamp_indirect = default_params.use_clamp_indirect;
	rand_seed = default_params.rand_seed;
	animate_rand_seed = default_params.animate_rand_seed;
	vol_max_steps = default_params.vol_max_steps;
	vol_step_rate = default_params.vol_step_rate;
	use_adaptive_sampling = default_params.use_adaptive_sampling;
	adaptive_threshold = default_params.adaptive_threshold;
	adaptive_min_samples = default_params.adaptive_min_samples;

	bg_intensity = default_params.bg_intensity;
	mis_map_size = default_params.mis_map_size;
	point_light_size = default_params.point_light_size;
	texmap_bake_width = default_params.texmap_bake_width;
	texmap_bake_height = default_params.texmap_bake_height;
	deform_blur_samples = default_params.deform_blur_samples;

	lp_max_bounce = default_params.lp_max_bounce;
	lp_min_bounce = default_params.lp_min_bounce;
	lp_diffuse_bounce = default_params.lp_diffuse_bounce;
	lp_glossy_bounce = default_params.lp_glossy_bounce;
	lp_transmission_bounce = default_params.lp_transmission_bounce;
	lp_transparent_bounce = default_params.lp_transparent_bounce;
	lp_volume_bounce = default_params.lp_volume_bounce;

	use_transparent_sky = default_params.use_transparent_sky;
	exposure_multiplier = default_params.exposure_multiplier;
	filter_type = default_params.filter_type;
	filter_size = default_params.filter_size;

	tile_width = default_params.tile_width;
	tile_height = default_params.tile_height;
	use_progressive_refine = default_params.use_progressive_refine;
	cpu_threads = default_params.cpu_threads;

	stereo_type = default_params.stereo_type;
	interocular_distance = default_params.interocular_distance;
	convergence_distance = default_params.convergence_distance;
	stereo_swap_eyes = default_params.stereo_swap_eyes;
	anaglyph_type = default_params.anaglyph_type;

	mist_near = default_params.mist_near;
	mist_depth = default_params.mist_depth;
	mist_exponent = default_params.mist_exponent;

	diagnostic_log = default_params.diagnostic_log;
	debug_multi_cuda = default_params.debug_multi_cuda;
}

void CyclesRenderParams::SetMtlEditParams()
{
	if (in_mtl_edit) {
		const float preview_samples = mtl_preview_samples;
		const int bake_width = texmap_bake_width;
		const int bake_height = texmap_bake_height;
		SetDefaults();

		texmap_bake_width = bake_width;
		texmap_bake_height = bake_height;
		mtl_preview_samples = preview_samples;
		vol_step_rate = 0.5f;
		samples = preview_samples;

		use_progressive_refine = false;
		tile_width = 60;
		tile_height = 60;
	}
}

IOResult CyclesRenderParams::LoadChunks(ILoad& iload)
{
	// Begin by loading all chunks into a [chunk id] -> [vector<char>] map
	std::map<USHORT, std::vector<char>> chunk_map;

	while (iload.OpenChunk() == IO_OK) {
		ULONG nb;
		std::vector<char> chunk_vector(iload.CurChunkLength(), '\0');
		iload.Read(chunk_vector.data(), static_cast<ULONG>(chunk_vector.size()), &nb);
		chunk_map[iload.CurChunkID()] = std::move(chunk_vector);
		iload.CloseChunk();
	}

	int file_compat_level = 0;
	load_chunk_value<int>(chunk_map, COMPATIBILITY_LEVEL_CHUNK, file_compat_level);

	if (file_compat_level > PARAMS_COMPATIBILITY_LEVEL) {
		// End without applying settings as we are unsure about what chunks mean
		return IO_OK;
	}

	// Legacy parameters
	if (chunk_map.count(CUDA_DEVICE_032_CHUNK) > 0) {
		wchar_t* const cuda_device_ptr = reinterpret_cast<wchar_t*>(chunk_map[CUDA_DEVICE_032_CHUNK].data());
		std::array<wchar_t, 32> cuda_device_buffer;
		cuda_device_buffer.fill(L'\0');
		for (auto i = 0; i < (cuda_device_buffer.size() - 1); i++) {
			cuda_device_buffer[i] = cuda_device_ptr[i];
			if (cuda_device_ptr[i] == L'\0') {
				break;
			}
		}
		cuda_device_buffer[cuda_device_buffer.size() - 1] = L'\0';
		cuda_device = std::wstring(cuda_device_buffer.data());
	}

	// Misc
	load_chunk_value_enum<RenderDevice>(chunk_map, RENDER_DEVICE_CHUNK, render_device);
	load_chunk_value<int>(chunk_map, MTL_PREVIEW_SAMPLES_CHUNK, mtl_preview_samples);

	if (chunk_map.count(CUDA_DEVICE_256_CHUNK) > 0) {
		wchar_t* const cuda_device_ptr = reinterpret_cast<wchar_t*>(chunk_map[CUDA_DEVICE_256_CHUNK].data());
		std::array<wchar_t, 256> cuda_device_buffer;
		cuda_device_buffer.fill(L'\0');
		for (auto i = 0; i < (cuda_device_buffer.size() - 1); i++) {
			cuda_device_buffer[i] = cuda_device_ptr[i];
			if (cuda_device_ptr[i] == L'\0') {
				break;
			}
		}
		cuda_device_buffer[cuda_device_buffer.size() - 1] = L'\0';
		cuda_device = std::wstring(cuda_device_buffer.data());
	}

	load_chunk_value<bool>(chunk_map, CUDA_WITH_CPU_CHUNK, cuda_with_cpu);

	if (chunk_map.count(OPTIX_DEVICE_256_CHUNK) > 0) {
		wchar_t* const optix_device_ptr = reinterpret_cast<wchar_t*>(chunk_map[OPTIX_DEVICE_256_CHUNK].data());
		std::array<wchar_t, 256> optix_device_buffer;
		optix_device_buffer.fill(L'\0');
		for (int i = 0; i < (optix_device_buffer.size() - 1); i++) {
			optix_device_buffer[i] = optix_device_ptr[i];
			if (optix_device_ptr[i] == L'\0') {
				break;
			}
		}
		optix_device_buffer[optix_device_buffer.size() - 1] = L'\0';
		optix_device = std::wstring(optix_device_buffer.data());
	}

	// Sampling
	load_chunk_value<int>  (chunk_map, SAMPLES_CHUNK, samples);
	load_chunk_value<bool> (chunk_map, USE_ADAPTIVE_SAMPLING_CHUNK, use_adaptive_sampling);
	load_chunk_value<float>(chunk_map, ADAPTIVE_SAMPLING_THRESHOLD_CHUNK, adaptive_threshold);
	load_chunk_value<int>  (chunk_map, ADAPTIVE_SAMPLING_MINIMUM_CHUNK, adaptive_min_samples);
	load_chunk_value<float>(chunk_map, CLAMP_DIRECT_CHUNK, clamp_direct);
	load_chunk_value<bool> (chunk_map, USE_CLAMP_DIRECT_CHUNK, use_clamp_direct);
	load_chunk_value<float>(chunk_map, CLAMP_INDIRECT_CHUNK, clamp_indirect);
	load_chunk_value<bool> (chunk_map, USE_CLAMP_INDIRECT_CHUNK, use_clamp_indirect);
	load_chunk_value<int>  (chunk_map, RAND_SEED_CHUNK, rand_seed);
	load_chunk_value<bool> (chunk_map, ANIMATE_RAND_SEED_CHUNK, animate_rand_seed);
	load_chunk_value<int>  (chunk_map, VOL_MAX_STEPS_CHUNK, vol_max_steps);
	load_chunk_value<float>(chunk_map, VOL_STEP_RATE_CHUNK, vol_step_rate);

	// Translation
	load_chunk_value<float>(chunk_map, BG_INTENSITY_CHUNK, bg_intensity);
	load_chunk_value<float>(chunk_map, POINT_LIGHT_SIZE_CHUNK, point_light_size);
	load_chunk_value<int>  (chunk_map, DEFORM_BLUR_SAMPLES_CHUNK, deform_blur_samples);
	if (file_compat_level >= 2) {
		// If compat level is below 2, this might be corrupt
		load_chunk_value<int>(chunk_map, MIS_MAP_SIZE_CHUNK, mis_map_size);
		load_chunk_value<int>(chunk_map, TEXMAP_BAKE_WIDTH_CHUNK, texmap_bake_width);
		load_chunk_value<int>(chunk_map, TEXMAP_BAKE_HEIGHT_CHUNK, texmap_bake_height);
	}

	// Light path
	load_chunk_value<int>(chunk_map, LP_MAX_BOUNCE_CHUNK, lp_max_bounce);
	load_chunk_value<int>(chunk_map, LP_MIN_BOUNCE_CHUNK, lp_min_bounce);
	load_chunk_value<int>(chunk_map, LP_DIFFUSE_BOUNCE_CHUNK, lp_diffuse_bounce);
	load_chunk_value<int>(chunk_map, LP_GLOSSY_BOUNCE_CHUNK, lp_glossy_bounce);
	load_chunk_value<int>(chunk_map, LP_TRANSMISSION_BOUNCE_CHUNK, lp_transmission_bounce);
	load_chunk_value<int>(chunk_map, LP_VOLUME_BOUNCE_CHUNK, lp_volume_bounce);
	load_chunk_value<int>(chunk_map, LP_TRANSPARENT_BOUNCE_CHUNK, lp_transparent_bounce);

	// Film
	load_chunk_value<bool> (chunk_map, TRANSPARENT_SKY_CHUNK, use_transparent_sky);
	load_chunk_value<float>(chunk_map, EXPOSURE_CHUNK, exposure_multiplier);
	load_chunk_value_enum<FilmFilterType>(chunk_map, FILM_FILTER_TYPE_CHUNK, filter_type);
	load_chunk_value<float>(chunk_map, FILM_FILTER_SIZE_CHUNK, filter_size);

	// Performance
	load_chunk_value<int> (chunk_map, TILE_WIDTH_CHUNK, tile_width);
	load_chunk_value<int> (chunk_map, TILE_HEIGHT_CHUNK, tile_height);
	load_chunk_value<bool>(chunk_map, USE_PROGRESSIVE_REFINE_CHUNK, use_progressive_refine);
	load_chunk_value<int> (chunk_map, PERF_CPU_THREADS_CHUNK, cpu_threads);

	// Stereoscopy
	load_chunk_value_enum<StereoscopyType>(chunk_map, STEREO_TYPE_CHUNK, stereo_type);
	load_chunk_value<float>(chunk_map, STEREO_INTEROCULAR_DIST_CHUNK, interocular_distance);
	load_chunk_value<float>(chunk_map, STEREO_CONVERGENCE_DIST_CHUNK, convergence_distance);
	load_chunk_value<bool> (chunk_map, STEREO_SWAP_EYES_CHUNK, stereo_swap_eyes);
	load_chunk_value_enum<AnaglyphType>(chunk_map, STEREO_ANAGLYPH_TYPE_CHUNK, anaglyph_type);

	// Render passes
	if (file_compat_level >= 2) {
		// If compat level is below 2, this might be corrupt
		load_chunk_value<float>(chunk_map, PASS_MIST_NEAR_CHUNK, mist_near);
		load_chunk_value<float>(chunk_map, PASS_MIST_DEPTH_CHUNK, mist_depth);
		load_chunk_value<float>(chunk_map, PASS_MIST_EXPONENT_CHUNK, mist_exponent);
	}

	// Defaults for new parameters - special case
	// Each line here should have an explanation of what it does

	// Beta 3.03 - Reconstruction filter options were added.
	// The new default is different from the old behavior.
	// If a file does not have a filter type set, use the old (box) filter instead of the default
	if (chunk_map.count(FILM_FILTER_TYPE_CHUNK) == 0) {
		filter_type = FilmFilterType::BOX;
	}

	return IO_OK;
}

IOResult CyclesRenderParams::SaveChunks(ISave& isave) const
{
	ULONG nb;

	isave.BeginChunk(COMPATIBILITY_LEVEL_CHUNK);
	isave.Write(&PARAMS_COMPATIBILITY_LEVEL, sizeof(int), &nb);
	isave.EndChunk();

	isave.BeginChunk(RENDER_DEVICE_CHUNK);
	const int render_device_int = static_cast<int>(render_device);
	isave.Write(&render_device_int, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(MTL_PREVIEW_SAMPLES_CHUNK);
	isave.Write(&mtl_preview_samples, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(CUDA_WITH_CPU_CHUNK);
	isave.Write(&cuda_with_cpu, sizeof(bool), &nb);
	isave.EndChunk();

	isave.BeginChunk(RAND_SEED_CHUNK);
	isave.Write(&rand_seed, sizeof(int), &nb);
	isave.EndChunk();

	{
		std::array<wchar_t, 256> buffer;
		buffer.fill(L'\0');
		for (int i = 0; i < (buffer.size() - 1) && i < cuda_device.size(); i++) {
			buffer[i] = cuda_device[i];
		}
		isave.BeginChunk(CUDA_DEVICE_256_CHUNK);
		isave.Write(buffer.data(), static_cast<ULONG>(buffer.size() * sizeof(wchar_t)), &nb);
		isave.EndChunk();
	}

	{
		std::array<wchar_t, 256> buffer;
		buffer.fill(L'\0');
		for (int i = 0; i < (buffer.size() - 1) && i < optix_device.size(); i++) {
			buffer[i] = optix_device[i];
		}
		isave.BeginChunk(OPTIX_DEVICE_256_CHUNK);
		isave.Write(buffer.data(), static_cast<ULONG>(buffer.size() * sizeof(wchar_t)), &nb);
		isave.EndChunk();
	}

	isave.BeginChunk(SAMPLES_CHUNK);
	isave.Write(&samples, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(CLAMP_DIRECT_CHUNK);
	isave.Write(&clamp_direct, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(USE_CLAMP_DIRECT_CHUNK);
	isave.Write(&use_clamp_direct, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(CLAMP_INDIRECT_CHUNK);
	isave.Write(&clamp_indirect, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(USE_CLAMP_INDIRECT_CHUNK);
	isave.Write(&use_clamp_indirect, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(ANIMATE_RAND_SEED_CHUNK);
	isave.Write(&animate_rand_seed, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(VOL_MAX_STEPS_CHUNK);
	isave.Write(&vol_max_steps, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(VOL_STEP_RATE_CHUNK);
	isave.Write(&vol_step_rate, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(USE_ADAPTIVE_SAMPLING_CHUNK);
	isave.Write(&use_adaptive_sampling, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(ADAPTIVE_SAMPLING_THRESHOLD_CHUNK);
	isave.Write(&adaptive_threshold, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(ADAPTIVE_SAMPLING_MINIMUM_CHUNK);
	isave.Write(&adaptive_min_samples, sizeof(int), &nb);
	isave.EndChunk();

	isave.BeginChunk(BG_INTENSITY_CHUNK);
	isave.Write(&bg_intensity, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(MIS_MAP_SIZE_CHUNK);
	isave.Write(&mis_map_size, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(POINT_LIGHT_SIZE_CHUNK);
	isave.Write(&point_light_size, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(TEXMAP_BAKE_WIDTH_CHUNK);
	isave.Write(&texmap_bake_width, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(TEXMAP_BAKE_HEIGHT_CHUNK);
	isave.Write(&texmap_bake_height, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(DEFORM_BLUR_SAMPLES_CHUNK);
	isave.Write(&deform_blur_samples, sizeof(int), &nb);
	isave.EndChunk();

	isave.BeginChunk(TRANSPARENT_SKY_CHUNK);
	isave.Write(&use_transparent_sky, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(EXPOSURE_CHUNK);
	isave.Write(&exposure_multiplier, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(FILM_FILTER_TYPE_CHUNK);
	const int filter_type_int = static_cast<int>(filter_type);
	isave.Write(&filter_type_int, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(FILM_FILTER_SIZE_CHUNK);
	isave.Write(&filter_size, sizeof(float), &nb);
	isave.EndChunk();

	isave.BeginChunk(TILE_WIDTH_CHUNK);
	isave.Write(&tile_width, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(TILE_HEIGHT_CHUNK);
	isave.Write(&tile_height, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(USE_PROGRESSIVE_REFINE_CHUNK);
	isave.Write(&use_progressive_refine, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(PERF_CPU_THREADS_CHUNK);
	isave.Write(&cpu_threads, sizeof(int), &nb);
	isave.EndChunk();


	isave.BeginChunk(LP_MAX_BOUNCE_CHUNK);
	isave.Write(&lp_max_bounce, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(LP_MIN_BOUNCE_CHUNK);
	isave.Write(&lp_min_bounce, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(LP_DIFFUSE_BOUNCE_CHUNK);
	isave.Write(&lp_diffuse_bounce, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(LP_GLOSSY_BOUNCE_CHUNK);
	isave.Write(&lp_glossy_bounce, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(LP_TRANSMISSION_BOUNCE_CHUNK);
	isave.Write(&lp_transmission_bounce, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(LP_VOLUME_BOUNCE_CHUNK);
	isave.Write(&lp_volume_bounce, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(LP_TRANSPARENT_BOUNCE_CHUNK);
	isave.Write(&lp_transparent_bounce, sizeof(int), &nb);
	isave.EndChunk();

	isave.BeginChunk(STEREO_TYPE_CHUNK);
	const int stereo_type_int = static_cast<int>(stereo_type);
	isave.Write(&stereo_type_int, sizeof(int), &nb);
	isave.EndChunk();
	isave.BeginChunk(STEREO_INTEROCULAR_DIST_CHUNK);
	isave.Write(&interocular_distance, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(STEREO_CONVERGENCE_DIST_CHUNK);
	isave.Write(&convergence_distance, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(STEREO_SWAP_EYES_CHUNK);
	isave.Write(&stereo_swap_eyes, sizeof(bool), &nb);
	isave.EndChunk();
	isave.BeginChunk(STEREO_ANAGLYPH_TYPE_CHUNK);
	const int stereo_anaglyph_type_int = static_cast<int>(anaglyph_type);
	isave.Write(&stereo_anaglyph_type_int, sizeof(int), &nb);
	isave.EndChunk();

	isave.BeginChunk(PASS_MIST_NEAR_CHUNK);
	isave.Write(&mist_near, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(PASS_MIST_DEPTH_CHUNK);
	isave.Write(&mist_depth, sizeof(float), &nb);
	isave.EndChunk();
	isave.BeginChunk(PASS_MIST_EXPONENT_CHUNK);
	isave.Write(&mist_exponent, sizeof(float), &nb);
	isave.EndChunk();

	return IO_OK;
}
