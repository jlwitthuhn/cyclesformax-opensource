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
 * @brief Defines tooltip strings.
 */

static const wchar_t* const TOOLTIP_CUDA_DEVICE{ L"Sets the desired CUDA render devices. A list of available devices can be found with the 'Devices...' button." };
static const wchar_t* const TOOLTIP_OPTIX_DEVICE{ L"Sets the desired OptiX render devices. A list of available devices can be found with the 'Devices...' button." };
static const wchar_t* const TOOLTIP_MTL_PREVIEW_SAMPLES{ L"Sets the number of samples to render in material editor thumbnails." };

static const wchar_t* const TOOLTIP_SAMPLING_SAMPLES{ L"Sets the number of samples per pixel to render. A higher value will produce an image with less noise, but will take longer to render. If adaptive sampling is selected, this is the maximum number of samples per pixel." };
static const wchar_t* const TOOLTIP_SAMPLING_ADAPTIVE{ L"Use adaptive sampling. This mode will render a different amount of samples per-pixel." };
static const wchar_t* const TOOLTIP_SAMPLING_ADAPTIVE_THRESHOLD{ L"Noise level at which to stop sampling. Lower values will cause more samples to be rendered for each pixel. Set to 0.0 to use a default value." };
static const wchar_t* const TOOLTIP_SAMPLING_ADAPTIVE_MINIMUM{ L"Minimum number of samples per pixel in adaptive mode. Set to 0 to use a default value." };
static const wchar_t* const TOOLTIP_SAMPLING_CLAMP_DIRECT{ L"Direct sample clamp. This sets a maximum intensity that may be sampled for the first bounce of a ray." };
static const wchar_t* const TOOLTIP_SAMPLING_CLAMP_INDIRECT{ L"Indirect sample clamp. This sets a maximum intensity that may be sampled for bounces of a ray after the first bounce." };
static const wchar_t* const TOOLTIP_SAMPLING_RAND_SEED{ L"Sets the seed used to randomize noise patterns." };
static const wchar_t* const TOOLTIP_SAMPLING_VOL_MAX_STEPS{ L"Maximum number of steps to sample in a volume." };
static const wchar_t* const TOOLTIP_SAMPLING_VOL_STEP_RATE{ L"Volume step rate. Higher values will produce a more accurate image but will take longer to render." };

static const wchar_t* const TOOLTIP_TRANS_BG_INTENSITY_MUL{ L"Sets the brightness of the sky or environment map." };
static const wchar_t* const TOOLTIP_TRANS_MIS_MAP_SIZE{ L"Sets the size of the MIS map to be used." };
static const wchar_t* const TOOLTIP_TRANS_POINT_LIGHT_SIZE{ L"Sets the diameter of point lights." };
static const wchar_t* const TOOLTIP_TRANS_BITMAP_WIDTH{ L"Sets the default width of baked texmaps." };
static const wchar_t* const TOOLTIP_TRANS_BITMAP_HEIGHT{ L"Sets the default height of baked texmaps." };
static const wchar_t* const TOOLTIP_TRANS_DEFORM_BLUR_SAMPLES{ L"Sets the number of samples to use for deform motion blur both before and after the 'current' frame. The total number of samples for motion blur is double this number plus one." };

static const wchar_t* const TOOLTIP_LP_MAX_BOUNCE{ L"Sets the maximum number of times a ray may bounce." };
static const wchar_t* const TOOLTIP_LP_MIN_BOUNCE{ L"Sets the minimum number of times a ray must bounce." };
static const wchar_t* const TOOLTIP_LP_DIFFUSE_BOUNCE{ L"Sets the maximum number of times a ray may bounce on diffuse surfaces." };
static const wchar_t* const TOOLTIP_LP_GLOSSY_BOUNCE{ L"Sets the maximum number of times a ray may bounce on glossy surfaces." };
static const wchar_t* const TOOLTIP_LP_TRANSMISSION_BOUNCE{ L"Sets the maximum number of times a ray may bounce through transmission surfaces." };
static const wchar_t* const TOOLTIP_LP_TRANSPARENT_BOUNCE{ L"Sets the maximum number of times a ray may bounce through transparent surfaces." };
static const wchar_t* const TOOLTIP_LP_VOLUME_BOUNCE{ L"Sets the maximum number of times a ray may bounce through volume boundaries." };

static const wchar_t* const TOOLTIP_FILM_EXPOSURE{ L"Sets the exposure multiplier for this render." };
static const wchar_t* const TOOLTIP_FILTER_SIZE{ L"Sets the size of the filter function. Does not apply if 'Type' is set to 'Box'." };

static const wchar_t* const TOOLTIP_PERF_CPU_THREADS{ L"Sets the number of threads to use for CPU renders. Selecting 0 will automatically determine the number of threads." };
static const wchar_t* const TOOLTIP_PERF_TILE_WIDTH{ L"Sets the width of a render tile in pixels." };
static const wchar_t* const TOOLTIP_PERF_TILE_HEIGHT{ L"Sets the height of a render tile in pixels." };

static const wchar_t* const TOOLTIP_STEREO_INTEROCULAR_DISTANCE{ L"Distance between the two cameras" };
static const wchar_t* const TOOLTIP_STEREO_CONVERGENCE_DISTANCE{ L"Stereoscopic convergence distance" };

static const wchar_t* const TOOLTIP_PASSES_MIST_NEAR{ L"The distance from the camera where mist begins." };
static const wchar_t* const TOOLTIP_PASSES_MIST_DEPTH{ L"The length over which the mist value will increase from 0 to 1." };
static const wchar_t* const TOOLTIP_PASSES_MIST_EXPONENT{ L"The exponent to be applied to all mist values. With an exponent of 1.0, the mist value will grow linearly from 0 at 'near' to 1 at 'near' + 'depth'." };
