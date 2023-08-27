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
 
#include "plugin_mat_emission.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { emission_pblock };

// Parameter enum
enum { param_color, param_color_map, param_color_map_on, param_strength, param_strength_map, param_strength_map_on };

// Subtex enum
enum { subtex_color, subtex_strength };

static ParamBlockDesc2 mat_emission_pblock_desc(
	// Pblock data
	emission_pblock,                      // paramblock id
	_T("pblock"),                         // maxscript name
	0,                                    // string resource id for track view
	GetCyclesMaterialEmissionClassDesc(), // pointer to classdesc
	P_AUTO_CONSTRUCT + P_AUTO_UI,         // flags
	PBLOCK_REF,                           // ref number for this pblock
	// Interface stuff
	IDD_PANEL_MAT_EMISSION,
	IDS_PARAMS,
	0,
	0,
	NULL,
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
		// Strength
		param_strength,
		_T("strength"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_STRENGTH,
		p_default, 1.0f,
		p_range, 0.0f, 10000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_STRENGTH, IDC_SPIN_STRENGTH, 0.1f,
		p_end,
		// Strength texmap
		param_strength_map,
		_T("strength_map"),
		TYPE_TEXMAP,
		0,
		IDS_STRENGTH_TEXMAP,
		p_subtexno, subtex_strength,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_STRENGTH,
		p_end,
		// Strength texmap on
		param_strength_map_on,
		_T("strength_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_STRENGTH_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_STRENGTH,
		p_end,
	p_end
	);

CyclesEmissionMat::CyclesEmissionMat() : CyclesPluginMatBase(true, false)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;
}

Color CyclesEmissionMat::GetLightColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesEmissionMat::GetLightColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}

	return nullptr;
}

float CyclesEmissionMat::GetLightStrength(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_strength, t, iv);
}

Texmap* CyclesEmissionMat::GetLightStrengthTexmap()
{
	if (SubTexmapOn(subtex_strength)) {
		return GetSubTexmap(subtex_strength);
	}

	return nullptr;
}

ClassDesc2* CyclesEmissionMat::GetClassDesc() const
{
	return GetCyclesMaterialEmissionClassDesc();
}

ParamBlockDesc2* CyclesEmissionMat::GetParamBlockDesc()
{
	return &mat_emission_pblock_desc;
}

void CyclesEmissionMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesEmissionMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Color");
	color_tex_slot.enabled_param_id = param_color_map_on;
	color_tex_slot.param_id = param_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;

	SubtexSlotDescriptor strength_tex_slot;
	strength_tex_slot.display_name = _T("Strength");
	strength_tex_slot.enabled_param_id = param_strength_map_on;
	strength_tex_slot.param_id = param_strength_map;
	strength_tex_slot.slot_type = MAPSLOT_TEXTURE;
	strength_tex_slot.subtex_id = subtex_strength;
	
	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(strength_tex_slot);
}

void CyclesEmissionMat::Update(const TimeValue t, Interval& valid)
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

	color_ambient = bsdf_color;
	color_diffuse = bsdf_color;
	self_illum_color = bsdf_color;

	Interval strength_valid = FOREVER;
	const float strength = pblock->GetFloat(param_strength, t, strength_valid);

	self_illum = std::max(0.0f, std::min(1.0f, strength));

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = color_valid & strength_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
