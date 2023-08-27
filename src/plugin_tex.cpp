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
 
#include "plugin_tex.h"

#include <iparamm2.h>

#define PBLOCK_REF 0

int CyclesPluginTexmap::NumSubTexmaps()
{
	return 0;
}

#if PLUGIN_SDK_VERSION < 2022
void CyclesPluginTexmap::GetClassName(MSTR& s)
#else
void CyclesPluginTexmap::GetClassName(MSTR& s, const bool) const
#endif
{
	s = GetClassDesc()->ClassName();
}

AColor CyclesPluginTexmap::EvalColor(ShadeContext& /*sc*/)
{
	return AColor(0.0f, 0.0f, 0.0f);
}

Point3 CyclesPluginTexmap::EvalNormalPerturb(ShadeContext& /*sc*/)
{
	return Point3(0.0f, 0.0f, 0.0f);
}

void CyclesPluginTexmap::Update(const TimeValue /*t*/, Interval &/*valid*/)
{
	// Do nothing
}

void CyclesPluginTexmap::Reset()
{
	DeleteReference(PBLOCK_REF);
	GetClassDesc()->MakeAutoParamBlocks(this);
}

ParamDlg* CyclesPluginTexmap::CreateParamDlg(const HWND hwMtlEdit, IMtlParams* const imp)
{
	IAutoMParamDlg* const param_dlg = GetClassDesc()->CreateParamDlgs(hwMtlEdit, imp, this);
	return param_dlg;
}

int CyclesPluginTexmap::NumRefs()
{
	return 1;
}

RefTargetHandle CyclesPluginTexmap::GetReference(const int i)
{
	if (i == PBLOCK_REF) {
		return pblock;
	}
	return nullptr;
}

void CyclesPluginTexmap::SetReference(const int i, const RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF) {
		pblock = static_cast<IParamBlock2*>(rtarg);
	}
}

RefResult CyclesPluginTexmap::NotifyRefChanged(const Interval& /*changeInt*/, const RefTargetHandle hTarget, PartID& /*partID*/, const RefMessage message, const BOOL /*propagate*/)
{
	switch (message)
	{
		case REFMSG_CHANGE:
			if (hTarget == pblock) {
				const ParamID changing_param = pblock->LastNotifyParamID();
				GetParamBlockDesc()->InvalidateUI(changing_param);
			}
			break;
		case REFMSG_TARGET_DELETED:
			if (hTarget == pblock) {
				pblock = nullptr;
			}
			break;
	}

	return REF_SUCCEED;
}

int CyclesPluginTexmap::NumSubs()
{
	return 1;
}

Animatable* CyclesPluginTexmap::SubAnim(const int i)
{
	if (i == PBLOCK_REF) {
		return pblock;
	}

	return nullptr;
}

#if PLUGIN_SDK_VERSION < 2022
MSTR CyclesPluginTexmap::SubAnimName(const int i)
#else
MSTR CyclesPluginTexmap::SubAnimName(const int i, const bool)
#endif
{
	if (i == PBLOCK_REF) {
		return TSTR(L"pblock");
	}

	return TSTR(L"");
}

void CyclesPluginTexmap::DeleteThis()
{
	delete this;
}

int CyclesPluginTexmap::NumParamBlocks()
{
	return 1;
}

IParamBlock2* CyclesPluginTexmap::GetParamBlock(const int i)
{
	return (i == PBLOCK_REF) ? pblock : nullptr;
}

IParamBlock2* CyclesPluginTexmap::GetParamBlockByID(const short id)
{
	if (pblock == nullptr) {
		return nullptr;
	}
	return (pblock->ID() == id) ? pblock : nullptr;
}

CyclesPluginTexmap::CyclesPluginTexmap()
{
	pblock = nullptr;
}

CyclesPluginTexmap::~CyclesPluginTexmap()
{
	DeleteReference(PBLOCK_REF);
}
