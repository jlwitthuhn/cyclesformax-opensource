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
 * @brief Defines Class_IDs that are known at compile time.
 */

#include <maxtypes.h>

// Cycles Plugin Class_IDs
static const Class_ID CYCLES_RENDERER_CLASS{ 0x3f47152, 0x6ba33ce3 };
static const Class_ID CYCLES_UNIFIED_RENDERER_CLASS{ 0x527211a1, 0x26d63adf };

static const Class_ID CYCLES_TAB_MAIN_CLASS{ 0x53d57d75, 0xdff1593 };
static const Class_ID CYCLES_TAB_ADVANCED_CLASS{ 0x35255f33, 0x1a45548c };
static const Class_ID CYCLES_TAB_ABOUT_CLASS{ 0x106e02bf, 0x39b0595d };
static const Class_ID CYCLES_TAB_DEBUG_CLASS{ 0x3db1718, 0x4b016b98 };

static const Class_ID CYCLES_CAMERA_PANORAMIC_CLASS{ 0x4a742011, 0x43ea4700 };

static const Class_ID CYCLES_MOD_PROPERTIES_CLASS{ 0xb9c56df, 0x4b35914 };

static const Class_ID CYCLES_TEXMAP_ENVIRONMENT_CLASS{ 0x274f32e4, 0x19eb79b2 };
static const Class_ID CYCLES_TEXMAP_FILTER_CLASS{ 0xfa132f, 0x1aa55633 };
static const Class_ID CYCLES_TEXMAP_SKY_CLASS{ 0x96c263e, 0x4d082a1a };

static const Class_ID CYCLES_MAT_SHADER_CLASS{ 0x441b69b5, 0x3f26dcc };
static const Class_ID CYCLES_MAT_SHADER_GRAPH_08_CLASS{ 0x9a25b74, 0xcd75c6 };
static const Class_ID CYCLES_MAT_SHADER_GRAPH_16_CLASS{ 0x24866a68, 0x2fc42556 };
static const Class_ID CYCLES_MAT_SHADER_GRAPH_32_CLASS{ 0x35357de5, 0x5efe6ba6 };

static const Class_ID CYCLES_MAT_ADD_CLASS{ 0x22dc4492, 0x20301c07 };
static const Class_ID CYCLES_MAT_ANISOTROPIC_CLASS{ 0x64325756, 0xf493051 };
static const Class_ID CYCLES_MAT_DIFFUSE_CLASS{ 0x5b463cf5, 0x5ca35653 };
static const Class_ID CYCLES_MAT_EMISSION_CLASS{ 0x44094363, 0x1632161b };
static const Class_ID CYCLES_MAT_GLASS_CLASS{ 0x75cc1aa5, 0x703e3837 };
static const Class_ID CYCLES_MAT_GLOSSY_CLASS{ 0x25ca312f, 0x35b25364 };
static const Class_ID CYCLES_MAT_HOLDOUT_CLASS{ 0x25221769, 0x42ad6b89 };
static const Class_ID CYCLES_MAT_HAIR_CLASS{ 0x61506373, 0x2cb65354 };
static const Class_ID CYCLES_MAT_MIX_CLASS{ 0x79da2bc4, 0x5dd9620d };
static const Class_ID CYCLES_MAT_PRINCIPLED_CLASS{ 0x36060a9d, 0x21312a59 };
static const Class_ID CYCLES_MAT_REFRACTION_CLASS{ 0x14d507fc, 0x54d320dd };
static const Class_ID CYCLES_MAT_SUBSURFACE_CLASS{ 0x4d9435f1, 0x19f11dd9 };
static const Class_ID CYCLES_MAT_TOON_CLASS{ 0x7d264902, 0x7eb44bbc };
static const Class_ID CYCLES_MAT_TRANSLUCENT_CLASS{ 0x111a5696, 0xd387fbe };
static const Class_ID CYCLES_MAT_TRANSPARENT_CLASS{ 0x386b67f9, 0x7e414b80 };
static const Class_ID CYCLES_MAT_VELVET_CLASS{ 0x10cd7129, 0x1b2f7cbc };

