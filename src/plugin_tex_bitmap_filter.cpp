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
 
#include "plugin_tex_bitmap_filter.h"

#include <iparamm2.h>

#include "max_classdesc_tex.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { bitmap_filter_pblock };

// Parameter enum
enum { param_width, param_height, param_texmap, param_texmap_enabled, param_precision };

// Precision enum
enum { prec_uchar, prec_float };

// Subtex enum
enum { subtex_texmap };

static ParamBlockDesc2 texmap_bitmap_filter_pblock_desc (
	// Pblock data
	bitmap_filter_pblock,
	_T("pblock"),
	0,
	GetCyclesTexmapBitmapFilterClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_TEXMAP_FILTER,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Width
		param_width,
		_T("width"),
		TYPE_INT,
		P_ANIMATABLE,
		IDS_WIDTH,
		p_default, 512,
		p_range, 1, 16384,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_EDIT_WIDTH, IDC_SPIN_WIDTH, 1,
		p_end,
		// Height
		param_height,
		_T("height"),
		TYPE_INT,
		P_ANIMATABLE,
		IDS_HEIGHT,
		p_default, 512,
		p_range, 1, 16384,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_EDIT_HEIGHT, IDC_SPIN_HEIGHT, 1,
		p_end,
		// Texmap reference
		param_texmap,
		_T("texmap"),
		TYPE_TEXMAP,
		0,
		IDS_TEXMAP,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_TEXMAP,
		p_end,
		// Texmap enabled
		param_texmap_enabled,
		_T("texmap_enabled"),
		TYPE_BOOL,
		0,
		IDS_TEXMAP_ENABLED,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_TEXMAP,
		p_end,
		// Precision
		param_precision,
		_T("precision"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_PRECISION,
		p_default, prec_uchar,
		p_range, prec_uchar, prec_float,
		p_ui, TYPE_RADIO, 2, IDC_RADIO_PREC_UCHAR, IDC_RADIO_PREC_FLOAT,
		p_end,
	p_end
	);

BitmapFilterTexmap::BitmapFilterTexmap() : CyclesPluginTexmap()
{
	this->Reset();
}

int BitmapFilterTexmap::GetParamWidth(const TimeValue t)
{
	Interval width_valid = FOREVER;
	return pblock->GetInt(param_width, t, width_valid);
}

int BitmapFilterTexmap::GetParamHeight(const TimeValue t)
{
	Interval height_valid = FOREVER;
	return pblock->GetInt(param_height, t, height_valid);
}

bool BitmapFilterTexmap::UseFloatPrecision(const TimeValue t)
{
	Interval prec_valid = FOREVER;
	const int precision = pblock->GetInt(param_precision, t, prec_valid);
	return (precision == prec_float);
}

ClassDesc2* BitmapFilterTexmap::GetClassDesc() const
{
	return GetCyclesTexmapBitmapFilterClassDesc();
}

ParamBlockDesc2* BitmapFilterTexmap::GetParamBlockDesc()
{
	return &texmap_bitmap_filter_pblock_desc;
}

int BitmapFilterTexmap::NumSubTexmaps()
{
	return 1;
}

Texmap* BitmapFilterTexmap::GetSubTexmap(const int i)
{
	Interval texmap_valid = FOREVER;
	if (i == subtex_texmap && pblock != nullptr) {
		return pblock->GetTexmap(param_texmap, 0, texmap_valid);
	}

	return nullptr;
}

int BitmapFilterTexmap::MapSlotType(const int /*i*/)
{
	return MAPSLOT_TEXTURE;
}

void BitmapFilterTexmap::SetSubTexmap(const int i, Texmap* const m)
{
	if (i == subtex_texmap && pblock != nullptr) {
		pblock->SetValue(param_texmap, 0, m);
	}
}

int BitmapFilterTexmap::SubTexmapOn(const int i)
{
	Interval texmap_on_valid = FOREVER;
	if (i == subtex_texmap && GetSubTexmap(i) != nullptr) {
		return pblock->GetInt(param_texmap_enabled, 0, texmap_on_valid);
	}
	return 0;
}

#if PLUGIN_SDK_VERSION < 2022
MSTR BitmapFilterTexmap::GetSubTexmapSlotName(const int i)
#else
MSTR BitmapFilterTexmap::GetSubTexmapSlotName(const int i, const bool)
#endif
{
	if (i == subtex_texmap) {
		return _T("Texmap");
	}
	return _T("");
}

AColor BitmapFilterTexmap::EvalColor(ShadeContext& sc)
{
	Texmap* const texmap = GetSubTexmap(subtex_texmap);
	if (SubTexmapOn(subtex_texmap) && texmap != nullptr) {
		return texmap->EvalColor(sc);
	}
	return AColor(0, 0, 0);
}

Point3 BitmapFilterTexmap::EvalNormalPerturb(ShadeContext& sc)
{
	Texmap* const texmap = GetSubTexmap(subtex_texmap);
	if (SubTexmapOn(subtex_texmap) && texmap != nullptr) {
		return texmap->EvalNormalPerturb(sc);
	}
	return Point3(0.0f, 0.0f, 0.0f);
}

Class_ID BitmapFilterTexmap::ClassID()
{
	return CYCLES_TEXMAP_FILTER_CLASS;
}

Interval BitmapFilterTexmap::Validity(const TimeValue t)
{
	Interval pblock_valid = FOREVER;
	pblock->GetValidity(t, pblock_valid);

	Interval texmap_valid = FOREVER;
	Texmap* const texmap = GetSubTexmap(subtex_texmap);
	if (texmap != nullptr) {
		texmap_valid = texmap->Validity(t);
	}

	return pblock_valid & texmap_valid;
}

BOOL BitmapFilterTexmap::SupportTexDisplay()
{
	return TRUE;
}

DWORD_PTR BitmapFilterTexmap::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
	if (GetSubTexmap(subtex_texmap) != nullptr) {
		return GetSubTexmap(subtex_texmap)->GetActiveTexHandle(t, thmaker);
	}

	return 0;
}

void BitmapFilterTexmap::Update(const TimeValue t, Interval &valid)
{
	if (GetSubTexmap(subtex_texmap) != nullptr) {
		GetSubTexmap(subtex_texmap)->Update(t, valid);
	}
}

RefTargetHandle BitmapFilterTexmap::Clone(RemapDir& remap)
{
	BitmapFilterTexmap* const new_tex = new BitmapFilterTexmap();

	BaseClone(this, new_tex, remap);

	new_tex->ReplaceReference(PBLOCK_REF, remap.CloneRef(pblock));

	return new_tex;
}
