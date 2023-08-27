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
 
#include "plugin_mat_toon.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0
#define NORMAL_PBLOCK_REF 1

// Pblock enum
enum { toon_pblock, normal_pblock };

// Parameter enum
enum {
		param_component,
		param_color, param_color_map, param_color_map_on,
		param_size, param_size_map, param_size_map_on,
		param_smooth, param_smooth_map, param_smooth_map_on
	};

// Subtex enum
enum {
	subtex_color,
	subtex_size,
	subtex_smooth,
	subtex_nparam_strength,
	subtex_nparam_color,
};

static ParamBlockDesc2 mat_toon_pblock_desc(
	// Pblock data
	toon_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialToonClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_TOON,
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
		p_default, ToonComponent::DIFFUSE,
		p_range, ToonComponent::GLOSSY, ToonComponent::DIFFUSE,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_COMP_GLOSSY, IDC_RADIO_COMP_DIFFUSE, // Order here must match enum
		p_end,
		// Color
		param_color,
		_T("color"),
		TYPE_RGBA,
		P_ANIMATABLE,
		IDS_COLOR,
		p_default, Color(1.0f, 1.0f, 1.0f),
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
		// Size
		param_size,
		_T("size"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SIZE,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SIZE, IDC_SPIN_SIZE, 0.01f,
		p_end,
		// Size texmap
		param_size_map,
		_T("size_map"),
		TYPE_TEXMAP,
		0,
		IDS_SIZE_TEXMAP,
		p_subtexno, subtex_size,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SIZE,
		p_end,
		// Size texmap on
		param_size_map_on,
		_T("size_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SIZE_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SIZE,
		p_end,
		// Smooth
		param_smooth,
		_T("smooth"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SMOOTH,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SMOOTH, IDC_SPIN_SMOOTH, 0.01f,
		p_end,
		// Smooth texmap
		param_smooth_map,
		_T("smooth_map"),
		TYPE_TEXMAP,
		0,
		IDS_SMOOTH_TEXMAP,
		p_subtexno, subtex_smooth,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SMOOTH,
		p_end,
		// Smooth texmap on
		param_smooth_map_on,
		_T("smooth_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SMOOTH_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SMOOTH,
		p_end,
	p_end
	);

MAKE_COMMON_NORMAL_PBLOCK(normal_pblock, GetCyclesMaterialToonClassDesc())

CyclesToonMat::CyclesToonMat() : CyclesPluginMatBase(true, true)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;
}

ToonComponent CyclesToonMat::GetComponent(const TimeValue t, Interval& iv)
{
	const int component = pblock->GetInt(param_component, t, iv);
	if (component < 0 || component >= static_cast<int>(ToonComponent::COUNT)) {
		return ToonComponent::DIFFUSE;
	}
	return static_cast<ToonComponent>(component);
}

Color CyclesToonMat::GetMatColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesToonMat::GetMatColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}
	return nullptr;
}

float CyclesToonMat::GetSize(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_size, t, iv);
}

Texmap* CyclesToonMat::GetSizeTexmap()
{
	if (SubTexmapOn(subtex_size)) {
		return GetSubTexmap(subtex_size);
	}
	return nullptr;
}

float CyclesToonMat::GetSmooth(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_smooth, t, iv);
}

Texmap* CyclesToonMat::GetSmoothTexmap()
{
	if (SubTexmapOn(subtex_smooth)) {
		return GetSubTexmap(subtex_smooth);
	}
	return nullptr;
}

ClassDesc2* CyclesToonMat::GetClassDesc() const
{
	return GetCyclesMaterialToonClassDesc();
}

ParamBlockDesc2* CyclesToonMat::GetParamBlockDesc()
{
	return &mat_toon_pblock_desc;
}

ParamBlockDesc2* CyclesToonMat::GetNormalParamBlockDesc()
{
	return &common_normal_pblock_desc;
}

void CyclesToonMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesToonMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Color");
	color_tex_slot.enabled_param_id = param_color_map_on;
	color_tex_slot.param_id = param_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;
	
	SubtexSlotDescriptor size_tex_slot;
	size_tex_slot.display_name = _T("Size");
	size_tex_slot.enabled_param_id = param_size_map_on;
	size_tex_slot.param_id = param_size_map;
	size_tex_slot.slot_type = MAPSLOT_TEXTURE;
	size_tex_slot.subtex_id = subtex_size;
	
	SubtexSlotDescriptor smooth_tex_slot;
	smooth_tex_slot.display_name = _T("Smooth");
	smooth_tex_slot.enabled_param_id = param_smooth_map_on;
	smooth_tex_slot.param_id = param_smooth_map;
	smooth_tex_slot.slot_type = MAPSLOT_TEXTURE;
	smooth_tex_slot.subtex_id = subtex_smooth;
	
	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(size_tex_slot);
	subtex_slots.push_back(smooth_tex_slot);
}

void CyclesToonMat::Update(const TimeValue t, Interval& valid)
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

	color_ambient = bsdf_color;
	color_diffuse = bsdf_color;

	Interval size_valid = FOREVER;
	pblock->GetFloat(param_size, t, size_valid);

	Interval smooth_valid = FOREVER;
	pblock->GetFloat(param_smooth, t, smooth_valid);

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = component_valid & color_valid & size_valid & smooth_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
