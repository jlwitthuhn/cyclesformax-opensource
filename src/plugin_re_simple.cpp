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
 
#include "plugin_re_simple.h"

#include <iparamm2.h>

#include "max_classdesc_re.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Basic pblock enum
enum { re_basic_pblock };

// Parameter enum
enum { param_enabled, param_filter_enabled, param_name, param_bitmap };

#define CPP_RENDER_ELEMENT_GEN(CLASS_NAME, CLASS_DESC_FUNC, SHORT_NAME, RENDER_PASS_TYPE, CLASS_ID, NAME_LSTR) \
static ParamBlockDesc2 SHORT_NAME ## _pblock_desc( \
	re_basic_pblock, \
	_T("pblock"), \
	0, \
	CLASS_DESC_FUNC(), \
	P_AUTO_CONSTRUCT, \
	PBLOCK_REF, \
	/* Enabled */ \
	param_enabled, \
	_T("enabled"), \
	TYPE_BOOL, \
	0, \
	IDS_RE_ENABLED, \
	p_default, TRUE, \
	p_end, \
	/* Filter enabled */ \
	param_filter_enabled, \
	_T("filter_enabled"), \
	TYPE_BOOL, \
	0, \
	IDS_RE_FILTER_ENABLED, \
	p_default, TRUE, \
	p_end, \
	/* Element name */ \
	param_name, \
	_T("element_name"), \
	TYPE_STRING, \
	0, \
	IDS_RE_NAME, \
	p_default, NAME_LSTR, \
	p_end, \
	/* Bitmap */ \
	param_bitmap, \
	_T("bitmap"), \
	TYPE_BITMAP, \
	0, \
	IDS_RE_BITMAP, \
	p_end, \
	p_end \
); \
\
CLASS_NAME::CLASS_NAME() : CyclesRenderElement(RENDER_PASS_TYPE, CLASS_ID, NAME_LSTR) \
{ \
	CLASS_DESC_FUNC()->MakeAutoParamBlocks(this); \
}

CyclesRenderElement::CyclesRenderElement(const RenderPassType render_pass_type, const Class_ID class_id, const TSTR& class_name) :
	render_pass_type(render_pass_type),
	class_id(class_id),
	class_name(class_name)
{

}

RenderPassType CyclesRenderElement::GetRenderPassType() const
{
	return render_pass_type;
}

Class_ID CyclesRenderElement::ClassID()
{
	return class_id;
}

#if PLUGIN_SDK_VERSION < 2022
void CyclesRenderElement::GetClassName(TSTR& s)
#else
void CyclesRenderElement::GetClassName(TSTR& s, const bool) const
#endif
{
	s = class_name;
}

int CyclesRenderElement::NumSubs()
{
	return NumRefs();
}

Animatable* CyclesRenderElement::SubAnim(const int i)
{
	return GetReference(i);
}

#if PLUGIN_SDK_VERSION < 2022
TSTR CyclesRenderElement::SubAnimName(const int i)
#else
TSTR CyclesRenderElement::SubAnimName(const int i, const bool)
#endif
{
	if (i == 0) {
		return L"pblock";
	}
	return L"";
}

int CyclesRenderElement::NumRefs()
{
	return NumParamBlocks();
}

RefTargetHandle CyclesRenderElement::GetReference(const int i)
{
	return GetParamBlock(i);
}

int CyclesRenderElement::NumParamBlocks()
{
	return 1;
}

IParamBlock2* CyclesRenderElement::GetParamBlock(const int i)
{
	if (i == 0) {
		return pblock;
	}
	return nullptr;
}

IParamBlock2* CyclesRenderElement::GetParamBlockByID(const BlockID id)
{
	if (pblock != nullptr && pblock->ID() == id) {
		return pblock;
	}
	return nullptr;
}

void CyclesRenderElement::SetEnabled(BOOL enabled)
{
	pblock->SetValue(param_enabled, 0, enabled);
}

BOOL CyclesRenderElement::IsEnabled() const
{
	Interval valid = FOREVER;
	BOOL result;
	pblock->GetValue(param_enabled, 0, result, valid);
	return result;
}

void CyclesRenderElement::SetFilterEnabled(BOOL enabled)
{
	pblock->SetValue(param_filter_enabled, 0, enabled);
}

