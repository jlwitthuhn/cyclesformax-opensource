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
 
#include "max_classdesc_re.h"

#include "plugin_re_simple.h"

void* CyclesRenderElementDiffuseColorClassDesc::Create(BOOL)
{
	return new DiffuseColorRenderElement{};
}

void* CyclesRenderElementDiffuseDirectClassDesc::Create(BOOL)
{
	return new DiffuseDirectRenderElement{};
}

void* CyclesRenderElementDiffuseIndirectClassDesc::Create(BOOL)
{
	return new DiffuseIndirectRenderElement{};
}

void* CyclesRenderElementGlossyColorClassDesc::Create(BOOL)
{
	return new GlossyColorRenderElement{};
}

void* CyclesRenderElementGlossyDirectClassDesc::Create(BOOL)
{
	return new GlossyDirectRenderElement{};
}

void* CyclesRenderElementGlossyIndirectClassDesc::Create(BOOL)
{
	return new GlossyIndirectRenderElement{};
}

void* CyclesRenderElementTransmissionColorClassDesc::Create(BOOL)
{
	return new TransmissionColorRenderElement{};
}

void* CyclesRenderElementTransmissionDirectClassDesc::Create(BOOL)
{
	return new TransmissionDirectRenderElement{};
}

void* CyclesRenderElementTransmissionIndirectClassDesc::Create(BOOL)
{
	return new TransmissionIndirectRenderElement{};
}

void* CyclesRenderElementSubsurfaceColorClassDesc::Create(BOOL)
{
	return new SubsurfaceColorRenderElement{};
}

void* CyclesRenderElementSubsurfaceDirectClassDesc::Create(BOOL)
{
	return new SubsurfaceDirectRenderElement{};
}

void* CyclesRenderElementSubsurfaceIndirectClassDesc::Create(BOOL)
{
	return new SubsurfaceIndirectRenderElement{};
}

void* CyclesRenderElementEmissionClassDesc::Create(BOOL)
{
	return new EmissionRenderElement{};
}

void* CyclesRenderElementEnvironmentClassDesc::Create(BOOL)
{
	return new EnvironmentRenderElement{};
}

void* CyclesRenderElementNormalClassDesc::Create(BOOL)
{
	return new NormalRenderElement{};
}

void* CyclesRenderElementUVClassDesc::Create(BOOL)
{
	return new UVRenderElement{};
}

void* CyclesRenderElementMotionClassDesc::Create(BOOL)
{
	return new MotionRenderElement{};
}

void* CyclesRenderElementDepthClassDesc::Create(BOOL)
{
	return new DepthRenderElement{};
}

void* CyclesRenderElementMistClassDesc::Create(BOOL)
{
	return new MistRenderElement{};
}

void* CyclesRenderElementCryptoObjectClassDesc::Create(BOOL)
{
	return new CryptomatteObjectRenderElement{};
}

void* CyclesRenderElementCryptoMaterialClassDesc::Create(BOOL)
{
	return new CryptomatteMaterialRenderElement{};
}

void* CyclesRenderElementCryptoAssetClassDesc::Create(BOOL)
{
	return new CryptomatteAssetRenderElement{};
}

CyclesRenderElementDiffuseColorClassDesc* GetCyclesRenderElementDiffuseColorClassDesc()
{
	static CyclesRenderElementDiffuseColorClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementDiffuseDirectClassDesc* GetCyclesRenderElementDiffuseDirectClassDesc()
{
	static CyclesRenderElementDiffuseDirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementDiffuseIndirectClassDesc* GetCyclesRenderElementDiffuseIndirectClassDesc()
{
	static CyclesRenderElementDiffuseIndirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementGlossyColorClassDesc* GetCyclesRenderElementGlossyColorClassDesc()
{
	static CyclesRenderElementGlossyColorClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementGlossyDirectClassDesc* GetCyclesRenderElementGlossyDirectClassDesc()
{
	static CyclesRenderElementGlossyDirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementGlossyIndirectClassDesc* GetCyclesRenderElementGlossyIndirectClassDesc()
{
	static CyclesRenderElementGlossyIndirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementTransmissionColorClassDesc* GetCyclesRenderElementTransmissionColorClassDesc()
{
	static CyclesRenderElementTransmissionColorClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementTransmissionDirectClassDesc* GetCyclesRenderElementTransmissionDirectClassDesc()
{
	static CyclesRenderElementTransmissionDirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementTransmissionIndirectClassDesc* GetCyclesRenderElementTransmissionIndirectClassDesc()
{
	static CyclesRenderElementTransmissionIndirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementSubsurfaceColorClassDesc* GetCyclesRenderElementSubsurfaceColorClassDesc()
{
	static CyclesRenderElementSubsurfaceColorClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementSubsurfaceDirectClassDesc* GetCyclesRenderElementSubsurfaceDirectClassDesc()
{
	static CyclesRenderElementSubsurfaceDirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementSubsurfaceIndirectClassDesc* GetCyclesRenderElementSubsurfaceIndirectClassDesc()
{
	static CyclesRenderElementSubsurfaceIndirectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementEmissionClassDesc* GetCyclesRenderElementEmissionClassDesc()
{
	static CyclesRenderElementEmissionClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementEnvironmentClassDesc* GetCyclesRenderElementEnvironmentClassDesc()
{
	static CyclesRenderElementEnvironmentClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementNormalClassDesc* GetCyclesRenderElementNormalClassDesc()
{
	static CyclesRenderElementNormalClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementUVClassDesc* GetCyclesRenderElementUVClassDesc()
{
	static CyclesRenderElementUVClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementMotionClassDesc* GetCyclesRenderElementMotionClassDesc()
{
	static CyclesRenderElementMotionClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementDepthClassDesc* GetCyclesRenderElementDepthClassDesc()
{
	static CyclesRenderElementDepthClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementMistClassDesc* GetCyclesRenderElementMistClassDesc()
{
	static CyclesRenderElementMistClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementCryptoObjectClassDesc* GetCyclesRenderElementCryptoObjectClassDesc()
{
	static CyclesRenderElementCryptoObjectClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementCryptoMaterialClassDesc* GetCyclesRenderElementCryptoMaterialClassDesc()
{
	static CyclesRenderElementCryptoMaterialClassDesc class_desc;
	return &class_desc;
}

CyclesRenderElementCryptoAssetClassDesc* GetCyclesRenderElementCryptoAssetClassDesc()
{
	static CyclesRenderElementCryptoAssetClassDesc class_desc;
	return &class_desc;
}
