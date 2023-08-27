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
 
#include "max_classdesc_cam.h"
#include "max_classdesc_gup.h"
#include "max_classdesc_mod.h"
#include "max_classdesc_mtl.h"
#include "max_classdesc_re.h"
#include "max_classdesc_tex.h"
#include "plugin_urender_cycles.h"
#include "rend_logger.h"

#include "ui_maxscript.h"

bool enable_plugin_debug{ false };

static bool already_checked_reg = false;

static void set_enable_plugin_debug()
{
	DWORD reg_dword{ 0 };
	DWORD size{ sizeof(DWORD) };
	std::wstring key_name{ L"Software\\CyclesForMax" };
	std::wstring value_name{ L"EnableDebugMode" };

	LSTATUS result{
		RegGetValue(
			HKEY_CURRENT_USER,
			key_name.c_str(),
			value_name.c_str(),
			RRF_RT_REG_DWORD,
			nullptr,
			&reg_dword,
			&size
		)
	};

	if (result == ERROR_SUCCESS) {
		if (reg_dword > 0) {
			enable_plugin_debug = true;
		}
	}

	global_log_manager.init();
	already_checked_reg = true;
}

enum {
	classdesc_camera_panorama,
	classdesc_texmap_bitmap_filter,
	classdesc_texmap_sky,
	classdesc_texmap_env,
	classdesc_mat_shader,
	classdesc_mat_shader_graph_08,
	classdesc_mat_shader_graph_16,
	classdesc_mat_shader_graph_32,
	classdesc_mat_add,
	classdesc_mat_anisotropic,
	classdesc_mat_diffuse,
	classdesc_mat_emission,
	classdesc_mat_glass,
	classdesc_mat_glossy,
	classdesc_mat_hair,
	classdesc_mat_holdout,
	classdesc_mat_mix,
	classdesc_mat_principled,
	classdesc_mat_refraction,
	classdesc_mat_subsurface_scatter,
	classdesc_mat_toon,
	classdesc_mat_translucent,
	classdesc_mat_transparent,
	classdesc_mat_velvet,
	classdesc_mat_vol_absorption,
	classdesc_mat_vol_scatter,
	classdesc_mat_vol_add,
	classdesc_mat_vol_mix,
	classdesc_mod_properties,
	classdesc_re_diffuse_color,
	classdesc_re_diffuse_direct,
	classdesc_re_diffuse_indirect,
	classdesc_re_glossy_color,
	classdesc_re_glossy_direct,
	classdesc_re_glossy_indirect,
	classdesc_re_transmission_color,
	classdesc_re_transmission_direct,
	classdesc_re_transmission_indirect,
	classdesc_re_subsurface_color,
	classdesc_re_subsurface_direct,
	classdesc_re_subsurface_indirect,
	classdesc_re_emission,
	classdesc_re_environment,
	classdesc_re_normal,
	classdesc_re_uv,
	classdesc_re_motion,
	classdesc_re_depth,
	classdesc_re_mist,
	classdesc_re_crypto_object,
	classdesc_re_crypto_material,
	classdesc_re_crypto_asset,
	classdesc_maxscript_gup,
	classdesc_urender_cycles,
	classdesc_count
};

HINSTANCE hInstance;

BOOL WINAPI DllMain(const HINSTANCE hinstDLL, const ULONG fdwReason, const LPVOID /*lpvReserved*/)
{
	if( fdwReason == DLL_PROCESS_ATTACH ) {
		MaxSDK::Util::UseLanguagePackLocale();
		hInstance = hinstDLL;
		DisableThreadLibraryCalls(hInstance);
	}

	if (already_checked_reg == false)
	{
		set_enable_plugin_debug();
	}

	return TRUE;
}

__declspec(dllexport) const TCHAR* LibDescription()
{
	return _T("Cycles for Max");
}

__declspec(dllexport) int LibNumberClasses()
{
	return classdesc_count;
}

