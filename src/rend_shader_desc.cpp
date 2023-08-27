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
 
#include "rend_shader_desc.h"

#include <map>

#include <iparamb2.h>

#include "plugin_mat_add.h"
#include "plugin_mat_anisotropic.h"
#include "plugin_mat_diffuse.h"
#include "plugin_mat_emission.h"
#include "plugin_mat_glass.h"
#include "plugin_mat_glossy.h"
#include "plugin_mat_hair.h"
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
#include "plugin_mat_velvet.h"
#include "plugin_mat_vol_absorption.h"
#include "plugin_mat_vol_add.h"
#include "plugin_mat_vol_mix.h"
#include "plugin_mat_vol_scatter.h"
#include "const_classid.h"

#define LT_COMPARE(value) if (value < other.value) return true; else if (other.value < value) return false;

static bool operator<(const ccl::float3& a, const ccl::float3& b)
{
	if (a.x < b.x) return true;
	else if (a.x > b.x) return false;

	if (a.y < b.y) return true;
	else if (a.y > b.y) return false;

	if (a.z < b.z) return true;
	else if (a.z > b.z) return false;

	return false;
}

static ccl::float3 float3_from_acolor(AColor& acolor)
{
	return ccl::make_float3(acolor.r, acolor.g, acolor.b);
}

static std::string bad_from_wstring(const std::wstring input)
{
	std::stringstream result_stream;

	for (const wchar_t this_char : input) {
		if (this_char < 256) {
			result_stream << static_cast<char>(this_char);
		}
		else {
			result_stream << '?';
		}
	}

	return result_stream.str();
}

