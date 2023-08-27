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
 
#include "plugin_tex_environment.h"

#include <iparamm2.h>

#include "max_classdesc_tex.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { environment_pblock };

// Param enum
enum { param_bg_projection, param_bg_map, param_bg_map_enabled, param_light_projection, param_light_map, param_light_map_enabled };

// Projection enum
enum { proj_mirror_ball, proj_equirectangular, proj_backplate };

// Subtex enum
enum { subtex_bg_map, subtex_light_map };

static ParamBlockDesc2 texmap_environment_pblock_desc (
	// Pblock data
	environment_pblock,
	_T("pblock"),
	0,
	GetCyclesTexmapEnvironmentClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_TEXMAP_ENVIRONMENT,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Projection type
		param_bg_projection,
		_T("projection"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_PROJECTION,
		p_default, proj_equirectangular,
		p_range, proj_mirror_ball, proj_backplate,
		p_ui, TYPE_RADIO, 3, IDC_RADIO_PROJ_MB, IDC_RADIO_PROJ_EQUIRECTANGULAR, IDC_RADIO_PROJ_BACKPLATE,
		p_end,
		// Environment map
		param_bg_map,
		_T("background_map"),
		TYPE_TEXMAP,
		0,
		IDS_BACKGROUND_MAP,
		p_subtexno, subtex_bg_map,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_TEXMAP,
		p_end,
		// Environment map enabled
		param_bg_map_enabled,
		_T("background_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_BACKGROUND_MAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_TEXMAP,
		p_end,
		// Reflection projection type
		param_light_projection,
		_T("refl_projection"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_REFL_PROJECTION,
		p_default, proj_equirectangular,
		p_range, proj_mirror_ball, proj_equirectangular,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_PROJ_MB_REFL, IDC_RADIO_PROJ_EQUIRECTANGULAR_REFL,
		p_end,
		// Reflection map
		param_light_map,
		_T("lighting_map"),
		TYPE_TEXMAP,
		0,
		IDS_LIGHTING_MAP,
		p_subtexno, subtex_light_map,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_REFLECTION,
		p_end,
		// Reflection map enabled
		param_light_map_enabled,
		_T("lighting_map_enabled"),
		TYPE_BOOL,
		0,
		IDS_LIGHTING_MAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_REFLECTION,
		p_end,
	p_end
	);


EnvironmentTexmap::EnvironmentTexmap() : CyclesPluginTexmap()
{
	this->Reset();
}

EnvironmentProjection EnvironmentTexmap::GetBGProjection(const TimeValue t, Interval& iv)
{
	int projection = pblock->GetInt(param_bg_projection, t, iv);

	switch (projection) {
	case proj_mirror_ball:
		return EnvironmentProjection::MIRROR_BALL;
	case proj_equirectangular:
		return EnvironmentProjection::EQUIRECTANGULAR;
	case proj_backplate:
		return EnvironmentProjection::BACKPLATE;
	}

	return EnvironmentProjection::EQUIRECTANGULAR;
}

Texmap* EnvironmentTexmap::GetBGMap()
{
	if (SubTexmapOn(subtex_bg_map)) {
		return GetSubTexmap(subtex_bg_map);
	}

	return nullptr;
}

EnvironmentProjection EnvironmentTexmap::GetLightingProjection(const TimeValue t, Interval& iv)
{
	const int projection = pblock->GetInt(param_light_projection, t, iv);

	switch (projection) {
	case proj_mirror_ball:
		return EnvironmentProjection::MIRROR_BALL;
	case proj_equirectangular:
		return EnvironmentProjection::EQUIRECTANGULAR;
	}

	return EnvironmentProjection::EQUIRECTANGULAR;
}

Texmap* EnvironmentTexmap::GetLightingMap()
{
	if (SubTexmapOn(subtex_light_map)) {
		return GetSubTexmap(subtex_light_map);
	}

	return nullptr;
}

ClassDesc2* EnvironmentTexmap::GetClassDesc() const
{
	return GetCyclesTexmapEnvironmentClassDesc();
}

ParamBlockDesc2* EnvironmentTexmap::GetParamBlockDesc()
{
	return &texmap_environment_pblock_desc;
}

int EnvironmentTexmap::NumSubTexmaps()
{
	return 2;
}

Texmap* EnvironmentTexmap::GetSubTexmap(const int i)
{
	Interval texmap_valid = FOREVER;
	if (pblock != nullptr) {
		if (i == subtex_bg_map) {
			return pblock->GetTexmap(param_bg_map, 0, texmap_valid);
		}
		else if (i == subtex_light_map) {
			return pblock->GetTexmap(param_light_map, 0, texmap_valid);
		}
	}

	return nullptr;
}

int EnvironmentTexmap::MapSlotType(const int /*i*/)
{
	return MAPSLOT_TEXTURE;
}

void EnvironmentTexmap::SetSubTexmap(const int i, Texmap* const m)
{
	if (pblock != nullptr) {
		if (i == subtex_bg_map) {
			pblock->SetValue(param_bg_map, 0, m);
		}
		else if (i == subtex_light_map) {
			pblock->SetValue(param_light_map, 0, m);
		}
	}
}

int EnvironmentTexmap::SubTexmapOn(const int i)
{
	Interval texmap_on_valid = FOREVER;
	if (i == subtex_bg_map && GetSubTexmap(i) != nullptr) {
		return pblock->GetInt(param_bg_map_enabled, 0, texmap_on_valid);
	}
	else if (i == subtex_light_map && GetSubTexmap(i) != nullptr) {
		return pblock->GetInt(param_light_map_enabled, 0, texmap_on_valid);
	}
	return 0;
}

#if PLUGIN_SDK_VERSION < 2022
MSTR EnvironmentTexmap::GetSubTexmapSlotName(const int i)
#else
MSTR EnvironmentTexmap::GetSubTexmapSlotName(const int i, const bool)
#endif
{
	if (i == subtex_bg_map) {
		return _T("Background");
	}
	else if (i == subtex_light_map) {
		return _T("Lighting");
	}
	return _T("");
}

AColor EnvironmentTexmap::EvalColor(ShadeContext& sc)
{
	Texmap* texmap = GetSubTexmap(subtex_bg_map);
	if (SubTexmapOn(subtex_bg_map) && texmap != nullptr) {
		return texmap->EvalColor(sc);
	}
	return AColor(0, 0, 0);
}

Point3 EnvironmentTexmap::EvalNormalPerturb(ShadeContext& sc)
{
	Texmap* texmap = GetSubTexmap(subtex_bg_map);
	if (SubTexmapOn(subtex_bg_map) && texmap != nullptr) {
		return texmap->EvalNormalPerturb(sc);
	}
	return Point3(0.0f, 0.0f, 0.0f);
}

Class_ID EnvironmentTexmap::ClassID()
{
	return CYCLES_TEXMAP_ENVIRONMENT_CLASS;
}

Interval EnvironmentTexmap::Validity(const TimeValue t)
{
	Interval pblock_valid = FOREVER;
	pblock->GetValidity(t, pblock_valid);

	Interval texmap_valid = FOREVER;
	Texmap* const texmap = GetSubTexmap(subtex_bg_map);
	if (texmap != nullptr) {
		texmap_valid = texmap->Validity(t);
	}

	Interval refl_map_valid = FOREVER;
	Texmap* const refl_texmap = GetSubTexmap(subtex_light_map);
	if (refl_texmap != nullptr) {
		refl_map_valid = refl_texmap->Validity(t);
	}

	return pblock_valid & texmap_valid & refl_map_valid;
}

RefTargetHandle EnvironmentTexmap::Clone(RemapDir& remap)
{
	EnvironmentTexmap* new_tex = new EnvironmentTexmap();

	BaseClone(this, new_tex, remap);

	new_tex->ReplaceReference(PBLOCK_REF, remap.CloneRef(pblock));

	return new_tex;
}
