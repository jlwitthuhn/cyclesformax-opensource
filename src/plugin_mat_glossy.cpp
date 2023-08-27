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
 
#include "plugin_mat_glossy.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0
#define NORMAL_PBLOCK_REF 1

// Pblock enum
enum { glossy_pblock, normal_pblock };

// Param enum
enum { param_dist, param_color, param_color_map, param_color_map_on, param_roughness, param_roughness_map, param_roughness_map_on };

// Distribution enum, the order here must match the p_ui flag for param_dist
enum { dist_as, dist_multi_ggx, dist_ggx, dist_beckmann };

// Subtexture enum
enum {
	subtex_color,
	subtex_roughness,
	subtex_nparam_strength,
	subtex_nparam_color,
};

static ParamBlockDesc2 mat_glossy_pblock_desc(
	// Pblock data
	glossy_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialGlossyClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_GLOSSY,
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
		p_default, dist_ggx,
		p_range, dist_as, dist_beckmann,
		p_ui, TYPE_RADIO, 4, IDC_RADIO_DIST_AS, IDC_RADIO_DIST_MULTI_GGX, IDC_RADIO_DIST_GGX, IDC_RADIO_DIST_BECKMANN,  // Order here must match enum
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
	p_end
	);

MAKE_COMMON_NORMAL_PBLOCK(normal_pblock, GetCyclesMaterialGlossyClassDesc())

CyclesGlossyMat::CyclesGlossyMat() : CyclesPluginMatBase(true, true)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;

	shine_strength = 1.0f;
}

GlossyDistribution CyclesGlossyMat::GetDistribution(const TimeValue t, Interval& iv)
{
	const int distribution = pblock->GetInt(param_dist, t, iv);

	if (distribution < 0 || distribution >= static_cast<int>(GlossyDistribution::COUNT)) {
		return GlossyDistribution::GGX;
	}

	return static_cast<GlossyDistribution>(distribution);
}

Color CyclesGlossyMat::GetGlossyColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesGlossyMat::GetGlossyColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}

	return nullptr;
}

float CyclesGlossyMat::GetRoughness(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_roughness, t, iv);
}

Texmap* CyclesGlossyMat::GetRoughnessTexmap()
{
	if (SubTexmapOn(subtex_roughness)) {
		return GetSubTexmap(subtex_roughness);
	}

	return nullptr;
}

ClassDesc2* CyclesGlossyMat::GetClassDesc() const
{
	return GetCyclesMaterialGlossyClassDesc();
}

ParamBlockDesc2* CyclesGlossyMat::GetParamBlockDesc()
{
	return &mat_glossy_pblock_desc;
}

ParamBlockDesc2* CyclesGlossyMat::GetNormalParamBlockDesc()
{
	return &common_normal_pblock_desc;
}

void CyclesGlossyMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesGlossyMat::PopulateSubtexSlots()
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
	
	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(rough_tex_slot);
}

void CyclesGlossyMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	UpdateVPTexmap(subtex_color);

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval distribution_valid = FOREVER;
	pblock->GetInt(param_dist, t, distribution_valid);

	Interval color_valid = FOREVER;
	const Color bsdf_color = pblock->GetColor(param_color, t, color_valid);

	color_diffuse = bsdf_color;
	color_specular = bsdf_color;

	Interval roughness_valid = FOREVER;
	const float roughness = pblock->GetFloat(param_roughness, t, roughness_valid);
	shininess = 1.0f - roughness;

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = distribution_valid & color_valid & roughness_valid & subthings_valid;
	valid &= valid_interval;
	
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
