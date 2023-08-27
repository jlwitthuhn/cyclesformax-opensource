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
 
#include "plugin_tex_sky.h"

#include <iparamm2.h>

#include "max_classdesc_tex.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { sky_pblock };

// Parameter enum
enum { param_sky_type, param_sun_x, param_sun_y, param_sun_z, param_turbidity, param_ground_albedo };

// Sky type enum
enum { sky_type_hw, sky_type_preetham };

static ParamBlockDesc2 texmap_sky_pblock_desc (
	// Pblock data
	sky_pblock,
	_T("pblock"),
	0,
	GetCyclesTexmapSkyClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_TEXMAP_SKY,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Sky type
		param_sky_type,
		_T("sky_type"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_SKY_TYPE,
		p_default, sky_type_hw,
		p_range, sky_type_hw, sky_type_preetham,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_SKY_TYPE_HW, IDC_RADIO_SKY_TYPE_PREETHAM,
		p_end,
		// Sun vector x
		param_sun_x,
		_T("sun_vec_x"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SUN_VEC_X,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SUN_X, IDC_SPIN_SUN_X, 0.1f,
		p_end,
		// Sun vector y
		param_sun_y,
		_T("sun_vec_y"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SUN_VEC_Y,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SUN_Y, IDC_SPIN_SUN_Y, 0.1f,
		p_end,
		// Sun vector z
		param_sun_z,
		_T("sun_vec_z"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SUN_VEC_Z,
		p_default, 1.0f,
		p_range, -100.0f, 100.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SUN_Z, IDC_SPIN_SUN_Z, 0.1f,
		p_end,
		// Turbidity
		param_turbidity,
		_T("turbidity"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_TURBIDITY,
		p_default, 2.2f,
		p_range, 1.0f, 10.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_TURB, IDC_SPIN_TURB, 0.1f,
		p_end,
		// Ground albedo
		param_ground_albedo,
		_T("ground_albedo"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_GROUND_ALBEDO,
		p_default, 0.3f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_GA, IDC_SPIN_GA, 0.1f,
		p_end,
	p_end
	);

CyclesSkyEnvTexmap::CyclesSkyEnvTexmap() : CyclesPluginTexmap()
{
	this->Reset();
}

SkyType CyclesSkyEnvTexmap::GetSkyType(const TimeValue t, Interval& iv)
{
	const int type = pblock->GetInt(param_sky_type, t, iv);

	switch (type) {
	case sky_type_hw:
		return SkyType::HOSEK_WILKIE;
	case sky_type_preetham:
		return SkyType::PREETHAM;
	}

	return SkyType::HOSEK_WILKIE;
}

ccl::float3 CyclesSkyEnvTexmap::GetSunVector(const TimeValue t, Interval& iv)
{
	Interval x_valid = FOREVER;
	const float x = pblock->GetFloat(param_sun_x, t, iv);

	Interval y_valid = FOREVER;
	const float y = pblock->GetFloat(param_sun_y, t, iv);

	Interval z_valid = FOREVER;
	const float z = pblock->GetFloat(param_sun_z, t, iv);

	iv &= (x_valid & y_valid & z_valid);
	return ccl::make_float3(x, y, z);
}

float CyclesSkyEnvTexmap::GetTurbidity(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_turbidity, t, iv);
}

float CyclesSkyEnvTexmap::GetGroundAlbedo(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_ground_albedo, t, iv);
}

ClassDesc2* CyclesSkyEnvTexmap::GetClassDesc() const
{
	return GetCyclesTexmapSkyClassDesc();
}

ParamBlockDesc2* CyclesSkyEnvTexmap::GetParamBlockDesc()
{
	return &texmap_sky_pblock_desc;
}

Class_ID CyclesSkyEnvTexmap::ClassID()
{
	return CYCLES_TEXMAP_SKY_CLASS;
}

Interval CyclesSkyEnvTexmap::Validity(const TimeValue t)
{
	Interval pblock_valid = FOREVER;
	pblock->GetValidity(t, pblock_valid);

	return pblock_valid;
}

RefTargetHandle CyclesSkyEnvTexmap::Clone(RemapDir& remap)
{
	CyclesSkyEnvTexmap* const new_tex = new CyclesSkyEnvTexmap();

	BaseClone(this, new_tex, remap);
	
	new_tex->ReplaceReference(PBLOCK_REF, remap.CloneRef(pblock));

	return new_tex;
}
