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
 
#include "plugin_mod_properties.h"

#include <iparamm2.h>
#include <modstack.h>

#include "const_classid.h"
#include "max_classdesc_mod.h"
#include "win_resource.h"

// Pblock enum
enum { pblock_ref_general, pblock_ref_count };

// Parameter enum
enum { param_gen_shadow_catcher };

static ParamBlockDesc2 mat_emission_pblock_desc(
	// Pblock data
	pblock_ref_general,
	_T("pblock_general"),
	0,
	GetCyclesPropertiesModifierClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	pblock_ref_general,
	IDD_PANEL_MOD_PROPERTIES,
	IDS_GENERAL,
	0,
	0,
	NULL,
		// Shadow catcher on
		param_gen_shadow_catcher,
		_T("is_shadow_catcher"),
		TYPE_BOOL,
		0,
		IDS_SHADOW_CATCHER,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_PROP_SHADOW_CATCHER,
		p_end,
	p_end
	);

CyclesPropertiesMod::CyclesPropertiesMod()
{
	GetCyclesPropertiesModifierClassDesc()->MakeAutoParamBlocks(this);
}

bool CyclesPropertiesMod::GetIsShadowCatcher(const TimeValue t)
{
	if (pblock_general == nullptr) {
		return false;
	}
	return pblock_general->GetInt(param_gen_shadow_catcher, t) != 0;
}

ChannelMask CyclesPropertiesMod::ChannelsUsed()
{
	return 0;
}

ChannelMask CyclesPropertiesMod::ChannelsChanged()
{
	return 0;
}

Class_ID CyclesPropertiesMod::InputType()
{
	return defObjectClassID;
}

void CyclesPropertiesMod::ModifyObject(const TimeValue /*t*/, ModContext& /*mc*/, ObjectState* const /*os*/, INode* const /*node*/)
{
	// Do nothing
}

void CyclesPropertiesMod::DeleteThis()
{
	delete this;
}

#if PLUGIN_SDK_VERSION < 2022
void CyclesPropertiesMod::GetClassName(MSTR& s)
#else
void CyclesPropertiesMod::GetClassName(MSTR& s, const bool) const
#endif
{
	s = L"mod_cycles_properties";
}

SClass_ID CyclesPropertiesMod::SuperClassID()
{
	return OSM_CLASS_ID;
}

Class_ID CyclesPropertiesMod::ClassID()
{
	return CYCLES_MOD_PROPERTIES_CLASS;
}

CreateMouseCallBack* CyclesPropertiesMod::GetCreateMouseCallBack()
{
	return nullptr;
}

#if PLUGIN_SDK_VERSION < 2022
const MCHAR* CyclesPropertiesMod::GetObjectName()
#else
const MCHAR* CyclesPropertiesMod::GetObjectName(const bool) const
#endif
{
	return L"Cycles Properties";
}

void CyclesPropertiesMod::NotifyPostCollapse(INode* const node, Object* const obj, IDerivedObject* const /*derObj*/, const int /*index*/)
{
	Object* const object_ref = node->GetObjectRef();
	IDerivedObject* derived_obj = nullptr;

	if (object_ref->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		derived_obj = dynamic_cast<IDerivedObject*>(object_ref);
	}
	else {
		derived_obj = CreateDerivedObject(obj);
		node->SetObjectRef(derived_obj);
	}

	derived_obj->AddModifier(this, nullptr, derived_obj->NumModifiers());
}

RefTargetHandle CyclesPropertiesMod::Clone(RemapDir& remap)
{
	CyclesPropertiesMod* const new_mod = new CyclesPropertiesMod();
	new_mod->ReplaceReference(pblock_ref_general, remap.CloneRef(pblock_general));
	BaseClone(this, new_mod, remap);

	return new_mod;
}

int CyclesPropertiesMod::NumSubs()
{
	return NumRefs();
}

Animatable* CyclesPropertiesMod::SubAnim(const int i)
{
	return GetReference(i);
}

#if PLUGIN_SDK_VERSION < 2022
TSTR CyclesPropertiesMod::SubAnimName(const int i)
#else
TSTR CyclesPropertiesMod::SubAnimName(const int i, const bool)
#endif
{
	if (i == pblock_ref_general) {
		return L"pblock_general";
	}
	return L"";
}

int CyclesPropertiesMod::SubNumToRefNum(const int subNum)
{
	return subNum;
}

int CyclesPropertiesMod::NumRefs()
{
	return NumParamBlocks();
}

RefTargetHandle CyclesPropertiesMod::GetReference(const int i)
{
	return GetParamBlock(i);
}

void CyclesPropertiesMod::SetReference(const int i, const RefTargetHandle rtarg)
{
	if (i == pblock_ref_general) {
		pblock_general = static_cast<IParamBlock2*>(rtarg);
	}
}

RefResult CyclesPropertiesMod::NotifyRefChanged(
	const Interval& /*changeInt*/,
	const RefTargetHandle /*hTarget*/,
	PartID& /*partID*/,
	const RefMessage /*message*/,
	const BOOL /*propagate*/
	)
{
	return REF_SUCCEED;
}

int CyclesPropertiesMod::NumParamBlocks()
{
	return 1;
}

IParamBlock2* CyclesPropertiesMod::GetParamBlock(const int i)
{
	if (i == pblock_ref_general) {
		return pblock_general;
	}
	return nullptr;
}

IParamBlock2* CyclesPropertiesMod::GetParamBlockByID(const BlockID id)
{
	if (pblock_general != nullptr && pblock_general->ID() == id) {
		return pblock_general;
	}
	return nullptr;
}

void CyclesPropertiesMod::BeginEditParams(IObjParam* const ip, const ULONG flags, Animatable* const prev)
{
	GetCyclesPropertiesModifierClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void CyclesPropertiesMod::EndEditParams(IObjParam* const ip, const ULONG flags, Animatable* const prev)
{
	GetCyclesPropertiesModifierClassDesc()->EndEditParams(ip, this, flags, prev);
}
