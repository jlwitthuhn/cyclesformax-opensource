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
 
#include "plugin_mat_shader_graph_32.h"

#include <maxapi.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { pblock_graph, pblock_shader_params };

// Parameter enum
enum {
	param_dummy
	, param_mat_vp
	, param_map_01, param_map_01_enabled
	, param_map_02, param_map_02_enabled
	, param_map_03, param_map_03_enabled
	, param_map_04, param_map_04_enabled
	, param_map_05, param_map_05_enabled
	, param_map_06, param_map_06_enabled
	, param_map_07, param_map_07_enabled
	, param_map_08, param_map_08_enabled
	, param_map_09, param_map_09_enabled
	, param_map_10, param_map_10_enabled
	, param_map_11, param_map_11_enabled
	, param_map_12, param_map_12_enabled
	, param_map_13, param_map_13_enabled
	, param_map_14, param_map_14_enabled
	, param_map_15, param_map_15_enabled
	, param_map_16, param_map_16_enabled
	, param_map_17, param_map_17_enabled
	, param_map_18, param_map_18_enabled
	, param_map_19, param_map_19_enabled
	, param_map_20, param_map_20_enabled
	, param_map_21, param_map_21_enabled
	, param_map_22, param_map_22_enabled
	, param_map_23, param_map_23_enabled
	, param_map_24, param_map_24_enabled
	, param_map_25, param_map_25_enabled
	, param_map_26, param_map_26_enabled
	, param_map_27, param_map_27_enabled
	, param_map_28, param_map_28_enabled
	, param_map_29, param_map_29_enabled
	, param_map_30, param_map_30_enabled
	, param_map_31, param_map_31_enabled
	, param_map_32, param_map_32_enabled
};

// Subtex enum
enum {
	subtex_01, subtex_02, subtex_03, subtex_04, subtex_05, subtex_06, subtex_07, subtex_08,
	subtex_09, subtex_10, subtex_11, subtex_12, subtex_13, subtex_14, subtex_15, subtex_16,
	subtex_17, subtex_18, subtex_19, subtex_20, subtex_21, subtex_22, subtex_23, subtex_24,
	subtex_25, subtex_26, subtex_27, subtex_28, subtex_29, subtex_30, subtex_31, subtex_32
};

// Submaterial enum
enum { submat_vp };