AdskPhysicalMaterialDescriptor::AdskPhysicalMaterialDescriptor(Mtl* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	if (mtl->ClassID() != PHYSICAL_MATERIAL_CLASS) {
		return;
	}

	static const std::wstring BASE_WEIGHT_NAME(L"base_weight");
	static const std::wstring BASE_WEIGHT_MAP_NAME(L"base_weight_map");
	static const std::wstring BASE_WEIGHT_MAP_ON_NAME(L"base_weight_map_on");
	static const std::wstring BASE_COLOR_NAME(L"base_color");
	static const std::wstring BASE_COLOR_MAP_NAME(L"base_color_map");
	static const std::wstring BASE_COLOR_MAP_ON_NAME(L"base_color_map_on");
	static const std::wstring BASE_ROUGHNESS_NAME(L"diff_roughness");
	static const std::wstring BASE_ROUGHNESS_MAP_NAME(L"diff_rough_map");
	static const std::wstring BASE_ROUGHNESS_MAP_ON_NAME(L"diff_rough_map_on");

	static const std::wstring REFLECTION_WEIGHT_NAME(L"reflectivity");
	static const std::wstring REFLECTION_WEIGHT_MAP_NAME(L"reflectivity_map");
	static const std::wstring REFLECTION_WEIGHT_MAP_ON_NAME(L"reflectivity_map_on");
	static const std::wstring REFLECTION_COLOR_NAME(L"refl_color");
	static const std::wstring REFLECTION_COLOR_MAP_NAME(L"refl_color_map");
	static const std::wstring REFLECTION_COLOR_MAP_ON_NAME(L"refl_color_map_on");
	static const std::wstring REFLECTION_IOR_NAME(L"trans_ior");
	static const std::wstring REFLECTION_IOR_MAP_NAME(L"trans_ior_map");
	static const std::wstring REFLECTION_IOR_MAP_ON_NAME(L"trans_ior_map_on");
	static const std::wstring REFLECTION_ROUGHNESS_INV_NAME(L"roughness_inv");
	static const std::wstring REFLECTION_ROUGHNESS_NAME(L"roughness");
	static const std::wstring REFLECTION_ROUGHNESS_MAP_NAME(L"roughness_map");
	static const std::wstring REFLECTION_ROUGHNESS_MAP_ON_NAME(L"roughness_map_on");
	static const std::wstring REFLECTION_METALNESS_NAME(L"metalness");
	static const std::wstring REFLECTION_METALNESS_MAP_NAME(L"metalness_map");
	static const std::wstring REFLECTION_METALNESS_MAP_ON_NAME(L"metalness_map_on");

	static const std::wstring TRANS_WEIGHT_NAME(L"transparency");
	static const std::wstring TRANS_WEIGHT_MAP_NAME(L"transparency_map");
	static const std::wstring TRANS_WEIGHT_MAP_ON_NAME(L"transparency_map_on");
	static const std::wstring TRANS_COLOR_NAME(L"trans_color");
	static const std::wstring TRANS_COLOR_MAP_NAME(L"trans_color_map");
	static const std::wstring TRANS_COLOR_MAP_ON_NAME(L"trans_color_map_on");
	static const std::wstring TRANS_ROUGHNESS_INV_NAME(L"trans_roughness_inv");
	static const std::wstring TRANS_ROUGHNESS_NAME(L"trans_roughness");
	static const std::wstring TRANS_ROUGHNESS_MAP_NAME(L"trans_rough_map");
	static const std::wstring TRANS_ROUGHNESS_MAP_ON_NAME(L"trans_rough_map_on");

	static const std::wstring CC_WEIGHT_NAME(L"coating");
	static const std::wstring CC_WEIGHT_MAP_NAME(L"coat_map");
	static const std::wstring CC_WEIGHT_MAP_ON_NAME(L"coat_map_on");
	static const std::wstring CC_COLOR_NAME(L"coat_color");
	static const std::wstring CC_COLOR_MAP_NAME(L"coat_color_map");
	static const std::wstring CC_COLOR_MAP_ON_NAME(L"coat_color_map_on");
	static const std::wstring CC_IOR_NAME(L"coat_ior");
	static const std::wstring CC_ROUGHNESS_INV_NAME(L"coat_roughness_inv");
	static const std::wstring CC_ROUGHNESS_NAME(L"coat_roughness");
	static const std::wstring CC_ROUGHNESS_MAP_NAME(L"coat_rough_map");
	static const std::wstring CC_ROUGHNESS_MAP_ON_NAME(L"coat_rough_map_on");

	static const std::wstring EMISSION_WEIGHT_NAME(L"emission");
	static const std::wstring EMISSION_WEIGHT_MAP_NAME(L"emission_map");
	static const std::wstring EMISSION_WEIGHT_MAP_ON_NAME(L"emission_map_on");
	static const std::wstring EMISSION_COLOR_NAME(L"emit_color");
	static const std::wstring EMISSION_COLOR_MAP_NAME(L"emit_color_map");
	static const std::wstring EMISSION_COLOR_MAP_ON_NAME(L"emit_color_map_on");
	static const std::wstring EMISSION_LUMINANCE_NAME(L"emit_luminance");
	static const std::wstring EMISSION_TEMP_KELVIN_NAME(L"emit_kelvin");

	static const std::wstring BASE_BUMP_STRENGTH_NAME(L"bump_map_amt");
	static const std::wstring BASE_BUMP_MAP_NAME(L"bump_map");
	static const std::wstring BASE_BUMP_MAP_ON_NAME(L"bump_map_on");
	static const std::wstring CC_BUMP_STRENGTH_NAME(L"clearcoat_bump_map_amt");
	static const std::wstring CC_BUMP_MAP_NAME(L"coat_bump_map");
	static const std::wstring CC_BUMP_MAP_ON_NAME(L"coat_bump_map_on");

	static const std::wstring CUTOUT_MAP_NAME(L"cutout_map");
	static const std::wstring CUTOUT_MAP_ON_NAME(L"cutout_map_on");

	static const std::wstring LOCK_TRANS_ROUGHNESS_NAME(L"trans_roughness_lock");

	IParamBlock2* const pblock = mtl->GetParamBlock(0);
	ParamBlockDesc2* const pbdesc = pblock->GetDesc();

	std::map<std::wstring, ParamID> name_to_paramid;
	for (int i = 0; i < pbdesc->Count(); i++) {
		const ParamDef* def = pbdesc->GetParamDefByIndex(i);
		name_to_paramid[std::wstring(def->int_name)] = def->ID;
	}

	// Base block
	{
		{
			const ParamID base_weight_id = name_to_paramid[BASE_WEIGHT_NAME];
			const ParamID base_weight_map_id = name_to_paramid[BASE_WEIGHT_MAP_NAME];
			const ParamID base_weight_map_on_id = name_to_paramid[BASE_WEIGHT_MAP_ON_NAME];

			Texmap* const base_weight_texmap = pblock->GetTexmap(base_weight_map_id, t);
			const int base_weight_map_on = pblock->GetInt(base_weight_map_on_id, t);

			if (base_weight_map_on != 0 && base_weight_texmap != nullptr) {
				base_weight_map = base_weight_texmap;
			}
			else {
				base_weight = pblock->GetFloat(base_weight_id, t);
			}
		}

		{
			const ParamID base_color_id = name_to_paramid[BASE_COLOR_NAME];
			const ParamID base_color_map_id = name_to_paramid[BASE_COLOR_MAP_NAME];
			const ParamID base_color_map_on_id = name_to_paramid[BASE_COLOR_MAP_ON_NAME];

			Texmap* const base_color_texmap = pblock->GetTexmap(base_color_map_id, t);
			const int base_color_map_on = pblock->GetInt(base_color_map_on_id, t);

			if (base_color_map_on != 0 && base_color_texmap != nullptr) {
				base_color_map = base_color_texmap;
			}
			else {
				AColor acolor_base_color = pblock->GetAColor(base_color_id, t);
				base_color = float3_from_acolor(acolor_base_color);
			}
		}

		{
			const ParamID base_roughness_id = name_to_paramid[BASE_ROUGHNESS_NAME];
			const ParamID base_roughness_map_id = name_to_paramid[BASE_ROUGHNESS_MAP_NAME];
			const ParamID base_roughness_map_on_id = name_to_paramid[BASE_ROUGHNESS_MAP_ON_NAME];

			Texmap* const base_roughness_texmap = pblock->GetTexmap(base_roughness_map_id, t);
			const int base_roughness_map_on = pblock->GetInt(base_roughness_map_on_id, t);

			if (base_roughness_map_on != 0 && base_roughness_texmap != nullptr) {
				base_roughness_map = base_roughness_texmap;
			}
			else {
				base_roughness = pblock->GetFloat(base_roughness_id, t);
			}
		}
	}

	// Reflection block
	{
		{
			const ParamID refl_roughness_inv_id = name_to_paramid[REFLECTION_ROUGHNESS_INV_NAME];
			refl_roughness_invert = (pblock->GetInt(refl_roughness_inv_id, t) != 0);
		}

		{
			const ParamID refl_weight_id = name_to_paramid[REFLECTION_WEIGHT_NAME];
			const ParamID refl_weight_map_id = name_to_paramid[REFLECTION_WEIGHT_MAP_NAME];
			const ParamID refl_weight_map_on_id = name_to_paramid[REFLECTION_WEIGHT_MAP_ON_NAME];
		
			Texmap* const refl_weight_texmap = pblock->GetTexmap(refl_weight_map_id, t);
			const int refl_weight_map_on = pblock->GetInt(refl_weight_map_on_id, t);

			if (refl_weight_map_on != 0 && refl_weight_texmap != nullptr) {
				refl_weight_map = refl_weight_texmap;
			}
			else {
				refl_weight = pblock->GetFloat(refl_weight_id, t);
			}
		}

		{
			const ParamID refl_color_id = name_to_paramid[REFLECTION_COLOR_NAME];
			const ParamID refl_color_map_id = name_to_paramid[REFLECTION_COLOR_MAP_NAME];
			const ParamID refl_color_map_on_id = name_to_paramid[REFLECTION_COLOR_MAP_ON_NAME];

			Texmap* const refl_color_texmap = pblock->GetTexmap(refl_color_map_id, t);
			const int refl_color_map_on = pblock->GetInt(refl_color_map_on_id, t);

			if (refl_color_map_on != 0 && refl_color_texmap != nullptr) {
				refl_color_map = refl_color_texmap;
			}
			else {
				AColor acolor_refl_color = pblock->GetAColor(refl_color_id, t);
				refl_color = float3_from_acolor(acolor_refl_color);
			}
		}

		{
			const ParamID refl_ior_id = name_to_paramid[REFLECTION_IOR_NAME];
			const ParamID refl_ior_map_id = name_to_paramid[REFLECTION_IOR_MAP_NAME];
			const ParamID refl_ior_map_on_id = name_to_paramid[REFLECTION_IOR_MAP_ON_NAME];

			Texmap* const refl_ior_texmap = pblock->GetTexmap(refl_ior_map_id, t);
			const int refl_ior_map_on = pblock->GetInt(refl_ior_map_on_id, t);

			if (refl_ior_map_on != 0 && refl_ior_texmap != nullptr) {
				refl_ior_map = refl_ior_texmap;
			}
			else {
				refl_ior = pblock->GetFloat(refl_ior_id, t);
			}
		}

		{
			const ParamID refl_roughness_id = name_to_paramid[REFLECTION_ROUGHNESS_NAME];
			const ParamID refl_roughness_map_id = name_to_paramid[REFLECTION_ROUGHNESS_MAP_NAME];
			const ParamID refl_roughness_map_on_id = name_to_paramid[REFLECTION_ROUGHNESS_MAP_ON_NAME];

			Texmap* const refl_roughness_texmap = pblock->GetTexmap(refl_roughness_map_id, t);
			const int refl_roughness_map_on = pblock->GetInt(refl_roughness_map_on_id, t);

			if (refl_roughness_map_on != 0 && refl_roughness_texmap != nullptr) {
				refl_roughness_map = refl_roughness_texmap;
			}
			else {
				refl_roughness = pblock->GetFloat(refl_roughness_id, t);
			}
		}

		{
			const ParamID refl_metalness_id = name_to_paramid[REFLECTION_METALNESS_NAME];
			const ParamID refl_metalness_map_id = name_to_paramid[REFLECTION_METALNESS_MAP_NAME];
			const ParamID refl_metalness_map_on_id = name_to_paramid[REFLECTION_METALNESS_MAP_ON_NAME];

			Texmap* const refl_metalness_texmap = pblock->GetTexmap(refl_metalness_map_id, t);
			const int refl_metalness_map_on = pblock->GetInt(refl_metalness_map_on_id, t);

			if (refl_metalness_map_on != 0 && refl_metalness_texmap != nullptr) {
				refl_metalness_map = refl_metalness_texmap;
			}
			else {
				refl_metalness = pblock->GetFloat(refl_metalness_id, t);
			}
		}
	}

	// Transparency block
	{
		{
			const ParamID trans_roughness_inv_id = name_to_paramid[TRANS_ROUGHNESS_INV_NAME];
			trans_roughness_invert = (pblock->GetInt(trans_roughness_inv_id, t) != 0);
		}

		{
			const ParamID trans_weight_id = name_to_paramid[TRANS_WEIGHT_NAME];
			const ParamID trans_weight_map_id = name_to_paramid[TRANS_WEIGHT_MAP_NAME];
			const ParamID trans_weight_map_on_id = name_to_paramid[TRANS_WEIGHT_MAP_ON_NAME];

			Texmap* const trans_weight_texmap = pblock->GetTexmap(trans_weight_map_id, t);
			const int trans_weight_map_on = pblock->GetInt(trans_weight_map_on_id, t);

			if (trans_weight_map_on != 0 && trans_weight_texmap != nullptr) {
				trans_weight_map = trans_weight_texmap;
			}
			else {
				trans_weight = pblock->GetFloat(trans_weight_id, t);
			}
		}

		{
			const ParamID trans_color_id = name_to_paramid[TRANS_COLOR_NAME];
			const ParamID trans_color_map_id = name_to_paramid[TRANS_COLOR_MAP_NAME];
			const ParamID trans_color_map_on_id = name_to_paramid[TRANS_COLOR_MAP_ON_NAME];

			Texmap* const trans_color_texmap = pblock->GetTexmap(trans_color_map_id, t);
			const int trans_color_map_on = pblock->GetInt(trans_color_map_on_id, t);

			if (trans_color_map_on != 0 && trans_color_texmap != nullptr) {
				trans_color_map = trans_color_texmap;
			}
			else {
				AColor acolor_trans_color = pblock->GetAColor(trans_color_id, t);
				trans_color = float3_from_acolor(acolor_trans_color);
			}
		}

		{
			const ParamID trans_roughness_id = name_to_paramid[TRANS_ROUGHNESS_NAME];
			const ParamID trans_roughness_map_id = name_to_paramid[TRANS_ROUGHNESS_MAP_NAME];
			const ParamID trans_roughness_map_on_id = name_to_paramid[TRANS_ROUGHNESS_MAP_ON_NAME];

			Texmap* const trans_roughness_texmap = pblock->GetTexmap(trans_roughness_map_id, t);
			const int trans_roughness_map_on = pblock->GetInt(trans_roughness_map_on_id, t);

			if (trans_roughness_map_on != 0 && trans_roughness_texmap != nullptr) {
				trans_roughness_map = trans_roughness_texmap;
			}
			else {
				trans_roughness = pblock->GetFloat(trans_roughness_id, t);
			}
		}
	}

	// Clearcoat block
	{
		{
			const ParamID cc_roughness_inv_id = name_to_paramid[CC_ROUGHNESS_INV_NAME];
			cc_roughness_invert = (pblock->GetInt(cc_roughness_inv_id, t) != 0);
		}

		{
			const ParamID cc_weight_map_id = name_to_paramid[CC_WEIGHT_MAP_NAME];
			const ParamID cc_weight_map_on_id = name_to_paramid[CC_WEIGHT_MAP_ON_NAME];
			const ParamID cc_weight_id = name_to_paramid[CC_WEIGHT_NAME];

			Texmap* const cc_weight_texmap = pblock->GetTexmap(cc_weight_map_id, t);
			int cc_weight_map_on = pblock->GetInt(cc_weight_map_on_id, t);

			if (cc_weight_map_on && cc_weight_texmap != nullptr) {
				cc_weight_map = cc_weight_texmap;
			}
			else {
				cc_weight = pblock->GetFloat(cc_weight_id, t);
			}
		}
	
		{
			const ParamID cc_color_map_id = name_to_paramid[CC_COLOR_MAP_NAME];
			const ParamID cc_color_map_on_id = name_to_paramid[CC_COLOR_MAP_ON_NAME];
			const ParamID cc_color_id = name_to_paramid[CC_COLOR_NAME];

			Texmap* const cc_color_texmap = pblock->GetTexmap(cc_color_map_id, t);
			const int cc_color_map_on = pblock->GetInt(cc_color_map_on_id, t);

			if (cc_color_map_on != 0 && cc_color_texmap != nullptr) {
				cc_color_map = cc_color_texmap;
			}
			else {
				AColor acolor_cc_color = pblock->GetAColor(cc_color_id, t);
				cc_color = float3_from_acolor(acolor_cc_color);
			}
		}

		const ParamID cc_ior_id = name_to_paramid[CC_IOR_NAME];
		cc_ior = pblock->GetFloat(cc_ior_id, t);

		{
			const ParamID cc_roughness_map_id = name_to_paramid[CC_ROUGHNESS_MAP_NAME];
			const ParamID cc_roughness_map_on_id = name_to_paramid[CC_ROUGHNESS_MAP_ON_NAME];
			const ParamID cc_roughness_id = name_to_paramid[CC_ROUGHNESS_NAME];

			Texmap* const cc_roughness_texmap = pblock->GetTexmap(cc_roughness_map_id, t);

			int cc_roughness_map_on = pblock->GetInt(cc_roughness_map_on_id, t);

			if (cc_roughness_map_on && cc_roughness_texmap != nullptr) {
				cc_roughness_map = cc_roughness_texmap;
			}
			else {
				cc_roughness = pblock->GetFloat(cc_roughness_id, t);
			}
		}
	}

	// Emission block
	{
		{
			const ParamID emission_weight_map_id = name_to_paramid[EMISSION_WEIGHT_MAP_NAME];
			const ParamID emission_weight_map_on_id = name_to_paramid[EMISSION_WEIGHT_MAP_ON_NAME];
			const ParamID emission_weight_id = name_to_paramid[EMISSION_WEIGHT_NAME];

			Texmap* const emission_weight_texmap = pblock->GetTexmap(emission_weight_map_id, t);

			int emission_weight_map_on = pblock->GetInt(emission_weight_map_on_id, t);

			if (emission_weight_map_on && emission_weight_texmap != nullptr) {
				emission_weight_map = emission_weight_texmap;
			}
			else {
				emission_weight = pblock->GetFloat(emission_weight_id, t);
			}
		}

		{
			const ParamID emission_color_map_id = name_to_paramid[EMISSION_COLOR_MAP_NAME];
			const ParamID emission_color_map_on_id = name_to_paramid[EMISSION_COLOR_MAP_ON_NAME];
			const ParamID emission_color_id = name_to_paramid[EMISSION_COLOR_NAME];

			Texmap* const emission_color_texmap = pblock->GetTexmap(emission_color_map_id, t);

			int emission_color_map_on = pblock->GetInt(emission_color_map_on_id, t);

			if (emission_color_map_on && emission_color_texmap != nullptr) {
				emission_color_map = emission_color_texmap;
			}
			else {
				AColor acolor_emission_color = pblock->GetAColor(emission_color_id, t);
				emission_color = float3_from_acolor(acolor_emission_color);
			}
		}


		const ParamID emission_luminance_id = name_to_paramid[EMISSION_LUMINANCE_NAME];
		emission_luminance = pblock->GetFloat(emission_luminance_id, t);

		const ParamID emission_temp_id = name_to_paramid[EMISSION_TEMP_KELVIN_NAME];
		emission_temp = pblock->GetFloat(emission_temp_id, t);
	}

	// Base bump map
	{
		{
			const ParamID base_bump_strength_id = name_to_paramid[BASE_BUMP_STRENGTH_NAME];
			base_bump_strength = pblock->GetFloat(base_bump_strength_id, t);
		}
		
		{
			const ParamID base_bump_map_id = name_to_paramid[BASE_BUMP_MAP_NAME];
			const ParamID base_bump_map_on_id = name_to_paramid[BASE_BUMP_MAP_ON_NAME];

			Texmap* const base_bump_texmap = pblock->GetTexmap(base_bump_map_id, t);

			int base_bump_map_on = pblock->GetInt(base_bump_map_on_id, t);

			if (base_bump_map_on && base_bump_texmap != nullptr) {
				base_bump_map = base_bump_texmap;
			}
		}
	}

	// Clearcoat bump map
	{
		{
			const ParamID cc_bump_strength_id = name_to_paramid[CC_BUMP_STRENGTH_NAME];
			cc_bump_strength = pblock->GetFloat(cc_bump_strength_id, t);
		}

		{
			const ParamID cc_bump_map_id = name_to_paramid[CC_BUMP_MAP_NAME];
			const ParamID cc_bump_map_on_id = name_to_paramid[CC_BUMP_MAP_ON_NAME];

			Texmap* const cc_bump_texmap = pblock->GetTexmap(cc_bump_map_id, t);

			int cc_bump_map_on = pblock->GetInt(cc_bump_map_on_id, t);

			if (cc_bump_map_on && cc_bump_texmap != nullptr) {
				cc_bump_map = cc_bump_texmap;
			}
		}
	}

	// Cutout map
	{
		const ParamID cutout_map_id = name_to_paramid[CUTOUT_MAP_NAME];
		const ParamID cutout_map_on_id = name_to_paramid[CUTOUT_MAP_ON_NAME];

		Texmap* const cutout_texmap = pblock->GetTexmap(cutout_map_id, t);

		const int cutout_map_on = pblock->GetInt(cutout_map_on_id, t);

		if (cutout_map_on && cutout_texmap != nullptr) {
			cutout_map = cutout_texmap;
		}
	}

	// Final correction based on locks
	{
		const ParamID lock_trans_roughness_id = name_to_paramid[LOCK_TRANS_ROUGHNESS_NAME];

		int lock_trans_roughness_on = pblock->GetInt(lock_trans_roughness_id, t);

		if (lock_trans_roughness_on) {
			trans_roughness_invert = refl_roughness_invert;
			trans_roughness = refl_roughness;
			trans_roughness_map = refl_roughness_map;
		}
	}
}

