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
 * @brief Defines ClassDesc2s for materials included with the plugin.
 */

#include <iparamb2.h>

#include "const_classid.h"
#include "max_mat_browser.h"

extern HINSTANCE hInstance;

class CyclesMaterialShaderClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Shader"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_SHADER_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_shader"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatShaderBroswerEntryInfo entry_info;
};

class CyclesMaterialShaderGraph08ClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Shader Graph - 08"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_SHADER_GRAPH_08_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_shader_graph_08"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatShaderGraph08BrowserEntryInfo entry_info;
};

class CyclesMaterialShaderGraph16ClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Shader Graph - 16"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_SHADER_GRAPH_16_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_shader_graph_16"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatShaderGraph16BrowserEntryInfo entry_info;
};

class CyclesMaterialShaderGraph32ClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Shader Graph - 32"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_SHADER_GRAPH_32_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_shader_graph_32"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatShaderGraph32BrowserEntryInfo entry_info;
};

class CyclesMaterialAddClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Add (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_ADD_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_add"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatAddBrowserEntryInfo entry_info;
};

class CyclesMaterialAnisotropicClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Anisotropic (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_ANISOTROPIC_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_anisotropic"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatAnisotropicBrowserEntryInfo entry_info;
};

class CyclesMaterialDiffuseClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Diffuse (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_DIFFUSE_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_diffuse"; }
	virtual HINSTANCE HInstance() {
		return hInstance;
	}
	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatDiffuseBrowserEntryInfo entry_info;
};

class CyclesMaterialEmissionClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Emission (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_EMISSION_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_emission"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatEmissionBrowserEntryInfo entry_info;
};

class CyclesMaterialGlassClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Glass (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_GLASS_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_glass"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatGlassBrowserEntryInfo entry_info;
};

class CyclesMaterialGlossyClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Glossy (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_GLOSSY_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_glossy"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatGlossyBrowserEntryInfo entry_info;
};

class CyclesMaterialHoldoutClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Holdout (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_HOLDOUT_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_holdout"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatHoldoutBrowserEntryInfo entry_info;
};

class CyclesMaterialHairClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Hair (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_HAIR_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_hair"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatHairBrowserEntryInfo entry_info;
};

class CyclesMaterialMixClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Mix (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_MIX_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_mix"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatMixBrowserEntryInfo entry_info;
};

class CyclesMaterialPrincipledBsdfClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Principled BSDF (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_PRINCIPLED_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_principled_bsdf"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatPrincipledBsdfBrowserEntryInfo entry_info;
};

class CyclesMaterialRefractionClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Refraction (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_REFRACTION_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_refraction"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatRefractionBrowserEntryInfo entry_info;
};

class CyclesMaterialSubsurfaceScatterClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Subsurface Scattering (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_SUBSURFACE_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_subsurface_scatter"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatSubsurfaceScatterBrowserEntryInfo entry_info;
};

class CyclesMaterialToonClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Toon (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_TOON_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_toon"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatToonBrowserEntryInfo entry_info;
};

class CyclesMaterialTranslucentClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Translucent (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_TRANSLUCENT_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_translucent"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatTranslucentBrowserEntryInfo entry_info;
};

class CyclesMaterialTransparentClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Transparent (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_TRANSPARENT_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_transparent"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatTransparentBrowserEntryInfo entry_info;
};

class CyclesMaterialVelvetClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Velvet (Surface)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_VELVET_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_velvet"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatVelvetBrowserEntryInfo entry_info;
};

class CyclesMaterialVolumeAbsorptionClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Volume Absorption (Volume)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_VOL_ABSORPTION_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_vol_absorption"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatVolAbsorptionBrowserEntryInfo entry_info;
};

class CyclesMaterialVolumeScatterClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Volume Scatter (Volume)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_VOL_SCATTER_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_vol_scatter"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatVolScatterBrowserEntryInfo entry_info;
};

class CyclesMaterialVolumeAddClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Volume Add (Volume)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_VOL_ADD_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_vol_add"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatVolAddBrowserEntryInfo entry_info;
};

class CyclesMaterialVolumeMixClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Volume Mix (Volume)"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MAT_VOL_MIX_CLASS; }
	virtual const MCHAR* Category() { return L"CyclesMaterial"; }
	virtual const MCHAR* InternalName() { return L"mat_cycles_vol_mix"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	MatVolMixBrowserEntryInfo entry_info;
};

CyclesMaterialShaderClassDesc* GetCyclesMaterialShaderClassDesc();
CyclesMaterialShaderGraph08ClassDesc* GetCyclesMaterialShaderGraph08ClassDesc();
CyclesMaterialShaderGraph16ClassDesc* GetCyclesMaterialShaderGraph16ClassDesc();
CyclesMaterialShaderGraph32ClassDesc* GetCyclesMaterialShaderGraph32ClassDesc();

CyclesMaterialAddClassDesc* GetCyclesMaterialAddClassDesc();
CyclesMaterialAnisotropicClassDesc* GetCyclesMaterialAnisotropicClassDesc();
CyclesMaterialDiffuseClassDesc* GetCyclesMaterialDiffuseClassDesc();
CyclesMaterialEmissionClassDesc* GetCyclesMaterialEmissionClassDesc();
CyclesMaterialGlassClassDesc* GetCyclesMaterialGlassClassDesc();
CyclesMaterialGlossyClassDesc* GetCyclesMaterialGlossyClassDesc();
CyclesMaterialHairClassDesc* GetCyclesMaterialHairClassDesc();
CyclesMaterialHoldoutClassDesc* GetCyclesMaterialHoldoutClassDesc();
CyclesMaterialMixClassDesc* GetCyclesMaterialMixClassDesc();
CyclesMaterialPrincipledBsdfClassDesc* GetCyclesMaterialPrincipledBsdfClassDesc();
CyclesMaterialRefractionClassDesc* GetCyclesMaterialRefractionClassDesc();
CyclesMaterialSubsurfaceScatterClassDesc* GetCyclesMaterialSubsurfaceScatterClassDesc();
CyclesMaterialToonClassDesc* GetCyclesMaterialToonClassDesc();
CyclesMaterialTranslucentClassDesc* GetCyclesMaterialTranslucentClassDesc();
CyclesMaterialTransparentClassDesc* GetCyclesMaterialTransparentClassDesc();
CyclesMaterialVelvetClassDesc* GetCyclesMaterialVelvetClassDesc();

CyclesMaterialVolumeAbsorptionClassDesc* GetCyclesMaterialVolumeAbsorptionClassDesc();
CyclesMaterialVolumeScatterClassDesc* GetCyclesMaterialVolumeScatterClassDesc();
CyclesMaterialVolumeAddClassDesc* GetCyclesMaterialVolumeAddClassDesc();
CyclesMaterialVolumeMixClassDesc* GetCyclesMaterialVolumeMixClassDesc();
