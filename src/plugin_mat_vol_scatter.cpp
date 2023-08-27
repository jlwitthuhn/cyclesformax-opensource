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
 
#include "plugin_mat_vol_scatter.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { scatter_pblock };

// Parameter enum
enum { param_color, param_density, param_anisotropy };

static ParamBlockDesc2 mat_scatter_pblock_desc(
	// Pblock data
	scatter_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialVolumeScatterClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_VOL_SCATTER,
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
		p_default, Color(0.7f, 0.7f, 0.7f),
		p_ui, TYPE_COLORSWATCH, IDC_SWATCH,
		p_end,
		// Density
		param_density,
		_T("density"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_DENSITY,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_DENSITY, IDC_SPIN_DENSITY, 0.1f,
		p_end,
		// Anisotropy
		param_anisotropy,
		_T("anisotropy"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_ANISOTROPY,
		p_default, 0.0f,
		p_range, -1.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_ANISOTROPY, IDC_SPIN_ANISOTROPY, 0.1f,
		p_end,
	p_end
	);

CyclesVolScatterMat::CyclesVolScatterMat() : CyclesPluginMatBase(false, false)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::VOLUME;
}

Color CyclesVolScatterMat::GetVolumeColor(const TimeValue t, Interval& iv)
{
	return pblock->GetColor(param_color, t, iv);
}

float CyclesVolScatterMat::GetDensity(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_density, t, iv);
}

float CyclesVolScatterMat::GetAnisotropy(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_anisotropy, t, iv);
}

ClassDesc2* CyclesVolScatterMat::GetClassDesc() const
{
	return GetCyclesMaterialVolumeScatterClassDesc();
}

ParamBlockDesc2* CyclesVolScatterMat::GetParamBlockDesc()
{
	return &mat_scatter_pblock_desc;
}

void CyclesVolScatterMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesVolScatterMat::PopulateSubtexSlots()
{
	// No sub-textures, do nothing
}

void CyclesVolScatterMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval color_valid = FOREVER;
	const Color vol_color = pblock->GetColor(param_color, t, color_valid);

	color_ambient = vol_color;
	color_diffuse = vol_color;

	Interval density_valid = FOREVER;
	pblock->GetFloat(param_density, t, density_valid);

	Interval anisotropy_valid = FOREVER;
	pblock->GetFloat(param_anisotropy, t, anisotropy_valid);

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = color_valid & density_valid & anisotropy_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
