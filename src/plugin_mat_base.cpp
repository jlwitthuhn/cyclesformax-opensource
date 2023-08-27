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
 
#include "plugin_mat_base.h"

#include <maxapi.h>
#include <iparamm2.h>

#include "rend_shader_desc.h"

COMMON_NORMAL_ENUMS(nparam)

CyclesMaterialViewportShading::CyclesMaterialViewportShading(CyclesPluginMatBase* const mat_in)
{
	mat_ptr = mat_in;
}

CyclesMaterialViewportShading::~CyclesMaterialViewportShading()
{

}

bool CyclesMaterialViewportShading::IsShadingModelSupported(const ShadingModel model) const
{
	if (model == ShadingModel::Standard) {
		return true;
	}

	return false;
}

IMaterialViewportShading::ShadingModel CyclesMaterialViewportShading::GetCurrentShadingModel() const
{
	return ShadingModel::Standard;
}

bool CyclesMaterialViewportShading::SetCurrentShadingModel(const ShadingModel model)
{
	if (model == ShadingModel::Standard) {
		return true;
	}

	return false;
}

int CyclesMaterialViewportShading::GetSupportedMapLevels() const
{
	return 1;
}

SubmatSlotDescriptor::SubmatSlotDescriptor()
{
	param_id = 0;
	submat_id = 0;
	accepted_closure_type = ClosureType::ANY;
}

ParamBlockDesc2* CyclesPluginMatBase::GetNormalParamBlockDesc()
{
	return nullptr;
}

void CyclesPluginMatBase::PopulateNormalMapDesc(NormalMapDescriptor* const desc, const TimeValue t)
{
	IParamBlock2* const normal_pblock = this->GetParamBlock(normal_pblock_ref);

	if (normal_pblock == nullptr) {
		return;
	}

	const int enabled = normal_pblock->GetInt(nparam_enable, t);
	if (enabled == 0) {
		return;
	}

	desc->enabled = true;

	desc->space = static_cast<NormalMapTangentSpace>(normal_pblock->GetInt(nparam_space, t));

	desc->strength = normal_pblock->GetFloat(nparam_strength, t);
	if (SubTexmapOn(subtex_normal_strength)) {
		desc->strength_map = GetSubTexmap(subtex_normal_strength);
	}

	if (SubTexmapOn(subtex_normal_color)) {
		desc->color_map = GetSubTexmap(subtex_normal_color);
	}
	else {
		const Color color = normal_pblock->GetColor(nparam_color);
		desc->color.x = color.r;
		desc->color.y = color.g;
		desc->color.z = color.b;
	}

	const int invert_green_int = normal_pblock->GetInt(nparam_invert_green, t);
	desc->invert_green = (invert_green_int != 0);
}

ClosureType CyclesPluginMatBase::GetClosureType() const
{
	return node_type;
}

void CyclesPluginMatBase::UpdateVPTexmap(const int subtex_id)
{
	if (TestMtlFlag(MTL_TEX_DISPLAY_ENABLED) && SubTexmapOn(subtex_id)) {
		Texmap* const new_vp_map = GetSubTexmap(subtex_id);
		if (new_vp_map != GetActiveTexmap()) {
			SetActiveTexmap(new_vp_map);
			GetCOREInterface()->ActivateTexture(new_vp_map, this);
		}
	}
}

ParamDlg* CyclesPluginMatBase::CreateParamDlg(const HWND hwMtlEdit, IMtlParams* const imp)
{
	IAutoMParamDlg* param_dlg = GetClassDesc()->CreateParamDlgs(hwMtlEdit, imp, this);

	return param_dlg;
}

Interval CyclesPluginMatBase::Validity(const TimeValue t)
{
	Interval validity = FOREVER;
	Update(t, validity);

	return validity;
}

void CyclesPluginMatBase::Reset()
{
	DeleteReference(main_pblock_ref);
	if (use_common_normal_map) {
		DeleteReference(normal_pblock_ref);
	}
	GetClassDesc()->MakeAutoParamBlocks(this);
}

void CyclesPluginMatBase::SetAmbient(const Color /*c*/, const TimeValue /*t*/)
{
	// Do nothing
}

void CyclesPluginMatBase::SetDiffuse(const Color /*c*/, const TimeValue /*t*/)
{
	// Do nothing
}

