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
 * @brief Defines texmap plugin class CyclesSkyEnvTexmap.
 */

#include <string>

#include <util/util_types.h>

#include "plugin_tex.h"
#include "util_enums.h"

 /**
  * @brief The texmap plugin class that allows users to configure a Cycles Sky.
  */
class CyclesSkyEnvTexmap: public CyclesPluginTexmap {
public:
	CyclesSkyEnvTexmap();

	// Access to pblock parameters
	SkyType GetSkyType(TimeValue t, Interval& iv);
	ccl::float3 GetSunVector(TimeValue t, Interval& iv);
	float GetTurbidity(TimeValue t, Interval& iv);
	float GetGroundAlbedo(TimeValue t, Interval& iv);

	// CyclesPluginTexmap functions
	virtual ClassDesc2* GetClassDesc() const override;
	virtual ParamBlockDesc2* GetParamBlockDesc() override;

	// MtlBase functions
	virtual Class_ID ClassID();
	virtual Interval Validity(TimeValue t);

	// ReferenceTarget functions
	virtual RefTargetHandle Clone(RemapDir& remap);
};
