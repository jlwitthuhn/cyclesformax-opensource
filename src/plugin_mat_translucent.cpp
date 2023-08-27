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
 
#include "plugin_mat_translucent.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0
#define NORMAL_PBLOCK_REF 1

// Pblock enum
enum { translucent_pblock, normal_pblock };

// Param enum
enum { param_color, param_color_map, param_color_map_on };

// Subtex enum
enum {
	subtex_color,
	subtex_nparam_strength,
	subtex_nparam_color,
};

static ParamBlockDesc2 mat_translucent_pblock_desc(
	// Pblock data
	translucent_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialTranslucentClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_TRANSLUCENT,
	IDS_PARAMS,
	0,
	0,
	NULL,
		//Color
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
		P_ANIMATABLE,
		IDS_COLOR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_COLOR,
		p_end,
	p_end
	);

MAKE_COMMON_NORMAL_PBLOCK(normal_pblock, GetCyclesMaterialTranslucentClassDesc())

CyclesTranslucentMat::CyclesTranslucentMat() : CyclesPluginMatBase(true, true)
{
	pblock = nullptr;

	this->Reset();

	transparency = 0.5f;
	shine_strength = 0.0f;

	Initialize();

	node_type = ClosureType::SURFACE;
}

Color CyclesTranslucentMat::GetTransColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesTranslucentMat::GetTransColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}

	return nullptr;
}

ClassDesc2* CyclesTranslucentMat::GetClassDesc() const
{
	return GetCyclesMaterialTranslucentClassDesc();
}

ParamBlockDesc2* CyclesTranslucentMat::GetParamBlockDesc()
{
	return &mat_translucent_pblock_desc;
}

ParamBlockDesc2* CyclesTranslucentMat::GetNormalParamBlockDesc()
{
	return &common_normal_pblock_desc;
}

void CyclesTranslucentMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesTranslucentMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Color");
	color_tex_slot.enabled_param_id = param_color_map_on;
	color_tex_slot.param_id = param_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;
	
	subtex_slots.push_back(color_tex_slot);
}

void CyclesTranslucentMat::Update(const TimeValue t, Interval& valid)
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

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = color_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
