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
 * @brief Defines texmap plugin class BitmapFilterTexmap.
 */

#include "plugin_tex.h"

 /**
  * @brief The texmap plugin class that allows users to configure texmap baking parameters.
  */
class BitmapFilterTexmap : public CyclesPluginTexmap {
public:
	BitmapFilterTexmap();

	// Easy access to pblock values
	int GetParamWidth(TimeValue t);
	int GetParamHeight(TimeValue t);
	bool UseFloatPrecision(TimeValue t);
	
	// CyclesPluginTexmap functions
	virtual ClassDesc2* GetClassDesc() const override;
	virtual ParamBlockDesc2* GetParamBlockDesc() override;

	// ISubMap functions
	virtual int NumSubTexmaps();
	virtual Texmap* GetSubTexmap(int i);
	virtual int MapSlotType(int i);
	virtual void SetSubTexmap(int i, Texmap* m);
	virtual int SubTexmapOn(int i);
#if PLUGIN_SDK_VERSION < 2022
	virtual MSTR GetSubTexmapSlotName(int i) override;
#else
	virtual MSTR GetSubTexmapSlotName(int i, bool localized) override;
#endif

	// Texmap functions
	virtual AColor EvalColor(ShadeContext& sc);
	virtual Point3 EvalNormalPerturb(ShadeContext& sc);

	// MtlBase functions
	virtual Class_ID ClassID();
	virtual Interval Validity(TimeValue t);
	virtual BOOL SupportTexDisplay();
	virtual DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
	virtual void Update(TimeValue t, Interval &valid);

	// ReferenceTarget functions
	virtual RefTargetHandle Clone(RemapDir& remap);
};
