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
 * @brief Defines ClassDesc2s for render elements included with the plugin.
 */

#include <iparamb2.h>

#include "const_classid.h"

extern HINSTANCE hInstance;

class CyclesRenderElementDiffuseColorClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Diffuse Color"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_DIFFUSE_COLOR_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_diffuse_color"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementDiffuseDirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Diffuse Direct"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_DIFFUSE_DIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_diffuse_direct"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementDiffuseIndirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Diffuse Indirect"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_DIFFUSE_INDIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_diffuse_indirect"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementGlossyColorClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Glossy Color"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_GLOSSY_COLOR_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_glossy_color"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementGlossyDirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Glossy Direct"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_GLOSSY_DIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_glossy_direct"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementGlossyIndirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Glossy Indirect"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_GLOSSY_INDIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_glossy_indirect"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementTransmissionColorClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Transmission Color"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_TRANSMISSION_COLOR_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_transmission_color"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementTransmissionDirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Transmission Direct"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_TRANSMISSION_DIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_transmission_direct"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementTransmissionIndirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Transmission Indirect"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_TRANSMISSION_INDIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_transmission_indirect"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementSubsurfaceColorClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Subsurface Color"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_SUBSURFACE_COLOR_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_subsurface_color"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementSubsurfaceDirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Subsurface Direct"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_SUBSURFACE_DIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_subsurface_direct"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementSubsurfaceIndirectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Subsurface Indirect"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_SUBSURFACE_INDIRECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_subsurface_indirect"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementEmissionClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Emission"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_EMISSION_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_emission"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementEnvironmentClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Environment"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_ENVIRONMENT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_environment"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementNormalClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Normal"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_NORMAL_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_normal"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementUVClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles UV"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_UV_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_uv"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementMotionClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Motion"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_MOTION_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_motion"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementDepthClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Depth"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_DEPTH_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_depth"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementMistClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Mist"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_MIST_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_mist"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementCryptoObjectClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Cryptomatte (Object Name)"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_CRYPTO_OBJECT_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_cryptomatte_object"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementCryptoMaterialClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Cryptomatte (Material Name)"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_CRYPTO_MATERIAL_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_cryptomatte_material"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

class CyclesRenderElementCryptoAssetClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return _T("Cycles Cryptomatte (Asset Name)"); }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_RE_CRYPTO_ASSET_CLASS; }
	virtual const MCHAR* Category() { return _T("CyclesRenderElement"); }
	virtual const MCHAR* InternalName() { return _T("re_cycles_cryptomatte_asset"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

CyclesRenderElementDiffuseColorClassDesc* GetCyclesRenderElementDiffuseColorClassDesc();
CyclesRenderElementDiffuseDirectClassDesc* GetCyclesRenderElementDiffuseDirectClassDesc();
CyclesRenderElementDiffuseIndirectClassDesc* GetCyclesRenderElementDiffuseIndirectClassDesc();
CyclesRenderElementGlossyColorClassDesc* GetCyclesRenderElementGlossyColorClassDesc();
CyclesRenderElementGlossyDirectClassDesc* GetCyclesRenderElementGlossyDirectClassDesc();
CyclesRenderElementGlossyIndirectClassDesc* GetCyclesRenderElementGlossyIndirectClassDesc();
CyclesRenderElementTransmissionColorClassDesc* GetCyclesRenderElementTransmissionColorClassDesc();
CyclesRenderElementTransmissionDirectClassDesc* GetCyclesRenderElementTransmissionDirectClassDesc();
CyclesRenderElementTransmissionIndirectClassDesc* GetCyclesRenderElementTransmissionIndirectClassDesc();
CyclesRenderElementSubsurfaceColorClassDesc* GetCyclesRenderElementSubsurfaceColorClassDesc();
CyclesRenderElementSubsurfaceDirectClassDesc* GetCyclesRenderElementSubsurfaceDirectClassDesc();
CyclesRenderElementSubsurfaceIndirectClassDesc* GetCyclesRenderElementSubsurfaceIndirectClassDesc();
CyclesRenderElementEmissionClassDesc* GetCyclesRenderElementEmissionClassDesc();
CyclesRenderElementEnvironmentClassDesc* GetCyclesRenderElementEnvironmentClassDesc();
CyclesRenderElementNormalClassDesc* GetCyclesRenderElementNormalClassDesc();
CyclesRenderElementUVClassDesc* GetCyclesRenderElementUVClassDesc();
CyclesRenderElementMotionClassDesc* GetCyclesRenderElementMotionClassDesc();
CyclesRenderElementDepthClassDesc* GetCyclesRenderElementDepthClassDesc();
CyclesRenderElementMistClassDesc* GetCyclesRenderElementMistClassDesc();
CyclesRenderElementCryptoObjectClassDesc* GetCyclesRenderElementCryptoObjectClassDesc();
CyclesRenderElementCryptoMaterialClassDesc* GetCyclesRenderElementCryptoMaterialClassDesc();
CyclesRenderElementCryptoAssetClassDesc* GetCyclesRenderElementCryptoAssetClassDesc();
