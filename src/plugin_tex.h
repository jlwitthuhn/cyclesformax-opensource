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
 
#pragma once

/**
 * @file
 * @brief Defines CyclesPluginTexmap.
 */

#include <Materials/Texmap.h>

class ClassDesc2;

/**
 * @brief The base class used by all texmap plugin classes.
 */
class CyclesPluginTexmap : public Texmap {
public:
	virtual ClassDesc2* GetClassDesc() const = 0;
	virtual ParamBlockDesc2* GetParamBlockDesc() = 0;

	// ISubMap functions
	virtual int NumSubTexmaps();

	// Texmap functions
#if PLUGIN_SDK_VERSION < 2022
	virtual void GetClassName(MSTR &s) override;
#else
	virtual void GetClassName(MSTR &s, bool localized) const override;
#endif
	virtual AColor EvalColor(ShadeContext& sc);
	virtual Point3 EvalNormalPerturb(ShadeContext& sc);

	// MtlBase functions
	virtual void Update(TimeValue t, Interval &valid);
	virtual void Reset();
	virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);

	// ReferenceMaker functions
	virtual int NumRefs();
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);
	virtual RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);

	// Animatable functions
	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
#if PLUGIN_SDK_VERSION < 2022
	virtual MSTR SubAnimName(int i) override;
#else
	virtual MSTR SubAnimName(int i, bool localized) override;
#endif

	virtual void DeleteThis();
	
	virtual int NumParamBlocks();
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(short  id);

protected:
	CyclesPluginTexmap();
	~CyclesPluginTexmap();

	IParamBlock2* pblock = nullptr;
};