bool AdskPhysicalMaterialDescriptor::operator<(const AdskPhysicalMaterialDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(base_weight);
	LT_COMPARE(base_weight_map);
	LT_COMPARE(base_color);
	LT_COMPARE(base_color_map);
	LT_COMPARE(base_roughness);
	LT_COMPARE(base_roughness_map);

	LT_COMPARE(refl_weight);
	LT_COMPARE(refl_weight_map);
	LT_COMPARE(refl_color);
	LT_COMPARE(refl_color_map);
	LT_COMPARE(refl_ior);
	LT_COMPARE(refl_ior_map);
	LT_COMPARE(refl_roughness_invert);
	LT_COMPARE(refl_roughness);
	LT_COMPARE(refl_roughness_map);
	LT_COMPARE(refl_metalness);
	LT_COMPARE(refl_metalness_map);

	LT_COMPARE(trans_weight);
	LT_COMPARE(trans_weight_map);
	LT_COMPARE(trans_color);
	LT_COMPARE(trans_color_map);
	LT_COMPARE(trans_roughness_invert);
	LT_COMPARE(trans_roughness);
	LT_COMPARE(trans_roughness_map);

	LT_COMPARE(cc_weight);
	LT_COMPARE(cc_weight_map);
	LT_COMPARE(cc_color);
	LT_COMPARE(cc_color_map);
	LT_COMPARE(cc_ior);
	LT_COMPARE(cc_roughness_invert);
	LT_COMPARE(cc_roughness);
	LT_COMPARE(cc_roughness_map);

	LT_COMPARE(emission_weight);
	LT_COMPARE(emission_weight_map);
	LT_COMPARE(emission_color);
	LT_COMPARE(emission_color_map);
	LT_COMPARE(emission_luminance);
	LT_COMPARE(emission_temp);

	LT_COMPARE(base_bump_strength);
	LT_COMPARE(base_bump_map);
	LT_COMPARE(cc_bump_strength);
	LT_COMPARE(cc_bump_map);

	LT_COMPARE(cutout_map);

	return false;
}