BOOL CyclesRenderElement::IsFilterEnabled() const
{
	Interval valid = FOREVER;
	BOOL result;
	pblock->GetValue(param_filter_enabled, 0, result, valid);
	return result;
}

BOOL CyclesRenderElement::BlendOnMultipass() const
{
	return TRUE;
}

BOOL CyclesRenderElement::AtmosphereApplied() const
{
	return TRUE;
}

BOOL CyclesRenderElement::ShadowsApplied() const
{
	return TRUE;
}

void CyclesRenderElement::SetElementName(const MCHAR* const new_name)
{
	pblock->SetValue(param_name, 0, new_name);
}

const MCHAR* CyclesRenderElement::ElementName() const
{
	Interval valid = FOREVER;
	const MCHAR* result;
	pblock->GetValue(param_name, 0, result, valid);
	return result;
}

void CyclesRenderElement::SetPBBitmap(PBBitmap*& bitmap) const
{
	pblock->SetValue(param_bitmap, 0, bitmap);
}

void CyclesRenderElement::GetPBBitmap(PBBitmap*& bitmap) const
{
	Interval valid;
	pblock->GetValue(param_bitmap, 0, bitmap, valid);
}

void* CyclesRenderElement::GetInterface(ULONG id)
{
	return SpecialFX::GetInterface(id);
}

void CyclesRenderElement::ReleaseInterface(ULONG, void*)
{

}

void CyclesRenderElement::SetReference(int i, ReferenceTarget* rtarg)
{
	if (i == 0) {
		pblock = dynamic_cast<IParamBlock2*>(rtarg);
	}
}

