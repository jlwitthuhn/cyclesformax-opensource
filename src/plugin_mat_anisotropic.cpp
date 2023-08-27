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
 
#include "plugin_mat_anisotropic.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0
#define NORMAL_PBLOCK_REF 1

// Pblock enum
enum { anisotropic_pblock, normal_pblock };

// Parameter enum
enum {
		param_distribution,
		param_color, param_color_map, param_color_map_on,
		param_roughness, param_roughness_map, param_roughness_map_on,
		param_anisotropy, param_anisotropy_map, param_anisotropy_map_on,
		param_rotation, param_rotation_map, param_rotation_map_on
	};

// Subtex enum
enum {
	subtex_color,
	subtex_roughness,
	subtex_anisotropy,
	subtex_rotation,
	subtex_nparam_strength,
	subtex_nparam_color,
};

static ParamBlockDesc2 mat_anisotropic_pblock_desc(
	// Pblock data
	anisotropic_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialAnisotropicClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_ANISOTROPIC,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Distribution
		param_distribution,
		_T("distribution"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_DISTRIBUTION,
		p_default, AnisotropicDistribution::GGX,
		p_range, AnisotropicDistribution::ASHIKHMIN_SHIRLEY, AnisotropicDistribution::BECKMANN,
		p_ui, TYPE_RADIO, 4, IDC_RADIO_DIST_AS, IDC_RADIO_DIST_MULTI_GGX, IDC_RADIO_DIST_GGX, IDC_RADIO_DIST_BECKMANN,
		p_end,
		// Color
		param_color,
		_T("color"),
		TYPE_RGBA,
		P_ANIMATABLE,
		IDS_COLOR,
		p_default, Color(0.7f, 0.7f, 0.7f),
		p_ui, TYPE_COLORSWATCH, IDC_SWATCH,
		p_end,
		// Color texmap
		param_color_map,
		_T("color_map"),
		TYPE_TEXMAP,
		0,
		IDS_COLOR_TEXMAP,
		p_subtexno, subtex_color,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_COLOR,
		p_end,
		// Color texmap on
		param_color_map_on,
		_T("color_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_COLOR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_COLOR,
		p_end,
		// Roughness
		param_roughness,
		_T("roughness"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ROUGHNESS,
		p_default, 0.0f,
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
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ROUGH,
		p_end,
		// Roughness texmap on
		param_roughness_map_on,
		_T("roughness_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ROUGHNESS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ROUGH,
		p_end,
		// Anisotropy
		param_anisotropy,
		_T("anisotropy"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ANISOTROPY,
		p_default, 0.5f,
		p_range, -1.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ANISO, IDC_SPIN_ANISO, 0.01f,
		p_end,
		// Anisotropy texmap
		param_anisotropy_map,
		_T("anisotropy_map"),
		TYPE_TEXMAP,
		0,
		IDS_ANISOTROPY_TEXMAP,
		p_subtexno, subtex_anisotropy,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ANISO,
		p_end,
		// Anisotropy texmap on
		param_anisotropy_map_on,
		_T("anisotropy_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ANISOTROPY_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ANISO,
		p_end,
		// Rotation
		param_rotation,
		_T("rotation"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ROTATION,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ROT, IDC_SPIN_ROT, 0.01f,
		p_end,
		// Rotation texmap
		param_rotation_map,
		_T("rotation_map"),
		TYPE_TEXMAP,
		0,
		IDS_ROTATION_TEXMAP,
		p_subtexno, subtex_rotation,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ROT,
		p_end,
		// Rotation texmap on
		param_rotation_map_on,
		_T("rotation_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ROTATION_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ROT,
		p_end,
	p_end
	);

MAKE_COMMON_NORMAL_PBLOCK(normal_pblock, GetCyclesMaterialAnisotropicClassDesc())

CyclesAnisotropicMat::CyclesAnisotropicMat() : CyclesPluginMatBase(true, true)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;

	color_specular = Color(1.0f, 1.0f, 1.0f);
	shine_strength = 1.0f;
}

AnisotropicDistribution CyclesAnisotropicMat::GetDistribution(const TimeValue t, Interval& iv)
{
	const int distribution = pblock->GetInt(param_distribution, t, iv);

	if (distribution < 0 || distribution >= static_cast<int>(AnisotropicDistribution::COUNT)) {
		return AnisotropicDistribution::GGX;
	}

	return static_cast<AnisotropicDistribution>(distribution);
}

Color CyclesAnisotropicMat::GetBsdfColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesAnisotropicMat::GetBsdfColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}

	return nullptr;
}

float CyclesAnisotropicMat::GetRoughness(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_roughness, t, iv);
}

Texmap* CyclesAnisotropicMat::GetRoughnessTexmap()
{
	if (SubTexmapOn(subtex_roughness)) {
		return GetSubTexmap(subtex_roughness);
	}

	return nullptr;
}

float CyclesAnisotropicMat::GetAnisotropy(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_anisotropy, t, iv);
}

Texmap* CyclesAnisotropicMat::GetAnisotropyTexmap()
{
	if (SubTexmapOn(subtex_anisotropy)) {
		return GetSubTexmap(subtex_anisotropy);
	}

	return nullptr;
}

float CyclesAnisotropicMat::GetRotation(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_rotation, t, iv);
}

Texmap* CyclesAnisotropicMat::GetRotationTexmap()
{
	if (SubTexmapOn(subtex_rotation)) {
		return GetSubTexmap(subtex_rotation);
	}

	return nullptr;
}

ClassDesc2* CyclesAnisotropicMat::GetClassDesc() const
{
	return GetCyclesMaterialAnisotropicClassDesc();
}

ParamBlockDesc2* CyclesAnisotropicMat::GetParamBlockDesc()
{
	return &mat_anisotropic_pblock_desc;
}

ParamBlockDesc2* CyclesAnisotropicMat::GetNormalParamBlockDesc()
{
	return &common_normal_pblock_desc;
}

void CyclesAnisotropicMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesAnisotropicMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Color");
	color_tex_slot.enabled_param_id = param_color_map_on;
	color_tex_slot.param_id = param_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;
	
	SubtexSlotDescriptor rough_tex_slot;
	rough_tex_slot.display_name = _T("Roughness");
	rough_tex_slot.enabled_param_id = param_roughness_map_on;
	rough_tex_slot.param_id = param_roughness_map;
	rough_tex_slot.slot_type = MAPSLOT_TEXTURE;
	rough_tex_slot.subtex_id = subtex_roughness;
	
	SubtexSlotDescriptor aniso_tex_slot;
	aniso_tex_slot.display_name = _T("Anisotropy");
	aniso_tex_slot.enabled_param_id = param_anisotropy_map_on;
	aniso_tex_slot.param_id = param_anisotropy_map;
	aniso_tex_slot.slot_type = MAPSLOT_TEXTURE;
	aniso_tex_slot.subtex_id = subtex_anisotropy;
	
	SubtexSlotDescriptor rotation_tex_slot;
	rotation_tex_slot.display_name = _T("Rotation");
	rotation_tex_slot.enabled_param_id = param_rotation_map_on;
	rotation_tex_slot.param_id = param_rotation_map;
	rotation_tex_slot.slot_type = MAPSLOT_TEXTURE;
	rotation_tex_slot.subtex_id = subtex_rotation;
	
	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(rough_tex_slot);
	subtex_slots.push_back(aniso_tex_slot);
	subtex_slots.push_back(rotation_tex_slot);
}

void CyclesAnisotropicMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	UpdateVPTexmap(subtex_color);

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval color_valid = FOREVER;
	const Color bsdf_color = pblock->GetColor(param_color, t, color_valid);
	color_diffuse = bsdf_color;

	Interval roughness_valid = FOREVER;
	const float roughness = pblock->GetFloat(param_roughness, t, roughness_valid);
	shininess = 1.0f - roughness;

	Interval anisotropy_valid = FOREVER;
	pblock->GetFloat(param_anisotropy, t, anisotropy_valid);

	Interval rotation_valid = FOREVER;
	pblock->GetFloat(param_rotation, t, rotation_valid);

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = color_valid & roughness_valid & anisotropy_valid & rotation_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