bool NormalMapDescriptor::operator<(const NormalMapDescriptor& other) const
{
	LT_COMPARE(enabled);
	LT_COMPARE(space);
	LT_COMPARE(strength);
	LT_COMPARE(strength_map);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(invert_green);

	return false;
}

bool ShaderParamsDescriptor::operator<(const ShaderParamsDescriptor& other) const
{
	LT_COMPARE(use_mis);
	LT_COMPARE(displacement);
	LT_COMPARE(vol_sampling);
	LT_COMPARE(vol_interp);
	LT_COMPARE(vol_is_homogeneous);

	return false;
}

ShaderGraphShaderDescriptor::ShaderGraphShaderDescriptor(CyclesShaderGraphMat08* const mtl, const TimeValue /*t*/) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	encoded_graph = mtl->GetNodeGraph();
	mtl->PopulateShaderParamsDesc(&shader_params);

	texmaps.resize(mtl->NumSubTexmaps(), nullptr);
	for (size_t i = 1; i <= texmaps.size(); ++i) {
		// Array is zero-indexed and texmaps are 1-indexed
		texmaps[i - 1] = mtl->GetNumberedTexmap(i);
	}
}

ShaderGraphShaderDescriptor::ShaderGraphShaderDescriptor(CyclesShaderGraphMat16* const mtl, const TimeValue /*t*/) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	encoded_graph = mtl->GetNodeGraph();
	mtl->PopulateShaderParamsDesc(&shader_params);

	texmaps.resize(mtl->NumSubTexmaps(), nullptr);
	for (size_t i = 1; i <= texmaps.size(); ++i) {
		// Array is zero-indexed and texmaps are 1-indexed
		texmaps[i - 1] = mtl->GetNumberedTexmap(i);
	}
}