CPP_RENDER_ELEMENT_GEN(DiffuseColorRenderElement, GetCyclesRenderElementDiffuseColorClassDesc, re_diffuse_color, RenderPassType::DIFFUSE_COLOR, CYCLES_RE_DIFFUSE_COLOR_CLASS, L"Cycles Diffuse Color")
CPP_RENDER_ELEMENT_GEN(DiffuseDirectRenderElement, GetCyclesRenderElementDiffuseDirectClassDesc, re_diffuse_direct, RenderPassType::DIFFUSE_DIRECT, CYCLES_RE_DIFFUSE_DIRECT_CLASS, L"Cycles Diffuse Direct")
CPP_RENDER_ELEMENT_GEN(DiffuseIndirectRenderElement, GetCyclesRenderElementDiffuseIndirectClassDesc, re_diffuse_indirect, RenderPassType::DIFFUSE_INDIRECT, CYCLES_RE_DIFFUSE_INDIRECT_CLASS, L"Cycles Diffuse Indirect")
CPP_RENDER_ELEMENT_GEN(GlossyColorRenderElement, GetCyclesRenderElementGlossyColorClassDesc, re_glossy_color, RenderPassType::GLOSSY_COLOR, CYCLES_RE_GLOSSY_COLOR_CLASS, L"Cycles Glossy Color")
CPP_RENDER_ELEMENT_GEN(GlossyDirectRenderElement, GetCyclesRenderElementGlossyDirectClassDesc, re_glossy_direct, RenderPassType::GLOSSY_DIRECT, CYCLES_RE_GLOSSY_DIRECT_CLASS, L"Cycles Glossy Direct")
CPP_RENDER_ELEMENT_GEN(GlossyIndirectRenderElement, GetCyclesRenderElementGlossyIndirectClassDesc, re_glossy_indirect, RenderPassType::GLOSSY_INDIRECT, CYCLES_RE_GLOSSY_INDIRECT_CLASS, L"Cycles Glossy Indirect")
CPP_RENDER_ELEMENT_GEN(TransmissionColorRenderElement, GetCyclesRenderElementTransmissionColorClassDesc, re_transmission_color, RenderPassType::TRANSMISSION_COLOR, CYCLES_RE_TRANSMISSION_COLOR_CLASS, L"Cycles Transmission Color")
CPP_RENDER_ELEMENT_GEN(TransmissionDirectRenderElement, GetCyclesRenderElementTransmissionDirectClassDesc, re_transmission_direct, RenderPassType::TRANSMISSION_DIRECT, CYCLES_RE_TRANSMISSION_DIRECT_CLASS, L"Cycles Transmission Direct")
CPP_RENDER_ELEMENT_GEN(TransmissionIndirectRenderElement, GetCyclesRenderElementTransmissionIndirectClassDesc, re_transmission_indirect, RenderPassType::TRANSMISSION_INDIRECT, CYCLES_RE_TRANSMISSION_INDIRECT_CLASS, L"Cycles Transmission Indirect")
CPP_RENDER_ELEMENT_GEN(SubsurfaceColorRenderElement, GetCyclesRenderElementSubsurfaceColorClassDesc, re_subsurface_color, RenderPassType::SUBSURFACE_COLOR, CYCLES_RE_SUBSURFACE_COLOR_CLASS, L"Cycles Subsurface Color (Deprecated)")
CPP_RENDER_ELEMENT_GEN(SubsurfaceDirectRenderElement, GetCyclesRenderElementSubsurfaceDirectClassDesc, re_subsurface_direct, RenderPassType::SUBSURFACE_DIRECT, CYCLES_RE_SUBSURFACE_DIRECT_CLASS, L"Cycles Subsurface Direct (Depracated)")
CPP_RENDER_ELEMENT_GEN(SubsurfaceIndirectRenderElement, GetCyclesRenderElementSubsurfaceIndirectClassDesc, re_subsurface_indirect, RenderPassType::SUBSURFACE_INDIRECT, CYCLES_RE_SUBSURFACE_INDIRECT_CLASS, L"Cycles Subsurface Indirect (Deprecated)")
CPP_RENDER_ELEMENT_GEN(EmissionRenderElement, GetCyclesRenderElementEmissionClassDesc, re_emission, RenderPassType::EMISSION, CYCLES_RE_EMISSION_CLASS, L"Cycles Emission")
CPP_RENDER_ELEMENT_GEN(EnvironmentRenderElement, GetCyclesRenderElementEnvironmentClassDesc, re_environment, RenderPassType::ENVIRONMENT, CYCLES_RE_ENVIRONMENT_CLASS, L"Cycles Environment")
CPP_RENDER_ELEMENT_GEN(NormalRenderElement, GetCyclesRenderElementNormalClassDesc, re_normal, RenderPassType::NORMAL, CYCLES_RE_NORMAL_CLASS, L"Cycles Normal")
CPP_RENDER_ELEMENT_GEN(UVRenderElement, GetCyclesRenderElementUVClassDesc, re_uv, RenderPassType::UV, CYCLES_RE_UV_CLASS, L"Cycles UV")
CPP_RENDER_ELEMENT_GEN(MotionRenderElement, GetCyclesRenderElementMotionClassDesc, re_motion, RenderPassType::MOTION, CYCLES_RE_MOTION_CLASS, L"Cycles Motion")
CPP_RENDER_ELEMENT_GEN(DepthRenderElement, GetCyclesRenderElementDepthClassDesc, re_depth, RenderPassType::DEPTH, CYCLES_RE_DEPTH_CLASS, L"Cycles Depth")
CPP_RENDER_ELEMENT_GEN(MistRenderElement, GetCyclesRenderElementMistClassDesc, re_mist, RenderPassType::MIST, CYCLES_RE_MIST_CLASS, L"Cycles Mist")
CPP_RENDER_ELEMENT_GEN(CryptomatteObjectRenderElement, GetCyclesRenderElementCryptoObjectClassDesc, re_cryptomatte_object, RenderPassType::CRYPTOMATTE_OBJ, CYCLES_RE_CRYPTO_OBJECT_CLASS, L"CryptomatteObject")
CPP_RENDER_ELEMENT_GEN(CryptomatteMaterialRenderElement, GetCyclesRenderElementCryptoMaterialClassDesc, re_cryptomatte_material, RenderPassType::CRYPTOMATTE_MTL, CYCLES_RE_CRYPTO_MATERIAL_CLASS, L"CryptomatteMaterial")
CPP_RENDER_ELEMENT_GEN(CryptomatteAssetRenderElement, GetCyclesRenderElementCryptoAssetClassDesc, re_cryptomatte_asset, RenderPassType::CRYPTOMATTE_ASSET, CYCLES_RE_CRYPTO_ASSET_CLASS, L"CryptomatteAsset")
