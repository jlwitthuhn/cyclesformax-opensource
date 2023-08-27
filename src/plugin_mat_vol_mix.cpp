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
 
#include "plugin_mat_vol_mix.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { mix_pblock };

// Parameter enum
enum { param_mat_a, param_mat_b, param_mat_vp, param_fac };

// Submaterial enum
enum { submat_a, submat_b, submat_vp };

static ParamBlockDesc2 mat_mix_pblock_desc(
	// Pblock data
	mix_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialVolumeMixClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_VOL_MIX,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Material A
		param_mat_a,
		_T("mat_a"),
		TYPE_MTL,
		0,
		IDS_MATERIAL_A,
		p_submtlno, submat_a,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_A,
		p_end,
		// Material B
		param_mat_b,
		_T("mat_b"),
		TYPE_MTL,
		0,
		IDS_MATERIAL_B,
		p_submtlno, submat_b,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_B,
		p_end,
		// Viewport material
		param_mat_vp,
		_T("mat_vp"),
		TYPE_MTL,
		0,
		IDS_VIEWPORT_MATERIAL,
		p_submtlno, submat_vp,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_VIEWPORT,
		p_end,
		// Factor
		param_fac,
		_T("fac_float"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_FAC,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_FAC, IDC_SPIN_FAC, 0.01f,
		p_end,
	p_end
);

CyclesVolMixMat::CyclesVolMixMat() : CyclesPluginMatBase(false, false)
{
	pblock = nullptr;

	this->Reset();

	viewport_submat_primary = submat_vp;

	Initialize();

	node_type = ClosureType::VOLUME;
}

float CyclesVolMixMat::GetFac(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_fac, t, iv);
}

Mtl* CyclesVolMixMat::GetMtlA()
{
	return GetSubMtl(submat_a);
}

Mtl* CyclesVolMixMat::GetMtlB()
{
	return GetSubMtl(submat_b);
}

ClassDesc2* CyclesVolMixMat::GetClassDesc() const
{
	return GetCyclesMaterialVolumeMixClassDesc();
}

ParamBlockDesc2* CyclesVolMixMat::GetParamBlockDesc()
{
	return &mat_mix_pblock_desc;
}

void CyclesVolMixMat::PopulateSubmatSlots()
{
	SubmatSlotDescriptor submat_a_desc;
	submat_a_desc.display_name = _T("Material A");
	submat_a_desc.param_id = param_mat_a;
	submat_a_desc.submat_id = submat_a;
	submat_a_desc.accepted_closure_type = ClosureType::VOLUME;

	SubmatSlotDescriptor submat_b_desc;
	submat_b_desc.display_name = _T("Material B");
	submat_b_desc.param_id = param_mat_b;
	submat_b_desc.submat_id = submat_b;
	submat_b_desc.accepted_closure_type = ClosureType::VOLUME;

	SubmatSlotDescriptor submat_vp_desc;
	submat_vp_desc.display_name = _T("Viewport");
	submat_vp_desc.param_id = param_mat_vp;
	submat_vp_desc.submat_id = submat_vp;

	submat_slots.push_back(submat_a_desc);
	submat_slots.push_back(submat_b_desc);
	submat_slots.push_back(submat_vp_desc);
}

void CyclesVolMixMat::PopulateSubtexSlots()
{

}

void CyclesVolMixMat::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	if (GetMtlA() != nullptr || GetMtlB() != nullptr) {
		color_diffuse = Color(0.8f, 0.8f, 0.8f);
	}
	else {
		color_diffuse = Color(0.0f, 0.0f, 0.0f);
	}

	Interval fac_valid = FOREVER;
	pblock->GetFloat(param_fac, t, fac_valid);

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = fac_valid & subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