ShaderGraphShaderDescriptor::ShaderGraphShaderDescriptor(CyclesShaderGraphMat32* const mtl, const TimeValue /*t*/) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	encoded_graph = mtl->GetNodeGraph();
	mtl->PopulateShaderParamsDesc(&shader_params);

	texmaps.resize(mtl->NumSubTexmaps(), nullptr);
	for (size_t i = 1; i <= texmaps.size(); ++i) {
		// Array is zero-indexed and texmaps are 1-indexed
		texmaps[i - 1] = mtl->GetNumberedTexmap(i);
	}
}

bool ShaderGraphShaderDescriptor::operator<(const ShaderGraphShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(encoded_graph);
	LT_COMPARE(shader_params);

	LT_COMPARE(texmaps.size());

	for (size_t i = 0; i < texmaps.size(); ++i) {
		LT_COMPARE(texmaps[i]);
	}

	return false;
}

ShaderShaderDescriptor::ShaderShaderDescriptor(CyclesShaderMat* const mtl, const TimeValue) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateShaderParamsDesc(&shader_params);
	surface_mtl = mtl->GetSurfaceMtl();
	volume_mtl = mtl->GetVolumeMtl();
}

bool ShaderShaderDescriptor::operator<(const ShaderShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(shader_params);
	LT_COMPARE(surface_mtl);
	LT_COMPARE(volume_mtl);

	return false;
}

AddShaderDescriptor::AddShaderDescriptor(CyclesAddMat* const mtl, const TimeValue) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl_a = mtl->GetMtlA();
	mtl_b = mtl->GetMtlB();
}

AddShaderDescriptor::AddShaderDescriptor(CyclesVolAddMat* const mtl, const TimeValue) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl_a = mtl->GetMtlA();
	mtl_b = mtl->GetMtlB();
}

bool AddShaderDescriptor::operator<(const AddShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(mtl_a);
	LT_COMPARE(mtl_b);

	return false;
}

AnisotropicShaderDescriptor::AnisotropicShaderDescriptor(CyclesAnisotropicMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	Interval dist_valid = FOREVER;
	distribution = mtl->GetDistribution(t, dist_valid);

	if (mtl->GetBsdfColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetBsdfColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		Color mat_color = mtl->GetBsdfColor(t, color_valid);
		color = ccl::make_float3(mat_color.r, mat_color.g, mat_color.b);
		color_map = nullptr;
	}

	if (mtl->GetRoughnessTexmap() != nullptr) {
		roughness = 0.0f;
		roughness_map = mtl->GetRoughnessTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		roughness = mtl->GetRoughness(t, roughness_valid);
		roughness_map = nullptr;
	}

	if (mtl->GetAnisotropyTexmap() != nullptr) {
		anisotropy = 0.0f;
		anisotropy_map = mtl->GetAnisotropyTexmap();
	}
	else {
		Interval anisotropy_valid = FOREVER;
		anisotropy = mtl->GetAnisotropy(t, anisotropy_valid);
		anisotropy_map = nullptr;
	}

	if (mtl->GetRotationTexmap() != nullptr) {
		rotation = 0.0f;
		rotation_map = mtl->GetRotationTexmap();
	}
	else {
		Interval rotation_valid = FOREVER;
		rotation = mtl->GetRotation(t, rotation_valid);
		rotation_map = nullptr;
	}
}

bool AnisotropicShaderDescriptor::operator<(const AnisotropicShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(distribution);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(roughness);
	LT_COMPARE(roughness_map);
	LT_COMPARE(anisotropy);
	LT_COMPARE(anisotropy_map);
	LT_COMPARE(rotation);
	LT_COMPARE(rotation_map);

	return false;
}

DiffuseShaderDescriptor::DiffuseShaderDescriptor(const float roughness_in, const ccl::float3 color_in)
{
	roughness = roughness_in;
	color = color_in;
	color_map = nullptr;
	roughness_map = nullptr;
	name = "__~SimpleDiffuse";
}

DiffuseShaderDescriptor::DiffuseShaderDescriptor(CyclesDiffuseMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	if (mtl->GetDiffuseColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetDiffuseColorTexmap();
	}
	else {
		Interval diffuse_valid = FOREVER;
		const Color diffuse_color = mtl->GetDiffuseColor(t, diffuse_valid);
		color = ccl::make_float3(diffuse_color.r, diffuse_color.g, diffuse_color.b);
		color_map = nullptr;
	}

	if (mtl->GetRoughnessTexmap() != nullptr) {
		roughness = 0.0f;
		roughness_map = mtl->GetRoughnessTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		roughness = mtl->GetRoughness(t, roughness_valid);
		roughness_map = nullptr;
	}
}

bool DiffuseShaderDescriptor::operator<(const DiffuseShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(roughness);
	LT_COMPARE(roughness_map);

	return false;
}

EmissionShaderDescriptor::EmissionShaderDescriptor(CyclesEmissionMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	if (mtl->GetLightColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetLightColorTexmap();
	}
	else {
		Interval light_color_valid = FOREVER;
		const Color light_color = mtl->GetLightColor(t, light_color_valid);
		color = ccl::make_float3(light_color.r, light_color.g, light_color.b);
		color_map = nullptr;
	}

	Interval strength_valid = FOREVER;
	strength = mtl->GetLightStrength(t, strength_valid);

	if (mtl->GetLightStrengthTexmap() != nullptr) {
		strength_map = mtl->GetLightStrengthTexmap();
	}
	else {
		strength_map = nullptr;
	}
}

bool EmissionShaderDescriptor::operator<(const EmissionShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(strength);
	LT_COMPARE(strength_map);

	return false;
}

GlassShaderDescriptor::GlassShaderDescriptor(CyclesGlassMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	Interval dist_valid = FOREVER;
	distribution = mtl->GetDistribution(t, dist_valid);

	if (mtl->GetGlassColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetGlassColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color glass_color = mtl->GetGlassColor(t, color_valid);
		color = ccl::make_float3(glass_color.r, glass_color.g, glass_color.b);
		color_map = nullptr;
	}

	if (mtl->GetRoughnessTexmap() != nullptr) {
		roughness = 0.0f;
		roughness_map = mtl->GetRoughnessTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		roughness = mtl->GetRoughness(t, roughness_valid);
		roughness_map = nullptr;
	}

	Interval ior_valid = FOREVER;
	ior = mtl->GetIOR(t, ior_valid);
}

bool GlassShaderDescriptor::operator<(const GlassShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(distribution);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(roughness);
	LT_COMPARE(roughness_map);
	LT_COMPARE(ior);

	return false;
}