static ParamBlockDesc2 mat_shader_graph_32_pblock_desc(
	// Pblock data
	pblock_graph,
	_T("pblock"),
	0,
	GetCyclesMaterialShaderGraph32ClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_SHADER_GRAPH_32_PARAMS,
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
		// Texmap 09
		param_map_09,
		_T("map_09"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_09,
		p_subtexno, subtex_09,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_09,
		p_end,
		// Texmap 09 enabled
		param_map_09_enabled,
		_T("map_09_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_09_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_09,
		p_end,
		// Texmap 10
		param_map_10,
		_T("map_10"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_10,
		p_subtexno, subtex_10,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_10,
		p_end,
		// Texmap 10 enabled
		param_map_10_enabled,
		_T("map_10_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_10_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_10,
		p_end,
		// Texmap 11
		param_map_11,
		_T("map_11"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_11,
		p_subtexno, subtex_11,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_11,
		p_end,
		// Texmap 11 enabled
		param_map_11_enabled,
		_T("map_11_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_11_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_11,
		p_end,
		// Texmap 12
		param_map_12,
		_T("map_12"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_12,
		p_subtexno, subtex_12,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_12,
		p_end,
		// Texmap 12 enabled
		param_map_12_enabled,
		_T("map_12_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_12_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_12,
		p_end,
		// Texmap 13
		param_map_13,
		_T("map_13"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_13,
		p_subtexno, subtex_13,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_13,
		p_end,
		// Texmap 13 enabled
		param_map_13_enabled,
		_T("map_13_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_13_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_13,
		p_end,
		// Texmap 14
		param_map_14,
		_T("map_14"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_14,
		p_subtexno, subtex_14,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_14,
		p_end,
		// Texmap 14 enabled
		param_map_14_enabled,
		_T("map_14_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_14_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_14,
		p_end,
		// Texmap 15
		param_map_15,
		_T("map_15"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_15,
		p_subtexno, subtex_15,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_15,
		p_end,
		// Texmap 15 enabled
		param_map_15_enabled,
		_T("map_15_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_15_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_15,
		p_end,
		// Texmap 16
		param_map_16,
		_T("map_16"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_16,
		p_subtexno, subtex_16,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_16,
		p_end,
		// Texmap 16 enabled
		param_map_16_enabled,
		_T("map_16_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_16_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_16,
		p_end,
		// Texmap 17
		param_map_17,
		_T("map_17"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_17,
		p_subtexno, subtex_17,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_17,
		p_end,
		// Texmap 17 enabled
		param_map_17_enabled,
		_T("map_17_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_17_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_17,
		p_end,
		// Texmap 18
		param_map_18,
		_T("map_18"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_18,
		p_subtexno, subtex_18,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_18,
		p_end,
		// Texmap 18 enabled
		param_map_18_enabled,
		_T("map_18_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_18_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_18,
		p_end,
		// Texmap 19
		param_map_19,
		_T("map_19"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_19,
		p_subtexno, subtex_19,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_19,
		p_end,
		// Texmap 19 enabled
		param_map_19_enabled,
		_T("map_19_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_19_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_19,
		p_end,
		// Texmap 20
		param_map_20,
		_T("map_20"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_20,
		p_subtexno, subtex_20,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_20,
		p_end,
		// Texmap 20 enabled
		param_map_20_enabled,
		_T("map_20_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_20_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_20,
		p_end,
		// Texmap 21
		param_map_21,
		_T("map_21"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_21,
		p_subtexno, subtex_21,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_21,
		p_end,
		// Texmap 21 enabled
		param_map_21_enabled,
		_T("map_21_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_21_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_21,
		p_end,
		// Texmap 22
		param_map_22,
		_T("map_22"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_22,
		p_subtexno, subtex_22,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_22,
		p_end,
		// Texmap 22 enabled
		param_map_22_enabled,
		_T("map_22_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_22_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_22,
		p_end,
		// Texmap 23
		param_map_23,
		_T("map_23"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_23,
		p_subtexno, subtex_23,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_23,
		p_end,
		// Texmap 23 enabled
		param_map_23_enabled,
		_T("map_23_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_23_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_23,
		p_end,
		// Texmap 24
		param_map_24,
		_T("map_24"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_24,
		p_subtexno, subtex_24,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_24,
		p_end,
		// Texmap 24 enabled
		param_map_24_enabled,
		_T("map_24_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_24_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_24,
		p_end,
		// Texmap 25
		param_map_25,
		_T("map_25"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_25,
		p_subtexno, subtex_25,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_25,
		p_end,
		// Texmap 25 enabled
		param_map_25_enabled,
		_T("map_25_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_25_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_25,
		p_end,
		// Texmap 26
		param_map_26,
		_T("map_26"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_26,
		p_subtexno, subtex_26,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_26,
		p_end,
		// Texmap 26 enabled
		param_map_26_enabled,
		_T("map_26_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_26_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_26,
		p_end,
		// Texmap 27
		param_map_27,
		_T("map_27"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_27,
		p_subtexno, subtex_27,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_27,
		p_end,
		// Texmap 27 enabled
		param_map_27_enabled,
		_T("map_27_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_27_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_27,
		p_end,
		// Texmap 28
		param_map_28,
		_T("map_28"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_28,
		p_subtexno, subtex_28,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_28,
		p_end,
		// Texmap 28 enabled
		param_map_28_enabled,
		_T("map_28_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_28_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_28,
		p_end,
		// Texmap 29
		param_map_29,
		_T("map_29"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_29,
		p_subtexno, subtex_29,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_29,
		p_end,
		// Texmap 29 enabled
		param_map_29_enabled,
		_T("map_29_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_29_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_29,
		p_end,
		// Texmap 30
		param_map_30,
		_T("map_30"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_30,
		p_subtexno, subtex_30,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_30,
		p_end,
		// Texmap 30 enabled
		param_map_30_enabled,
		_T("map_30_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_30_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_30,
		p_end,
		// Texmap 31
		param_map_31,
		_T("map_31"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_31,
		p_subtexno, subtex_31,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_31,
		p_end,
		// Texmap 31 enabled
		param_map_31_enabled,
		_T("map_31_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_31_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_31,
		p_end,
		// Texmap 32
		param_map_32,
		_T("map_32"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP_32,
		p_subtexno, subtex_32,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_TEX_32,
		p_end,
		// Texmap 32 enabled
		param_map_32_enabled,
		_T("map_32_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_32_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_TEX_32,
		p_end,
	p_end
);

SHADER_PARAMS_ENUMS
SHADER_PARAMS_PBLOCK_DESC(GetCyclesMaterialShaderGraph32ClassDesc())

CyclesShaderGraphMat32::CyclesShaderGraphMat32()
{
	ParamBlockSlotDescriptor pblock_params_desc;
	pblock_params_desc.name = TSTR(L"pblock_params");
	pblock_params_desc.ref = pblock_shader_params;
	pblock_vec.push_back(pblock_params_desc);

	this->Reset();

	viewport_submat_primary = submat_vp;

	Initialize();
}

Texmap* CyclesShaderGraphMat32::GetNumberedTexmap(const size_t number) {
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
	case 9:
		return GetSubTexmap(subtex_09);
	case 10:
		return GetSubTexmap(subtex_10);
	case 11:
		return GetSubTexmap(subtex_11);
	case 12:
		return GetSubTexmap(subtex_12);
	case 13:
		return GetSubTexmap(subtex_13);
	case 14:
		return GetSubTexmap(subtex_14);
	case 15:
		return GetSubTexmap(subtex_15);
	case 16:
		return GetSubTexmap(subtex_16);
	case 17:
		return GetSubTexmap(subtex_17);
	case 18:
		return GetSubTexmap(subtex_18);
	case 19:
		return GetSubTexmap(subtex_19);
	case 20:
		return GetSubTexmap(subtex_20);
	case 21:
		return GetSubTexmap(subtex_21);
	case 22:
		return GetSubTexmap(subtex_22);
	case 23:
		return GetSubTexmap(subtex_23);
	case 24:
		return GetSubTexmap(subtex_24);
	case 25:
		return GetSubTexmap(subtex_25);
	case 26:
		return GetSubTexmap(subtex_26);
	case 27:
		return GetSubTexmap(subtex_27);
	case 28:
		return GetSubTexmap(subtex_28);
	case 29:
		return GetSubTexmap(subtex_29);
	case 30:
		return GetSubTexmap(subtex_30);
	case 31:
		return GetSubTexmap(subtex_31);
	case 32:
		return GetSubTexmap(subtex_32);
	default:
		return nullptr;
	}
}

ClassDesc2* CyclesShaderGraphMat32::GetClassDesc() const
{
	return GetCyclesMaterialShaderGraph32ClassDesc();
}

ParamBlockDesc2* CyclesShaderGraphMat32::GetParamBlockDesc()
{
	return &mat_shader_graph_32_pblock_desc;
}

void CyclesShaderGraphMat32::PopulateSubmatSlots()
{
	SubmatSlotDescriptor submat_vp_desc;
	submat_vp_desc.display_name = _T("Viewport");
	submat_vp_desc.param_id = param_mat_vp;
	submat_vp_desc.submat_id = submat_vp;

	submat_slots.push_back(submat_vp_desc);
}

void CyclesShaderGraphMat32::PopulateSubtexSlots()
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

	SubtexSlotDescriptor tex_09_slot;
	tex_09_slot.display_name = _T("Texmap 09");
	tex_09_slot.enabled_param_id = param_map_09_enabled;
	tex_09_slot.param_id = param_map_09;
	tex_09_slot.slot_type = MAPSLOT_TEXTURE;
	tex_09_slot.subtex_id = subtex_09;

	SubtexSlotDescriptor tex_10_slot;
	tex_10_slot.display_name = _T("Texmap 10");
	tex_10_slot.enabled_param_id = param_map_10_enabled;
	tex_10_slot.param_id = param_map_10;
	tex_10_slot.slot_type = MAPSLOT_TEXTURE;
	tex_10_slot.subtex_id = subtex_10;

	SubtexSlotDescriptor tex_11_slot;
	tex_11_slot.display_name = _T("Texmap 11");
	tex_11_slot.enabled_param_id = param_map_11_enabled;
	tex_11_slot.param_id = param_map_11;
	tex_11_slot.slot_type = MAPSLOT_TEXTURE;
	tex_11_slot.subtex_id = subtex_11;

	SubtexSlotDescriptor tex_12_slot;
	tex_12_slot.display_name = _T("Texmap 12");
	tex_12_slot.enabled_param_id = param_map_12_enabled;
	tex_12_slot.param_id = param_map_12;
	tex_12_slot.slot_type = MAPSLOT_TEXTURE;
	tex_12_slot.subtex_id = subtex_12;

	SubtexSlotDescriptor tex_13_slot;
	tex_13_slot.display_name = _T("Texmap 13");
	tex_13_slot.enabled_param_id = param_map_13_enabled;
	tex_13_slot.param_id = param_map_13;
	tex_13_slot.slot_type = MAPSLOT_TEXTURE;
	tex_13_slot.subtex_id = subtex_13;

	SubtexSlotDescriptor tex_14_slot;
	tex_14_slot.display_name = _T("Texmap 14");
	tex_14_slot.enabled_param_id = param_map_14_enabled;
	tex_14_slot.param_id = param_map_14;
	tex_14_slot.slot_type = MAPSLOT_TEXTURE;
	tex_14_slot.subtex_id = subtex_14;

	SubtexSlotDescriptor tex_15_slot;
	tex_15_slot.display_name = _T("Texmap 15");
	tex_15_slot.enabled_param_id = param_map_15_enabled;
	tex_15_slot.param_id = param_map_15;
	tex_15_slot.slot_type = MAPSLOT_TEXTURE;
	tex_15_slot.subtex_id = subtex_15;

	SubtexSlotDescriptor tex_16_slot;
	tex_16_slot.display_name = _T("Texmap 16");
	tex_16_slot.enabled_param_id = param_map_16_enabled;
	tex_16_slot.param_id = param_map_16;
	tex_16_slot.slot_type = MAPSLOT_TEXTURE;
	tex_16_slot.subtex_id = subtex_16;

	SubtexSlotDescriptor tex_17_slot;
	tex_17_slot.display_name = _T("Texmap 17");
	tex_17_slot.enabled_param_id = param_map_17_enabled;
	tex_17_slot.param_id = param_map_17;
	tex_17_slot.slot_type = MAPSLOT_TEXTURE;
	tex_17_slot.subtex_id = subtex_17;

	SubtexSlotDescriptor tex_18_slot;
	tex_18_slot.display_name = _T("Texmap 18");
	tex_18_slot.enabled_param_id = param_map_18_enabled;
	tex_18_slot.param_id = param_map_18;
	tex_18_slot.slot_type = MAPSLOT_TEXTURE;
	tex_18_slot.subtex_id = subtex_18;

	SubtexSlotDescriptor tex_19_slot;
	tex_19_slot.display_name = _T("Texmap 19");
	tex_19_slot.enabled_param_id = param_map_19_enabled;
	tex_19_slot.param_id = param_map_19;
	tex_19_slot.slot_type = MAPSLOT_TEXTURE;
	tex_19_slot.subtex_id = subtex_19;

	SubtexSlotDescriptor tex_20_slot;
	tex_20_slot.display_name = _T("Texmap 20");
	tex_20_slot.enabled_param_id = param_map_20_enabled;
	tex_20_slot.param_id = param_map_20;
	tex_20_slot.slot_type = MAPSLOT_TEXTURE;
	tex_20_slot.subtex_id = subtex_20;

	SubtexSlotDescriptor tex_21_slot;
	tex_21_slot.display_name = _T("Texmap 21");
	tex_21_slot.enabled_param_id = param_map_21_enabled;
	tex_21_slot.param_id = param_map_21;
	tex_21_slot.slot_type = MAPSLOT_TEXTURE;
	tex_21_slot.subtex_id = subtex_21;

	SubtexSlotDescriptor tex_22_slot;
	tex_22_slot.display_name = _T("Texmap 22");
	tex_22_slot.enabled_param_id = param_map_22_enabled;
	tex_22_slot.param_id = param_map_22;
	tex_22_slot.slot_type = MAPSLOT_TEXTURE;
	tex_22_slot.subtex_id = subtex_22;

	SubtexSlotDescriptor tex_23_slot;
	tex_23_slot.display_name = _T("Texmap 23");
	tex_23_slot.enabled_param_id = param_map_23_enabled;
	tex_23_slot.param_id = param_map_23;
	tex_23_slot.slot_type = MAPSLOT_TEXTURE;
	tex_23_slot.subtex_id = subtex_23;

	SubtexSlotDescriptor tex_24_slot;
	tex_24_slot.display_name = _T("Texmap 24");
	tex_24_slot.enabled_param_id = param_map_24_enabled;
	tex_24_slot.param_id = param_map_24;
	tex_24_slot.slot_type = MAPSLOT_TEXTURE;
	tex_24_slot.subtex_id = subtex_24;

	SubtexSlotDescriptor tex_25_slot;
	tex_25_slot.display_name = _T("Texmap 25");
	tex_25_slot.enabled_param_id = param_map_25_enabled;
	tex_25_slot.param_id = param_map_25;
	tex_25_slot.slot_type = MAPSLOT_TEXTURE;
	tex_25_slot.subtex_id = subtex_25;

	SubtexSlotDescriptor tex_26_slot;
	tex_26_slot.display_name = _T("Texmap 26");
	tex_26_slot.enabled_param_id = param_map_26_enabled;
	tex_26_slot.param_id = param_map_26;
	tex_26_slot.slot_type = MAPSLOT_TEXTURE;
	tex_26_slot.subtex_id = subtex_26;

	SubtexSlotDescriptor tex_27_slot;
	tex_27_slot.display_name = _T("Texmap 27");
	tex_27_slot.enabled_param_id = param_map_27_enabled;
	tex_27_slot.param_id = param_map_27;
	tex_27_slot.slot_type = MAPSLOT_TEXTURE;
	tex_27_slot.subtex_id = subtex_27;

	SubtexSlotDescriptor tex_28_slot;
	tex_28_slot.display_name = _T("Texmap 28");
	tex_28_slot.enabled_param_id = param_map_28_enabled;
	tex_28_slot.param_id = param_map_28;
	tex_28_slot.slot_type = MAPSLOT_TEXTURE;
	tex_28_slot.subtex_id = subtex_28;

	SubtexSlotDescriptor tex_29_slot;
	tex_29_slot.display_name = _T("Texmap 29");
	tex_29_slot.enabled_param_id = param_map_29_enabled;
	tex_29_slot.param_id = param_map_29;
	tex_29_slot.slot_type = MAPSLOT_TEXTURE;
	tex_29_slot.subtex_id = subtex_29;

	SubtexSlotDescriptor tex_30_slot;
	tex_30_slot.display_name = _T("Texmap 30");
	tex_30_slot.enabled_param_id = param_map_30_enabled;
	tex_30_slot.param_id = param_map_30;
	tex_30_slot.slot_type = MAPSLOT_TEXTURE;
	tex_30_slot.subtex_id = subtex_30;

	SubtexSlotDescriptor tex_31_slot;
	tex_31_slot.display_name = _T("Texmap 31");
	tex_31_slot.enabled_param_id = param_map_31_enabled;
	tex_31_slot.param_id = param_map_31;
	tex_31_slot.slot_type = MAPSLOT_TEXTURE;
	tex_31_slot.subtex_id = subtex_31;

	SubtexSlotDescriptor tex_32_slot;
	tex_32_slot.display_name = _T("Texmap 32");
	tex_32_slot.enabled_param_id = param_map_32_enabled;
	tex_32_slot.param_id = param_map_32;
	tex_32_slot.slot_type = MAPSLOT_TEXTURE;
	tex_32_slot.subtex_id = subtex_32;

	subtex_slots.push_back(tex_01_slot);
	subtex_slots.push_back(tex_02_slot);
	subtex_slots.push_back(tex_03_slot);
	subtex_slots.push_back(tex_04_slot);
	subtex_slots.push_back(tex_05_slot);
	subtex_slots.push_back(tex_06_slot);
	subtex_slots.push_back(tex_07_slot);
	subtex_slots.push_back(tex_08_slot);
	subtex_slots.push_back(tex_09_slot);
	subtex_slots.push_back(tex_10_slot);
	subtex_slots.push_back(tex_11_slot);
	subtex_slots.push_back(tex_12_slot);
	subtex_slots.push_back(tex_13_slot);
	subtex_slots.push_back(tex_14_slot);
	subtex_slots.push_back(tex_15_slot);
	subtex_slots.push_back(tex_16_slot);
	subtex_slots.push_back(tex_17_slot);
	subtex_slots.push_back(tex_18_slot);
	subtex_slots.push_back(tex_19_slot);
	subtex_slots.push_back(tex_20_slot);
	subtex_slots.push_back(tex_21_slot);
	subtex_slots.push_back(tex_22_slot);
	subtex_slots.push_back(tex_23_slot);
	subtex_slots.push_back(tex_24_slot);
	subtex_slots.push_back(tex_25_slot);
	subtex_slots.push_back(tex_26_slot);
	subtex_slots.push_back(tex_27_slot);
	subtex_slots.push_back(tex_28_slot);
	subtex_slots.push_back(tex_29_slot);
	subtex_slots.push_back(tex_30_slot);
	subtex_slots.push_back(tex_31_slot);
	subtex_slots.push_back(tex_32_slot);
}

RefTargetHandle CyclesShaderGraphMat32::Clone(RemapDir &remap)
{
	CyclesShaderGraphMat32* new_mat = new CyclesShaderGraphMat32();

	BaseClone(this, new_mat, remap);
	new_mat->ReplaceReference(PBLOCK_REF, remap.CloneRef(pblock));
	new_mat->ReplaceReference(pblock_shader_params, remap.CloneRef(GetShaderParamsPblock()));

	Interval material_valid = FOREVER;
	new_mat->Update(GetCOREInterface()->GetTime(), material_valid);

	new_mat->loaded_graph = GetNodeGraph();
	new_mat->loaded_graph_set = true;

	return new_mat;
}

IParamBlock2* CyclesShaderGraphMat32::GetShaderParamsPblock()
{
	return GetParamBlock(pblock_shader_params);
}

TSTR CyclesShaderGraphMat32::GetShaderGraph()
{
	return CyclesShaderGraphMatBase::GetShaderGraph();
}

void CyclesShaderGraphMat32::SetShaderGraph(TSTR input)
{
	CyclesShaderGraphMatBase::SetShaderGraph(input);
}
