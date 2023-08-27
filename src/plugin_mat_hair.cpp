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
 
#include "plugin_mat_hair.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { hair_pblock };

// Parameter enum
enum {
	param_component,
	param_color, param_color_map, param_color_map_on,
	param_offset, param_offset_map, param_offset_map_on,
	param_roughness_u, param_roughness_u_map, param_roughness_u_map_on,
	param_roughness_v, param_roughness_v_map, param_roughness_v_map_on
};

// Subtex enum
enum {
	subtex_color,
	subtex_offset,
	subtex_roughness_u,
	subtex_roughness_v
};

static ParamBlockDesc2 mat_hair_pblock_desc(
	// Pblock data
	hair_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialHairClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_HAIR,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Component
		param_component,
		_T("component"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_COMPONENT,
		p_default, HairComponent::REFLECTION,
		p_range, HairComponent::TRANSMISSION, HairComponent::REFLECTION,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_COMP_TRANSMISSION, IDC_RADIO_COMP_REFLECTION, // Order here must match enum
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
		// Offset
		param_offset,
		_T("offset"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_OFFSET,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_OFFSET, IDC_SPIN_OFFSET, 0.01f,
		p_end,
		// Offset texmap
		param_offset_map,
		_T("offset_map"),
		TYPE_TEXMAP,
		0,
		IDS_OFFSET_TEXMAP,
		p_subtexno, subtex_offset,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_OFFSET,
		p_end,
		// Offset texmap on
		param_offset_map_on,
		_T("offset_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_OFFSET_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_OFFSET,
		p_end,
		// Roughness U
		param_roughness_u,
		_T("roughness_u"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ROUGHNESS_U,
		p_default, 0.1f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ROUGHNESS_U, IDC_SPIN_ROUGHNESS_U, 0.01f,
		p_end,
		// Roughness U texmap
		param_roughness_u_map,
		_T("roughness_u_map"),
		TYPE_TEXMAP,
		0,
		IDS_ROUGHNESS_U_TEXMAP,
		p_subtexno, subtex_roughness_u,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ROUGHNESS_U,
		p_end,
		// Roughness U texmap on
		param_roughness_u_map_on,
		_T("roughness_u_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ROUGHNESS_U_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ROUGHNESS_U,
		p_end,
		// Roughness V
		param_roughness_v,
		_T("roughness_v"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ROUGHNESS_V,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ROUGHNESS_V, IDC_SPIN_ROUGHNESS_V, 0.01f,
		p_end,
		// Roughness V texmap
		param_roughness_v_map,
		_T("roughness_v_map"),
		TYPE_TEXMAP,
		0,
		IDS_ROUGHNESS_V_TEXMAP,
		p_subtexno, subtex_roughness_v,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_ROUGHNESS_V,
		p_end,
		// Roughness V texmap on
		param_roughness_v_map_on,
		_T("roughness_v_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_ROUGHNESS_V_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_ROUGHNESS_V,
		p_end,
	p_end
	);


CyclesHairMat::CyclesHairMat() : CyclesPluginMatBase(true, false)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;
}

HairComponent CyclesHairMat::GetComponent(const TimeValue t, Interval& iv)
{
	const int distribution = pblock->GetInt(param_component, t, iv);

	if (distribution < 0 || distribution >= static_cast<int>(HairComponent::COUNT)) {
		return HairComponent::REFLECTION;
	}

	return static_cast<HairComponent>(distribution);
}

Color CyclesHairMat::GetHairColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesHairMat::GetHairColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}

	return nullptr;
}

float CyclesHairMat::GetOffset(const TimeValue t, Interval& iv)
{
	const float paramOffset = pblock->GetFloat(param_offset, t, iv);
	return (paramOffset - 0.5f) * 180;
}

Texmap* CyclesHairMat::GetOffsetTexmap()
{
	if (SubTexmapOn(subtex_offset)) {
		return GetSubTexmap(subtex_offset);
	}

	return nullptr;
}

float CyclesHairMat::GetRoughnessU(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_roughness_u, t, iv);
}

Texmap* CyclesHairMat::GetRoughnessUTexmap()
{
	if (SubTexmapOn(subtex_roughness_u)) {
		return GetSubTexmap(subtex_roughness_u);
	}

	return nullptr;
}

float CyclesHairMat::GetRoughnessV(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_roughness_v, t, iv);
}

Texmap* CyclesHairMat::GetRoughnessVTexmap()
{
	if (SubTexmapOn(subtex_roughness_v)) {
		return GetSubTexmap(subtex_roughness_v);
	}

	return nullptr;
}

ClassDesc2* CyclesHairMat::GetClassDesc() const
{
	return GetCyclesMaterialHairClassDesc();
}

ParamBlockDesc2* CyclesHairMat::GetParamBlockDesc()
{
	return &mat_hair_pblock_desc;
}

void CyclesHairMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesHairMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Color");
	color_tex_slot.enabled_param_id = param_color_map_on;
	color_tex_slot.param_id = param_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;

	SubtexSlotDescriptor offset_tex_slot;
	offset_tex_slot.display_name = _T("Offset");
	offset_tex_slot.enabled_param_id = param_offset_map_on;
	offset_tex_slot.param_id = param_offset_map;
	offset_tex_slot.slot_type = MAPSLOT_TEXTURE;
	offset_tex_slot.subtex_id = subtex_offset;

	SubtexSlotDescriptor roughness_u_tex_slot;
	roughness_u_tex_slot.display_name = _T("Roughness U");
	roughness_u_tex_slot.enabled_param_id = param_roughness_u_map_on;
	roughness_u_tex_slot.param_id = param_roughness_u_map;
	roughness_u_tex_slot.slot_type = MAPSLOT_TEXTURE;
	roughness_u_tex_slot.subtex_id = subtex_roughness_u;

	SubtexSlotDescriptor roughness_v_tex_slot;
	roughness_v_tex_slot.display_name = _T("Roughness V");
	roughness_v_tex_slot.enabled_param_id = param_roughness_v_map_on;
	roughness_v_tex_slot.param_id = param_roughness_v_map;
	roughness_v_tex_slot.slot_type = MAPSLOT_TEXTURE;
	roughness_v_tex_slot.subtex_id = subtex_roughness_v;

	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(offset_tex_slot);
	subtex_slots.push_back(roughness_u_tex_slot);
	subtex_slots.push_back(roughness_v_tex_slot);
}

void CyclesHairMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	UpdateVPTexmap(subtex_color);

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval component_valid = FOREVER;
	pblock->GetInt(param_component, t, component_valid);

	Interval color_valid = FOREVER;
	const Color bsdf_color = pblock->GetColor(param_color, t, color_valid);

	color_diffuse = bsdf_color;
	color_specular = bsdf_color;

	Interval offset_valid = FOREVER;
	pblock->GetFloat(param_offset, t, offset_valid);

	Interval rough_u_valid = FOREVER;
	pblock->GetFloat(param_roughness_u, t, rough_u_valid);

	Interval rough_v_valid = FOREVER;
	pblock->GetFloat(param_roughness_v, t, rough_v_valid);

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = component_valid & color_valid & offset_valid & rough_u_valid & rough_v_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