GlossyShaderDescriptor::GlossyShaderDescriptor(CyclesGlossyMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	Interval dist_valid = FOREVER;
	distribution = mtl->GetDistribution(t, dist_valid);

	if (mtl->GetGlossyColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetGlossyColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color glossy_color = mtl->GetGlossyColor(t, color_valid);
		color = ccl::make_float3(glossy_color.r, glossy_color.g, glossy_color.b);
		color_map = nullptr;
	}

	if (mtl->GetRoughnessTexmap() != nullptr) {
		roughness = 0.0f;
		roughness_map = mtl->GetRoughnessTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		roughness = mtl->GetRoughness(t, roughness_valid);
		roughness_map = nullptr;
	}
}

bool GlossyShaderDescriptor::operator<(const GlossyShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(distribution);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(roughness);
	LT_COMPARE(roughness_map);

	return false;
}

HairShaderDescriptor::HairShaderDescriptor(CyclesHairMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	Interval comp_valid = FOREVER;
	component = mtl->GetComponent(t, comp_valid);

	if (mtl->GetHairColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetHairColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color hair_color = mtl->GetHairColor(t, color_valid);
		color = ccl::make_float3(hair_color.r, hair_color.g, hair_color.b);
		color_map = nullptr;
	}

	if (mtl->GetOffsetTexmap() != nullptr) {
		offset = 0.0f;
		offset_map = mtl->GetOffsetTexmap();
	}
	else {
		Interval offset_valid = FOREVER;
		offset = mtl->GetOffset(t, offset_valid);
		offset_map = nullptr;
	}

	if (mtl->GetRoughnessUTexmap() != nullptr) {
		roughness_u = 0.0f;
		roughness_u_map = mtl->GetRoughnessUTexmap();
	}
	else {
		Interval roughness_u_valid = FOREVER;
		roughness_u = mtl->GetRoughnessU(t, roughness_u_valid);
		roughness_u_map = nullptr;
	}

	if (mtl->GetRoughnessVTexmap() != nullptr) {
		roughness_v = 0.0f;
		roughness_v_map = mtl->GetRoughnessVTexmap();
	}
	else {
		Interval roughness_v_valid = FOREVER;
		roughness_v = mtl->GetRoughnessV(t, roughness_v_valid);
		roughness_v_map = nullptr;
	}
}

bool HairShaderDescriptor::operator<(const HairShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(component);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(offset);
	LT_COMPARE(offset_map);
	LT_COMPARE(roughness_u);
	LT_COMPARE(roughness_u_map);
	LT_COMPARE(roughness_v);
	LT_COMPARE(roughness_v_map);

	return false;
}

MixShaderDescriptor::MixShaderDescriptor(CyclesMixMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	if (mtl->GetFacTexmap() != nullptr) {
		fac = 0.0f;
		fac_map = mtl->GetFacTexmap();
	}
	else {
		Interval fac_valid = FOREVER;
		fac = mtl->GetFac(t, fac_valid);
		fac_map = nullptr;
	}
	
	mtl_a = mtl->GetMtlA();
	mtl_b = mtl->GetMtlB();

	Interval blend_valid = FOREVER;
	use_fresnel_blending = mtl->UseFresnelBlending(t, blend_valid);
}

MixShaderDescriptor::MixShaderDescriptor(CyclesVolMixMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	Interval fac_valid = FOREVER;
	fac = mtl->GetFac(t, fac_valid);
	fac_map = nullptr;

	mtl_a = mtl->GetMtlA();
	mtl_b = mtl->GetMtlB();

	use_fresnel_blending = false;
}

bool MixShaderDescriptor::operator<(const MixShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(fac);
	LT_COMPARE(fac_map);
	LT_COMPARE(mtl_a);
	LT_COMPARE(mtl_b);
	LT_COMPARE(use_fresnel_blending);

	return false;
}

