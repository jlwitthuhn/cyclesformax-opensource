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
 * @brief Defines material plugin class CyclesPrincipledMat.
 */

#include "plugin_mat_base.h"
#include "util_enums.h"

 /**
  * @brief Material class that allows the use of the Cycles Principled BSDF shader.
  */
class CyclesPrincipledBSDFMat : public CyclesPluginMatBase {
public:
	CyclesPrincipledBSDFMat();

	// CyclesPluginMatBase overrides
	virtual void Initialize() override;

	virtual IOResult Load(ILoad *iload) override;
	virtual IOResult Save(ISave *isave) override;

	// Paramblock access
	virtual void PopulateCCNormalMapDesc(NormalMapDescriptor* desc, TimeValue t);

	// General
	PrincipledBsdfDistribution GetDistribution(TimeValue t, Interval& valid);
	float GetIOR(TimeValue t, Interval& valid);
	Color GetBaseColor(TimeValue t, Interval& valid);
	Texmap* GetBaseColorTexmap();
	float GetMetallic(TimeValue t, Interval& valid);
	Texmap* GetMetallicTexmap();
	float GetRoughness(TimeValue t, Interval& valid);
	Texmap* GetRoughnessTexmap();
	float GetSheen(TimeValue t, Interval& valid);
	Texmap* GetSheenTexmap();
	float GetSheenTint(TimeValue t, Interval& valid);
	Texmap* GetSheenTintTexmap();
	float GetTransmission(TimeValue t, Interval& valid);
	Texmap* GetTransmissionTexmap();
	float GetTransmissionRoughness(TimeValue t, Interval& valid);
	Texmap* GetTransmissionRoughnessTexmap();

	// Specular
	float GetSpecular(TimeValue t, Interval& valid);
	Texmap* GetSpecularTexmap();
	float GetSpecularTint(TimeValue t, Interval& valid);
	Texmap* GetSpecularTintTexmap();
	float GetAnisotropic(TimeValue t, Interval& valid);
	Texmap* GetAnisotropicTexmap();
	float GetAnisotropicRotation(TimeValue t, Interval& valid);
	Texmap* GetAnisotropicRotationTexmap();

	// Subsurface
	PrincipledBsdfSSSMethod GetSubsurfaceMethod(TimeValue t, Interval& valid);
	float GetSubsurface(TimeValue t, Interval& valid);
	Texmap* GetSubsurfaceTexmap();
	Color GetSubsurfaceColor(TimeValue t, Interval& valid);
	Texmap* GetSubsurfaceColorTexmap();
	Point3 GetSubsurfaceRadius(TimeValue t, Interval& valid);

	// Clearcoat
	float GetClearcoat(TimeValue t, Interval& valid);
	Texmap* GetClearcoatTexmap();
	float GetClearcoatRoughness(TimeValue t, Interval& valid);
	Texmap* GetClearcoatRoughnessTexmap();

	// Other
	Color GetEmissionColor(TimeValue t, Interval& valid);
	Texmap* GetEmissionColorTexmap();
	float GetAlpha(TimeValue t, Interval& valid);
	Texmap* GetAlphaTexmap();

	// CyclesPluginMatBase pure virtual functions
	virtual ClassDesc2* GetClassDesc() const override;
	virtual ParamBlockDesc2* GetParamBlockDesc() override;
	virtual ParamBlockDesc2* GetNormalParamBlockDesc() override;
	virtual void PopulateSubmatSlots() override;
	virtual void PopulateSubtexSlots() override;

	// Max functions
	virtual void Update(TimeValue t, Interval& valid) override;

private:
	void AddClearcoatNormalSubtexSlots();

	int subtex_cc_normal_strength = 0;
	int subtex_cc_normal_color = 0;
};