void CyclesPluginMatBase::SetSpecular(const Color /*c*/, const TimeValue /*t*/)
{
	// Do nothing
}

void CyclesPluginMatBase::SetShininess(const float /*v*/, const TimeValue /*t*/)
{
	// Do nothing
}

Color CyclesPluginMatBase::GetAmbient(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return color_ambient;
}

Color CyclesPluginMatBase::GetDiffuse(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return color_diffuse;
}

Color CyclesPluginMatBase::GetSpecular(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return color_specular;
}

float CyclesPluginMatBase::GetXParency(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return transparency;
}

float CyclesPluginMatBase::GetShininess(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return shininess;
}

float CyclesPluginMatBase::GetShinStr(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return shine_strength;
}

float CyclesPluginMatBase::WireSize(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return wire_size;
}

float CyclesPluginMatBase::GetSelfIllum(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return self_illum;
}

Color CyclesPluginMatBase::GetSelfIllumColor(const int /*mtlNum*/, const BOOL /*backFace*/)
{
	return self_illum_color;
}

int CyclesPluginMatBase::NumSubTexmaps()
{
	return static_cast<int>(subtex_slots.size());
}

Texmap* CyclesPluginMatBase::GetSubTexmap(const int i)
{
	if (pblock == nullptr) {
		return nullptr;
	}

	for (const SubtexSlotDescriptor desc : subtex_slots) {
		if (i == desc.subtex_id) {
			Interval texmap_valid = FOREVER;
			return GetParamBlock(desc.pblock_ref)->GetTexmap(desc.param_id, 0, texmap_valid);
		}
	}

	return nullptr;
}

int CyclesPluginMatBase::MapSlotType(const int i)
{
	for (const SubtexSlotDescriptor desc : subtex_slots) {
		if (i == desc.subtex_id) {
			return desc.slot_type;
		}
	}

	return MAPSLOT_TEXTURE;
}

void CyclesPluginMatBase::SetSubTexmap(const int i, Texmap* const m)
{
	*base_logger << "CyclesPluginMatBase::SetSubTexmap called: " << i << LogCtl::WRITE_LINE;

	if (pblock == nullptr) {
		return;
	}

	for (const SubtexSlotDescriptor desc : subtex_slots) {
		if (i == desc.subtex_id) {
			GetParamBlock(desc.pblock_ref)->SetValue(desc.param_id, 0, m);
		}
	}
}

int CyclesPluginMatBase::SubTexmapOn(const int i)
{
	if (GetSubTexmap(i) == nullptr) {
		return 0;
	}

	for (const SubtexSlotDescriptor desc : subtex_slots) {
		if (i == desc.subtex_id) {
			Interval texmap_enabled_valid = FOREVER;
			return GetParamBlock(desc.pblock_ref)->GetInt(desc.enabled_param_id, 0, texmap_enabled_valid);
		}
	}

	return 0;
}

#if PLUGIN_SDK_VERSION < 2022
MSTR CyclesPluginMatBase::GetSubTexmapSlotName(const int i)
#else
MSTR CyclesPluginMatBase::GetSubTexmapSlotName(const int i, const bool)
#endif
{
	for (const SubtexSlotDescriptor desc : subtex_slots) {
		if (i == desc.subtex_id) {
			return desc.display_name;
		}
	}

	return _T("");
}

void CyclesPluginMatBase::Shade(ShadeContext& sc)
{
	// Always render black, this material shouldn't be used in a normal max render
	sc.out.c = Color(0.0f, 0.0f, 0.0f);
	sc.out.t = Color(0.0f, 0.0f, 0.0f);
	sc.out.ior = 0.0f;
	sc.out.gbufId = 0;
}

float CyclesPluginMatBase::EvalDisplacement(ShadeContext& /*sc*/)
{
	return 0.0f;
}

Interval CyclesPluginMatBase::DisplacementValidity(TimeValue /*t*/)
{
	return FOREVER;
}

int CyclesPluginMatBase::NumSubMtls()
{
	return static_cast<int>(submat_slots.size());
}