static const Class_ID CYCLES_MAT_VOL_ABSORPTION_CLASS{ 0x18496f8e, 0x37222268 };
static const Class_ID CYCLES_MAT_VOL_SCATTER_CLASS{ 0x29f475f8, 0x72784241 };
static const Class_ID CYCLES_MAT_VOL_ADD_CLASS{ 0x73684071, 0xdfe708d };
static const Class_ID CYCLES_MAT_VOL_MIX_CLASS{ 0x57773a54, 0x375f1386 };

static const Class_ID CYCLES_RE_DIFFUSE_COLOR_CLASS{ 0x3ce1725e, 0x475d2870 };
static const Class_ID CYCLES_RE_DIFFUSE_DIRECT_CLASS{ 0xe5769df, 0x44a04f01 };
static const Class_ID CYCLES_RE_DIFFUSE_INDIRECT_CLASS{ 0x50ea5e9c, 0x64771b03 };
static const Class_ID CYCLES_RE_GLOSSY_COLOR_CLASS{ 0x172b4b02, 0x23ca0976 };
static const Class_ID CYCLES_RE_GLOSSY_DIRECT_CLASS{ 0x1ca93f41, 0x1816718 };
static const Class_ID CYCLES_RE_GLOSSY_INDIRECT_CLASS{ 0x547847d7, 0x11674d5e };
static const Class_ID CYCLES_RE_TRANSMISSION_COLOR_CLASS{ 0x4f2b4962, 0x4a6b71b1 };
static const Class_ID CYCLES_RE_TRANSMISSION_DIRECT_CLASS{ 0x40180367, 0x5f762cc0 };
static const Class_ID CYCLES_RE_TRANSMISSION_INDIRECT_CLASS{ 0x12d11a5f, 0x1736515d };
static const Class_ID CYCLES_RE_SUBSURFACE_COLOR_CLASS{ 0x3bd92222, 0x7ed9072f };
static const Class_ID CYCLES_RE_SUBSURFACE_DIRECT_CLASS{ 0x488c5f86, 0x3e013e46 };
static const Class_ID CYCLES_RE_SUBSURFACE_INDIRECT_CLASS{ 0x533b1e0e, 0x6ad2723b };
static const Class_ID CYCLES_RE_EMISSION_CLASS{ 0x4972698, 0x61192e77 };
static const Class_ID CYCLES_RE_ENVIRONMENT_CLASS{ 0x4f231c8f, 0x5ea376cd };
static const Class_ID CYCLES_RE_NORMAL_CLASS{ 0x42e3649a, 0x66b11fe8 };
static const Class_ID CYCLES_RE_UV_CLASS{ 0x989704b, 0x44446687 };
static const Class_ID CYCLES_RE_MOTION_CLASS{ 0x56ac3169, 0x17893851 };
static const Class_ID CYCLES_RE_DEPTH_CLASS{ 0x753c1a1e, 0x66e769ad };
static const Class_ID CYCLES_RE_MIST_CLASS{ 0x5f657194, 0x61636f91 };
static const Class_ID CYCLES_RE_CRYPTO_OBJECT_CLASS{ 0x3d9948d4, 0x70d04184 };
static const Class_ID CYCLES_RE_CRYPTO_MATERIAL_CLASS{ 0x58df7076, 0x62bf71a7 };
static const Class_ID CYCLES_RE_CRYPTO_ASSET_CLASS{ 0x390093e, 0x11722f8e };

static const Class_ID CYCLES_MAXSCRIPT_GUP_CLASS{ 0x317e15cd, 0x3f902566 };

static const Class_ID CYCLES_TEXMAP_SCONTEXT_CLASS{ 0x4f062c95, 0x2fb52f98 };

// IDs used internally by Max

// Misc IDs
static const Class_ID RAYTRACE_TAB_CLASS{ 0x4fa95e9b, 0x9a26e66 };

// Camera IDs
static const Class_ID PHYS_CAMERA_CLASS{ 0x46697218, 0x28E8008D };

// Light Class_IDs
static const Class_ID LIGHT_STD_OMNI_CLASS{ 0x1011, 0x0 };

