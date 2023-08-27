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
 
#include "plugin_mat_sss.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0
#define NORMAL_PBLOCK_REF 1

// Pblock enum
enum { sss_pblock, normal_pblock };

// Parameter enum
enum {
	param_falloff,
	param_color,
	param_color_map,
	param_color_map_on,
	param_scale,
	param_scale_map,
	param_scale_map_on,
	param_radius_x,
	param_radius_y,
	param_radius_z,
	param_tex_blur,
	param_tex_blur_map,
	param_tex_blur_map_on
};

// Subtex enum
enum {
	subtex_color,
	subtex_scale,
	subtex_tex_blur,
	subtex_nparam_strength,
	subtex_nparam_color,
};

static ParamBlockDesc2 mat_sss_pblock_desc(
	// Pblock data
	sss_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialSubsurfaceScatterClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_SUBSURFACE_SCATTER,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Falloff
		param_falloff,
		_T("falloff"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_FALLOFF,
		p_default, SubsurfaceFalloff::BURLEY,
		p_range, SubsurfaceFalloff::CUBIC, SubsurfaceFalloff::RANDOM_WALK,
		p_ui, TYPE_RADIO, 4, IDC_RADIO_FALLOFF_CUBIC, IDC_RADIO_FALLOFF_GAUSSIAN, IDC_RADIO_FALLOFF_BURLEY, IDC_RADIO_FALLOFF_RANDOM_WALK,
		p_end,
		// Color
		param_color,
		_T("color"),
		TYPE_RGBA,
		P_ANIMATABLE,
		IDS_COLOR,
		p_default, Color(0.8f, 0.8f, 0.8f),
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
		// Scale
		param_scale,
		_T("scale"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SCALE,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SCALE, IDC_SPIN_SCALE, 0.1f,
		p_end,
		// Scale texmap
		param_scale_map,
		_T("scale_map"),
		TYPE_TEXMAP,
		0,
		IDS_SCALE_TEXMAP,
		p_subtexno, subtex_scale,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_SCALE,
		p_end,
		// Scale texmap on
		param_scale_map_on,
		_T("scale_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_SCALE_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_SCALE,
		p_end,
		// Radius x
		param_radius_x,
		_T("radius_x"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_RADIUS_X,
		p_default, 1.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_RADIUS_X, IDC_SPIN_RADIUS_X, 0.1f,
		p_end,
		// Radius y
		param_radius_y,
		_T("radius_y"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_RADIUS_Y,
		p_default, 1.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_RADIUS_Y, IDC_SPIN_RADIUS_Y, 0.1f,
		p_end,
		// Radius z
		param_radius_z,
		_T("radius_z"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_RADIUS_Z,
		p_default, 1.0f,
		p_range, 0.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_RADIUS_Z, IDC_SPIN_RADIUS_Z, 0.1f,
		p_end,
		// Texture blur
		param_tex_blur,
		_T("texture_blur"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_TEX_BLUR,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_TEX_BLUR, IDC_SPIN_TEX_BLUR, 0.1f,
		p_end,
		// Texture blur texmap
		param_tex_blur_map,
		_T("texture_blur_map"),
		TYPE_TEXMAP,
		0,
		IDS_TEX_BLUR_TEXMAP,
		p_subtexno, subtex_tex_blur,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_BLUR,
		p_end,
		// Texture blur texmap on
		param_tex_blur_map_on,
		_T("texture_blur_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEX_BLUR_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_BLUR,
		p_end,
	p_end
	);

MAKE_COMMON_NORMAL_PBLOCK(normal_pblock, GetCyclesMaterialSubsurfaceScatterClassDesc())

CyclesSubsurfaceScatterMat::CyclesSubsurfaceScatterMat() : CyclesPluginMatBase(true, true)
{
	pblock = nullptr;

	this->Reset();

	shine_strength = 0.0f;

	Initialize();

	node_type = ClosureType::SURFACE;
}

SubsurfaceFalloff CyclesSubsurfaceScatterMat::GetFalloff(const TimeValue t, Interval& iv)
{
	const int falloff = pblock->GetInt(param_falloff, t, iv);
	if (falloff < 0 || falloff >= static_cast<int>(SubsurfaceFalloff::COUNT)) {
		return SubsurfaceFalloff::BURLEY;
	}
	return static_cast<SubsurfaceFalloff>(falloff);
}

Color CyclesSubsurfaceScatterMat::GetBsdfColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

Texmap* CyclesSubsurfaceScatterMat::GetBsdfColorTexmap()
{
	if (SubTexmapOn(subtex_color)) {
		return GetSubTexmap(subtex_color);
	}

	return nullptr;
}

float CyclesSubsurfaceScatterMat::GetScale(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_scale, t, iv);
}

Texmap* CyclesSubsurfaceScatterMat::GetScaleTexmap()
{
	if (SubTexmapOn(subtex_scale)) {
		return GetSubTexmap(subtex_scale);
	}

	return nullptr;
}

Point3 CyclesSubsurfaceScatterMat::GetRadius(const TimeValue t, Interval& iv)
{
	Point3 result;

	Interval x_valid = FOREVER;
	result.x = pblock->GetFloat(param_radius_x, t, x_valid);
	Interval y_valid = FOREVER;
	result.y = pblock->GetFloat(param_radius_y, t, y_valid);
	Interval z_valid = FOREVER;
	result.z = pblock->GetFloat(param_radius_z, t, z_valid);

	iv &= x_valid & y_valid & z_valid;

	return result;
}

float CyclesSubsurfaceScatterMat::GetTexBlur(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_tex_blur, t, iv);
}

Texmap* CyclesSubsurfaceScatterMat::GetTexBlurTexmap()
{
	if (SubTexmapOn(subtex_tex_blur)) {
		return GetSubTexmap(subtex_tex_blur);
	}

	return nullptr;
}

ClassDesc2* CyclesSubsurfaceScatterMat::GetClassDesc() const
{
	return GetCyclesMaterialSubsurfaceScatterClassDesc();
}

ParamBlockDesc2* CyclesSubsurfaceScatterMat::GetParamBlockDesc()
{
	return &mat_sss_pblock_desc;
}

ParamBlockDesc2* CyclesSubsurfaceScatterMat::GetNormalParamBlockDesc()
{
	return &common_normal_pblock_desc;
}

void CyclesSubsurfaceScatterMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesSubsurfaceScatterMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor color_tex_slot;
	color_tex_slot.display_name = _T("Color");
	color_tex_slot.enabled_param_id = param_color_map_on;
	color_tex_slot.param_id = param_color_map;
	color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	color_tex_slot.subtex_id = subtex_color;

	SubtexSlotDescriptor scale_tex_slot;
	scale_tex_slot.display_name = _T("Scale");
	scale_tex_slot.enabled_param_id = param_scale_map_on;
	scale_tex_slot.param_id = param_scale_map;
	scale_tex_slot.slot_type = MAPSLOT_TEXTURE;
	scale_tex_slot.subtex_id = subtex_scale;

	SubtexSlotDescriptor blur_tex_slot;
	blur_tex_slot.display_name = _T("Texture Blur");
	blur_tex_slot.enabled_param_id = param_tex_blur_map_on;
	blur_tex_slot.param_id = param_tex_blur_map;
	blur_tex_slot.slot_type = MAPSLOT_TEXTURE;
	blur_tex_slot.subtex_id = subtex_tex_blur;
	
	subtex_slots.push_back(color_tex_slot);
	subtex_slots.push_back(scale_tex_slot);
	subtex_slots.push_back(blur_tex_slot);
}

void CyclesSubsurfaceScatterMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	UpdateVPTexmap(subtex_color);

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval falloff_valid = FOREVER;
	pblock->GetInt(param_falloff, t, falloff_valid);

	Interval color_valid = FOREVER;
	const Color bsdf_color = pblock->GetColor(param_color, t, color_valid);

	color_ambient = bsdf_color;
	color_diffuse = bsdf_color;

	Interval scale_valid = FOREVER;
	pblock->GetFloat(param_scale, t, scale_valid);

	Interval radius_x_valid = FOREVER;
	pblock->GetFloat(param_radius_x, t, radius_x_valid);

	Interval radius_y_valid = FOREVER;
	pblock->GetFloat(param_radius_y, t, radius_y_valid);

	Interval radius_z_valid = FOREVER;
	pblock->GetFloat(param_radius_z, t, radius_z_valid);

	Interval tex_blur_valid = FOREVER;
	pblock->GetFloat(param_tex_blur, t, scale_valid);

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = falloff_valid & color_valid & scale_valid & radius_x_valid & radius_y_valid & radius_z_valid & tex_blur_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
