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
 
#include "plugin_mat_principled_bsdf.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "plugin_mat_principled_bsdf_compat.h"
#include "rend_shader_desc.h"
#include "util_enums.h"
#include "win_resource.h"

static ParamBlockDesc2 mat_principled_pblock_desc(
	// Pblock data
	static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK),
	_T("pblock"),
	0,
	GetCyclesMaterialPrincipledBsdfClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI + P_VERSION,
	1,
	static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK),
	// Interface stuff
	IDD_PANEL_MAT_PRINCIPLED,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Distribution
		param_dist,
		_T("dist"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_DISTRIBUTION,
		p_default, PrincipledBsdfDistribution::MULTISCATTER_GGX,
		p_range, PrincipledBsdfDistribution::MULTISCATTER_GGX, PrincipledBsdfDistribution::GGX,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_DIST_MULTI_GGX, IDC_RADIO_DIST_GGX,  // Order here must match above enum
		p_end,

		// IOR
		param_ior,
		_T("ior"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_IOR,
		p_default, 1.46f,
		p_range, 0.0f, 5.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_IOR, IDC_SPIN_IOR, 0.01f,
		p_end,

		// Color
		param_base_color,
		_T("color"),
		TYPE_FRGBA,
		P_ANIMATABLE,
		IDS_COLOR,
		p_default, AColor(0.7f, 0.7f, 0.7f, 1.0f),
		p_ui, TYPE_COLORSWATCH, IDC_SWATCH,
		p_end,
		// Color texmap
		param_base_color_map,
		_T("color_map"),
		TYPE_TEXMAP,
		0,
		IDS_COLOR_TEXMAP,
		p_subtexno, subtex_color,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_COLOR,
		p_end,
		// Color texmap on
		param_base_color_map_on,
		_T("color_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_COLOR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_COLOR,
		p_end,

		// Metallic
		param_metal,
		_T("metallic"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_METALLIC,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_METALLIC, IDC_SPIN_METALLIC, 0.01f,
		p_end,
		// Metallic texmap
		param_metal_map,
		_T("metallic_map"),
		TYPE_TEXMAP,
		0,
		IDS_METALLIC_TEXMAP,
		p_subtexno, subtex_metal,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_METALLIC,
		p_end,
		// Metallic texmap on
		param_metal_map_on,
		_T("metallic_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_METALLIC_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_METALLIC,
		p_end,

		// Roughness
		param_roughness,
		_T("roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ROUGHNESS,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ROUGHNESS, IDC_SPIN_ROUGHNESS, 0.01f,
		p_end,
		// Roughness texmap
		param_roughness_map,
		_T("roughness_map"),
		TYPE_TEXMAP,
		0,
		IDS_ROUGHNESS_TEXMAP,
		p_subtexno, subtex_roughness,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ROUGHNESS,
		p_end,
		// Roughness texmap on
		param_roughness_map_on,
		_T("roughness_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ROUGHNESS,
		p_end,

		// Sheen
		param_sheen,
		_T("sheen"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SHEEN,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SHEEN, IDC_SPIN_SHEEN, 0.01f,
		p_end,
		// Sheen texmap
		param_sheen_map,
		_T("sheen_map"),
		TYPE_TEXMAP,
		0,
		IDS_SHEEN_TEXMAP,
		p_subtexno, subtex_sheen,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SHEEN,
		p_end,
		// Sheen texmap on
		param_sheen_map_on,
		_T("sheen_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SHEEN_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SHEEN,
		p_end,

		// Sheen tint
		param_sheen_tint,
		_T("sheen_tint"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SHEEN_TINT,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SHEEN_TINT, IDC_SPIN_SHEEN_TINT, 0.01f,
		p_end,
		// Sheen tint texmap
		param_sheen_tint_map,
		_T("sheen_tint_map"),
		TYPE_TEXMAP,
		0,
		IDS_SHEEN_TINT_TEXMAP,
		p_subtexno, subtex_sheen_tint,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SHEEN_TINT,
		p_end,
		// Sheen tint texmap on
		param_sheen_tint_map_on,
		_T("sheen_tint_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SHEEN_TINT_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SHEEN_TINT,
		p_end,

		// Transmission
		param_transmission,
		_T("transmission"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_TRANSMISSION,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_TRANSMISSION, IDC_SPIN_TRANSMISSION, 0.01f,
		p_end,
		// Transmission texmap
		param_transmission_map,
		_T("transmission_map"),
		TYPE_TEXMAP,
		0,
		IDS_TRANSMISSION_TEXMAP,
		p_subtexno, subtex_transmission,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TRANSMISSION,
		p_end,
		// Transmission texmap on
		param_transmission_map_on,
		_T("transmission_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_TRANSMISSION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TRANSMISSION,
		p_end,

		// Transmission roughness
		param_trans_roughness,
		_T("transmission_roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_TRANSMISSION_ROUGHNESS,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_TRANSMISSION_ROUGHNESS, IDC_SPIN_TRANSMISSION_ROUGHNESS, 0.01f,
		p_end,
		// Transmission roughness texmap
		param_trans_roughness_map,
		_T("transmission_roughness_map"),
		TYPE_TEXMAP,
		0,
		IDS_TRANSMISSION_ROUGHNESS_TEXMAP,
		p_subtexno, subtex_trans_roughness,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TRANSMISSION_ROUGHNESS,
		p_end,
		// Transmission roughness texmap on
		param_trans_roughness_map_on,
		_T("transmission_roughness_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_TRANSMISSION_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TRANSMISSION_ROUGHNESS,
		p_end,

		// Specular
		param_specular,
		_T("specular"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SPECULAR,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SPECULAR, IDC_SPIN_SPECULAR, 0.01f,
		p_end,
		// Specular texmap
		param_specular_map,
		_T("specular_map"),
		TYPE_TEXMAP,
		0,
		IDS_SPECULAR_TEXMAP,
		p_subtexno, subtex_specular,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SPECULAR,
		p_end,
		// Specular texmap on
		param_specular_map_on,
		_T("specular_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SPECULAR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SPECULAR,
		p_end,

		// Specular tint
		param_specular_tint,
		_T("specular_tint"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SPECULAR_TINT,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SPECULAR_TINT, IDC_SPIN_SPECULAR_TINT, 0.01f,
		p_end,
		// Specular tint texmap
		param_specular_tint_map,
		_T("specular_tint_map"),
		TYPE_TEXMAP,
		0,
		IDS_SPECULAR_TINT_TEXMAP,
		p_subtexno, subtex_specular_tint,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SPECULAR_TINT,
		p_end,
		// Specular tint texmap on
		param_specular_tint_map_on,
		_T("specular_tint_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SPECULAR_TINT_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SPECULAR_TINT,
		p_end,

		// Anisotropic
		param_anisotropic,
		_T("anisotropic"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ANISOTROPIC,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ANISOTROPIC, IDC_SPIN_ANISOTROPIC, 0.01f,
		p_end,
		// Anisotropic texmap
		param_anisotropic_map,
		_T("anisotropic_map"),
		TYPE_TEXMAP,
		0,
		IDS_ANISOTROPIC_TEXMAP,
		p_subtexno, subtex_anisotropic,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ANISOTROPIC,
		p_end,
		// Anisotropic texmap on
		param_anisotropic_map_on,
		_T("anisotropic_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ANISOTROPIC_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ANISOTROPIC,
		p_end,
		// Anisotropic rotation
		param_anisotropic_rotation,
		_T("anisotropic_rotation"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ANISOTROPIC_ROTATION,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ANISOTROPIC_ROT, IDC_SPIN_ANISOTROPIC_ROT, 0.01f,
		p_end,
		// Anisotropic rotation texmap
		param_anisotropic_rotation_map,
		_T("anisotropic_rotation_map"),
		TYPE_TEXMAP,
		0,
		IDS_ANISOTROPIC_ROTATION_TEXMAP,
		p_subtexno, subtex_anisotropic_rotation,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ANISOTROPIC_ROT,
		p_end,
		// Anisotropic rotation texmap on
		param_anisotropic_rotation_map_on,
		_T("anisotropic_rotation_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ANISOTROPIC_ROTATION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ANISOTROPIC_ROT,
		p_end,

		// Subsurface Method
		param_subsurface_method,
		_T("subsurface_method"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_SUBSURFACE_METHOD,
		p_default, PrincipledBsdfSSSMethod::BURLEY,
		p_range, PrincipledBsdfSSSMethod::BURLEY, PrincipledBsdfSSSMethod::RANDOM_WALK,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_DIST_SSS_BURLEY, IDC_RADIO_DIST_SSS_RW,  // Order here must match above enum
		p_end,
		// Subsurface
		param_subsurface,
		_T("subsurface"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SUBSURFACE,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SUBSURFACE, IDC_SPIN_SUBSURFACE, 0.01f,
		p_end,
		// Subsurface texmap
		param_subsurface_map,
		_T("subsurface_map"),
		TYPE_TEXMAP,
		0,
		IDS_SUBSURFACE_TEXMAP,
		p_subtexno, subtex_subsurface,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SUBSURFACE,
		p_end,
		// Subsurface texmap on
		param_subsurface_map_on,
		_T("subsurface_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SUBSURFACE_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SUBSURFACE,
		p_end,
		// Subsurface color
		param_subsurface_color,
		_T("subsurface_color"),
		TYPE_FRGBA,
		P_ANIMATABLE,
		IDS_SUBSURFACE_COLOR,
		p_default, AColor(0.7f, 0.1f, 0.1f, 1.0f),
		p_ui, TYPE_COLORSWATCH, IDC_SWATCH_SUBSURFACE,
		p_end,
		// Subsurface color texmap
		param_subsurface_color_map,
		_T("subsurface_color_map"),
		TYPE_TEXMAP,
		0,
		IDS_SUBSURFACE_COLOR_TEXMAP,
		p_subtexno, subtex_subsurface_color,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SS_COLOR,
		p_end,
		// Subsurface color texmap on
		param_subsurface_color_map_on,
		_T("subsurface_color_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SUBSURFACE_COLOR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SS_COLOR,
		p_end,
		// Subsurface x radius
		param_subsurface_radius_x,
		_T("subsurface_radius_x"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_RADIUS_X,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_RADIUS_X, IDC_SPIN_RADIUS_X, 0.1f,
		p_end,
		// Subsurface y radius
		param_subsurface_radius_y,
		_T("subsurface_radius_y"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_RADIUS_Y,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_RADIUS_Y, IDC_SPIN_RADIUS_Y, 0.1f,
		p_end,
		// Subsurface z radius
		param_subsurface_radius_z,
		_T("subsurface_radius_z"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_RADIUS_Z,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_RADIUS_Z, IDC_SPIN_RADIUS_Z, 0.1f,
		p_end,

		// Clearcoat
		param_clearcoat,
		_T("clearcoat"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_CLEARCOAT,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_CLEARCOAT, IDC_SPIN_CLEARCOAT, 0.01f,
		p_end,
		// Clearcoat texmap
		param_clearcoat_map,
		_T("clearcoat_map"),
		TYPE_TEXMAP,
		0,
		IDS_CLEARCOAT_TEXMAP,
		p_subtexno, subtex_clearcoat,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_CLEARCOAT,
		p_end,
		// Clearcoat texmap on
		param_clearcoat_map_on,
		_T("clearcoat_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_CLEARCOAT_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_CLEARCOAT,
		p_end,
		// Clearcoat roughness
		param_clearcoat_roughness,
		_T("clearcoat_roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_CLEARCOAT_ROUGHNESS,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_CC_ROUGHNESS, IDC_SPIN_CC_ROUGHNESS, 0.01f,
		p_end,
		// Clearcoat roughness texmap
		param_clearcoat_roughness_map,
		_T("clearcoat_roughness_map"),
		TYPE_TEXMAP,
		0,
		IDS_CLEARCOAT_ROUGHNESS_TEXMAP,
		p_subtexno, subtex_clearcoat_roughness,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_CC_ROUGHNESS,
		p_end,
		// Clearcoat roughness texmap on
		param_clearcoat_roughness_map_on,
		_T("clearcoat_roughness_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_CLEARCOAT_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_CC_ROUGHNESS,
		p_end,

		// Emission
		param_emission,
		_T("emission_color"),
		TYPE_FRGBA,
		P_ANIMATABLE,
		IDS_EMISSION,
		p_default, AColor(0.0f, 0.0f, 0.0f, 1.0f),
		p_ui, TYPE_COLORSWATCH, IDC_SWATCH_EMISSION,
		p_end,
		// Emission texmap
		param_emission_map,
		_T("emission_color_map"),
		TYPE_TEXMAP,
		0,
		IDS_EMISSION_TEXMAP,
		p_subtexno, subtex_emission,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_EMISSION,
		p_end,
		// Emission texmap on
		param_emission_map_on,
		_T("emission_color_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_EMISSION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_EMISSION,
		p_end,

		// Alpha
		param_alpha,
		_T("alpha"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ALPHA,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ALPHA, IDC_SPIN_ALPHA, 0.01f,
		p_end,
		// Alpha texmap
		param_alpha_map,
		_T("alpha_map"),
		TYPE_TEXMAP,
		0,
		IDS_ALPHA_TEXMAP,
		p_subtexno, subtex_alpha,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ALPHA,
		p_end,
		// Alpha texmap on
		param_alpha_map_on,
		_T("alpha_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ALPHA_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ALPHA,
		p_end,

		// Ignore original pblock
		param_ignore_original_pblock,
		_T("ignore_original_pblock"),
		TYPE_BOOL,
		0,
		IDS_IGNORE_ORIGINAL_PBLOCK,
		p_default, FALSE,
		p_end,

	p_end
);

MAKE_COMMON_NORMAL_PBLOCK(static_cast<int>(PrincipledBsdfPblocks::NORMAL_PBLOCK), GetCyclesMaterialPrincipledBsdfClassDesc())

MAKE_CC_NORMAL_PBLOCK(static_cast<int>(PrincipledBsdfPblocks::CC_NORMAL_PBLOCK), GetCyclesMaterialPrincipledBsdfClassDesc())

// PLCB to handle converting to new pblock index
class PrincipledBsdfPlbc : public PostLoadCallback
{
private:
	CyclesPrincipledBSDFMat* const mat;

public:
	PrincipledBsdfPlbc(CyclesPrincipledBSDFMat* const mat) : mat(mat) {}

	bool should_do_copy() {
		const int ignore_pblock = get_new_pblock()->GetInt(param_ignore_original_pblock);
		return ignore_pblock == 0;
	}

	IParamBlock2* get_old_pblock() {
		return mat->GetParamBlockByID(static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK_DEPRECATED));
	}

	IParamBlock2* get_new_pblock() {
		return mat->GetParamBlockByID(static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK));
	}

	void copy_float(const int old_slot, const int new_slot) {
		Control* const control = get_old_pblock()->GetControllerByID(old_slot);
		if (control) {
			get_new_pblock()->SetControllerByID(new_slot, 0, control);
		}
		else {
			const float value = get_old_pblock()->GetFloat(old_slot);
			get_new_pblock()->SetValue(new_slot, 0, value);
		}
	}

	void copy_int(const int old_slot, const int new_slot) {
		Control* const control = get_old_pblock()->GetControllerByID(old_slot);
		if (control) {
			get_new_pblock()->SetControllerByID(new_slot, 0, control);
		}
		else {
			const int value = get_old_pblock()->GetInt(old_slot);
			get_new_pblock()->SetValue(new_slot, 0, value);
		}
	}

	void copy_color(const int old_slot, const int new_slot) {
		Control* const control = get_old_pblock()->GetControllerByID(old_slot);
		if (control) {
			get_new_pblock()->SetControllerByID(new_slot, 0, control);
		}
		else {
			const Color value = get_old_pblock()->GetColor(old_slot);
			get_new_pblock()->SetValue(new_slot, 0, value);
		}
	}

	void copy_texmap(const int old_slot, const int new_slot) {
		Texmap* const texmap = get_old_pblock()->GetTexmap(old_slot);
		if (texmap) {
			get_new_pblock()->SetValue(new_slot, 0, texmap);
		}
	}

	void proc(ILoad*) override {

		if (should_do_copy()) {
			// Texmaps
			copy_texmap(static_cast<int>(Deprecated::param_base_color_map), param_base_color_map);
			copy_texmap(static_cast<int>(Deprecated::param_metal_map), param_metal_map);
			copy_texmap(static_cast<int>(Deprecated::param_roughness_map), param_roughness_map);
			copy_texmap(static_cast<int>(Deprecated::param_sheen_map), param_sheen_map);
			copy_texmap(static_cast<int>(Deprecated::param_sheen_tint_map), param_sheen_tint_map);
			copy_texmap(static_cast<int>(Deprecated::param_transmission_map), param_transmission_map);
			copy_texmap(static_cast<int>(Deprecated::param_trans_roughness_map), param_trans_roughness_map);
			copy_texmap(static_cast<int>(Deprecated::param_specular_map), param_specular_map);
			copy_texmap(static_cast<int>(Deprecated::param_specular_tint_map), param_specular_tint_map);
			copy_texmap(static_cast<int>(Deprecated::param_anisotropic_map), param_anisotropic_map);
			copy_texmap(static_cast<int>(Deprecated::param_anisotropic_rotation_map), param_anisotropic_rotation_map);
			copy_texmap(static_cast<int>(Deprecated::param_subsurface_map), param_subsurface_map);
			copy_texmap(static_cast<int>(Deprecated::param_subsurface_color_map), param_subsurface_color_map);
			copy_texmap(static_cast<int>(Deprecated::param_clearcoat_map), param_clearcoat_map);
			copy_texmap(static_cast<int>(Deprecated::param_clearcoat_roughness_map), param_clearcoat_roughness_map);

			// Ints
			copy_int(static_cast<int>(Deprecated::param_dist), param_dist);

			// Bools (these are really ints)
			copy_int(static_cast<int>(Deprecated::param_base_color_map_on), param_base_color_map_on);
			copy_int(static_cast<int>(Deprecated::param_metal_map_on), param_metal_map_on);
			copy_int(static_cast<int>(Deprecated::param_roughness_map_on), param_roughness_map_on);
			copy_int(static_cast<int>(Deprecated::param_sheen_map_on), param_sheen_map_on);
			copy_int(static_cast<int>(Deprecated::param_sheen_tint_map_on), param_sheen_tint_map_on);
			copy_int(static_cast<int>(Deprecated::param_transmission_map_on), param_transmission_map_on);
			copy_int(static_cast<int>(Deprecated::param_trans_roughness_map_on), param_trans_roughness_map_on);
			copy_int(static_cast<int>(Deprecated::param_specular_map_on), param_specular_map_on);
			copy_int(static_cast<int>(Deprecated::param_specular_tint_map_on), param_specular_tint_map_on);
			copy_int(static_cast<int>(Deprecated::param_anisotropic_map_on), param_anisotropic_map_on);
			copy_int(static_cast<int>(Deprecated::param_anisotropic_rotation_map_on), param_anisotropic_rotation_map_on);
			copy_int(static_cast<int>(Deprecated::param_subsurface_map_on), param_subsurface_map_on);
			copy_int(static_cast<int>(Deprecated::param_subsurface_color_map_on), param_subsurface_color_map_on);
			copy_int(static_cast<int>(Deprecated::param_clearcoat_map_on), param_clearcoat_map_on);
			copy_int(static_cast<int>(Deprecated::param_clearcoat_roughness_map_on), param_clearcoat_roughness_map_on);

			// Floats
			copy_float(static_cast<int>(Deprecated::param_ior), param_ior);
			copy_float(static_cast<int>(Deprecated::param_metal), param_metal);
			copy_float(static_cast<int>(Deprecated::param_roughness), param_roughness);
			copy_float(static_cast<int>(Deprecated::param_sheen), param_sheen);
			copy_float(static_cast<int>(Deprecated::param_sheen_tint), param_sheen_tint);
			copy_float(static_cast<int>(Deprecated::param_transmission), param_transmission);
			copy_float(static_cast<int>(Deprecated::param_trans_roughness), param_trans_roughness);
			copy_float(static_cast<int>(Deprecated::param_specular), param_specular);
			copy_float(static_cast<int>(Deprecated::param_specular_tint), param_specular_tint);
			copy_float(static_cast<int>(Deprecated::param_anisotropic), param_anisotropic);
			copy_float(static_cast<int>(Deprecated::param_anisotropic_rotation), param_anisotropic_rotation);
			copy_float(static_cast<int>(Deprecated::param_subsurface), param_subsurface);
			copy_float(static_cast<int>(Deprecated::param_subsurface_radius_x), param_subsurface_radius_x);
			copy_float(static_cast<int>(Deprecated::param_subsurface_radius_y), param_subsurface_radius_y);
			copy_float(static_cast<int>(Deprecated::param_subsurface_radius_z), param_subsurface_radius_z);
			copy_float(static_cast<int>(Deprecated::param_clearcoat), param_clearcoat);
			copy_float(static_cast<int>(Deprecated::param_clearcoat_roughness), param_clearcoat_roughness);

			// Colors
			copy_color(static_cast<int>(Deprecated::param_base_color), param_base_color);
			copy_color(static_cast<int>(Deprecated::param_subsurface_color), param_subsurface_color);

			// Stuff that doesn't exist in 2.06
#ifndef COMPAT_206
			copy_texmap(static_cast<int>(Deprecated::param_emission_map), param_emission_map);
			copy_texmap(static_cast<int>(Deprecated::param_alpha_map), param_alpha_map);
			copy_int(static_cast<int>(Deprecated::param_subsurface_method), param_subsurface_method);
			copy_int(static_cast<int>(Deprecated::param_emission_map_on), param_emission_map_on);
			copy_int(static_cast<int>(Deprecated::param_alpha_map_on), param_alpha_map_on);
			copy_float(static_cast<int>(Deprecated::param_alpha), param_alpha);
			copy_color(static_cast<int>(Deprecated::param_emission), param_emission);
#endif
		}
		get_new_pblock()->SetValue(param_ignore_original_pblock, 0, TRUE);
		delete this;
	}
};

CyclesPrincipledBSDFMat::CyclesPrincipledBSDFMat() : CyclesPluginMatBase(true, true, static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK), static_cast<int>(PrincipledBsdfPblocks::NORMAL_PBLOCK))
{
	// We have to manually make the pblock vector because the base plugin material class assumes the main pblock is reference 0
	{
		pblock_vec.clear();

		ParamBlockSlotDescriptor old_pblock_desc;
		old_pblock_desc.name = TSTR(L"pblock_old");
		old_pblock_desc.ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK_DEPRECATED);
		pblock_vec.push_back(old_pblock_desc);

		ParamBlockSlotDescriptor normal_pblock_desc;
		normal_pblock_desc.name = TSTR(L"normal_pblock");
		normal_pblock_desc.ref = static_cast<int>(PrincipledBsdfPblocks::NORMAL_PBLOCK);
		pblock_vec.push_back(normal_pblock_desc);

		ParamBlockSlotDescriptor cc_normal_pblock_desc;
		cc_normal_pblock_desc.name = TSTR(L"cc_normal_pblock");
		cc_normal_pblock_desc.ref = static_cast<int>(PrincipledBsdfPblocks::CC_NORMAL_PBLOCK);
		pblock_vec.push_back(cc_normal_pblock_desc);

		ParamBlockSlotDescriptor main_pblock_desc;
		main_pblock_desc.name = TSTR(L"pblock");
		main_pblock_desc.ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);
		pblock_vec.push_back(main_pblock_desc);

	}

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;
}

void CyclesPrincipledBSDFMat::Initialize()
{
	CyclesPluginMatBase::Initialize();
	AddClearcoatNormalSubtexSlots();
}

IOResult CyclesPrincipledBSDFMat::Load(ILoad* const iload)
{
	const IOResult result = CyclesPluginMatBase::Load(iload);

	iload->RegisterPostLoadCallback(new PrincipledBsdfPlbc(this));

	return result;
}

IOResult CyclesPrincipledBSDFMat::Save(ISave* const isave)
{
	GetParamBlockByID(static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK))->SetValue(param_ignore_original_pblock, 0, TRUE);
	return CyclesPluginMatBase::Save(isave);
}

void CyclesPrincipledBSDFMat::PopulateCCNormalMapDesc(NormalMapDescriptor* const desc, const TimeValue t)
{
	IParamBlock2* const normal_pblock = this->GetParamBlock(static_cast<int>(PrincipledBsdfPblocks::CC_NORMAL_PBLOCK));

	if (normal_pblock == nullptr) {
		return;
	}

	const int enabled = normal_pblock->GetInt(nparam_enable, t);
	if (enabled == 0) {
		return;
	}

	desc->enabled = true;

	desc->space = static_cast<NormalMapTangentSpace>(normal_pblock->GetInt(nparam_space, t));

	desc->strength = normal_pblock->GetFloat(nparam_strength, t);
	if (SubTexmapOn(subtex_normal_strength)) {
		desc->strength_map = GetSubTexmap(subtex_normal_strength);
	}

	if (SubTexmapOn(subtex_normal_color)) {
		desc->color_map = GetSubTexmap(subtex_normal_color);
	}
	else {
		Color color = normal_pblock->GetColor(nparam_color);
		desc->color.x = color.r;
		desc->color.y = color.g;
		desc->color.z = color.b;
	}

	const int invert_green_int = normal_pblock->GetInt(nparam_invert_green, t);
	desc->invert_green = (invert_green_int != 0);
}

PrincipledBsdfDistribution CyclesPrincipledBSDFMat::GetDistribution(const TimeValue t, Interval& valid)
{
	const int distribution = pblock->GetInt(param_dist, t, valid);
	if (distribution < 0 || distribution >= static_cast<int>(PrincipledBsdfDistribution::COUNT)) {
		return PrincipledBsdfDistribution::MULTISCATTER_GGX;
	}
	return static_cast<PrincipledBsdfDistribution>(distribution);
}

float CyclesPrincipledBSDFMat::GetIOR(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_ior, t, valid);
}

Color CyclesPrincipledBSDFMat::GetBaseColor(const TimeValue t, Interval& valid)
{
	return pblock->GetColor(param_base_color, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetBaseColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetMetallic(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_metal, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetMetallicTexmap()
{
	if (SubTexmapOn(subtex_metal)) {
		return GetSubTexmap(subtex_metal);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetRoughness(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_roughness, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetRoughnessTexmap()
{
	if (SubTexmapOn(subtex_roughness)) {
		return GetSubTexmap(subtex_roughness);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetSheen(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_sheen, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetSheenTexmap()
{
	if (SubTexmapOn(subtex_sheen)) {
		return GetSubTexmap(subtex_sheen);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetSheenTint(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_sheen_tint, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetSheenTintTexmap()
{
	if (SubTexmapOn(subtex_sheen_tint)) {
		return GetSubTexmap(subtex_sheen_tint);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetTransmission(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_transmission, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetTransmissionTexmap()
{
	if (SubTexmapOn(subtex_transmission)) {
		return GetSubTexmap(subtex_transmission);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetTransmissionRoughness(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_trans_roughness, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetTransmissionRoughnessTexmap()
{
	if (SubTexmapOn(subtex_trans_roughness)) {
		return GetSubTexmap(subtex_trans_roughness);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetSpecular(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_specular, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetSpecularTexmap()
{
	if (SubTexmapOn(subtex_specular)) {
		return GetSubTexmap(subtex_specular);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetSpecularTint(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_specular_tint, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetSpecularTintTexmap()
{
	if (SubTexmapOn(subtex_specular_tint)) {
		return GetSubTexmap(subtex_specular_tint);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetAnisotropic(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_anisotropic, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetAnisotropicTexmap()
{
	if (SubTexmapOn(subtex_anisotropic)) {
		return GetSubTexmap(subtex_anisotropic);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetAnisotropicRotation(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_anisotropic_rotation, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetAnisotropicRotationTexmap()
{
	if (SubTexmapOn(subtex_anisotropic_rotation)) {
		return GetSubTexmap(subtex_anisotropic_rotation);
	}
	return nullptr;
}

PrincipledBsdfSSSMethod CyclesPrincipledBSDFMat::GetSubsurfaceMethod(const TimeValue t, Interval& valid)
{
	const int method = pblock->GetInt(param_subsurface_method, t, valid);
	if (method < 0 || method >= static_cast<int>(PrincipledBsdfSSSMethod::COUNT)) {
		return PrincipledBsdfSSSMethod::BURLEY;
	}
	return static_cast<PrincipledBsdfSSSMethod>(method);
}

float CyclesPrincipledBSDFMat::GetSubsurface(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_subsurface, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetSubsurfaceTexmap()
{
	if (SubTexmapOn(subtex_subsurface)) {
		return GetSubTexmap(subtex_subsurface);
	}
	return nullptr;
}

Color CyclesPrincipledBSDFMat::GetSubsurfaceColor(const TimeValue t, Interval& valid)
{
	return pblock->GetColor(param_subsurface_color, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetSubsurfaceColorTexmap()
{
	if (SubTexmapOn(subtex_subsurface_color)) {
		return GetSubTexmap(subtex_subsurface_color);
	}
	return nullptr;
}

Point3 CyclesPrincipledBSDFMat::GetSubsurfaceRadius(const TimeValue t, Interval& valid)
{
	Point3 result;

	Interval x_valid = FOREVER;
	result.x = pblock->GetFloat(param_subsurface_radius_x, t, x_valid);
	Interval y_valid = FOREVER;
	result.y = pblock->GetFloat(param_subsurface_radius_y, t, y_valid);
	Interval z_valid = FOREVER;
	result.z = pblock->GetFloat(param_subsurface_radius_z, t, z_valid);

	valid &= x_valid & y_valid & z_valid;

	return result;
}

float CyclesPrincipledBSDFMat::GetClearcoat(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_clearcoat, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetClearcoatTexmap()
{
	if (SubTexmapOn(subtex_clearcoat)) {
		return GetSubTexmap(subtex_clearcoat);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetClearcoatRoughness(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_clearcoat_roughness, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetClearcoatRoughnessTexmap()
{
	if (SubTexmapOn(subtex_clearcoat_roughness)) {
		return GetSubTexmap(subtex_clearcoat_roughness);
	}
	return nullptr;
}


Color CyclesPrincipledBSDFMat::GetEmissionColor(const TimeValue t, Interval& valid)
{
	return pblock->GetColor(param_emission, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetEmissionColorTexmap()
{
	if (SubTexmapOn(subtex_emission)) {
		return GetSubTexmap(subtex_emission);
	}
	return nullptr;
}

float CyclesPrincipledBSDFMat::GetAlpha(const TimeValue t, Interval& valid)
{
	return pblock->GetFloat(param_alpha, t, valid);
}

Texmap* CyclesPrincipledBSDFMat::GetAlphaTexmap()
{
	if (SubTexmapOn(subtex_alpha)) {
		return GetSubTexmap(subtex_alpha);
	}
	return nullptr;
}

ClassDesc2* CyclesPrincipledBSDFMat::GetClassDesc() const
{
	return GetCyclesMaterialPrincipledBsdfClassDesc();
}

ParamBlockDesc2* CyclesPrincipledBSDFMat::GetParamBlockDesc()
{
	return &mat_principled_pblock_desc;
}

ParamBlockDesc2* CyclesPrincipledBSDFMat::GetNormalParamBlockDesc()
{
	return &common_normal_pblock_desc;
}

void CyclesPrincipledBSDFMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesPrincipledBSDFMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Base Color");
	color_tex_slot.enabled_param_id = param_base_color_map_on;
	color_tex_slot.param_id = param_base_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;
	color_tex_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor metal_slot;
	metal_slot.display_name = _T("Metallic");
	metal_slot.enabled_param_id = param_metal_map_on;
	metal_slot.param_id = param_metal_map;
	metal_slot.slot_type = MAPSLOT_TEXTURE;
	metal_slot.subtex_id = subtex_metal;
	metal_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor roughness_slot;
	roughness_slot.display_name = _T("Roughness");
	roughness_slot.enabled_param_id = param_roughness_map_on;
	roughness_slot.param_id = param_roughness_map;
	roughness_slot.slot_type = MAPSLOT_TEXTURE;
	roughness_slot.subtex_id = subtex_roughness;
	roughness_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor sheen_slot;
	sheen_slot.display_name = _T("Sheen");
	sheen_slot.enabled_param_id = param_sheen_map_on;
	sheen_slot.param_id = param_sheen_map;
	sheen_slot.slot_type = MAPSLOT_TEXTURE;
	sheen_slot.subtex_id = subtex_sheen;
	sheen_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor sheen_tint_slot;
	sheen_tint_slot.display_name = _T("Sheen Tint");
	sheen_tint_slot.enabled_param_id = param_sheen_tint_map_on;
	sheen_tint_slot.param_id = param_sheen_tint_map;
	sheen_tint_slot.slot_type = MAPSLOT_TEXTURE;
	sheen_tint_slot.subtex_id = subtex_sheen_tint;
	sheen_tint_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor transmission_slot;
	transmission_slot.display_name = _T("Transmission");
	transmission_slot.enabled_param_id = param_transmission_map_on;
	transmission_slot.param_id = param_transmission_map;
	transmission_slot.slot_type = MAPSLOT_TEXTURE;
	transmission_slot.subtex_id = subtex_transmission;
	transmission_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor transmission_roughness_slot;
	transmission_roughness_slot.display_name = _T("Transmission Roughness");
	transmission_roughness_slot.enabled_param_id = param_trans_roughness_map_on;
	transmission_roughness_slot.param_id = param_trans_roughness_map;
	transmission_roughness_slot.slot_type = MAPSLOT_TEXTURE;
	transmission_roughness_slot.subtex_id = subtex_trans_roughness;
	transmission_roughness_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor specular_slot;
	specular_slot.display_name = _T("Specular");
	specular_slot.enabled_param_id = param_specular_map_on;
	specular_slot.param_id = param_specular_map;
	specular_slot.slot_type = MAPSLOT_TEXTURE;
	specular_slot.subtex_id = subtex_specular;
	specular_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor specular_tint_slot;
	specular_tint_slot.display_name = _T("Specular Tint");
	specular_tint_slot.enabled_param_id = param_specular_tint_map_on;
	specular_tint_slot.param_id = param_specular_tint_map;
	specular_tint_slot.slot_type = MAPSLOT_TEXTURE;
	specular_tint_slot.subtex_id = subtex_specular_tint;
	specular_tint_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor anisotropic_slot;
	anisotropic_slot.display_name = _T("Anisotropic");
	anisotropic_slot.enabled_param_id = param_anisotropic_map_on;
	anisotropic_slot.param_id = param_anisotropic_map;
	anisotropic_slot.slot_type = MAPSLOT_TEXTURE;
	anisotropic_slot.subtex_id = subtex_anisotropic;
	anisotropic_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor anisotropic_rot_slot;
	anisotropic_rot_slot.display_name = _T("Anisotropic Rotation");
	anisotropic_rot_slot.enabled_param_id = param_anisotropic_rotation_map_on;
	anisotropic_rot_slot.param_id = param_anisotropic_rotation_map;
	anisotropic_rot_slot.slot_type = MAPSLOT_TEXTURE;
	anisotropic_rot_slot.subtex_id = subtex_anisotropic_rotation;
	anisotropic_rot_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor subsurface_slot;
	subsurface_slot.display_name = _T("Subsurface");
	subsurface_slot.enabled_param_id = param_subsurface_map_on;
	subsurface_slot.param_id = param_subsurface_map;
	subsurface_slot.slot_type = MAPSLOT_TEXTURE;
	subsurface_slot.subtex_id = subtex_subsurface;
	subsurface_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor subsurface_color_slot;
	subsurface_color_slot.display_name = _T("Subsurface Color");
	subsurface_color_slot.enabled_param_id = param_subsurface_color_map_on;
	subsurface_color_slot.param_id = param_subsurface_color_map;
	subsurface_color_slot.slot_type = MAPSLOT_TEXTURE;
	subsurface_color_slot.subtex_id = subtex_subsurface_color;
	subsurface_color_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor clearcoat_slot;
	clearcoat_slot.display_name = _T("Clearcoat");
	clearcoat_slot.enabled_param_id = param_clearcoat_map_on;
	clearcoat_slot.param_id = param_clearcoat_map;
	clearcoat_slot.slot_type = MAPSLOT_TEXTURE;
	clearcoat_slot.subtex_id = subtex_clearcoat;
	clearcoat_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor clearcoat_roughness_slot;
	clearcoat_roughness_slot.display_name = _T("Clearcoat Roughness");
	clearcoat_roughness_slot.enabled_param_id = param_clearcoat_roughness_map_on;
	clearcoat_roughness_slot.param_id = param_clearcoat_roughness_map;
	clearcoat_roughness_slot.slot_type = MAPSLOT_TEXTURE;
	clearcoat_roughness_slot.subtex_id = subtex_clearcoat_roughness;
	clearcoat_roughness_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor emission_slot;
	emission_slot.display_name = _T("Emission");
	emission_slot.enabled_param_id = param_emission_map_on;
	emission_slot.param_id = param_emission_map;
	emission_slot.slot_type = MAPSLOT_TEXTURE;
	emission_slot.subtex_id = subtex_emission;
	emission_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	SubtexSlotDescriptor alpha_slot;
	alpha_slot.display_name = _T("Alpha");
	alpha_slot.enabled_param_id = param_alpha_map_on;
	alpha_slot.param_id = param_alpha_map;
	alpha_slot.slot_type = MAPSLOT_TEXTURE;
	alpha_slot.subtex_id = subtex_alpha;
	alpha_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::MAIN_PBLOCK);

	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(metal_slot);
	subtex_slots.push_back(roughness_slot);
	subtex_slots.push_back(sheen_slot);
	subtex_slots.push_back(sheen_tint_slot);
	subtex_slots.push_back(transmission_slot);
	subtex_slots.push_back(transmission_roughness_slot);
	subtex_slots.push_back(specular_slot);
	subtex_slots.push_back(specular_tint_slot);
	subtex_slots.push_back(anisotropic_slot);
	subtex_slots.push_back(anisotropic_rot_slot);
	subtex_slots.push_back(subsurface_slot);
	subtex_slots.push_back(subsurface_color_slot);
	subtex_slots.push_back(clearcoat_slot);
	subtex_slots.push_back(clearcoat_roughness_slot);
	subtex_slots.push_back(emission_slot);
	subtex_slots.push_back(alpha_slot);
}

void CyclesPrincipledBSDFMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	UpdateVPTexmap(subtex_color);

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval base_color_valid = FOREVER;
	const Color base_color = pblock->GetColor(param_base_color, t, base_color_valid);
	color_ambient = base_color;
	color_diffuse = base_color;

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = base_color_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void CyclesPrincipledBSDFMat::AddClearcoatNormalSubtexSlots()
{
	subtex_cc_normal_strength = static_cast<int>(subtex_slots.size());
	subtex_cc_normal_color = static_cast<int>(subtex_slots.size()) + 1;

	SubtexSlotDescriptor normal_strength_tex_slot;
	normal_strength_tex_slot.display_name = _T("CC Normal Strength");
	normal_strength_tex_slot.enabled_param_id = cc_nparam_strength_map_on;
	normal_strength_tex_slot.param_id = cc_nparam_strength_map;
	normal_strength_tex_slot.slot_type = MAPSLOT_TEXTURE;
	normal_strength_tex_slot.subtex_id = subtex_cc_normal_strength;
	normal_strength_tex_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::CC_NORMAL_PBLOCK);

	SubtexSlotDescriptor normal_color_tex_slot;
	normal_color_tex_slot.display_name = _T("CC Normal Color");
	normal_color_tex_slot.enabled_param_id = cc_nparam_color_map_on;
	normal_color_tex_slot.param_id = cc_nparam_color_map;
	normal_color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	normal_color_tex_slot.subtex_id = subtex_cc_normal_color;
	normal_color_tex_slot.pblock_ref = static_cast<int>(PrincipledBsdfPblocks::CC_NORMAL_PBLOCK);

	subtex_slots.push_back(normal_strength_tex_slot);
	subtex_slots.push_back(normal_color_tex_slot);
}