Mtl* CyclesPluginMatBase::GetSubMtl(int i)
{
	if (pblock == nullptr) {
		return nullptr;
	}

	for (const SubmatSlotDescriptor desc : submat_slots) {
		if (i == desc.submat_id) {
			Interval submat_valid = FOREVER;
			return pblock->GetMtl(desc.param_id, 0, submat_valid);
		}
	}

	return nullptr;
}

void CyclesPluginMatBase::SetSubMtl(const int i, Mtl* const m)
{
	if (pblock == nullptr) {
		return;
	}

	CyclesPluginMatBase* const cycles_mtl = dynamic_cast<CyclesPluginMatBase*>(m);
	if (cycles_mtl == nullptr) {
		// Non-cycles type, only allow connection to ANY slot
		for (const SubmatSlotDescriptor desc : submat_slots) {
			if (i == desc.submat_id) {
				if (desc.accepted_closure_type == ClosureType::ANY || m == nullptr) {
					pblock->SetValue(desc.param_id, 0, m);
					return;
				}
			}
		}
		return;
	}

	for (const SubmatSlotDescriptor desc : submat_slots) {
		const bool closure_matches = (desc.accepted_closure_type == ClosureType::ANY) || desc.accepted_closure_type == cycles_mtl->GetClosureType();
		if (closure_matches && i == desc.submat_id) {
			pblock->SetValue(desc.param_id, 0, m);
			return;
		}
	}
}

int CyclesPluginMatBase::VPDisplaySubMtl()
{
	if (viewport_submat_primary >= 0 && GetSubMtl(viewport_submat_primary) != nullptr) {
		return viewport_submat_primary;
	}

	if (viewport_submat_secondary >= 0 && GetSubMtl(viewport_submat_secondary) != nullptr) {
		return viewport_submat_secondary;
	}

	return -1;
}

#if PLUGIN_SDK_VERSION < 2022
MSTR CyclesPluginMatBase::GetSubMtlSlotName(const int i)
#else
MSTR CyclesPluginMatBase::GetSubMtlSlotName(const int i, const bool)
#endif
{
	for (const SubmatSlotDescriptor desc : submat_slots) {
		if (i == desc.submat_id) {
			return desc.display_name;
		}
	}

	return _T("");
}

BOOL CyclesPluginMatBase::SupportTexDisplay()
{
	if (supports_viewport_tex) {
		return TRUE;
	}
	return FALSE;
}

BOOL CyclesPluginMatBase::SupportMultiMapsInViewport()
{
	if (supports_viewport_tex) {
		return TRUE;
	}
	return FALSE;
}

BOOL CyclesPluginMatBase::SetDlgThing(ParamDlg* const /*dlg*/)
{
	return FALSE;
}

IOResult CyclesPluginMatBase::Load(ILoad* const iload)
{
	return Mtl::Load(iload);
}

IOResult CyclesPluginMatBase::Save(ISave* const isave)
{
	return Mtl::Save(isave);
}

SClass_ID CyclesPluginMatBase::SuperClassID()
{
	return MATERIAL_CLASS_ID;
}

Class_ID CyclesPluginMatBase::ClassID()
{
	return GetClassDesc()->ClassID();
}

#if PLUGIN_SDK_VERSION < 2022
void CyclesPluginMatBase::GetClassName(MSTR& s)
#else
void CyclesPluginMatBase::GetClassName(MSTR& s, const bool) const
#endif
{
	s = GetClassDesc()->ClassName();
}

RefResult CyclesPluginMatBase::NotifyRefChanged(const Interval& /*changeInt*/, const RefTargetHandle hTarget, PartID& /*partID*/, const RefMessage message, const BOOL /*propagate*/)
{
	std::string str_message = std::to_string(message);
	if (message == REFMSG_TARGET_DELETED) {
		str_message = "REFMSG_TARGET_DELETED";
	}
	else if (message == REFMSG_CHANGE) {
		str_message = "REFMSG_CHANGE";
	}
	else if (message == REFMSG_SUBANIM_STRUCTURE_CHANGED) {
		str_message = "REFMSG_SUBANIM_STRUCTURE_CHANGED";
	}
	else if (message == REFMSG_WANT_SHOWPARAMLEVEL) {
		str_message = "REFMSG_WANT_SHOWPARAMLEVEL";
	}
	*base_logger << "CyclesPluginMatBase::NotifyRefChanged called with message: " << str_message.c_str() << LogCtl::WRITE_LINE;

	Interval update_valid = FOREVER;

	switch (message)
	{
	case REFMSG_CHANGE:
		valid_interval.SetEmpty();
		if (hTarget != nullptr) {
			for (ParamBlockSlotDescriptor desc : pblock_vec) {
				if (hTarget == desc.pblock) {
					const ParamID changing_param = desc.pblock->LastNotifyParamID();
					GetParamBlockDesc()->InvalidateUI(changing_param);
					break;
				}
			}
		}
		break;

	case REFMSG_TARGET_DELETED:
		if (hTarget != nullptr) {
			if (hTarget == pblock) {
				pblock = nullptr;
			}
			for (ParamBlockSlotDescriptor& desc : pblock_vec) {
				if (hTarget == desc.pblock) {
					desc.pblock = nullptr;
				}
			}
		}
		break;

	default:
		break;
	}
	return REF_SUCCEED;
}