PrincipledBSDFShaderDescriptor::PrincipledBSDFShaderDescriptor(CyclesPrincipledBSDFMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);
	mtl->PopulateCCNormalMapDesc(&cc_normal_desc, t);

	// General

	Interval dist_valid = FOREVER;
	distribution = mtl->GetDistribution(t, dist_valid);

	Interval ior_valid = FOREVER;
	ior = mtl->GetIOR(t, ior_valid);

	if (mtl->GetBaseColorTexmap() != nullptr) {
		base_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		base_color_map = mtl->GetBaseColorTexmap();
	}
	else {
		Interval base_color_valid = FOREVER;
		const Color mat_color = mtl->GetBaseColor(t, base_color_valid);
		base_color = ccl::make_float3(mat_color.r, mat_color.g, mat_color.b);
		base_color_map = nullptr;
	}

	if (mtl->GetMetallicTexmap() != nullptr) {
		metallic = 0.0f;
		metallic_map = mtl->GetMetallicTexmap();
	}
	else {
		Interval metallic_valid = FOREVER;
		metallic = mtl->GetMetallic(t, metallic_valid);
		metallic_map = nullptr;
	}

	if (mtl->GetRoughnessTexmap() != nullptr) {
		roughness = 0.0f;
		roughness_map = mtl->GetRoughnessTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		roughness = mtl->GetRoughness(t, roughness_valid);
		roughness_map = nullptr;
	}

	if (mtl->GetSheenTexmap() != nullptr) {
		sheen = 0.0f;
		sheen_map = mtl->GetSheenTexmap();
	}
	else {
		Interval sheen_valid = FOREVER;
		sheen = mtl->GetSheen(t, sheen_valid);
		sheen_map = nullptr;
	}

	if (mtl->GetSheenTintTexmap() != nullptr) {
		sheen_tint = 0.0f;
		sheen_tint_map = mtl->GetSheenTintTexmap();
	}
	else {
		Interval sheen_tint_valid = FOREVER;
		sheen_tint = mtl->GetSheenTint(t, sheen_tint_valid);
		sheen_tint_map = nullptr;
	}

	if (mtl->GetTransmissionTexmap() != nullptr) {
		transmission = 0.0f;
		transmission_map = mtl->GetTransmissionTexmap();
	}
	else {
		Interval transmission_valid = FOREVER;
		transmission = mtl->GetTransmission(t, transmission_valid);
		transmission_map = nullptr;
	}

	if (mtl->GetTransmissionRoughnessTexmap() != nullptr) {
		transmission_roughness = 0.0f;
		transmission_roughness_map = mtl->GetTransmissionRoughnessTexmap();
	}
	else {
		Interval transmission_roughness_valid = FOREVER;
		transmission_roughness = mtl->GetTransmissionRoughness(t, transmission_roughness_valid);
		transmission_roughness_map = nullptr;
	}

	// Specular

	if (mtl->GetSpecularTexmap() != nullptr) {
		specular = 0.0f;
		specular_map = mtl->GetSpecularTexmap();
	}
	else {
		Interval specular_valid = FOREVER;
		specular = mtl->GetSpecular(t, specular_valid);
		specular_map = nullptr;
	}

	if (mtl->GetSpecularTintTexmap() != nullptr) {
		specular_tint = 0.0f;
		specular_tint_map = mtl->GetSpecularTintTexmap();
	}
	else {
		Interval specular_tint_valid = FOREVER;
		specular_tint = mtl->GetSpecularTint(t, specular_tint_valid);
		specular_tint_map = nullptr;
	}

	if (mtl->GetAnisotropicTexmap() != nullptr) {
		anisotropic = 0.0f;
		anisotropic_map = mtl->GetAnisotropicTexmap();
	}
	else {
		Interval anisotropic_valid = FOREVER;
		anisotropic = mtl->GetAnisotropic(t, anisotropic_valid);
		anisotropic_map = nullptr;
	}

	if (mtl->GetAnisotropicRotationTexmap() != nullptr) {
		anisotropic_rotation = 0.0f;
		anisotropic_rotation_map = mtl->GetAnisotropicRotationTexmap();
	}
	else {
		Interval anisotropic_rotation_valid = FOREVER;
		anisotropic_rotation = mtl->GetAnisotropicRotation(t, anisotropic_rotation_valid);
		anisotropic_rotation_map = nullptr;
	}

	// Subsurface

	Interval subsurface_method_valid = FOREVER;
	subsurface_method = mtl->GetSubsurfaceMethod(t, subsurface_method_valid);

	if (mtl->GetSubsurfaceTexmap() != nullptr) {
		subsurface = 0.0f;
		subsurface_map = mtl->GetSubsurfaceTexmap();
	}
	else {
		Interval subsurface_valid = FOREVER;
		subsurface = mtl->GetSubsurface(t, subsurface_valid);
		subsurface_map = nullptr;
	}

	if (mtl->GetSubsurfaceColorTexmap() != nullptr) {
		subsurface_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		subsurface_color_map = mtl->GetSubsurfaceColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color mat_color = mtl->GetSubsurfaceColor(t, color_valid);
		subsurface_color = ccl::make_float3(mat_color.r, mat_color.g, mat_color.b);
		subsurface_color_map = nullptr;
	}

	{
		Interval radius_valid = FOREVER;
		Point3 radius = mtl->GetSubsurfaceRadius(t, radius_valid);
		subsurface_radius = ccl::make_float3(radius.x, radius.y, radius.z);
	}

	// Clearcoat

	if (mtl->GetClearcoatTexmap() != nullptr) {
		clearcoat = 0.0f;
		clearcoat_map = mtl->GetClearcoatTexmap();
	}
	else {
		Interval clearcoat_valid = FOREVER;
		clearcoat = mtl->GetClearcoat(t, clearcoat_valid);
		clearcoat_map = nullptr;
	}

	if (mtl->GetClearcoatRoughnessTexmap() != nullptr) {
		clearcoat_roughness = 0.0f;
		clearcoat_roughness_map = mtl->GetClearcoatRoughnessTexmap();
	}
	else {
		Interval clearcoat_roughness_valid = FOREVER;
		clearcoat_roughness = mtl->GetClearcoatRoughness(t, clearcoat_roughness_valid);
		clearcoat_roughness_map = nullptr;
	}

	// Other

	if (mtl->GetEmissionColorTexmap() != nullptr) {
		emission_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		emission_color_map = mtl->GetEmissionColorTexmap();
	}
	else {
		Interval emission_color_valid = FOREVER;
		const Color mat_color = mtl->GetEmissionColor(t, emission_color_valid);
		emission_color = ccl::make_float3(mat_color.r, mat_color.g, mat_color.b);
		emission_color_map = nullptr;
	}

	if (mtl->GetAlphaTexmap() != nullptr) {
		alpha = 1.0f;
		alpha_map = mtl->GetAlphaTexmap();
	}
	else {
		Interval alpha_valid = FOREVER;
		alpha = mtl->GetAlpha(t, alpha_valid);
		alpha_map = nullptr;
	}
}

bool PrincipledBSDFShaderDescriptor::operator<(const PrincipledBSDFShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(cc_normal_desc);

	LT_COMPARE(distribution);
	LT_COMPARE(ior);
	LT_COMPARE(base_color);
	LT_COMPARE(base_color_map);
	LT_COMPARE(metallic);
	LT_COMPARE(metallic_map);
	LT_COMPARE(roughness);
	LT_COMPARE(roughness_map);
	LT_COMPARE(sheen);
	LT_COMPARE(sheen_map);
	LT_COMPARE(sheen_tint);
	LT_COMPARE(sheen_tint_map);
	LT_COMPARE(transmission);
	LT_COMPARE(transmission_map);
	LT_COMPARE(transmission_roughness);
	LT_COMPARE(transmission_roughness_map);

	LT_COMPARE(specular);
	LT_COMPARE(specular_map);
	LT_COMPARE(specular_tint);
	LT_COMPARE(specular_tint_map);
	LT_COMPARE(anisotropic);
	LT_COMPARE(anisotropic_map);
	LT_COMPARE(anisotropic_rotation);
	LT_COMPARE(anisotropic_rotation_map);

	LT_COMPARE(subsurface_method);
	LT_COMPARE(subsurface);
	LT_COMPARE(subsurface_map);
	LT_COMPARE(subsurface_color);
	LT_COMPARE(subsurface_color_map);
	LT_COMPARE(subsurface_radius);

	LT_COMPARE(clearcoat);
	LT_COMPARE(clearcoat_map);
	LT_COMPARE(clearcoat_roughness);
	LT_COMPARE(clearcoat_roughness_map);

	LT_COMPARE(emission_color);
	LT_COMPARE(emission_color_map);
	LT_COMPARE(alpha);
	LT_COMPARE(alpha_map);

	return false;
}

RefractionShaderDescriptor::RefractionShaderDescriptor(CyclesRefractionMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	Interval dist_valid = FOREVER;
	distribution = mtl->GetDistribution(t, dist_valid);

	if (mtl->GetMatColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetMatColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color mat_color = mtl->GetMatColor(t, color_valid);
		color = ccl::make_float3(mat_color.r, mat_color.g, mat_color.b);
		color_map = nullptr;
	}

	if (mtl->GetRoughnessTexmap() != nullptr) {
		roughness = 0.0f;
		roughness_map = mtl->GetRoughnessTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		roughness = mtl->GetRoughness(t, roughness_valid);
		roughness_map = nullptr;
	}

	Interval ior_valid = FOREVER;
	ior = mtl->GetIOR(t, ior_valid);
}

bool RefractionShaderDescriptor::operator<(const RefractionShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(distribution);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(roughness);
	LT_COMPARE(roughness_map);
	LT_COMPARE(ior);

	return false;
}

