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
 
#include "plugin_mat_mix.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { mix_pblock };

// Parameter enum
enum { param_mat_a, param_mat_b, param_mat_vp, param_fac, param_fac_map, param_fac_map_on, param_blend_mode };

// Blend mode enum
enum { blend_mode_normal, blend_mode_fresnel };

// Subtexmap enum, these numbers are used for getsubtexmap, setsubtexmap, etc
enum { subtex_fac };

// Submaterial enum
enum { submat_a, submat_b, submat_vp };

static ParamBlockDesc2 mat_mix_pblock_desc(
	// Pblock data
	mix_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialMixClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_MIX,
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
		// Factor texmap
		param_fac_map,
		_T("fac_map"),
		TYPE_TEXMAP,
		0,
		IDS_FAC_TEXMAP,
		p_subtexno, subtex_fac,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_FAC,
		p_end,
		// Factor texmap on
		param_fac_map_on,
		_T("fac_map_enabled"),
		TYPE_BOOL,
		P_ANIMATABLE,
		IDS_FAC_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_FAC,
		p_end,
		// Blend mode
		param_blend_mode,
		_T("blend_mode"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_BLEND_MODE,
		p_default, blend_mode_normal,
		p_range, blend_mode_normal, blend_mode_fresnel,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_BLEND_NORMAL, IDC_RADIO_BLEND_FRESNEL,
		p_end,
	p_end
	);

CyclesMixMat::CyclesMixMat() : CyclesPluginMatBase(false, false)
{
	pblock = nullptr;

	this->Reset();

	viewport_submat_primary = submat_vp;

	Initialize();

	node_type = ClosureType::SURFACE;
}

float CyclesMixMat::GetFac(const TimeValue t, Interval& iv)
{
	return pblock->GetFloat(param_fac, t, iv);
}

Texmap* CyclesMixMat::GetFacTexmap()
{
	if (SubTexmapOn(subtex_fac)) {
		return GetSubTexmap(subtex_fac);
	}

	return nullptr;
}

Mtl* CyclesMixMat::GetMtlA()
{
	return GetSubMtl(submat_a);
}

Mtl* CyclesMixMat::GetMtlB()
{
	return GetSubMtl(submat_b);
}

bool CyclesMixMat::UseFresnelBlending(const TimeValue t, Interval& iv)
{
	const int mode = pblock->GetInt(param_blend_mode, t, iv);

	if (mode == blend_mode_normal) {
		return false;
	}
	else if (mode == blend_mode_fresnel) {
		return true;
	}

	return false;
}

ClassDesc2* CyclesMixMat::GetClassDesc() const
{
	return GetCyclesMaterialMixClassDesc();
}

ParamBlockDesc2* CyclesMixMat::GetParamBlockDesc()
{
	return &mat_mix_pblock_desc;
}

void CyclesMixMat::PopulateSubmatSlots()
{
	SubmatSlotDescriptor submat_a_desc;
	submat_a_desc.display_name = _T("Material A");
	submat_a_desc.param_id = param_mat_a;
	submat_a_desc.submat_id = submat_a;
	submat_a_desc.accepted_closure_type = ClosureType::SURFACE;

	SubmatSlotDescriptor submat_b_desc;
	submat_b_desc.display_name = _T("Material B");
	submat_b_desc.param_id = param_mat_b;
	submat_b_desc.submat_id = submat_b;
	submat_b_desc.accepted_closure_type = ClosureType::SURFACE;
	
	SubmatSlotDescriptor submat_vp_desc;
	submat_vp_desc.display_name = _T("Viewport");
	submat_vp_desc.param_id = param_mat_vp;
	submat_vp_desc.submat_id = submat_vp;

	submat_slots.push_back(submat_a_desc);
	submat_slots.push_back(submat_b_desc);
	submat_slots.push_back(submat_vp_desc);
}

void CyclesMixMat::PopulateSubtexSlots()
{
	SubtexSlotDescriptor fac_tex_slot;
	fac_tex_slot.display_name = _T("Fac Map");
	fac_tex_slot.enabled_param_id = param_fac_map_on;
	fac_tex_slot.param_id = param_fac_map;
	fac_tex_slot.slot_type = MAPSLOT_TEXTURE;
	fac_tex_slot.subtex_id = subtex_fac;
	fac_tex_slot.pblock_ref = PBLOCK_REF;

	subtex_slots.push_back(fac_tex_slot);
}

void CyclesMixMat::Update(const TimeValue t, Interval& valid)
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
