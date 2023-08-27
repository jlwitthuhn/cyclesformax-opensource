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
 
#include "max_classdesc_mtl.h"

#include "plugin_mat_add.h"
#include "plugin_mat_anisotropic.h"
#include "plugin_mat_diffuse.h"
#include "plugin_mat_emission.h"
#include "plugin_mat_glass.h"
#include "plugin_mat_glossy.h"
#include "plugin_mat_hair.h"
#include "plugin_mat_holdout.h"
#include "plugin_mat_mix.h"
#include "plugin_mat_principled_bsdf.h"
#include "plugin_mat_refraction.h"
#include "plugin_mat_shader.h"
#include "plugin_mat_shader_graph_08.h"
#include "plugin_mat_shader_graph_16.h"
#include "plugin_mat_shader_graph_32.h"
#include "plugin_mat_sss.h"
#include "plugin_mat_toon.h"
#include "plugin_mat_translucent.h"
#include "plugin_mat_transparent.h"
#include "plugin_mat_vol_absorption.h"
#include "plugin_mat_vol_add.h"
#include "plugin_mat_vol_mix.h"
#include "plugin_mat_vol_scatter.h"
#include "plugin_mat_velvet.h"

void* CyclesMaterialShaderClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesShaderMat();
}

FPInterface* CyclesMaterialShaderClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialShaderGraph08ClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesShaderGraphMat08();
}

FPInterface* CyclesMaterialShaderGraph08ClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialShaderGraph16ClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesShaderGraphMat16();
}

FPInterface* CyclesMaterialShaderGraph16ClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialShaderGraph32ClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesShaderGraphMat32();
}

FPInterface* CyclesMaterialShaderGraph32ClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialAddClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesAddMat();
}

FPInterface* CyclesMaterialAddClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialAnisotropicClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesAnisotropicMat();
}

FPInterface* CyclesMaterialAnisotropicClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialDiffuseClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesDiffuseMat();
}

FPInterface* CyclesMaterialDiffuseClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialEmissionClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesEmissionMat();
}

FPInterface* CyclesMaterialEmissionClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialGlassClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesGlassMat();
}

FPInterface* CyclesMaterialGlassClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialGlossyClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesGlossyMat();
}

FPInterface* CyclesMaterialGlossyClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialHoldoutClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesHoldoutMat();
}

FPInterface* CyclesMaterialHoldoutClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialHairClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesHairMat();
}

FPInterface* CyclesMaterialHairClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialMixClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesMixMat();
}

FPInterface* CyclesMaterialMixClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialPrincipledBsdfClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesPrincipledBSDFMat();
}

FPInterface* CyclesMaterialPrincipledBsdfClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialRefractionClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesRefractionMat();
}

FPInterface* CyclesMaterialRefractionClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialSubsurfaceScatterClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesSubsurfaceScatterMat();
}

FPInterface* CyclesMaterialSubsurfaceScatterClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialToonClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesToonMat();
}

FPInterface* CyclesMaterialToonClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialTranslucentClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesTranslucentMat();
}

FPInterface* CyclesMaterialTranslucentClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialTransparentClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesTransparentMat();
}

FPInterface* CyclesMaterialTransparentClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialVelvetClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesVelvetMat();
}

FPInterface* CyclesMaterialVelvetClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialVolumeAbsorptionClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesVolAbsorptionMat();
}

FPInterface* CyclesMaterialVolumeAbsorptionClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialVolumeScatterClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesVolScatterMat();
}

FPInterface* CyclesMaterialVolumeScatterClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialVolumeAddClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesVolAddMat();
}

FPInterface* CyclesMaterialVolumeAddClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesMaterialVolumeMixClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesVolMixMat();
}

FPInterface* CyclesMaterialVolumeMixClassDesc::GetInterface(Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

CyclesMaterialShaderClassDesc* GetCyclesMaterialShaderClassDesc()
{
	static CyclesMaterialShaderClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialShaderGraph08ClassDesc* GetCyclesMaterialShaderGraph08ClassDesc()
{
	static CyclesMaterialShaderGraph08ClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialShaderGraph16ClassDesc* GetCyclesMaterialShaderGraph16ClassDesc()
{
	static CyclesMaterialShaderGraph16ClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialShaderGraph32ClassDesc* GetCyclesMaterialShaderGraph32ClassDesc()
{
	static CyclesMaterialShaderGraph32ClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialAddClassDesc* GetCyclesMaterialAddClassDesc()
{
	static CyclesMaterialAddClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialAnisotropicClassDesc* GetCyclesMaterialAnisotropicClassDesc()
{
	static CyclesMaterialAnisotropicClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialDiffuseClassDesc* GetCyclesMaterialDiffuseClassDesc()
{
	static CyclesMaterialDiffuseClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialEmissionClassDesc* GetCyclesMaterialEmissionClassDesc()
{
	static CyclesMaterialEmissionClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialGlassClassDesc* GetCyclesMaterialGlassClassDesc()
{
	static CyclesMaterialGlassClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialGlossyClassDesc* GetCyclesMaterialGlossyClassDesc()
{
	static CyclesMaterialGlossyClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialHairClassDesc* GetCyclesMaterialHairClassDesc()
{
	static CyclesMaterialHairClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialHoldoutClassDesc* GetCyclesMaterialHoldoutClassDesc()
{
	static CyclesMaterialHoldoutClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialMixClassDesc* GetCyclesMaterialMixClassDesc()
{
	static CyclesMaterialMixClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialPrincipledBsdfClassDesc* GetCyclesMaterialPrincipledBsdfClassDesc()
{
	static CyclesMaterialPrincipledBsdfClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialRefractionClassDesc* GetCyclesMaterialRefractionClassDesc()
{
	static CyclesMaterialRefractionClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialSubsurfaceScatterClassDesc* GetCyclesMaterialSubsurfaceScatterClassDesc()
{
	static CyclesMaterialSubsurfaceScatterClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialToonClassDesc* GetCyclesMaterialToonClassDesc()
{
	static CyclesMaterialToonClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialTranslucentClassDesc* GetCyclesMaterialTranslucentClassDesc()
{
	static CyclesMaterialTranslucentClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialTransparentClassDesc* GetCyclesMaterialTransparentClassDesc()
{
	static CyclesMaterialTransparentClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialVelvetClassDesc* GetCyclesMaterialVelvetClassDesc()
{
	static CyclesMaterialVelvetClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialVolumeAbsorptionClassDesc* GetCyclesMaterialVolumeAbsorptionClassDesc()
{
	static CyclesMaterialVolumeAbsorptionClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialVolumeScatterClassDesc* GetCyclesMaterialVolumeScatterClassDesc()
{
	static CyclesMaterialVolumeScatterClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialVolumeAddClassDesc* GetCyclesMaterialVolumeAddClassDesc()
{
	static CyclesMaterialVolumeAddClassDesc class_desc;
	return &class_desc;
}

CyclesMaterialVolumeMixClassDesc* GetCyclesMaterialVolumeMixClassDesc()
{
	static CyclesMaterialVolumeMixClassDesc class_desc;
	return &class_desc;
}