SubsurfaceScatterShaderDescriptor::SubsurfaceScatterShaderDescriptor(CyclesSubsurfaceScatterMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	Interval falloff_valid = FOREVER;
	falloff = mtl->GetFalloff(t, falloff_valid);

	if (mtl->GetBsdfColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetBsdfColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color bsdf_color = mtl->GetBsdfColor(t, color_valid);
		color = ccl::make_float3(bsdf_color.r, bsdf_color.g, bsdf_color.b);
		color_map = nullptr;
	}

	if (mtl->GetScaleTexmap() != nullptr) {
		scale = 1.0f;
		scale_map = mtl->GetScaleTexmap();
	}
	else {
		Interval scale_valid = FOREVER;
		scale = mtl->GetScale(t, scale_valid);
		scale_map = nullptr;
	}

	Interval radius_valid = FOREVER;
	const Point3 radius_point3 = mtl->GetRadius(t, radius_valid);
	radius = ccl::make_float3(radius_point3.x, radius_point3.y, radius_point3.z);

	if (mtl->GetTexBlurTexmap() != nullptr) {
		tex_blur = 1.0f;
		tex_blur_map = mtl->GetTexBlurTexmap();
	}
	else {
		Interval blur_valid = FOREVER;
		tex_blur = mtl->GetTexBlur(t, blur_valid);
		tex_blur_map = nullptr;
	}
}

bool SubsurfaceScatterShaderDescriptor::operator<(const SubsurfaceScatterShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(falloff);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(scale);
	LT_COMPARE(scale_map);
	LT_COMPARE(radius);
	LT_COMPARE(tex_blur);
	LT_COMPARE(tex_blur_map);

	return false;
}

ToonShaderDescriptor::ToonShaderDescriptor(CyclesToonMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	Interval component_valid = FOREVER;
	component = mtl->GetComponent(t, component_valid);

	if (mtl->GetMatColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetMatColorTexmap();
	}
	else {
		Interval color_valid = FOREVER;
		const Color mat_color = mtl->GetMatColor(t, color_valid);
		color = ccl::make_float3(mat_color.r, mat_color.g, mat_color.b);
		color_map = nullptr;
	}

	if (mtl->GetSizeTexmap() != nullptr) {
		size = 0.0f;
		size_map = mtl->GetSizeTexmap();
	}
	else {
		Interval size_valid = FOREVER;
		size = mtl->GetSize(t, size_valid);
		size_map = nullptr;
	}

	if (mtl->GetSmoothTexmap() != nullptr) {
		smooth = 0.0f;
		smooth_map = mtl->GetSmoothTexmap();
	}
	else {
		Interval smooth_valid = FOREVER;
		smooth = mtl->GetSmooth(t, smooth_valid);
		smooth_map = nullptr;
	}
}

bool ToonShaderDescriptor::operator<(const ToonShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(component);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(size);
	LT_COMPARE(size_map);
	LT_COMPARE(smooth);
	LT_COMPARE(smooth_map);

	return false;
}

TranslucentShaderDescriptor::TranslucentShaderDescriptor(CyclesTranslucentMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	if (mtl->GetTransColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetTransColorTexmap();
	}
	else {
		Interval colorValid = FOREVER;
		Color transColor = mtl->GetTransColor(t, colorValid);
		color = ccl::make_float3(transColor.r, transColor.g, transColor.b);
		color_map = nullptr;
	}
}

bool TranslucentShaderDescriptor::operator<(const TranslucentShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(color);
	LT_COMPARE(color_map);

	return false;
}

TransparentShaderDescriptor::TransparentShaderDescriptor(CyclesTransparentMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	if (mtl->GetTransColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetTransColorTexmap();
	}
	else {
		Interval colorValid = FOREVER;
		const Color transColor = mtl->GetTransColor(t, colorValid);
		color = ccl::make_float3(transColor.r, transColor.g, transColor.b);
		color_map = nullptr;
	}
}

bool TransparentShaderDescriptor::operator<(const TransparentShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(color);
	LT_COMPARE(color_map);

	return false;
}

VelvetShaderDescriptor::VelvetShaderDescriptor(CyclesVelvetMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	mtl->PopulateNormalMapDesc(&normal_desc, t);

	if (mtl->GetMatColorTexmap() != nullptr) {
		color = ccl::make_float3(0.0f, 0.0f, 0.0f);
		color_map = mtl->GetMatColorTexmap();
	}
	else {
		Interval diffuse_valid = FOREVER;
		const Color diffuse_color = mtl->GetMatColor(t, diffuse_valid);
		color = ccl::make_float3(diffuse_color.r, diffuse_color.g, diffuse_color.b);
		color_map = nullptr;
	}

	if (mtl->GetSigmaTexmap() != nullptr) {
		sigma = 0.0f;
		sigma_map = mtl->GetSigmaTexmap();
	}
	else {
		Interval roughness_valid = FOREVER;
		sigma = mtl->GetSigma(t, roughness_valid);
		sigma_map = nullptr;
	}
}

bool VelvetShaderDescriptor::operator<(const VelvetShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(normal_desc);
	LT_COMPARE(color);
	LT_COMPARE(color_map);
	LT_COMPARE(sigma);
	LT_COMPARE(sigma_map);

	return false;
}

VolAbsorbtionShaderDescriptor::VolAbsorbtionShaderDescriptor(CyclesVolAbsorptionMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	Interval color_valid = FOREVER;
	const Color vol_color = mtl->GetVolumeColor(t, color_valid);
	color = ccl::make_float3(vol_color.r, vol_color.g, vol_color.b);

	Interval density_valid = FOREVER;
	density = mtl->GetDensity(t, density_valid);
}

bool VolAbsorbtionShaderDescriptor::operator<(const VolAbsorbtionShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(color);
	LT_COMPARE(density);

	return false;
}

VolScatterShaderDescriptor::VolScatterShaderDescriptor(CyclesVolScatterMat* const mtl, const TimeValue t) :
	name(bad_from_wstring(mtl->GetName().data()))
{
	Interval color_valid = FOREVER;
	const Color vol_color = mtl->GetVolumeColor(t, color_valid);
	color = ccl::make_float3(vol_color.r, vol_color.g, vol_color.b);

	Interval density_valid = FOREVER;
	density = mtl->GetDensity(t, density_valid);

	Interval anisotropy_valid = FOREVER;
	anisotropy = mtl->GetAnisotropy(t, anisotropy_valid);
}

bool VolScatterShaderDescriptor::operator<(const VolScatterShaderDescriptor& other) const
{
	LT_COMPARE(name);

	LT_COMPARE(color);
	LT_COMPARE(density);
	LT_COMPARE(anisotropy);

	return false;
}

LightShaderDescriptor::LightShaderDescriptor(const ccl::float3 color_in, const float intensity_in)
{
	color = color_in;
	intensity = intensity_in;
}

bool LightShaderDescriptor::operator<(const LightShaderDescriptor& other) const
{
	LT_COMPARE(color);
	LT_COMPARE(intensity);

	return false;
}

ComparableColor::ComparableColor(const ccl::float3 color)
{
	r = color.x;
	g = color.y;
	b = color.z;
}

bool ComparableColor::operator<(const ComparableColor& other) const
{
	LT_COMPARE(r);
	LT_COMPARE(g);
	LT_COMPARE(b);

	return false;
}
