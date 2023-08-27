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
 
#include "plugin_mat_shader_graph_08.h"

#include <maxapi.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { pblock_graph, pblock_shader_params };

// Parameter enum
enum { param_dummy
	, param_mat_vp
	, param_map_01, param_map_01_enabled
	, param_map_02, param_map_02_enabled
	, param_map_03, param_map_03_enabled
	, param_map_04, param_map_04_enabled
	, param_map_05, param_map_05_enabled
	, param_map_06, param_map_06_enabled
	, param_map_07, param_map_07_enabled
	, param_map_08, param_map_08_enabled
};

// Subtex enum
enum { subtex_01, subtex_02, subtex_03, subtex_04, subtex_05, subtex_06, subtex_07, subtex_08 };

// Submaterial enum
enum { submat_vp };

static ParamBlockDesc2 mat_shader_graph_08_pblock_desc(
	// Pblock data
	pblock_graph,
	_T("pblock"),
	0,
	GetCyclesMaterialShaderGraph08ClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_SHADER_GRAPH_08_PARAMS,
	IDS_PARAMS,
	0,
	0,
	nullptr,
		// Dummy
		param_dummy,
		_T("dummy"),
		TYPE_INT,
		P_ANIMATABLE,
		IDS_DUMMY,
		p_default, 0,
		p_end,
		// Viewport submat
		param_mat_vp,
		_T("mat_vp"),
		TYPE_MTL,
		0,
		IDS_VIEWPORT_MATERIAL,
		p_ui, TYPE_MTLBUTTON, IDC_BUTTON_MAT_VIEWPORT,
		p_end,
		// Texmap 01
		param_map_01,
		_T("map_01"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_01,
		p_subtexno, subtex_01,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_01,
		p_end,
		// Texmap 01 enabled
		param_map_01_enabled,
		_T("map_01_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_01_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_01,
		p_end,
		// Texmap 02
		param_map_02,
		_T("map_02"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_02,
		p_subtexno, subtex_02,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_02,
		p_end,
		// Texmap 02 enabled
		param_map_02_enabled,
		_T("map_02_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_02_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_02,
		p_end,
		// Texmap 03
		param_map_03,
		_T("map_03"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_03,
		p_subtexno, subtex_03,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_03,
		p_end,
		// Texmap 03 enabled
		param_map_03_enabled,
		_T("map_03_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_03_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_03,
		p_end,
		// Texmap 04
		param_map_04,
		_T("map_04"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_04,
		p_subtexno, subtex_04,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_04,
		p_end,
		// Texmap 04 enabled
		param_map_04_enabled,
		_T("map_04_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_04_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_04,
		p_end,
		// Texmap 05
		param_map_05,
		_T("map_05"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_05,
		p_subtexno, subtex_05,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_05,
		p_end,
		// Texmap 05 enabled
		param_map_05_enabled,
		_T("map_05_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_05_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_05,
		p_end,
		// Texmap 06
		param_map_06,
		_T("map_06"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_06,
		p_subtexno, subtex_06,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_06,
		p_end,
		// Texmap 06 enabled
		param_map_06_enabled,
		_T("map_06_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_06_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_06,
		p_end,
		// Texmap 07
		param_map_07,
		_T("map_07"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_07,
		p_subtexno, subtex_07,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_07,
		p_end,
		// Texmap 07 enabled
		param_map_07_enabled,
		_T("map_07_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_07_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_07,
		p_end,
		// Texmap 08
		param_map_08,
		_T("map_08"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_08,
		p_subtexno, subtex_08,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_08,
		p_end,
		// Texmap 08 enabled
		param_map_08_enabled,
		_T("map_08_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_08_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_08,
		p_end,
	p_end
	);

SHADER_PARAMS_ENUMS
SHADER_PARAMS_PBLOCK_DESC(GetCyclesMaterialShaderGraph08ClassDesc())

CyclesShaderGraphMat08::CyclesShaderGraphMat08()
{
	ParamBlockSlotDescriptor pblock_params_desc;
	pblock_params_desc.name = TSTR(L"pblock_params");
	pblock_params_desc.ref = pblock_shader_params;
	pblock_vec.push_back(pblock_params_desc);

	this->Reset();

	viewport_submat_primary = submat_vp;

	Initialize();
}

Texmap* CyclesShaderGraphMat08::GetNumberedTexmap(const size_t number) {
	switch (number) {
	case 1:
		return GetSubTexmap(subtex_01);
	case 2:
		return GetSubTexmap(subtex_02);
	case 3:
		return GetSubTexmap(subtex_03);
	case 4:
		return GetSubTexmap(subtex_04);
	case 5:
		return GetSubTexmap(subtex_05);
	case 6:
		return GetSubTexmap(subtex_06);
	case 7:
		return GetSubTexmap(subtex_07);
	case 8:
		return GetSubTexmap(subtex_08);
	default:
		return nullptr;
	}
}

ClassDesc2* CyclesShaderGraphMat08::GetClassDesc() const
{
	return GetCyclesMaterialShaderGraph08ClassDesc();
}

ParamBlockDesc2* CyclesShaderGraphMat08::GetParamBlockDesc()
{
	return &mat_shader_graph_08_pblock_desc;
}

void CyclesShaderGraphMat08::PopulateSubmatSlots()
{
	SubmatSlotDescriptor submat_vp_desc;
	submat_vp_desc.display_name = _T("Viewport");
	submat_vp_desc.param_id = param_mat_vp;
	submat_vp_desc.submat_id = submat_vp;

	submat_slots.push_back(submat_vp_desc);
}

void CyclesShaderGraphMat08::PopulateSubtexSlots()
{
	SubtexSlotDescriptor tex_01_slot;
	tex_01_slot.display_name = _T("Texmap 01");
	tex_01_slot.enabled_param_id = param_map_01_enabled;
	tex_01_slot.param_id = param_map_01;
	tex_01_slot.slot_type = MAPSLOT_TEXTURE;
	tex_01_slot.subtex_id = subtex_01;

	SubtexSlotDescriptor tex_02_slot;
	tex_02_slot.display_name = _T("Texmap 02");
	tex_02_slot.enabled_param_id = param_map_02_enabled;
	tex_02_slot.param_id = param_map_02;
	tex_02_slot.slot_type = MAPSLOT_TEXTURE;
	tex_02_slot.subtex_id = subtex_02;

	SubtexSlotDescriptor tex_03_slot;
	tex_03_slot.display_name = _T("Texmap 03");
	tex_03_slot.enabled_param_id = param_map_03_enabled;
	tex_03_slot.param_id = param_map_03;
	tex_03_slot.slot_type = MAPSLOT_TEXTURE;
	tex_03_slot.subtex_id = subtex_03;

	SubtexSlotDescriptor tex_04_slot;
	tex_04_slot.display_name = _T("Texmap 04");
	tex_04_slot.enabled_param_id = param_map_04_enabled;
	tex_04_slot.param_id = param_map_04;
	tex_04_slot.slot_type = MAPSLOT_TEXTURE;
	tex_04_slot.subtex_id = subtex_04;

	SubtexSlotDescriptor tex_05_slot;
	tex_05_slot.display_name = _T("Texmap 05");
	tex_05_slot.enabled_param_id = param_map_05_enabled;
	tex_05_slot.param_id = param_map_05;
	tex_05_slot.slot_type = MAPSLOT_TEXTURE;
	tex_05_slot.subtex_id = subtex_05;

	SubtexSlotDescriptor tex_06_slot;
	tex_06_slot.display_name = _T("Texmap 06");
	tex_06_slot.enabled_param_id = param_map_06_enabled;
	tex_06_slot.param_id = param_map_06;
	tex_06_slot.slot_type = MAPSLOT_TEXTURE;
	tex_06_slot.subtex_id = subtex_06;

	SubtexSlotDescriptor tex_07_slot;
	tex_07_slot.display_name = _T("Texmap 07");
	tex_07_slot.enabled_param_id = param_map_07_enabled;
	tex_07_slot.param_id = param_map_07;
	tex_07_slot.slot_type = MAPSLOT_TEXTURE;
	tex_07_slot.subtex_id = subtex_07;

	SubtexSlotDescriptor tex_08_slot;
	tex_08_slot.display_name = _T("Texmap 08");
	tex_08_slot.enabled_param_id = param_map_08_enabled;
	tex_08_slot.param_id = param_map_08;
	tex_08_slot.slot_type = MAPSLOT_TEXTURE;
	tex_08_slot.subtex_id = subtex_08;

	subtex_slots.push_back(tex_01_slot);
	subtex_slots.push_back(tex_02_slot);
	subtex_slots.push_back(tex_03_slot);
	subtex_slots.push_back(tex_04_slot);
	subtex_slots.push_back(tex_05_slot);
	subtex_slots.push_back(tex_06_slot);
	subtex_slots.push_back(tex_07_slot);
	subtex_slots.push_back(tex_08_slot);
}

RefTargetHandle CyclesShaderGraphMat08::Clone(RemapDir &remap)
{
	CyclesShaderGraphMat08* new_mat = new CyclesShaderGraphMat08();

	BaseClone(this, new_mat, remap);
	new_mat->ReplaceReference(PBLOCK_REF, remap.CloneRef(pblock));
	new_mat->ReplaceReference(pblock_shader_params, remap.CloneRef(GetShaderParamsPblock()));

	Interval material_valid = FOREVER;
	new_mat->Update(GetCOREInterface()->GetTime(), material_valid);

	new_mat->loaded_graph = GetNodeGraph();
	new_mat->loaded_graph_set = true;

	return new_mat;
}

IParamBlock2* CyclesShaderGraphMat08::GetShaderParamsPblock()
{
	return GetParamBlock(pblock_shader_params);
}

TSTR CyclesShaderGraphMat08::GetShaderGraph()
{
	return CyclesShaderGraphMatBase::GetShaderGraph();
}

void CyclesShaderGraphMat08::SetShaderGraph(TSTR input)
{
	CyclesShaderGraphMatBase::SetShaderGraph(input);
}
