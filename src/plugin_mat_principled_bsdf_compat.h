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

#include <ioapi.h>

#include "win_resource.h"

//#define COMPAT_206

// Pblock enum
enum class PrincipledBsdfPblocks {
	MAIN_PBLOCK_DEPRECATED,
	NORMAL_PBLOCK,
	CC_NORMAL_PBLOCK,
	MAIN_PBLOCK,
};

// Parameter enum
enum {
	// General
	param_dist, param_ior,
	param_base_color, param_base_color_map, param_base_color_map_on,
	param_metal, param_metal_map, param_metal_map_on,
	param_roughness, param_roughness_map, param_roughness_map_on,
	param_sheen, param_sheen_map, param_sheen_map_on,
	param_sheen_tint, param_sheen_tint_map, param_sheen_tint_map_on,
	param_transmission, param_transmission_map, param_transmission_map_on,
	param_trans_roughness, param_trans_roughness_map, param_trans_roughness_map_on,
	// Specular
	param_specular, param_specular_map, param_specular_map_on,
	param_specular_tint, param_specular_tint_map, param_specular_tint_map_on,
	param_anisotropic, param_anisotropic_map, param_anisotropic_map_on,
	param_anisotropic_rotation, param_anisotropic_rotation_map, param_anisotropic_rotation_map_on,
	// Subsurface
	param_subsurface_method,
	param_subsurface, param_subsurface_map, param_subsurface_map_on,
	param_subsurface_color, param_subsurface_color_map, param_subsurface_color_map_on,
	param_subsurface_radius_x, param_subsurface_radius_y, param_subsurface_radius_z,
	// Clearcoat
	param_clearcoat, param_clearcoat_map, param_clearcoat_map_on,
	param_clearcoat_roughness, param_clearcoat_roughness_map, param_clearcoat_roughness_map_on,
	// Other
	param_emission, param_emission_map, param_emission_map_on,
	param_alpha, param_alpha_map, param_alpha_map_on,
	// Internal
	param_ignore_original_pblock
};

// Subtex enum
enum {
	subtex_color, subtex_metal, subtex_roughness, subtex_sheen, subtex_sheen_tint, subtex_transmission, subtex_trans_roughness,
	subtex_specular, subtex_specular_tint, subtex_anisotropic, subtex_anisotropic_rotation,
	subtex_subsurface, subtex_subsurface_color,
	subtex_clearcoat, subtex_clearcoat_roughness,
	subtex_emission,
	subtex_alpha,
	subtex_nparam_strength,
	subtex_nparam_color,
	subtex_cc_nparam_strength,
	subtex_cc_nparam_color,
};

enum class Deprecated {
	// General
	param_dist, param_ior,
	param_base_color, param_base_color_map, param_base_color_map_on,
	param_metal, param_metal_map, param_metal_map_on,
	param_roughness, param_roughness_map, param_roughness_map_on,
	param_sheen, param_sheen_map, param_sheen_map_on,
	param_sheen_tint, param_sheen_tint_map, param_sheen_tint_map_on,
	param_transmission, param_transmission_map, param_transmission_map_on,
	param_trans_roughness, param_trans_roughness_map, param_trans_roughness_map_on,
	// Specular
	param_specular, param_specular_map, param_specular_map_on,
	param_specular_tint, param_specular_tint_map, param_specular_tint_map_on,
	param_anisotropic, param_anisotropic_map, param_anisotropic_map_on,
	param_anisotropic_rotation, param_anisotropic_rotation_map, param_anisotropic_rotation_map_on,
	// Subsurface
#ifndef COMPAT_206
	param_subsurface_method,
#endif
	param_subsurface, param_subsurface_map, param_subsurface_map_on,
	param_subsurface_color, param_subsurface_color_map, param_subsurface_color_map_on,
	param_subsurface_radius_x, param_subsurface_radius_y, param_subsurface_radius_z,
	// Clearcoat
	param_clearcoat, param_clearcoat_map, param_clearcoat_map_on,
	param_clearcoat_roughness, param_clearcoat_roughness_map, param_clearcoat_roughness_map_on,
	// Other
#ifndef COMPAT_206
	param_emission, param_emission_map, param_emission_map_on,
	param_alpha, param_alpha_map, param_alpha_map_on,
#endif
};

