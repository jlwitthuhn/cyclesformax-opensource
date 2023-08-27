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
 * @brief Defines all render element plugins.
 */

#include <renderelements.h>

#include "util_enums.h"

/**
 * @brief Base class to be used for all simple render elements.
 */
class CyclesRenderElement : public IRenderElement {
public:
	CyclesRenderElement(RenderPassType render_pass_type, Class_ID class_id, const TSTR& class_name);

	// Functions that differ by child class
	virtual RenderPassType GetRenderPassType() const;
	virtual Class_ID ClassID();
#if PLUGIN_SDK_VERSION < 2022
	virtual void GetClassName(TSTR& s) override;
#else
	virtual void GetClassName(TSTR& s, bool localized) const override;
#endif

	// Animatable/ReferenceMaker stuff
	virtual int NumSubs() override;
	virtual Animatable* SubAnim(int i) override;
#if PLUGIN_SDK_VERSION < 2022
	virtual TSTR SubAnimName(int i) override;
#else
	virtual TSTR SubAnimName(int i, bool localized) override;
#endif

	virtual int NumRefs() override;
	virtual RefTargetHandle GetReference(int i) override;

	virtual int NumParamBlocks() override;
	virtual IParamBlock2* GetParamBlock(int i) override;
	virtual IParamBlock2* GetParamBlockByID(BlockID id) override;

	// IRenderElement abstract methods
	virtual void SetEnabled(BOOL enabled) override;
	virtual BOOL IsEnabled() const override;
	virtual void SetFilterEnabled(BOOL enabled) override;
	virtual BOOL IsFilterEnabled() const override;
	virtual BOOL BlendOnMultipass() const override;
	virtual BOOL AtmosphereApplied() const override;
	virtual BOOL ShadowsApplied() const override;
	virtual void SetElementName(const MCHAR* new_name) override;
	virtual const MCHAR* ElementName() const override;
	virtual void SetPBBitmap(PBBitmap*& bitmap) const override;
	virtual void GetPBBitmap(PBBitmap*& bitmap) const override;

	virtual void* GetInterface(ULONG id) override;
	virtual void ReleaseInterface(ULONG id, void* i) override;

protected:
	const RenderPassType render_pass_type;
	const Class_ID class_id;
	const TSTR class_name;

	virtual void SetReference(int i, ReferenceTarget* rtarg) override;

private:
	IParamBlock2* pblock = nullptr;
};

/**
 * @brief Macro to generate a simple render element class.
 */
#define H_RENDER_ELEMENT_GEN(CLASS_NAME) \
class CLASS_NAME : public CyclesRenderElement { \
public: \
	CLASS_NAME(); \
};

H_RENDER_ELEMENT_GEN(DiffuseColorRenderElement)
H_RENDER_ELEMENT_GEN(DiffuseDirectRenderElement)
H_RENDER_ELEMENT_GEN(DiffuseIndirectRenderElement)
H_RENDER_ELEMENT_GEN(GlossyColorRenderElement)
H_RENDER_ELEMENT_GEN(GlossyDirectRenderElement)
H_RENDER_ELEMENT_GEN(GlossyIndirectRenderElement)
H_RENDER_ELEMENT_GEN(TransmissionColorRenderElement)
H_RENDER_ELEMENT_GEN(TransmissionDirectRenderElement)
H_RENDER_ELEMENT_GEN(TransmissionIndirectRenderElement)
H_RENDER_ELEMENT_GEN(SubsurfaceColorRenderElement)
H_RENDER_ELEMENT_GEN(SubsurfaceDirectRenderElement)
H_RENDER_ELEMENT_GEN(SubsurfaceIndirectRenderElement)
H_RENDER_ELEMENT_GEN(EmissionRenderElement)
H_RENDER_ELEMENT_GEN(EnvironmentRenderElement)
H_RENDER_ELEMENT_GEN(NormalRenderElement)
H_RENDER_ELEMENT_GEN(UVRenderElement)
H_RENDER_ELEMENT_GEN(MotionRenderElement)
H_RENDER_ELEMENT_GEN(DepthRenderElement)
H_RENDER_ELEMENT_GEN(MistRenderElement)
H_RENDER_ELEMENT_GEN(CryptomatteObjectRenderElement)
H_RENDER_ELEMENT_GEN(CryptomatteMaterialRenderElement)
H_RENDER_ELEMENT_GEN(CryptomatteAssetRenderElement)