// Material Class_IDs
static const Class_ID ARCHITECTURAL_MATERIAL_CLASS{ 0x13d11bbe, 0x691e3037 };
static const Class_ID MULTI_SUB_MATERIAL_CLASS{ 0x200, 0x0 };
static const Class_ID STANDARD_MATERIAL_CLASS{ 0x2, 0x0 };
static const Class_ID PHYSICAL_MATERIAL_CLASS{ 0x3d6b1cec, 0xdeadc001 };

// Particle flow
static const Class_ID PFLOW_SOURCE{ 0x50320c9a, 0x0 };
static const Class_ID PFLOW_PARTICLE_GROUP{ 0x74f93b02, 0x1eb34300 };

// Map Class_IDs
static const Class_ID BITMAP_MAP_CLASS{ 0x240, 0x0 };
static const Class_ID CELLULAR_MAP_CLASS{ 0xc90017a5, 0x111940bb };
static const Class_ID CHECKER_MAP_CLASS{ 0x200, 0x0 };
static const Class_ID COLOR_CORRECTION_MAP_CLASS{ 0x2d0, 0x0 };
static const Class_ID COMPOSITE_MAP_CLASS{ 0x280, 0x0 };
static const Class_ID DENT_MAP_CLASS{ 0x218, 0x0 };
static const Class_ID FALLOFF_MAP_CLASS{ 0x6ec3730c, 0x0 };
static const Class_ID GRADIENT_MAP_CLASS{ 0x270, 0x0 };
static const Class_ID GRADIENT_RAMP_MAP_CLASS{ 0x1dec5b86, 0x43383a51 };
static const Class_ID MARBLE_MAP_CLASS{ 0x210, 0x0 };
static const Class_ID MASK_MAP_CLASS{ 0x220, 0x0 };
static const Class_ID MIX_MAP_CLASS{ 0x230, 0x0 };
static const Class_ID NOISE_MAP_CLASS{ 0x234, 0x0 };
static const Class_ID NORMALBUMP_MAP_CLASS{ 0x243e22c6, 0x63f6a014 };
static const Class_ID OUTPUT_MAP_CLASS{ 0x2b0, 0x0 };
static const Class_ID PERLIN_MARBLE_MAP_CLASS{ 0x23ad0ae9, 0x158d7a88 };
static const Class_ID RGB_MULTIPLY_MAP_CLASS{ 0x290, 0x0 };
static const Class_ID RGB_TINT_MAP_CLASS{ 0x224, 0x0 };
static const Class_ID SHAPEMAP_MAP_CLASS{ 0x724a4566, 0x501e75c1 };
static const Class_ID SMOKE_MAP_CLASS{ 0xa845e7c, 0x0 };
static const Class_ID SPECKLE_MAP_CLASS{ 0x62c32b8a, 0x0 };
static const Class_ID SPLAT_MAP_CLASS{ 0x90b04f9, 0x0 };
static const Class_ID STUCCO_MAP_CLASS{ 0x9312fbe, 0x0 };
static const Class_ID SWIRL_MAP_CLASS{ 0x72c8577f, 0x39a00a1b };
static const Class_ID TEXTMAP_MAP_CLASS{ 0x5b823d4f, 0x3908364a };
static const Class_ID TILES_MAP_CLASS{ 0x64035fb9, 0x69664cdc };
static const Class_ID VECTORMAP_MAP_CLASS{ 0x93a92749, 0x6b8d470a };
static const Class_ID WAVES_MAP_CLASS{ 0x7712634e, 0x0 };
static const Class_ID WOOD_MAP_CLASS{ 0x214, 0x0 };
static const Class_ID COLOR_MAP_CLASS{ 0x139f22c6, 0x13f6a914 };
static const Class_ID PHYS_SKY_MAP_CLASS{ 0x223349b4, 0x562a7d8b };

// Class IDs for third-party plugins
static const Class_ID TYFLOW_BASE{ 0x31322891, 0x70f5b9ca};

static const Class_ID FSTORM_LIGHT_CLASS{ 0x241e7f26, 0x51b14ed4 };
static const Class_ID FSTORM_SUN_CLASS{ 0x30260fdf, 0x2ec940bb };