__declspec(dllexport) ClassDesc* LibClassDesc(const int i)
{
	switch(i) {
		case classdesc_camera_panorama:
			return GetCyclesCameraPanoramaClassDesc();
		case classdesc_texmap_bitmap_filter:
			return GetCyclesTexmapBitmapFilterClassDesc();
		case classdesc_texmap_sky:
			return GetCyclesTexmapSkyClassDesc();
		case classdesc_texmap_env:
			return GetCyclesTexmapEnvironmentClassDesc();
		case classdesc_mat_shader:
			return GetCyclesMaterialShaderClassDesc();
		case classdesc_mat_shader_graph_08:
			return GetCyclesMaterialShaderGraph08ClassDesc();
		case classdesc_mat_shader_graph_16:
			return GetCyclesMaterialShaderGraph16ClassDesc();
		case classdesc_mat_shader_graph_32:
			return GetCyclesMaterialShaderGraph32ClassDesc();
		case classdesc_mat_add:
			return GetCyclesMaterialAddClassDesc();
		case classdesc_mat_anisotropic:
			return GetCyclesMaterialAnisotropicClassDesc();
		case classdesc_mat_diffuse:
			return GetCyclesMaterialDiffuseClassDesc();
		case classdesc_mat_emission:
			return GetCyclesMaterialEmissionClassDesc();
		case classdesc_mat_glass:
			return GetCyclesMaterialGlassClassDesc();
		case classdesc_mat_glossy:
			return GetCyclesMaterialGlossyClassDesc();
		case classdesc_mat_hair:
			return GetCyclesMaterialHairClassDesc();
		case classdesc_mat_holdout:
			return GetCyclesMaterialHoldoutClassDesc();
		case classdesc_mat_mix:
			return GetCyclesMaterialMixClassDesc();
		case classdesc_mat_principled:
			return GetCyclesMaterialPrincipledBsdfClassDesc();
		case classdesc_mat_refraction:
			return GetCyclesMaterialRefractionClassDesc();
		case classdesc_mat_subsurface_scatter:
			return GetCyclesMaterialSubsurfaceScatterClassDesc();
		case classdesc_mat_toon:
			return GetCyclesMaterialToonClassDesc();
		case classdesc_mat_translucent:
			return GetCyclesMaterialTranslucentClassDesc();
		case classdesc_mat_transparent:
			return GetCyclesMaterialTransparentClassDesc();
		case classdesc_mat_velvet:
			return GetCyclesMaterialVelvetClassDesc();
		case classdesc_mat_vol_absorption:
			return GetCyclesMaterialVolumeAbsorptionClassDesc();
		case classdesc_mat_vol_scatter:
			return GetCyclesMaterialVolumeScatterClassDesc();
		case classdesc_mat_vol_add:
			return GetCyclesMaterialVolumeAddClassDesc();
		case classdesc_mat_vol_mix:
			return GetCyclesMaterialVolumeMixClassDesc();
		case classdesc_mod_properties:
			return GetCyclesPropertiesModifierClassDesc();
		case classdesc_re_diffuse_color:
			return GetCyclesRenderElementDiffuseColorClassDesc();
		case classdesc_re_diffuse_direct:
			return GetCyclesRenderElementDiffuseDirectClassDesc();
		case classdesc_re_diffuse_indirect:
			return GetCyclesRenderElementDiffuseIndirectClassDesc();
		case classdesc_re_glossy_color:
			return GetCyclesRenderElementGlossyColorClassDesc();
		case classdesc_re_glossy_direct:
			return GetCyclesRenderElementGlossyDirectClassDesc();
		case classdesc_re_glossy_indirect:
			return GetCyclesRenderElementGlossyIndirectClassDesc();
		case classdesc_re_transmission_color:
			return GetCyclesRenderElementTransmissionColorClassDesc();
		case classdesc_re_transmission_direct:
			return GetCyclesRenderElementTransmissionDirectClassDesc();
		case classdesc_re_transmission_indirect:
			return GetCyclesRenderElementTransmissionIndirectClassDesc();
		case classdesc_re_subsurface_color:
			return GetCyclesRenderElementSubsurfaceColorClassDesc();
		case classdesc_re_subsurface_direct:
			return GetCyclesRenderElementSubsurfaceDirectClassDesc();
		case classdesc_re_subsurface_indirect:
			return GetCyclesRenderElementSubsurfaceIndirectClassDesc();
		case classdesc_re_emission:
			return GetCyclesRenderElementEmissionClassDesc();
		case classdesc_re_environment:
			return GetCyclesRenderElementEnvironmentClassDesc();
		case classdesc_re_normal:
			return GetCyclesRenderElementNormalClassDesc();
		case classdesc_re_uv:
			return GetCyclesRenderElementUVClassDesc();
		case classdesc_re_motion:
			return GetCyclesRenderElementMotionClassDesc();
		case classdesc_re_depth:
			return GetCyclesRenderElementDepthClassDesc();
		case classdesc_re_mist:
			return GetCyclesRenderElementMistClassDesc();
		case classdesc_re_crypto_object:
			return GetCyclesRenderElementCryptoObjectClassDesc();
		case classdesc_re_crypto_material:
			return GetCyclesRenderElementCryptoMaterialClassDesc();
		case classdesc_re_crypto_asset:
			return GetCyclesRenderElementCryptoAssetClassDesc();
		case classdesc_maxscript_gup:
			return GetCyclesMaxscriptGUPClassDesc();
		case classdesc_urender_cycles:
			return &(GetCyclesUnifiedRendererDescriptor());
		default:
			return nullptr;
	}
}

__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}
