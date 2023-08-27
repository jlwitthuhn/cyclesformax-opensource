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
 * @brief Defines material plugin class CyclesHairMat.
 */

#include "plugin_mat_base.h"

 /**
  * @brief Material class that allows the use of the Cycles Hair shader.
  */
class CyclesHairMat : public CyclesPluginMatBase {
public:
	CyclesHairMat();

	// Paramblock access
	HairComponent GetComponent(TimeValue t, Interval& iv);
	Color GetHairColor(TimeValue t, Interval& iv);
	Texmap* GetHairColorTexmap();
	float GetOffset(TimeValue t, Interval& iv);
	Texmap* GetOffsetTexmap();
	float GetRoughnessU(TimeValue t, Interval& iv);
	Texmap* GetRoughnessUTexmap();
	float GetRoughnessV(TimeValue t, Interval& iv);
	Texmap* GetRoughnessVTexmap();

	// CyclesPluginMatBase pure virtual functions
	virtual ClassDesc2* GetClassDesc() const override;
	virtual ParamBlockDesc2* GetParamBlockDesc() override;
	virtual void PopulateSubmatSlots() override;
	virtual void PopulateSubtexSlots() override;

	// Max functions
	virtual void Update(TimeValue t, Interval& valid) override;
};