static ParamBlockDesc2 mat_principled_pblock_desc_depecated(
	// Pblock data
	static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK_DEPRECATED),
	_T("pblock"),
	0,
	GetCyclesMaterialPrincipledBsdfClassDesc(),
	P_AUTO_CONSTRUCT + P_VERSION,
	4,
	static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK_DEPRECATED),
		// Distribution
		Deprecated::param_dist,
		_T("dist"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_DISTRIBUTION,
		p_default, PrincipledBsdfDistribution::MULTISCATTER_GGX,
		p_range, PrincipledBsdfDistribution::MULTISCATTER_GGX, PrincipledBsdfDistribution::GGX,
		p_end,

		// IOR
		Deprecated::param_ior,
		_T("ior"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_IOR,
		p_default, 1.46f,
		p_range, 0.0f, 5.0f,
		p_end,

		// Color
		Deprecated::param_base_color,
		_T("color"),
		TYPE_FRGBA,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_COLOR,
		p_default, AColor(0.7f, 0.7f, 0.7f, 1.0f),
		p_end,
		// Color texmap
		Deprecated::param_base_color_map,
		_T("color_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_COLOR_TEXMAP,
		p_subtexno, subtex_color,
		p_end,
		// Color texmap on
		Deprecated::param_base_color_map_on,
		_T("color_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_COLOR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Metallic
		Deprecated::param_metal,
		_T("metallic"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_METALLIC,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Metallic texmap
		Deprecated::param_metal_map,
		_T("metallic_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_METALLIC_TEXMAP,
		p_subtexno, subtex_metal,
		p_end,
		// Metallic texmap on
		Deprecated::param_metal_map_on,
		_T("metallic_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_METALLIC_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Roughness
		Deprecated::param_roughness,
		_T("roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_ROUGHNESS,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Roughness texmap
		Deprecated::param_roughness_map,
		_T("roughness_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_ROUGHNESS_TEXMAP,
		p_subtexno, subtex_roughness,
		p_end,
		// Roughness texmap on
		Deprecated::param_roughness_map_on,
		_T("roughness_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Sheen
		Deprecated::param_sheen,
		_T("sheen"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SHEEN,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Sheen texmap
		Deprecated::param_sheen_map,
		_T("sheen_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_SHEEN_TEXMAP,
		p_subtexno, subtex_sheen,
		p_end,
		// Sheen texmap on
		Deprecated::param_sheen_map_on,
		_T("sheen_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_SHEEN_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Sheen tint
		Deprecated::param_sheen_tint,
		_T("sheen_tint"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SHEEN_TINT,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Sheen tint texmap
		Deprecated::param_sheen_tint_map,
		_T("sheen_tint_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_SHEEN_TINT_TEXMAP,
		p_subtexno, subtex_sheen_tint,
		p_end,
		// Sheen tint texmap on
		Deprecated::param_sheen_tint_map_on,
		_T("sheen_tint_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_SHEEN_TINT_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Transmission
		Deprecated::param_transmission,
		_T("transmission"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_TRANSMISSION,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Transmission texmap
		Deprecated::param_transmission_map,
		_T("transmission_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_TRANSMISSION_TEXMAP,
		p_subtexno, subtex_transmission,
		p_end,
		// Transmission texmap on
		Deprecated::param_transmission_map_on,
		_T("transmission_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_TRANSMISSION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Transmission roughness
		Deprecated::param_trans_roughness,
		_T("transmission_roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_TRANSMISSION_ROUGHNESS,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Transmission roughness texmap
		Deprecated::param_trans_roughness_map,
		_T("transmission_roughness_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_TRANSMISSION_ROUGHNESS_TEXMAP,
		p_subtexno, subtex_trans_roughness,
		p_end,
		// Transmission roughness texmap on
		Deprecated::param_trans_roughness_map_on,
		_T("transmission_roughness_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_TRANSMISSION_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Specular
		Deprecated::param_specular,
		_T("specular"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SPECULAR,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Specular texmap
		Deprecated::param_specular_map,
		_T("specular_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_SPECULAR_TEXMAP,
		p_subtexno, subtex_specular,
		p_end,
		// Specular texmap on
		Deprecated::param_specular_map_on,
		_T("specular_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_SPECULAR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Specular tint
		Deprecated::param_specular_tint,
		_T("specular_tint"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SPECULAR_TINT,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Specular tint texmap
		Deprecated::param_specular_tint_map,
		_T("specular_tint_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_SPECULAR_TINT_TEXMAP,
		p_subtexno, subtex_specular_tint,
		p_end,
		// Specular tint texmap on
		Deprecated::param_specular_tint_map_on,
		_T("specular_tint_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_SPECULAR_TINT_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Anisotropic
		Deprecated::param_anisotropic,
		_T("anisotropic"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_ANISOTROPIC,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Anisotropic texmap
		Deprecated::param_anisotropic_map,
		_T("anisotropic_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_ANISOTROPIC_TEXMAP,
		p_subtexno, subtex_anisotropic,
		p_end,
		// Anisotropic texmap on
		Deprecated::param_anisotropic_map_on,
		_T("anisotropic_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_ANISOTROPIC_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,
		// Anisotropic rotation
		Deprecated::param_anisotropic_rotation,
		_T("anisotropic_rotation"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_ANISOTROPIC_ROTATION,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Anisotropic rotation texmap
		Deprecated::param_anisotropic_rotation_map,
		_T("anisotropic_rotation_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_ANISOTROPIC_ROTATION_TEXMAP,
		p_subtexno, subtex_anisotropic_rotation,
		p_end,
		// Anisotropic rotation texmap on
		Deprecated::param_anisotropic_rotation_map_on,
		_T("anisotropic_rotation_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_ANISOTROPIC_ROTATION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

#ifndef COMPAT_206
		// Subsurface Method
		Deprecated::param_subsurface_method,
		_T("subsurface_method"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SUBSURFACE_METHOD,
		p_default, PrincipledBsdfSSSMethod::BURLEY,
		p_range, PrincipledBsdfSSSMethod::BURLEY, PrincipledBsdfSSSMethod::RANDOM_WALK,
		p_end,
#endif
		// Subsurface
		Deprecated::param_subsurface,
		_T("subsurface"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SUBSURFACE,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Subsurface texmap
		Deprecated::param_subsurface_map,
		_T("subsurface_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_SUBSURFACE_TEXMAP,
		p_subtexno, subtex_subsurface,
		p_end,
		// Subsurface texmap on
		Deprecated::param_subsurface_map_on,
		_T("subsurface_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_SUBSURFACE_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,
		// Subsurface color
		Deprecated::param_subsurface_color,
		_T("subsurface_color"),
		TYPE_FRGBA,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_SUBSURFACE_COLOR,
		p_default, AColor(0.7f, 0.1f, 0.1f, 1.0f),
		p_end,
		// Subsurface color texmap
		Deprecated::param_subsurface_color_map,
		_T("subsurface_color_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_SUBSURFACE_COLOR_TEXMAP,
		p_subtexno, subtex_subsurface_color,
		p_end,
		// Subsurface color texmap on
		Deprecated::param_subsurface_color_map_on,
		_T("subsurface_color_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_SUBSURFACE_COLOR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,
		// Subsurface x radius
		Deprecated::param_subsurface_radius_x,
		_T("subsurface_radius_x"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_RADIUS_X,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_end,
		// Subsurface y radius
		Deprecated::param_subsurface_radius_y,
		_T("subsurface_radius_y"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_RADIUS_Y,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_end,
		// Subsurface z radius
		Deprecated::param_subsurface_radius_z,
		_T("subsurface_radius_z"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_RADIUS_Z,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_end,

		// Clearcoat
		Deprecated::param_clearcoat,
		_T("clearcoat"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_CLEARCOAT,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Clearcoat texmap
		Deprecated::param_clearcoat_map,
		_T("clearcoat_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_CLEARCOAT_TEXMAP,
		p_subtexno, subtex_clearcoat,
		p_end,
		// Clearcoat texmap on
		Deprecated::param_clearcoat_map_on,
		_T("clearcoat_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_CLEARCOAT_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,
		// Clearcoat roughness
		Deprecated::param_clearcoat_roughness,
		_T("clearcoat_roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_CLEARCOAT_ROUGHNESS,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Clearcoat roughness texmap
		Deprecated::param_clearcoat_roughness_map,
		_T("clearcoat_roughness_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_CLEARCOAT_ROUGHNESS_TEXMAP,
		p_subtexno, subtex_clearcoat_roughness,
		p_end,
		// Clearcoat roughness texmap on
		Deprecated::param_clearcoat_roughness_map_on,
		_T("clearcoat_roughness_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_CLEARCOAT_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

#ifndef COMPAT_206
		// Emission
		Deprecated::param_emission,
		_T("emission_color"),
		TYPE_FRGBA,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_EMISSION,
		p_default, AColor(0.0f, 0.0f, 0.0f, 1.0f),
		p_end,
		// Emission texmap
		Deprecated::param_emission_map,
		_T("emission_color_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_EMISSION_TEXMAP,
		p_subtexno, subtex_emission,
		p_end,
		// Emission texmap on
		Deprecated::param_emission_map_on,
		_T("emission_color_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_EMISSION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,

		// Alpha
		Deprecated::param_alpha,
		_T("alpha"),
		TYPE_FLOAT,
		P_ANIMATABLE + P_OBSOLETE,
		IDS_ALPHA,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_end,
		// Alpha texmap
		Deprecated::param_alpha_map,
		_T("alpha_map"),
		TYPE_TEXMAP,
		P_OBSOLETE,
		IDS_ALPHA_TEXMAP,
		p_subtexno, subtex_alpha,
		p_end,
		// Alpha texmap on
		Deprecated::param_alpha_map_on,
		_T("alpha_map_enabled"),
		TYPE_BOOL,
		P_OBSOLETE,
		IDS_ALPHA_TEXMAP_ENABLED,
		p_default, TRUE,
		p_end,
#endif
	p_end
);