int CyclesPluginMatBase::NumSubs()
{
	return NumRefs();
}

Animatable* CyclesPluginMatBase::SubAnim(const int i)
{
	return GetReference(i);
}

#if PLUGIN_SDK_VERSION < 2022
MSTR CyclesPluginMatBase::SubAnimName(const int i)
#else
MSTR CyclesPluginMatBase::SubAnimName(const int i, const bool)
#endif
{
	if (i < 0 || i >= pblock_vec.size()) {
		return MSTR(L"");
	}
	return pblock_vec[i].name;

}

int CyclesPluginMatBase::NumRefs()
{
	return NumParamBlocks();
}

RefTargetHandle CyclesPluginMatBase::GetReference(const int i)
{
	return GetParamBlock(i);
}

int CyclesPluginMatBase::NumParamBlocks()
{
	return static_cast<int>(pblock_vec.size());
}

IParamBlock2* CyclesPluginMatBase::GetParamBlock(const int i)
{
	if (i < 0 || i >= pblock_vec.size()) {
		return nullptr;
	}
	return pblock_vec[i].pblock;
}

IParamBlock2* CyclesPluginMatBase::GetParamBlockByID(const BlockID id)
{
	for (const ParamBlockSlotDescriptor desc : pblock_vec) {
		if (desc.pblock != nullptr && desc.pblock->ID() == id) {
			return desc.pblock;
		}
	}
	return nullptr;
}

RefTargetHandle CyclesPluginMatBase::Clone(RemapDir &remap)
{
	CyclesPluginMatBase* const new_mat = static_cast<CyclesPluginMatBase*>(GetClassDesc()->Create());

	BaseClone(this, new_mat, remap);
	for (int i = 0; i < static_cast<int>(pblock_vec.size()); i++) {
		const ParamBlockSlotDescriptor desc = pblock_vec[i];
		new_mat->ReplaceReference(i, remap.CloneRef(desc.pblock));
	}

	Interval material_valid = FOREVER;
	new_mat->Update(GetCOREInterface()->GetTime(), material_valid);

	return new_mat;
}

void CyclesPluginMatBase::DeleteThis()
{
	delete this;
}

BaseInterface* CyclesPluginMatBase::GetInterface(const Interface_ID id)
{
	if (supports_viewport_tex && id == IID_MATERIAL_VIEWPORT_SHADING) {
		return static_cast<CyclesMaterialViewportShading*>(this);
	}

	return Mtl::GetInterface(id);
}

