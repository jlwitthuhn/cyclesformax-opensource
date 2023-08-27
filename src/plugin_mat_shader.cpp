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
 
#include "plugin_mat_shader.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "rend_shader_desc.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { pblock_shader, pblock_shader_params };

// Parameter enum
enum { param_mat_surface, param_mat_volume, param_mat_viewport };

// Submaterial enum
enum { submat_surface, submat_volume, submat_viewport };

static ParamBlockDesc2 mat_shader_pblock_desc(
	// Pblock data
	pblock_shader,
	_T("pblock"),
	0,
	GetCyclesMaterialShaderClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_SHADER,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Surface
		param_mat_surface,
		_T("mat_surface"),
		TYPE_MTL,
		0,
		IDS_SURFACE_MATERIAL,
		p_submtlno, submat_surface,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_SURFACE,
		p_end,
		//Volume
		param_mat_volume,
		_T("mat_volume"),
		TYPE_MTL,
		0,
		IDS_VOLUME_MATERIAL,
		p_submtlno, submat_volume,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_VOLUME,
		p_end,
		// Viewport
		param_mat_viewport,
		_T("mat_viewport"),
		TYPE_MTL,
		0,
		IDS_VIEWPORT_MATERIAL,
		p_submtlno, submat_viewport,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_VIEWPORT,
		p_end,
	p_end
	);

SHADER_PARAMS_ENUMS
SHADER_PARAMS_PBLOCK_DESC(GetCyclesMaterialShaderClassDesc())

CyclesShaderMat::CyclesShaderMat() : CyclesPluginMatBase(false, false)
{
	ParamBlockSlotDescriptor pblock_params_desc;
	pblock_params_desc.name = TSTR(L"pblock_params");
	pblock_params_desc.ref = pblock_shader_params;
	pblock_vec.push_back(pblock_params_desc);
	
	pblock = nullptr;

	this->Reset();

	viewport_submat_primary = submat_viewport;
	viewport_submat_secondary = submat_surface;

	Initialize();

	node_type = ClosureType::SPECIAL;
}

Mtl* CyclesShaderMat::GetSurfaceMtl()
{
	return GetSubMtl(submat_surface);
}

Mtl* CyclesShaderMat::GetVolumeMtl()
{
	return GetSubMtl(submat_volume);
}

void CyclesShaderMat::PopulateShaderParamsDesc(ShaderParamsDescriptor* const desc)
{
	IParamBlock2* params_pblock = GetParamBlock(pblock_shader_params);
	if (params_pblock == nullptr) {
		return;
	}

	desc->use_mis = (params_pblock->GetInt(param_shader_surface_use_mis) != 0);
	desc->vol_sampling = static_cast<VolumeSamplingMethod>(params_pblock->GetInt(param_shader_vol_sampling));
	desc->vol_interp = static_cast<VolumeInterpolationMethod>(params_pblock->GetInt(param_shader_vol_interp));
	desc->vol_is_homogeneous = (params_pblock->GetInt(param_shader_vol_homogeneous) != 0);
}

ClassDesc2* CyclesShaderMat::GetClassDesc() const
{
	return GetCyclesMaterialShaderClassDesc();
}

ParamBlockDesc2* CyclesShaderMat::GetParamBlockDesc()
{
	return &mat_shader_pblock_desc;
}

void CyclesShaderMat::PopulateSubmatSlots()
{
	SubmatSlotDescriptor submat_surface_desc;
	submat_surface_desc.display_name = _T("Surface");
	submat_surface_desc.param_id = param_mat_surface;
	submat_surface_desc.submat_id = submat_surface;
	submat_surface_desc.accepted_closure_type = ClosureType::SURFACE;

	SubmatSlotDescriptor submat_volume_desc;
	submat_volume_desc.display_name = _T("Volume");
	submat_volume_desc.param_id = param_mat_volume;
	submat_volume_desc.submat_id = submat_volume;
	submat_volume_desc.accepted_closure_type = ClosureType::VOLUME;

	SubmatSlotDescriptor submat_vp_desc;
	submat_vp_desc.display_name = _T("Viewport");
	submat_vp_desc.param_id = param_mat_viewport;
	submat_vp_desc.submat_id = submat_viewport;

	submat_slots.push_back(submat_surface_desc);
	submat_slots.push_back(submat_volume_desc);
	submat_slots.push_back(submat_vp_desc);
}

void CyclesShaderMat::PopulateSubtexSlots()
{
	// No sub-textures, do nothing
}

void CyclesShaderMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