CyclesPluginMatBase::CyclesPluginMatBase(const bool supports_viewport_texmap, const bool use_common_normal_map, const int main_pblock_ref, const int normal_pblock_ref) :
	CyclesMaterialViewportShading(this),
	main_pblock_ref(main_pblock_ref),
	normal_pblock_ref(normal_pblock_ref),
	use_common_normal_map(use_common_normal_map),
	base_logger(global_log_manager.new_logger(L"CyclesPluginMatBase", true))
{
	*base_logger << LogCtl::SEPARATOR;

	color_ambient = Color(0.0f, 0.0f, 0.0f);
	color_diffuse = Color(0.0f, 0.0f, 0.0f);
	color_specular = Color(0.0f, 0.0f, 0.0f);
	transparency = 0.0f;
	shininess = 0.0f;
	shine_strength = 0.0f;
	wire_size = 1.0f;
	self_illum = 0.0f;
	self_illum_color = Color(0.0f, 0.0f, 0.0f);

	node_type = ClosureType::SPECIAL;

	supports_viewport_tex = supports_viewport_texmap;
	viewport_submat_primary = -1;
	viewport_submat_secondary = -1;

	valid_interval = NEVER;

	ParamBlockSlotDescriptor main_pblock_desc;
	main_pblock_desc.name = MSTR(L"pblock");
	main_pblock_desc.ref = main_pblock_ref;
	pblock_vec.push_back(main_pblock_desc);

	if (use_common_normal_map) {
		ParamBlockSlotDescriptor normal_pblock_desc;
		normal_pblock_desc.name = MSTR(L"normal_pblock");
		normal_pblock_desc.ref = normal_pblock_ref;
		pblock_vec.push_back(normal_pblock_desc);
	}

	*base_logger << "Constructor complete" << LogCtl::WRITE_LINE;
}

void CyclesPluginMatBase::SetReference(const int i, const RefTargetHandle rtarg)
{
	IParamBlock2* const as_pblock = static_cast<IParamBlock2*>(rtarg);

	*base_logger << "CyclesPluginMatBase::SetReference called: " << i << ", " << static_cast<void*>(rtarg) << LogCtl::WRITE_LINE;

	if (i < 0 || i >= pblock_vec.size()) {
		return;
	}

	if (i == main_pblock_ref) {
		pblock = as_pblock;
	}

	pblock_vec[i].pblock = as_pblock;
}

void CyclesPluginMatBase::Initialize()
{
	PopulateSubmatSlots();
	PopulateSubtexSlots();

	if (use_common_normal_map) {
		assert(GetNormalParamBlockDesc() != nullptr);
		SetupCommonNormalStuff();
	}

	Interval initial_valid = FOREVER;
	Update(0, initial_valid);

	*base_logger << "Initialize complete." << LogCtl::WRITE_LINE;
}

void CyclesPluginMatBase::UpdateSubThings(const TimeValue t, Interval& iv)
{
	Interval subtexes_valid = FOREVER;
	for (int i = 0; i < NumSubTexmaps(); ++i) {
		Texmap* this_texmap = GetSubTexmap(i);
		if (this_texmap != nullptr) {
			Interval this_texmap_valid = FOREVER;
			this_texmap->Update(t, this_texmap_valid);
			subtexes_valid &= this_texmap_valid;
		}
	}

	Interval submats_valid = FOREVER;
	for (int i = 0; i < NumSubMtls(); ++i) {
		Mtl* this_mtl = GetSubMtl(i);
		if (this_mtl != nullptr) {
			Interval this_mtl_valid = FOREVER;
			this_mtl->Update(t, this_mtl_valid);
			submats_valid &= this_mtl_valid;
		}
	}

	iv &= (subtexes_valid & submats_valid);
}

void CyclesPluginMatBase::SetupCommonNormalStuff()
{
	subtex_normal_strength = static_cast<int>(subtex_slots.size());
	subtex_normal_color = static_cast<int>(subtex_slots.size()) + 1;

	SubtexSlotDescriptor normal_strength_tex_slot;
	normal_strength_tex_slot.display_name = _T("Normal Strength");
	normal_strength_tex_slot.enabled_param_id = nparam_strength_map_on;
	normal_strength_tex_slot.param_id = nparam_strength_map;
	normal_strength_tex_slot.slot_type = MAPSLOT_TEXTURE;
	normal_strength_tex_slot.subtex_id = subtex_normal_strength;
	normal_strength_tex_slot.pblock_ref = normal_pblock_ref;

	SubtexSlotDescriptor normal_color_tex_slot;
	normal_color_tex_slot.display_name = _T("Normal Color");
	normal_color_tex_slot.enabled_param_id = nparam_color_map_on;
	normal_color_tex_slot.param_id = nparam_color_map;
	normal_color_tex_slot.slot_type = MAPSLOT_TEXTURE;
	normal_color_tex_slot.subtex_id = subtex_normal_color;
	normal_color_tex_slot.pblock_ref = normal_pblock_ref;

	subtex_slots.push_back(normal_strength_tex_slot);
	subtex_slots.push_back(normal_color_tex_slot);
}
