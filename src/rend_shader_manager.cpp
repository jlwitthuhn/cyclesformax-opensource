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
 
#include "rend_shader_manager.h"

#include <render/light.h>
#include <render/nodes.h>
#include <render/scene.h>

#include "cache_baked_texmap.h"
#include "const_classid.h"
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
#include "plugin_tex_environment.h"
#include "plugin_tex_sky.h"
#include "rend_logger_ext.h"
#include "rend_shader_graph_converter.h"
#include "util_color_temp.h"
#include "util_pblock_dump.h"

static ccl::ShaderOutput* get_closure_output(ccl::ShaderNode* const node)
{
	if (node == nullptr) {
		return nullptr;
	}

	for (auto i = 0; i < node->outputs.size(); ++i) {
		ccl::ShaderOutput* const this_output = node->outputs[i];
		if (this_output->type() == ccl::SocketType::Type::CLOSURE) {
			return this_output;
		}
	}

	return nullptr;
}

static bool add_node_and_connect_texmap(
	Texmap* const texmap,
	ccl::ShaderGraph* const graph,
	ccl::ShaderInput* const input,
	BakedTexmapCache& texmap_cache,
	ccl::Scene* const scene)
{
	if (texmap == nullptr) {
		return false;
	}
	ccl::ShaderNode* const texmap_node{ texmap_cache.get_node_from_texmap(texmap, scene) };
	graph->add(texmap_node);
	graph->connect(texmap_node->output("Color"), input);
	return true;
}

static bool use_env_node_for_projection(const EnvironmentProjection proj)
{
	return (proj == EnvironmentProjection::EQUIRECTANGULAR || proj == EnvironmentProjection::MIRROR_BALL);
}

static ccl::NodeEnvironmentProjection get_ccl_projection(const EnvironmentProjection proj)
{
	if (proj == EnvironmentProjection::EQUIRECTANGULAR) {
		return ccl::NodeEnvironmentProjection::NODE_ENVIRONMENT_EQUIRECTANGULAR;
	}
	else if (proj == EnvironmentProjection::MIRROR_BALL) {
		return ccl::NodeEnvironmentProjection::NODE_ENVIRONMENT_MIRROR_BALL;
	}
	return ccl::NodeEnvironmentProjection::NODE_ENVIRONMENT_EQUIRECTANGULAR;
}

MaxShaderManager::MaxShaderManager(BakedTexmapCache& texmap_cache, ccl::Scene* const scene, const TimeValue t) :
	logger(global_log_manager.new_logger(L"MaxShaderManager")),
	scene(scene),
	texmap_cache(texmap_cache)
{
	*logger << LogCtl::SEPARATOR;

	frame_time = t;

	unsupported_shader_index = -1;
	holdout_shader_index = -1;

	// Populate shaders with no params
	get_unsupported_shader();
	get_holdout_shader();

	*logger << "Constructor complete" << LogCtl::WRITE_LINE;
}

MaxShaderManager::~MaxShaderManager()
{

}

ccl::Shader* MaxShaderManager::get_ccl_shader(const int index) const
{
	if (scene != nullptr && index < scene->shaders.size()) {
		return scene->shaders[index];
	}
	return nullptr;
}

void MaxShaderManager::set_mis_map_size(const int size)
{
	mis_map_size = size;
}

void MaxShaderManager::set_sky_from_env_map(Texmap* const env_map, const float strength)
{
	*logger << "set_sky_from_env_map called..." << LogCtl::WRITE_LINE;

	if (env_map->ClassID() == CYCLES_TEXMAP_SKY_CLASS) {
		*logger << "cycles texmap sky found..." << LogCtl::WRITE_LINE;

		CyclesSkyEnvTexmap* const sky_env_map = dynamic_cast<CyclesSkyEnvTexmap*>(env_map);
		ccl::ShaderGraph* const graph = new ccl::ShaderGraph();

		ccl::SkyTextureNode* const sky_node = new ccl::SkyTextureNode();
		graph->add(sky_node);

		Interval sky_type_valid = FOREVER;
		if (sky_env_map->GetSkyType(frame_time, sky_type_valid) == SkyType::PREETHAM) {
			sky_node->set_sky_type(ccl::NodeSkyType::NODE_SKY_PREETHAM);
		}
		else {
			sky_node->set_sky_type(ccl::NodeSkyType::NODE_SKY_HOSEK);
		}

		Interval sun_vector_valid = FOREVER;
		const ccl::float3 sun_vec = sky_env_map->GetSunVector(frame_time, sun_vector_valid);

		*logger << "sun vec: " << sun_vec.x << " " << sun_vec.y << " " << sun_vec.z << LogCtl::WRITE_LINE;

		sky_node->set_sun_direction(sun_vec);
		Interval turbidity_valid = FOREVER;
		sky_node->set_turbidity(sky_env_map->GetTurbidity(frame_time, turbidity_valid));
		Interval ground_albedo_valid = FOREVER;
		sky_node->set_ground_albedo(sky_env_map->GetGroundAlbedo(frame_time, ground_albedo_valid));

		ccl::BackgroundNode* const bg_node = new ccl::BackgroundNode();
		bg_node->set_strength(strength);
		graph->add(bg_node);

		graph->connect(sky_node->output("Color"), bg_node->input("Color"));
		graph->connect(bg_node->output("Background"), graph->output()->input("Surface"));

		ccl::Shader* const bg_shader = scene->default_background;
		bg_shader->set_use_mis(true);
		bg_shader->set_graph(graph);
		bg_shader->tag_update(scene);
	}
	else if (env_map->ClassID() == CYCLES_TEXMAP_ENVIRONMENT_CLASS) {
		*logger << "cycles texmap environment found..." << LogCtl::WRITE_LINE;

		EnvironmentTexmap* const sky_env_map = dynamic_cast<EnvironmentTexmap*>(env_map);

		Texmap* const bg_texmap = sky_env_map->GetBGMap();
		Interval bg_proj_valid = FOREVER;
		const EnvironmentProjection bg_proj = sky_env_map->GetBGProjection(frame_time, bg_proj_valid);

		Texmap* const light_map = sky_env_map->GetLightingMap();
		Interval light_proj_valid = FOREVER;
		const EnvironmentProjection light_proj = sky_env_map->GetLightingProjection(frame_time, light_proj_valid);

		if (bg_texmap != nullptr) {
			set_bitmap_sky_shader(strength, bg_texmap, bg_proj, light_map, light_proj);
		}
		else {
			// With no bg texmap, use light map for both
			set_bitmap_sky_shader(strength, light_map, light_proj, light_map, light_proj);
		}
	}
	else {
		*logger << "bitmap environment found..." << LogCtl::WRITE_LINE;

		// Attempt to use any other map type as a HDRI
		set_bitmap_sky_shader(strength, env_map);
	}
}

void MaxShaderManager::set_simple_sky_shader(const SimpleSkyDescriptor& in)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();

	ccl::BackgroundNode* const bg_node = new ccl::BackgroundNode();
	bg_node->set_strength(in.strength);
	bg_node->set_color(in.color);
	graph->add(bg_node);

	graph->connect(bg_node->output("Background"), graph->output()->input("Surface"));

	ccl::Shader* const bg_shader = scene->default_background;
	bg_shader->set_graph(graph);
	bg_shader->tag_update(scene);
}

void MaxShaderManager::set_bitmap_sky_shader(
	const float strength,
	Texmap* const bg_map,
	const EnvironmentProjection bg_proj,
	Texmap* const light_map,
	const EnvironmentProjection light_proj)
{
	// Clear backplate, it will be set later in this function if needed
	texmap_cache.set_backplate_texmap(nullptr);

	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();

	ccl::ShaderNode* bg_tex_node = nullptr;
	if (bg_map != nullptr) {
		if (use_env_node_for_projection(bg_proj)) {
			ccl::EnvironmentTextureNode* const bg_env_node = texmap_cache.get_env_node_from_texmap(bg_map, scene);
			bg_env_node->set_projection(get_ccl_projection(bg_proj));
			graph->add(bg_env_node);

			if (bg_map->ClassID() != PHYS_SKY_MAP_CLASS) {
				ccl::TextureMapping env_mapping = bg_env_node->tex_mapping;
				env_mapping.scale = ccl::make_float3(-1.0f, 1.0f, 1.0f);
				env_mapping.rotation = ccl::make_float3(0.0f, 0.0f, 3.14159f / 2.0f);
				bg_env_node->tex_mapping = env_mapping;
			}

			bg_tex_node = bg_env_node;
		}
		else {
			// Backplate
			texmap_cache.set_backplate_texmap(bg_map);

			ccl::ColorNode* const bg_color_node = new ccl::ColorNode();
			bg_color_node->set_value(ccl::make_float3(0.0f, 0.0f, 0.0f));
			graph->add(bg_color_node);
			bg_tex_node = bg_color_node;
		}
	}

	ccl::ImageSlotTextureNode* light_tex_node = nullptr;
	if (light_map != nullptr) {
		ccl::EnvironmentTextureNode* light_env_node = texmap_cache.get_env_node_from_texmap(light_map, scene);
		light_env_node->set_projection(get_ccl_projection(light_proj));
		graph->add(light_env_node);

		if (light_map->ClassID() != PHYS_SKY_MAP_CLASS) {
			ccl::TextureMapping env_mapping = light_env_node->tex_mapping;
			env_mapping.scale = ccl::make_float3(-1.0f, 1.0f, 1.0f);
			env_mapping.rotation = ccl::make_float3(0.0f, 0.0f, 3.14159f / 2.0f);
			light_env_node->tex_mapping = env_mapping;
		}

		light_tex_node = light_env_node;
	}

	ccl::BackgroundNode* const bg_node = new ccl::BackgroundNode();
	bg_node->set_strength(strength);
	bg_node->set_color(ccl::make_float3(0.0f, 0.0f, 0.0f));
	graph->add(bg_node);

	if (bg_tex_node != nullptr) {
		if (light_tex_node == nullptr) {
			graph->connect(bg_tex_node->output("Color"), bg_node->input("Color"));
		}
		else {
			ccl::LightPathNode* const light_path = new ccl::LightPathNode();
			graph->add(light_path);
			ccl::MixNode* const mix_node = new ccl::MixNode();
			graph->add(mix_node);

			graph->connect(light_path->output("Is Camera Ray"), mix_node->input("Fac"));
			graph->connect(light_tex_node->output("Color"), mix_node->input("Color1"));
			graph->connect(bg_tex_node->output("Color"), mix_node->input("Color2"));
			graph->connect(mix_node->output("Color"), bg_node->input("Color"));
		}
	}

	graph->connect(bg_node->output("Background"), graph->output()->input("Surface"));

	ccl::Shader* const bg_shader = scene->default_background;
	bg_shader->set_use_mis(true);
	bg_shader->set_graph(graph);
	bg_shader->tag_update(scene);

	ccl::Light* const light = new ccl::Light();
	light->set_shader(bg_shader);
	light->set_light_type(ccl::LightType::LIGHT_BACKGROUND);
	light->set_map_resolution(mis_map_size);
	light->set_use_mis(true);
	light->set_max_bounces(1024);

	scene->lights.push_back(light);
}

MaxMultiShaderHelper MaxShaderManager::get_mtl_multishader(Mtl* const mtl)
{
	if (mtl->ClassID() == MULTI_SUB_MATERIAL_CLASS) {
		*logger << "Multi-material found, getting submaterials..." << LogCtl::WRITE_LINE;
		MaxMultiShaderHelper result(scene->shaders[unsupported_shader_index], mtl->NumSubMtls());

		for (auto i = 0; i < mtl->NumSubMtls(); ++i) {
			if (mtl->GetSubMtl(i) != nullptr) {
				int shader_scene_index = get_mtl_shader(mtl->GetSubMtl(i));
				result.add_shader(i, scene->shaders[shader_scene_index]);
			}
		}

		return result;
	}
	else {
		*logger << "Non-multi-material found, getting material shader..." << LogCtl::WRITE_LINE;
		const int scene_shader_index = get_mtl_shader(mtl);
		*logger << "Found shader in slot: " << scene_shader_index << LogCtl::WRITE_LINE;
		return MaxMultiShaderHelper(scene->shaders[scene_shader_index]);
	}
}

int MaxShaderManager::get_mtl_shader(Mtl* const mtl)
{
	*logger << "Getting shader for max material..." << LogCtl::WRITE_LINE;
	*logger << "Mat name: " << mtl->ClassName() << LogCtl::WRITE_LINE;
	const auto initial_format = logger->number_format(LogNumberFormat::HEX);
	*logger << "Class id: " << mtl->ClassID() << LogCtl::WRITE_LINE;
	logger->number_format(initial_format);

	if (mtl->ClassID() == CYCLES_MAT_SHADER_GRAPH_08_CLASS) {
		const ShaderGraphShaderDescriptor desc(dynamic_cast<CyclesShaderGraphMat08*>(mtl), frame_time);
		return get_shader_graph_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_SHADER_GRAPH_16_CLASS) {
		const ShaderGraphShaderDescriptor desc(dynamic_cast<CyclesShaderGraphMat16*>(mtl), frame_time);
		return get_shader_graph_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_SHADER_GRAPH_32_CLASS) {
		const ShaderGraphShaderDescriptor desc(dynamic_cast<CyclesShaderGraphMat32*>(mtl), frame_time);
		return get_shader_graph_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_SHADER_CLASS) {
		const ShaderShaderDescriptor desc(dynamic_cast<CyclesShaderMat*>(mtl), frame_time);
		return get_shader_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_ADD_CLASS) {
		const AddShaderDescriptor desc(dynamic_cast<CyclesAddMat*>(mtl), frame_time);
		return get_add_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_ANISOTROPIC_CLASS) {
		const AnisotropicShaderDescriptor desc(dynamic_cast<CyclesAnisotropicMat*>(mtl), frame_time);
		return get_anisotropic_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_DIFFUSE_CLASS) {
		const DiffuseShaderDescriptor desc(dynamic_cast<CyclesDiffuseMat*>(mtl), frame_time);
		return get_diffuse_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_EMISSION_CLASS) {
		const EmissionShaderDescriptor desc(dynamic_cast<CyclesEmissionMat*>(mtl), frame_time);
		return get_emission_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_GLASS_CLASS) {
		const GlassShaderDescriptor desc(dynamic_cast<CyclesGlassMat*>(mtl), frame_time);
		return get_glass_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_GLOSSY_CLASS) {
		const GlossyShaderDescriptor desc(dynamic_cast<CyclesGlossyMat*>(mtl), frame_time);
		return get_glossy_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_HAIR_CLASS) {
		const HairShaderDescriptor desc(dynamic_cast<CyclesHairMat*>(mtl), frame_time);
		return get_hair_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_HOLDOUT_CLASS) {
		return get_holdout_shader();
	}
	else if (mtl->ClassID() == CYCLES_MAT_MIX_CLASS) {
		const MixShaderDescriptor desc(dynamic_cast<CyclesMixMat*>(mtl), frame_time);
		return get_mix_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_PRINCIPLED_CLASS) {
		const PrincipledBSDFShaderDescriptor desc(dynamic_cast<CyclesPrincipledBSDFMat*>(mtl), frame_time);
		return get_principled_bsdf_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_REFRACTION_CLASS) {
		const RefractionShaderDescriptor desc(dynamic_cast<CyclesRefractionMat*>(mtl), frame_time);
		return get_refraction_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_SUBSURFACE_CLASS) {
		const SubsurfaceScatterShaderDescriptor desc(dynamic_cast<CyclesSubsurfaceScatterMat*>(mtl), frame_time);
		return get_subsurface_scatter_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_TOON_CLASS) {
		const ToonShaderDescriptor desc(dynamic_cast<CyclesToonMat*>(mtl), frame_time);
		return get_toon_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_TRANSLUCENT_CLASS) {
		const TranslucentShaderDescriptor desc(dynamic_cast<CyclesTranslucentMat*>(mtl), frame_time);
		return get_translucent_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_TRANSPARENT_CLASS) {
		const TransparentShaderDescriptor desc(dynamic_cast<CyclesTransparentMat*>(mtl), frame_time);
		return get_transparent_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_VELVET_CLASS) {
		const VelvetShaderDescriptor desc(dynamic_cast<CyclesVelvetMat*>(mtl), frame_time);
		return get_velvet_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_VOL_ABSORPTION_CLASS) {
		const VolAbsorbtionShaderDescriptor desc(dynamic_cast<CyclesVolAbsorptionMat*>(mtl), frame_time);
		return get_vol_absorption_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_VOL_SCATTER_CLASS) {
		const VolScatterShaderDescriptor desc(dynamic_cast<CyclesVolScatterMat*>(mtl), frame_time);
		return get_vol_scatter_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_VOL_ADD_CLASS) {
		const AddShaderDescriptor desc(dynamic_cast<CyclesVolAddMat*>(mtl), frame_time);
		return get_vol_add_shader(desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_VOL_MIX_CLASS) {
		const MixShaderDescriptor desc(dynamic_cast<CyclesVolMixMat*>(mtl), frame_time);
		return get_vol_mix_shader(desc);
	}
	else if (mtl->ClassID() == PHYSICAL_MATERIAL_CLASS) {
		const AdskPhysicalMaterialDescriptor desc(mtl, frame_time);
		return get_adsk_phys_shader(desc);
	}
	else {
		return get_unsupported_shader();
	}
}

int MaxShaderManager::get_unsupported_shader()
{
	if (unsupported_shader_index < 0) {
		unsupported_shader_index = add_unsupported_shader();
	}

	return unsupported_shader_index;
}

int MaxShaderManager::get_holdout_shader()
{
	if (holdout_shader_index < 0) {
		holdout_shader_index = add_holdout_shader();
	}

	return holdout_shader_index;
}

int MaxShaderManager::get_shader_graph_shader(const ShaderGraphShaderDescriptor& desc)
{
	if (shader_graph_shaders.count(desc) == 0) {
		shader_graph_shaders[desc] = add_shader_graph_shader(desc);
	}

	return shader_graph_shaders[desc];
}

int MaxShaderManager::get_shader_shader(const ShaderShaderDescriptor& desc)
{
	if (shader_shaders.count(desc) == 0) {
		shader_shaders[desc] = add_shader_shader(desc);
	}

	return shader_shaders[desc];
}

int MaxShaderManager::get_add_shader(const AddShaderDescriptor& desc)
{
	*logger << "MaxShaderManager::get_add_shader called" << LogCtl::WRITE_LINE;

	if (add_shaders.count(desc) == 0) {
		*logger << "creating add shader" << LogCtl::WRITE_LINE;
		add_shaders[desc] = add_add_shader(desc);
	}

	return add_shaders[desc];
}
int MaxShaderManager::get_anisotropic_shader(const AnisotropicShaderDescriptor& desc)
{
	if (anisotropic_shaders.count(desc) == 0) {
		anisotropic_shaders[desc] = add_anisotropic_shader(desc);
	}

	return anisotropic_shaders[desc];
}

int MaxShaderManager::get_diffuse_shader(const DiffuseShaderDescriptor& desc)
{
	if (diffuse_shaders.count(desc) == 0) {
		diffuse_shaders[desc] = add_diffuse_shader(desc);
	}

	return diffuse_shaders[desc];
}

int MaxShaderManager::get_emission_shader(const EmissionShaderDescriptor& desc)
{
	if (emission_shaders.count(desc) == 0) {
		emission_shaders[desc] = add_emission_shader(desc);
	}
	return emission_shaders[desc];
}

int MaxShaderManager::get_glass_shader(const GlassShaderDescriptor& desc)
{
	if (glass_shaders.count(desc) == 0) {
		glass_shaders[desc] = add_glass_shader(desc);
	}
	return glass_shaders[desc];
}

int MaxShaderManager::get_glossy_shader(const GlossyShaderDescriptor& desc)
{
	if (glossy_shaders.count(desc) == 0) {
		glossy_shaders[desc] = add_glossy_shader(desc);
	}
	return glossy_shaders[desc];
}

int MaxShaderManager::get_hair_shader(const HairShaderDescriptor& desc)
{
	if (hair_shaders.count(desc) == 0) {
		hair_shaders[desc] = add_hair_shader(desc);
	}
	return hair_shaders[desc];
}

int MaxShaderManager::get_mix_shader(const MixShaderDescriptor& desc)
{
	if (mix_shaders.count(desc) == 0) {
		mix_shaders[desc] = add_mix_shader(desc);
	}
	return mix_shaders[desc];
}

int MaxShaderManager::get_principled_bsdf_shader(const PrincipledBSDFShaderDescriptor& desc)
{
	if (principled_bsdf_shaders.count(desc) == 0) {
		principled_bsdf_shaders[desc] = add_principled_bsdf_shader(desc);
	}
	return principled_bsdf_shaders[desc];
}

int MaxShaderManager::get_refraction_shader(const RefractionShaderDescriptor& desc)
{
	if (refraction_shaders.count(desc) == 0) {
		refraction_shaders[desc] = add_refraction_shader(desc);
	}
	return refraction_shaders[desc];
}

int MaxShaderManager::get_subsurface_scatter_shader(const SubsurfaceScatterShaderDescriptor& desc)
{
	if (subsurface_scatter_shaders.count(desc) == 0) {
		subsurface_scatter_shaders[desc] = add_subsurface_scatter_shader(desc);
	}
	return subsurface_scatter_shaders[desc];
}

int MaxShaderManager::get_toon_shader(const ToonShaderDescriptor& desc)
{
	if (toon_shaders.count(desc) == 0) {
		toon_shaders[desc] = add_toon_shader(desc);
	}
	return toon_shaders[desc];
}

int MaxShaderManager::get_translucent_shader(const TranslucentShaderDescriptor& desc)
{
	if (translucent_shaders.count(desc) == 0) {
		translucent_shaders[desc] = add_translucent_shader(desc);
	}
	return translucent_shaders[desc];
}

int MaxShaderManager::get_transparent_shader(const TransparentShaderDescriptor& desc)
{
	if (transparent_shaders.count(desc) == 0) {
		transparent_shaders[desc] = add_transparent_shader(desc);
	}
	return transparent_shaders[desc];
}

int MaxShaderManager::get_velvet_shader(const VelvetShaderDescriptor& desc)
{
	if (velvet_shaders.count(desc) == 0) {
		velvet_shaders[desc] = add_velvet_shader(desc);
	}
	return velvet_shaders[desc];
}

int MaxShaderManager::get_vol_absorption_shader(const VolAbsorbtionShaderDescriptor& desc)
{
	if (vol_absorption_shaders.count(desc) == 0) {
		vol_absorption_shaders[desc] = add_vol_absorption_shader(desc);
	}
	return vol_absorption_shaders[desc];
}

int MaxShaderManager::get_vol_add_shader(const AddShaderDescriptor& desc)
{
	if (vol_add_shaders.count(desc) == 0) {
		vol_add_shaders[desc] = add_vol_add_shader(desc);
	}
	return vol_add_shaders[desc];
}

int MaxShaderManager::get_vol_mix_shader(const MixShaderDescriptor& desc)
{
	if (vol_mix_shaders.count(desc) == 0) {
		vol_mix_shaders[desc] = add_vol_mix_shader(desc);
	}
	return vol_mix_shaders[desc];
}

int MaxShaderManager::get_vol_scatter_shader(const VolScatterShaderDescriptor& desc)
{
	if (vol_scatter_shaders.count(desc) == 0) {
		vol_scatter_shaders[desc] = add_vol_scatter_shader(desc);
	}
	return vol_scatter_shaders[desc];
}

int MaxShaderManager::get_adsk_phys_shader(const AdskPhysicalMaterialDescriptor& desc)
{
	if (adsk_phys_shaders.count(desc) == 0) {
		adsk_phys_shaders[desc] = add_adsk_phys_shader(desc);
	}
	return adsk_phys_shaders[desc];
}

int MaxShaderManager::get_simple_color_shader(const ccl::float3 color)
{
	const ComparableColor comp_color(color);
	if (simple_color_shaders.count(comp_color) == 0) {
		simple_color_shaders[comp_color] = add_simple_color_shader(color);
	}

	return simple_color_shaders[comp_color];
}

int MaxShaderManager::get_light_shader(const ccl::float3 color, const float intensity)
{
	const LightShaderDescriptor desc(color, intensity);

	if (light_shaders.count(desc) == 0) {
		light_shaders[desc] = add_light_shader(desc);
	}
	return light_shaders[desc];
}

void MaxShaderManager::log_shader_stats() const
{
	*logger << LogCtl::SEPARATOR;
	*logger << "Shader stats summary" << LogCtl::WRITE_LINE;
	*logger << "     Shader graph shaders: " << shader_graph_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "           Shader shaders: " << shader_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "              Add shaders: " << add_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "      Anisotropic shaders: " << anisotropic_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "          Diffuse shaders: " << diffuse_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "         Emission shaders: " << emission_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "            Glass shaders: " << glass_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "           Glossy shaders: " << glossy_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "             Hair shaders: " << hair_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "              Mix shaders: " << mix_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "  Principled BSDF shaders: " << principled_bsdf_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "       Refraction shaders: " << refraction_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "       Subsurface shaders: " << subsurface_scatter_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "             Toon shaders: " << toon_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "      Translucent shaders: " << translucent_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "      Transparent shaders: " << transparent_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "           Velvet shaders: " << velvet_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "  Vol. Absorption shaders: " << vol_absorption_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "         Vol. Add shaders: " << vol_add_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "         Vol. Mix shaders: " << vol_mix_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "     Vol. Scatter shaders: " << vol_scatter_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "Physical Material shaders: " << adsk_phys_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "           Simple shaders: " << simple_color_shaders.size() << LogCtl::WRITE_LINE;
	*logger << "            Light shaders: " << light_shaders.size() << LogCtl::WRITE_LINE;
	*logger << LogCtl::SEPARATOR;
}

int MaxShaderManager::add_unsupported_shader()
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_unsupported_shader_nodes(graph);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = "__~Unsupported";
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_holdout_shader()
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const holdout_node = add_holdout_shader_nodes(graph);
	graph->connect(holdout_node->output("Holdout"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = "__~Holdout";
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_shader_graph_shader(const ShaderGraphShaderDescriptor& desc)
{
	*logger << "add_shader_graph_shader called..." << LogCtl::WRITE_LINE;
	// Empty string means no valid graph, skip attempting to decode
	if (desc.encoded_graph == std::string()) {
		return get_unsupported_shader();
	}

	ShaderGraphConverter converter(texmap_cache);
	for (size_t i = 0; i < desc.texmaps.size(); ++i) {
		converter.set_texmap_slot(desc.texmaps[i], i + 1);
	}
	*logger << "Got shader graph from converter" << LogCtl::WRITE_LINE;

	ccl::ShaderGraph* const graph = converter.get_shader_graph(desc.encoded_graph, scene);
	ccl::Shader* const shader = new ccl::Shader();

	shader->set_use_mis(desc.shader_params.use_mis);

	if (desc.shader_params.displacement == DisplacementMethod::DISPLACEMENT) {
		shader->set_displacement_method(ccl::DisplacementMethod::DISPLACE_TRUE);
	}
	else if (desc.shader_params.displacement == DisplacementMethod::BOTH) {
		shader->set_displacement_method(ccl::DisplacementMethod::DISPLACE_BOTH);
	}
	else {
		shader->set_displacement_method(ccl::DisplacementMethod::DISPLACE_BUMP);
	}

	if (desc.shader_params.vol_sampling == VolumeSamplingMethod::EQUIANGULAR) {
		shader->set_volume_sampling_method(ccl::VolumeSampling::VOLUME_SAMPLING_EQUIANGULAR);
	}
	else if (desc.shader_params.vol_sampling == VolumeSamplingMethod::DISTANCE) {
		shader->set_volume_sampling_method(ccl::VolumeSampling::VOLUME_SAMPLING_DISTANCE);
	}
	else {
		shader->set_volume_sampling_method(ccl::VolumeSampling::VOLUME_SAMPLING_MULTIPLE_IMPORTANCE);
	}

	if (desc.shader_params.vol_interp == VolumeInterpolationMethod::CUBIC) {
		shader->set_volume_interpolation_method(ccl::VolumeInterpolation::VOLUME_INTERPOLATION_CUBIC);
	}
	else {
		shader->set_volume_interpolation_method(ccl::VolumeInterpolation::VOLUME_INTERPOLATION_LINEAR);
	}

	shader->set_heterogeneous_volume(!(desc.shader_params.vol_is_homogeneous));

	shader->name = desc.name;

	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_shader_shader(const ShaderShaderDescriptor& desc)
{
	if (desc.surface_mtl == nullptr && desc.volume_mtl == nullptr) {
		return get_unsupported_shader();
	}
	
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();

	if (desc.surface_mtl != nullptr) {
		ccl::ShaderNode* const surface_node = add_nodes_for_mtl(graph, desc.surface_mtl);
		ccl::ShaderOutput* const surface_closure_output = get_closure_output(surface_node);
		graph->connect(surface_closure_output, graph->output()->input("Surface"));
	}

	if (desc.volume_mtl != nullptr) {
		ccl::ShaderNode* const volume_node = add_nodes_for_mtl(graph, desc.volume_mtl);
		ccl::ShaderOutput* const volume_closure_output = get_closure_output(volume_node);
		graph->connect(volume_closure_output, graph->output()->input("Volume"));
	}

	ccl::Shader* const shader = new ccl::Shader();

	*logger << "shader shader params: " << LogCtl::WRITE_LINE;
	*logger << "use_mis: " << desc.shader_params.use_mis << LogCtl::WRITE_LINE;
	*logger << "vol_sampling: " << static_cast<int>(desc.shader_params.vol_sampling) << LogCtl::WRITE_LINE;
	*logger << "vol_interp: " << static_cast<int>(desc.shader_params.vol_interp) << LogCtl::WRITE_LINE;
	*logger << "vol_is_homogeneous: " << desc.shader_params.vol_is_homogeneous << LogCtl::WRITE_LINE;

	shader->set_use_mis(desc.shader_params.use_mis);

	if (desc.shader_params.displacement == DisplacementMethod::DISPLACEMENT) {
		shader->set_displacement_method(ccl::DisplacementMethod::DISPLACE_TRUE);
	}
	else if (desc.shader_params.displacement == DisplacementMethod::BOTH) {
		shader->set_displacement_method(ccl::DisplacementMethod::DISPLACE_BOTH);
	}
	else {
		shader->set_displacement_method(ccl::DisplacementMethod::DISPLACE_BUMP);
	}

	if (desc.shader_params.vol_sampling == VolumeSamplingMethod::EQUIANGULAR) {
		shader->set_volume_sampling_method(ccl::VolumeSampling::VOLUME_SAMPLING_EQUIANGULAR);
	}
	else if (desc.shader_params.vol_sampling == VolumeSamplingMethod::DISTANCE) {
		shader->set_volume_sampling_method(ccl::VolumeSampling::VOLUME_SAMPLING_DISTANCE);
	}
	else {
		shader->set_volume_sampling_method(ccl::VolumeSampling::VOLUME_SAMPLING_MULTIPLE_IMPORTANCE);
	}

	if (desc.shader_params.vol_interp == VolumeInterpolationMethod::CUBIC) {
		shader->set_volume_interpolation_method(ccl::VolumeInterpolation::VOLUME_INTERPOLATION_CUBIC);
	}
	else {
		shader->set_volume_interpolation_method(ccl::VolumeInterpolation::VOLUME_INTERPOLATION_LINEAR);
	}

	shader->set_heterogeneous_volume(!(desc.shader_params.vol_is_homogeneous));

	shader->name = desc.name;

	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_add_shader(const AddShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const add_node = add_add_shader_nodes(graph, desc);
	graph->connect(add_node->output("Closure"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_anisotropic_shader(const AnisotropicShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_anisotropic_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_diffuse_shader(const DiffuseShaderDescriptor& desc)
{
	*logger << "add_diffuse_shader called..." << LogCtl::WRITE_LINE;

	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_diffuse_shader_nodes(graph, desc);

	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_emission_shader(const EmissionShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const em_node = add_emission_shader_nodes(graph, desc);
	graph->connect(em_node->output("Emission"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_glass_shader(const GlassShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const glass_node = add_glass_shader_nodes(graph, desc);
	graph->connect(glass_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_glossy_shader(const GlossyShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const em_node = add_glossy_shader_nodes(graph, desc);
	graph->connect(em_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_hair_shader(const HairShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const em_node = add_hair_shader_nodes(graph, desc);
	graph->connect(em_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_mix_shader(const MixShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const mix_node = add_mix_shader_nodes(graph, desc);
	graph->connect(mix_node->output("Closure"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_principled_bsdf_shader(const PrincipledBSDFShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_principled_bsdf_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_refraction_shader(const RefractionShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_refraction_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_subsurface_scatter_shader(const SubsurfaceScatterShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_subsurface_scatter_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSSRDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_toon_shader(const ToonShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_toon_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_translucent_shader(const TranslucentShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_translucent_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_transparent_shader(const TransparentShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_transparent_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_velvet_shader(const VelvetShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_velvet_shader_nodes(graph, desc);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_vol_absorption_shader(const VolAbsorbtionShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const vol_node = add_vol_absorption_shader_nodes(graph, desc);
	graph->connect(vol_node->output("Volume"), graph->output()->input("Volume"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_vol_add_shader(const AddShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const add_node = add_add_shader_nodes(graph, desc);
	graph->connect(add_node->output("Closure"), graph->output()->input("Volume"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_vol_mix_shader(const MixShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const mix_node = add_mix_shader_nodes(graph, desc);
	graph->connect(mix_node->output("Closure"), graph->output()->input("Volume"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_vol_scatter_shader(const VolScatterShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const vol_node = add_vol_scatter_shader_nodes(graph, desc);
	graph->connect(vol_node->output("Volume"), graph->output()->input("Volume"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_adsk_phys_shader(const AdskPhysicalMaterialDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const phys_node = add_adsk_phys_shader_nodes(graph, desc);
	graph->connect(phys_node->output("Closure"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->name = desc.name;
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

int MaxShaderManager::add_light_shader(const LightShaderDescriptor& desc)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const light_node = add_light_shader_nodes(graph, desc);
	graph->connect(light_node->output("Emission"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->set_graph(graph);
	shader->tag_update(scene);
	shader->has_surface_emission = true;

	return add_shader_to_scene(shader);
}

ccl::ShaderNode* MaxShaderManager::add_unsupported_shader_nodes(ccl::ShaderGraph* const graph)
{
	ccl::DiffuseBsdfNode* const diffuse_node = new ccl::DiffuseBsdfNode();
	diffuse_node->set_color(ccl::make_float3(0.0f, 0.0f, 0.0f));
	diffuse_node->set_roughness(0.0f);
	graph->add(diffuse_node);

	return diffuse_node;
}

ccl::ShaderNode* MaxShaderManager::add_holdout_shader_nodes(ccl::ShaderGraph* const graph)
{
	ccl::HoldoutNode* const holdout_node = new ccl::HoldoutNode();
	graph->add(holdout_node);

	return holdout_node;
}

ccl::ShaderNode* MaxShaderManager::add_add_shader_nodes(ccl::ShaderGraph* const graph, const AddShaderDescriptor& desc)
{
	*logger << "MaxShaderManager::add_add_shader_nodes called" << LogCtl::WRITE_LINE;

	ccl::ShaderNode* mat_a_node = nullptr;
	if (desc.mtl_a != nullptr) {
		mat_a_node = add_nodes_for_mtl(graph, desc.mtl_a);
	}

	ccl::ShaderNode* mat_b_node = nullptr;
	if (desc.mtl_b != nullptr) {
		mat_b_node = add_nodes_for_mtl(graph, desc.mtl_b);
	}

	ccl::AddClosureNode* const add_node = new ccl::AddClosureNode();
	graph->add(add_node);

	if (mat_a_node != nullptr) {
		graph->connect(get_closure_output(mat_a_node), add_node->input("Closure1"));
	}

	if (mat_b_node != nullptr) {
		graph->connect(get_closure_output(mat_b_node), add_node->input("Closure2"));
	}

	*logger << "MaxShaderManager::add_add_shader_nodes complete" << LogCtl::WRITE_LINE;

	return add_node;
}

ccl::ShaderNode* MaxShaderManager::add_anisotropic_shader_nodes(ccl::ShaderGraph* const graph, const AnisotropicShaderDescriptor& desc)
{
	ccl::AnisotropicBsdfNode* const bsdf_node = new ccl::AnisotropicBsdfNode();
	graph->add(bsdf_node);
	
	if (desc.distribution == AnisotropicDistribution::ASHIKHMIN_SHIRLEY) {
		bsdf_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_ASHIKHMIN_SHIRLEY_ID);
	}
	else if (desc.distribution == AnisotropicDistribution::MULTISCATTER_GGX) {
		bsdf_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_ID);
	}
	else if (desc.distribution == AnisotropicDistribution::BECKMANN) {
		bsdf_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_ID);
	}
	else {
		bsdf_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		bsdf_node->set_color(desc.color);
	}
	
	ccl::ShaderNode* roughness_map_node = nullptr;
	if (desc.roughness_map != nullptr) {
		roughness_map_node = texmap_cache.get_node_from_texmap(desc.roughness_map, scene);
		graph->add(roughness_map_node);
	}
	else {
		bsdf_node->set_roughness(desc.roughness);
	}

	ccl::ShaderNode* anisotropy_map_node = nullptr;
	if (desc.anisotropy_map != nullptr) {
		anisotropy_map_node = texmap_cache.get_node_from_texmap(desc.anisotropy_map, scene);
		graph->add(anisotropy_map_node);
	}
	else {
		bsdf_node->set_anisotropy(desc.anisotropy);
	}

	ccl::ShaderNode* rotation_map_node = nullptr;
	if (desc.rotation_map != nullptr) {
		rotation_map_node = texmap_cache.get_node_from_texmap(desc.rotation_map, scene);
		graph->add(rotation_map_node);
	}
	else {
		bsdf_node->set_rotation(desc.rotation);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}

	ccl::TangentNode* const tangent_node = new ccl::TangentNode();
	tangent_node->set_attribute(ccl::ustring{});
	tangent_node->set_direction_type(ccl::NodeTangentDirectionType::NODE_TANGENT_UVMAP);
	graph->add(tangent_node);

	graph->connect(tangent_node->output("Tangent"), bsdf_node->input("Tangent"));

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), bsdf_node->input("Color"));
	}
	if (roughness_map_node != nullptr) {
		graph->connect(roughness_map_node->output("Color"), bsdf_node->input("Roughness"));
	}
	if (anisotropy_map_node != nullptr) {
		graph->connect(anisotropy_map_node->output("Color"), bsdf_node->input("Anisotropy"));
	}
	if (rotation_map_node != nullptr) {
		graph->connect(rotation_map_node->output("Color"), bsdf_node->input("Rotation"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), bsdf_node->input("Normal"));
	}

	return bsdf_node;
}

ccl::ShaderNode* MaxShaderManager::add_diffuse_shader_nodes(ccl::ShaderGraph* const graph, const DiffuseShaderDescriptor& desc)
{
	*logger << "Adding shader nodes for diffuse mat..." << LogCtl::WRITE_LINE;

	ccl::DiffuseBsdfNode* const bsdf_node = new ccl::DiffuseBsdfNode();
	graph->add(bsdf_node);

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		bsdf_node->set_color(desc.color);
	}

	ccl::ShaderNode* roughness_map_node = nullptr;
	if (desc.roughness_map != nullptr) {
		roughness_map_node = texmap_cache.get_node_from_texmap(desc.roughness_map, scene);
		graph->add(roughness_map_node);
	}
	else {
		bsdf_node->set_roughness(desc.roughness);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		*logger << "Adding normal map nodes..." << LogCtl::WRITE_LINE;
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}

	*logger << "Adding connections..." << LogCtl::WRITE_LINE;

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), bsdf_node->input("Color"));
	}
	if (roughness_map_node != nullptr) {
		graph->connect(roughness_map_node->output("Color"), bsdf_node->input("Roughness"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), bsdf_node->input("Normal"));
	}

	*logger << "Completed adding diffuse shader nodes" << LogCtl::WRITE_LINE;

	return bsdf_node;
}

ccl::ShaderNode* MaxShaderManager::add_emission_shader_nodes(ccl::ShaderGraph* const graph, const EmissionShaderDescriptor& desc)
{
	ccl::EmissionNode* const emission_node = new ccl::EmissionNode();
	graph->add(emission_node);

	ccl::ShaderNode* light_color_node = nullptr;
	if (desc.color_map != nullptr) {
		light_color_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(light_color_node);
	}
	else {
		emission_node->set_color(desc.color);
	}

	ccl::ShaderNode* light_strength_node = nullptr;
	ccl::MathNode* multiply_node = nullptr;
	if (desc.strength_map != nullptr) {
		light_strength_node = texmap_cache.get_node_from_texmap(desc.strength_map, scene);
		graph->add(light_strength_node);

		multiply_node = new ccl::MathNode();
		multiply_node->set_math_type(ccl::NodeMathType::NODE_MATH_MULTIPLY);
		multiply_node->set_value2(desc.strength);
		graph->add(multiply_node);

		graph->connect(light_strength_node->output("Color"), multiply_node->input("Value1"));
	}
	else {
		emission_node->set_strength(desc.strength);
	}

	if (light_color_node != nullptr) {
		graph->connect(light_color_node->output("Color"), emission_node->input("Color"));
	}
	
	if (multiply_node != nullptr) {
		graph->connect(multiply_node->output("Value"), emission_node->input("Strength"));
	}

	return emission_node;
}

ccl::ShaderNode* MaxShaderManager::add_glass_shader_nodes(ccl::ShaderGraph* const graph, const GlassShaderDescriptor& desc)
{
	ccl::GlassBsdfNode* const glass_node = new ccl::GlassBsdfNode();
	graph->add(glass_node);

	if (desc.distribution == GlassDistribution::MULTISCATTER_GGX) {
		glass_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_GLASS_ID);
	}
	else if (desc.distribution == GlassDistribution::BECKMANN) {
		glass_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_GLASS_ID);
	}
	else {
		glass_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_GLASS_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		glass_node->set_color(desc.color);
	}

	ccl::ShaderNode* rough_map_node = nullptr;
	if (desc.roughness_map != nullptr) {
		rough_map_node = texmap_cache.get_node_from_texmap(desc.roughness_map, scene);
		graph->add(rough_map_node);
	}
	else {
		glass_node->set_roughness(desc.roughness);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}

	glass_node->set_IOR(desc.ior);

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), glass_node->input("Color"));
	}
	if (rough_map_node != nullptr) {
		graph->connect(rough_map_node->output("Color"), glass_node->input("Roughness"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), glass_node->input("Normal"));
	}

	return glass_node;
}

ccl::ShaderNode* MaxShaderManager::add_glossy_shader_nodes(ccl::ShaderGraph* const graph, const GlossyShaderDescriptor& desc)
{
	ccl::GlossyBsdfNode* const glossy_node = new ccl::GlossyBsdfNode();
	graph->add(glossy_node);

	if (desc.distribution == GlossyDistribution::ASHIKHMIN_SHIRLEY) {
		glossy_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_ASHIKHMIN_SHIRLEY_ID);
	}
	else if (desc.distribution == GlossyDistribution::MULTISCATTER_GGX) {
		glossy_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_ID);
	}
	else if (desc.distribution == GlossyDistribution::BECKMANN) {
		glossy_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_ID);
	}
	else {
		glossy_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		glossy_node->set_color(desc.color);
	}

	ccl::ShaderNode* rough_map_node = nullptr;
	if (desc.roughness_map != nullptr) {
		rough_map_node = texmap_cache.get_node_from_texmap(desc.roughness_map, scene);
		graph->add(rough_map_node);
	}
	else {
		glossy_node->set_roughness(desc.roughness);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}

	// Hook up any texmap nodes that we found
	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), glossy_node->input("Color"));
	}
	if (rough_map_node != nullptr) {
		graph->connect(rough_map_node->output("Color"), glossy_node->input("Roughness"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), glossy_node->input("Normal"));
	}

	return glossy_node;
}


ccl::ShaderNode* MaxShaderManager::add_hair_shader_nodes(ccl::ShaderGraph* const graph, const HairShaderDescriptor& desc)
{
	ccl::HairBsdfNode* const hair_node = new ccl::HairBsdfNode();
	graph->add(hair_node);

	if (desc.component == HairComponent::TRANSMISSION) {
		hair_node->set_component(ccl::ClosureType::CLOSURE_BSDF_HAIR_TRANSMISSION_ID);
	}
	else {
		hair_node->set_component(ccl::ClosureType::CLOSURE_BSDF_HAIR_REFLECTION_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		hair_node->set_color(desc.color);
	}

	// Offset node must be mapped from [0, 1] to [-90, 90]
	ccl::ShaderNode* offset_map_final_node = nullptr;
	if (desc.offset_map != nullptr) {
		ccl::ShaderNode* offset_map_node = texmap_cache.get_node_from_texmap(desc.offset_map, scene);
		graph->add(offset_map_node);

		ccl::SeparateRGBNode* separate_rgb_node = new ccl::SeparateRGBNode();
		graph->add(separate_rgb_node);

		ccl::MathNode* subtract_node = new ccl::MathNode();
		subtract_node->set_math_type(ccl::NodeMathType::NODE_MATH_SUBTRACT);
		subtract_node->set_value2(0.5f);
		graph->add(subtract_node);

		ccl::MathNode* multiply_node = new ccl::MathNode();
		multiply_node->set_math_type(ccl::NodeMathType::NODE_MATH_MULTIPLY);
		multiply_node->set_value2(180.0f);
		graph->add(multiply_node);

		graph->connect(offset_map_node->output("Color"), separate_rgb_node->input("Image"));
		graph->connect(separate_rgb_node->output("R"), subtract_node->input("Value1"));
		graph->connect(subtract_node->output("Value"), multiply_node->input("Value1"));

		offset_map_final_node = multiply_node;
	}
	else {
		hair_node->set_offset(desc.offset);
	}

	ccl::ShaderNode* roughness_u_map_node = nullptr;
	if (desc.roughness_u_map != nullptr) {
		roughness_u_map_node = texmap_cache.get_node_from_texmap(desc.roughness_u_map, scene);
		graph->add(roughness_u_map_node);
	}
	else {
		hair_node->set_roughness_u(desc.roughness_u);
	}

	ccl::ShaderNode* roughness_v_map_node = nullptr;
	if (desc.roughness_v_map != nullptr) {
		roughness_v_map_node = texmap_cache.get_node_from_texmap(desc.roughness_v_map, scene);
		graph->add(roughness_v_map_node);
	}
	else {
		hair_node->set_roughness_v(desc.roughness_v);
	}

	ccl::TangentNode* const tangent_node = new ccl::TangentNode();
	tangent_node->set_attribute(ccl::ustring{});
	tangent_node->set_direction_type(ccl::NodeTangentDirectionType::NODE_TANGENT_UVMAP);
	graph->add(tangent_node);

	graph->connect(tangent_node->output("Tangent"), hair_node->input("Tangent"));

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), hair_node->input("Color"));
	}
	if (offset_map_final_node != nullptr) {
		graph->connect(offset_map_final_node->output("Value"), hair_node->input("Offset"));
	}
	if (roughness_u_map_node != nullptr) {
		graph->connect(roughness_u_map_node->output("Color"), hair_node->input("RoughnessU"));
	}
	if (roughness_v_map_node != nullptr) {
		graph->connect(roughness_v_map_node->output("Color"), hair_node->input("RoughnessV"));
	}

	return hair_node;
}

ccl::ShaderNode* MaxShaderManager::add_mix_shader_nodes(ccl::ShaderGraph* const graph, const MixShaderDescriptor& desc)
{
	ccl::ShaderNode* fac_map_node = nullptr;

	if (desc.fac_map != nullptr) {
		fac_map_node = texmap_cache.get_node_from_texmap(desc.fac_map, scene);
		graph->add(fac_map_node);
	}

	ccl::ShaderNode* mat_a_node = nullptr;
	if (desc.mtl_a != nullptr) {
		mat_a_node = add_nodes_for_mtl(graph, desc.mtl_a);
	}

	ccl::ShaderNode* mat_b_node = nullptr;
	if (desc.mtl_b != nullptr) {
		mat_b_node = add_nodes_for_mtl(graph, desc.mtl_b);
	}

	ccl::LayerWeightNode* layer_weight_node = nullptr;
	if (desc.use_fresnel_blending) {
		layer_weight_node = new ccl::LayerWeightNode();
		layer_weight_node->set_blend(desc.fac);
		graph->add(layer_weight_node);
	}

	ccl::MixClosureNode* const mix_node = new ccl::MixClosureNode();
	graph->add(mix_node);

	if (layer_weight_node == nullptr) {
		mix_node->set_fac(desc.fac);
	}

	if (desc.use_fresnel_blending) {
		if (fac_map_node != nullptr) {
			graph->connect(fac_map_node->output("Color"), layer_weight_node->input("Blend"));
		}

		assert(layer_weight_node != nullptr);
		graph->connect(layer_weight_node->output("Fresnel"), mix_node->input("Fac"));
	}
	else {
		if (fac_map_node != nullptr) {
			graph->connect(fac_map_node->output("Color"), mix_node->input("Fac"));
		}
	}

	if (mat_a_node != nullptr) {
		graph->connect(get_closure_output(mat_a_node), mix_node->input("Closure1"));
	}

	if (mat_b_node != nullptr) {
		graph->connect(get_closure_output(mat_b_node), mix_node->input("Closure2"));
	}

	return mix_node;
}

ccl::ShaderNode* MaxShaderManager::add_principled_bsdf_shader_nodes(ccl::ShaderGraph* const graph, const PrincipledBSDFShaderDescriptor& desc)
{
	ccl::PrincipledBsdfNode* const principled_node = new ccl::PrincipledBsdfNode();
	graph->add(principled_node);

	if (desc.distribution == PrincipledBsdfDistribution::GGX) {
		principled_node->set_distribution(ccl::CLOSURE_BSDF_MICROFACET_GGX_GLASS_ID);
	}
	else {
		principled_node->set_distribution(ccl::CLOSURE_BSDF_MICROFACET_MULTI_GGX_GLASS_ID);
	}

	if (desc.subsurface_method == PrincipledBsdfSSSMethod::RANDOM_WALK) {
		principled_node->set_subsurface_method(ccl::CLOSURE_BSSRDF_PRINCIPLED_RANDOM_WALK_ID);
	}
	else {
		principled_node->set_subsurface_method(ccl::CLOSURE_BSSRDF_PRINCIPLED_ID);
	}

	principled_node->set_ior(desc.ior);
	principled_node->set_subsurface_radius(desc.subsurface_radius);

	if (add_node_and_connect_texmap(desc.base_color_map, graph, principled_node->input("Base Color"), texmap_cache, scene) == false) {
		principled_node->set_base_color(desc.base_color);
	}
	if (add_node_and_connect_texmap(desc.metallic_map, graph, principled_node->input("Metallic"), texmap_cache, scene) == false) {
		principled_node->set_metallic(desc.metallic);
	}
	if (add_node_and_connect_texmap(desc.roughness_map, graph, principled_node->input("Roughness"), texmap_cache, scene) == false) {
		principled_node->set_roughness(desc.roughness);
	}
	if (add_node_and_connect_texmap(desc.sheen_map, graph, principled_node->input("Sheen"), texmap_cache, scene) == false) {
		principled_node->set_sheen(desc.sheen);
	}
	if (add_node_and_connect_texmap(desc.sheen_tint_map, graph, principled_node->input("Sheen Tint"), texmap_cache, scene) == false) {
		principled_node->set_sheen_tint(desc.sheen_tint);
	}
	if (add_node_and_connect_texmap(desc.transmission_map, graph, principled_node->input("Transmission"), texmap_cache, scene) == false) {
		principled_node->set_transmission(desc.transmission);
	}
	if (add_node_and_connect_texmap(desc.transmission_roughness_map, graph, principled_node->input("Transmission Roughness"), texmap_cache, scene) == false) {
		principled_node->set_transmission_roughness(desc.transmission_roughness);
	}
	if (add_node_and_connect_texmap(desc.specular_map, graph, principled_node->input("Specular"), texmap_cache, scene) == false) {
		principled_node->set_specular(desc.specular);
	}
	if (add_node_and_connect_texmap(desc.specular_tint_map, graph, principled_node->input("Specular Tint"), texmap_cache, scene) == false) {
		principled_node->set_specular_tint(desc.specular_tint);
	}
	if (add_node_and_connect_texmap(desc.anisotropic_map, graph, principled_node->input("Anisotropic"), texmap_cache, scene) == false) {
		principled_node->set_anisotropic(desc.anisotropic);
	}
	if (add_node_and_connect_texmap(desc.anisotropic_rotation_map, graph, principled_node->input("Anisotropic Rotation"), texmap_cache, scene) == false) {
		principled_node->set_anisotropic_rotation(desc.anisotropic_rotation);
	}
	if (add_node_and_connect_texmap(desc.subsurface_map, graph, principled_node->input("Subsurface"), texmap_cache, scene) == false) {
		principled_node->set_subsurface(desc.subsurface);
	}
	if (add_node_and_connect_texmap(desc.subsurface_color_map, graph, principled_node->input("Subsurface Color"), texmap_cache, scene) == false) {
		principled_node->set_subsurface_color(desc.subsurface_color);
	}
	if (add_node_and_connect_texmap(desc.clearcoat_map, graph, principled_node->input("Clearcoat"), texmap_cache, scene) == false) {
		principled_node->set_clearcoat(desc.clearcoat);
	}
	if (add_node_and_connect_texmap(desc.clearcoat_roughness_map, graph, principled_node->input("Clearcoat Roughness"), texmap_cache, scene) == false) {
		principled_node->set_clearcoat_roughness(desc.clearcoat_roughness);
	}
	if (add_node_and_connect_texmap(desc.emission_color_map, graph, principled_node->input("Emission"), texmap_cache, scene) == false) {
		principled_node->set_emission(desc.emission_color);
	}
	if (add_node_and_connect_texmap(desc.alpha_map, graph, principled_node->input("Alpha"), texmap_cache, scene) == false) {
		principled_node->set_alpha(desc.alpha);
	}

	if (desc.normal_desc.enabled) {
		ccl::NormalMapNode* const normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
		graph->connect(normal_map_node->output("Normal"), principled_node->input("Normal"));
	}

	if (desc.cc_normal_desc.enabled) {
		ccl::NormalMapNode* const normal_map_node = add_normal_map_to_graph(graph, desc.cc_normal_desc);
		graph->connect(normal_map_node->output("Normal"), principled_node->input("Clearcoat Normal"));
	}

	ccl::TangentNode* const tangent_node = new ccl::TangentNode();
	tangent_node->set_attribute(ccl::ustring{});
	tangent_node->set_direction_type(ccl::NodeTangentDirectionType::NODE_TANGENT_UVMAP);
	graph->add(tangent_node);

	graph->connect(tangent_node->output("Tangent"), principled_node->input("Tangent"));

	return principled_node;
}

ccl::ShaderNode* MaxShaderManager::add_refraction_shader_nodes(ccl::ShaderGraph* const graph, const RefractionShaderDescriptor& desc)
{
	ccl::RefractionBsdfNode* const refraction_node = new ccl::RefractionBsdfNode();
	graph->add(refraction_node);

	if (desc.distribution == RefractionDistribution::BECKMANN) {
		refraction_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_REFRACTION_ID);
	}
	else {
		refraction_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_REFRACTION_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		refraction_node->set_color(desc.color);
	}

	ccl::ShaderNode* rough_map_node = nullptr;
	if (desc.roughness_map != nullptr) {
		rough_map_node = texmap_cache.get_node_from_texmap(desc.roughness_map, scene);
		graph->add(rough_map_node);
	}
	else {
		refraction_node->set_roughness(desc.roughness);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}

	refraction_node->set_IOR(desc.ior);

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), refraction_node->input("Color"));
	}
	if (rough_map_node != nullptr) {
		graph->connect(rough_map_node->output("Color"), refraction_node->input("Roughness"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), refraction_node->input("Normal"));
	}

	return refraction_node;
}

ccl::ShaderNode* MaxShaderManager::add_subsurface_scatter_shader_nodes(ccl::ShaderGraph* const graph, const SubsurfaceScatterShaderDescriptor& desc)
{
	ccl::SubsurfaceScatteringNode* const sss_node = new ccl::SubsurfaceScatteringNode();
	graph->add(sss_node);

	if (desc.falloff == SubsurfaceFalloff::GAUSSIAN) {
		sss_node->set_falloff(ccl::ClosureType::CLOSURE_BSSRDF_GAUSSIAN_ID);
	}
	else if (desc.falloff == SubsurfaceFalloff::CUBIC) {
		sss_node->set_falloff(ccl::ClosureType::CLOSURE_BSSRDF_CUBIC_ID);
	}
	else if (desc.falloff == SubsurfaceFalloff::RANDOM_WALK) {
		sss_node->set_falloff(ccl::ClosureType::CLOSURE_BSSRDF_RANDOM_WALK_ID);
	}
	else {
		sss_node->set_falloff(ccl::ClosureType::CLOSURE_BSSRDF_BURLEY_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		sss_node->set_color(desc.color);
	}

	ccl::ShaderNode* scale_map_node = nullptr;
	if (desc.scale_map != nullptr) {
		scale_map_node = texmap_cache.get_node_from_texmap(desc.scale_map, scene);
		graph->add(scale_map_node);
	}
	else {
		sss_node->set_scale(desc.scale);
	}

	sss_node->set_radius(desc.radius);

	ccl::ShaderNode* tex_blur_map_node = nullptr;
	if (desc.tex_blur_map != nullptr) {
		tex_blur_map_node = texmap_cache.get_node_from_texmap(desc.tex_blur_map, scene);
		graph->add(tex_blur_map_node);
	}
	else {
		sss_node->set_texture_blur(desc.tex_blur);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}
	
	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), sss_node->input("Color"));
	}
	if (scale_map_node != nullptr) {
		graph->connect(scale_map_node->output("Color"), sss_node->input("Scale"));
	}
	if (tex_blur_map_node != nullptr) {
		graph->connect(tex_blur_map_node->output("Color"), sss_node->input("Texture Blur"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), sss_node->input("Normal"));
	}

	return sss_node;
}

ccl::ShaderNode* MaxShaderManager::add_toon_shader_nodes(ccl::ShaderGraph* const graph, const ToonShaderDescriptor& desc)
{
	ccl::ToonBsdfNode* const toon_node = new ccl::ToonBsdfNode();
	graph->add(toon_node);

	if (desc.component == ToonComponent::GLOSSY) {
		toon_node->set_component(ccl::ClosureType::CLOSURE_BSDF_GLOSSY_TOON_ID);
	}
	else {
		toon_node->set_component(ccl::ClosureType::CLOSURE_BSDF_DIFFUSE_TOON_ID);
	}

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		toon_node->set_color(desc.color);
	}

	ccl::ShaderNode* size_map_node = nullptr;
	if (desc.size_map != nullptr) {
		size_map_node = texmap_cache.get_node_from_texmap(desc.size_map, scene);
		graph->add(size_map_node);
	}
	else {
		toon_node->set_size(desc.size);
	}

	ccl::ShaderNode* smooth_map_node = nullptr;
	if (desc.smooth_map != nullptr) {
		smooth_map_node = texmap_cache.get_node_from_texmap(desc.smooth_map, scene);
		graph->add(smooth_map_node);
	}
	else {
		toon_node->set_smooth(desc.smooth);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}
	
	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), toon_node->input("Color"));
	}
	if (size_map_node != nullptr) {
		graph->connect(size_map_node->output("Color"), toon_node->input("Size"));
	}
	if (smooth_map_node != nullptr) {
		graph->connect(smooth_map_node->output("Color"), toon_node->input("Smooth"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), toon_node->input("Normal"));
	}

	return toon_node;
}

ccl::ShaderNode* MaxShaderManager::add_translucent_shader_nodes(ccl::ShaderGraph* const graph, const TranslucentShaderDescriptor& desc)
{
	ccl::TranslucentBsdfNode* const translucent_node = new ccl::TranslucentBsdfNode();
	graph->add(translucent_node);

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		translucent_node->set_color(desc.color);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), translucent_node->input("Color"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), translucent_node->input("Normal"));
	}

	return translucent_node;
}

ccl::ShaderNode* MaxShaderManager::add_transparent_shader_nodes(ccl::ShaderGraph* const graph, const TransparentShaderDescriptor& desc)
{
	ccl::TransparentBsdfNode* const transparent_node = new ccl::TransparentBsdfNode();
	graph->add(transparent_node);

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		transparent_node->set_color(desc.color);
	}

	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), transparent_node->input("Color"));
	}

	return transparent_node;
}

ccl::ShaderNode* MaxShaderManager::add_velvet_shader_nodes(ccl::ShaderGraph* const graph, const VelvetShaderDescriptor& desc)
{
	ccl::VelvetBsdfNode* const velvet_node = new ccl::VelvetBsdfNode();
	graph->add(velvet_node);

	ccl::ShaderNode* color_map_node = nullptr;
	if (desc.color_map != nullptr) {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}
	else {
		velvet_node->set_color(desc.color);
	}

	ccl::ShaderNode* sigma_map_node = nullptr;
	if (desc.sigma_map != nullptr) {
		sigma_map_node = texmap_cache.get_node_from_texmap(desc.sigma_map, scene);
		graph->add(sigma_map_node);
	}
	else {
		velvet_node->set_sigma(desc.sigma);
	}

	ccl::NormalMapNode* normal_map_node = nullptr;
	if (desc.normal_desc.enabled) {
		normal_map_node = add_normal_map_to_graph(graph, desc.normal_desc);
	}
	
	if (color_map_node != nullptr) {
		graph->connect(color_map_node->output("Color"), velvet_node->input("Color"));
	}
	if (sigma_map_node != nullptr) {
		graph->connect(sigma_map_node->output("Color"), velvet_node->input("Sigma"));
	}
	if (normal_map_node != nullptr) {
		graph->connect(normal_map_node->output("Normal"), velvet_node->input("Normal"));
	}

	return velvet_node;
}

ccl::ShaderNode* MaxShaderManager::add_vol_absorption_shader_nodes(ccl::ShaderGraph* const graph, const VolAbsorbtionShaderDescriptor& desc)
{
	ccl::AbsorptionVolumeNode* const absorption_node = new ccl::AbsorptionVolumeNode();
	graph->add(absorption_node);

	absorption_node->set_color(desc.color);
	absorption_node->set_density(desc.density);

	return absorption_node;
}

ccl::ShaderNode* MaxShaderManager::add_vol_scatter_shader_nodes(ccl::ShaderGraph* const graph, const VolScatterShaderDescriptor& desc)
{
	ccl::ScatterVolumeNode* scatter_node = new ccl::ScatterVolumeNode();
	graph->add(scatter_node);

	scatter_node->set_color(desc.color);
	scatter_node->set_density(desc.density);
	scatter_node->set_anisotropy(desc.anisotropy);

	return scatter_node;
}

ccl::ShaderNode* MaxShaderManager::add_adsk_phys_shader_nodes(ccl::ShaderGraph* const graph, const AdskPhysicalMaterialDescriptor& desc)
{
	*logger << "converting autodesk physical shader..." << LogCtl::WRITE_LINE;

	static const float ROUGHNESS_CONVERSION_POWER = 2.05f;

	// Convert all present texmaps to nodes
	ccl::ImageTextureNode* base_color_map_node = nullptr;
	if (desc.base_color_map != nullptr) {
		base_color_map_node = texmap_cache.get_node_from_texmap(desc.base_color_map, scene);
		graph->add(base_color_map_node);
	}
	ccl::ImageTextureNode* base_roughness_map_node = nullptr;
	if (desc.base_roughness_map != nullptr) {
		base_roughness_map_node = texmap_cache.get_node_from_texmap(desc.base_roughness_map, scene);
		graph->add(base_roughness_map_node);
	}
	ccl::ImageTextureNode* base_weight_map_node = nullptr;
	if (desc.base_weight_map != nullptr) {
		base_weight_map_node = texmap_cache.get_node_from_texmap(desc.base_weight_map, scene);
		graph->add(base_weight_map_node);
	}
	ccl::ImageTextureNode* refl_color_map_node = nullptr;
	if (desc.refl_color_map != nullptr) {
		refl_color_map_node = texmap_cache.get_node_from_texmap(desc.refl_color_map, scene);
		graph->add(refl_color_map_node);
	}
	ccl::ImageTextureNode* refl_roughness_map_node = nullptr;
	if (desc.refl_roughness_map != nullptr) {
		refl_roughness_map_node = texmap_cache.get_node_from_texmap(desc.refl_roughness_map, scene);
		graph->add(refl_roughness_map_node);
	}
	ccl::ImageTextureNode* refl_ior_map_node = nullptr;
	if (desc.refl_ior_map != nullptr) {
		refl_ior_map_node = texmap_cache.get_node_from_texmap(desc.refl_ior_map, scene);
		graph->add(refl_ior_map_node);
	}
	ccl::ImageTextureNode* refl_weight_map_node = nullptr;
	if (desc.refl_weight_map != nullptr) {
		refl_weight_map_node = texmap_cache.get_node_from_texmap(desc.refl_weight_map, scene);
		graph->add(refl_weight_map_node);
	}
	ccl::ImageTextureNode* refl_metalness_map_node = nullptr;
	if (desc.refl_metalness_map != nullptr) {
		refl_metalness_map_node = texmap_cache.get_node_from_texmap(desc.refl_metalness_map, scene);
		graph->add(refl_metalness_map_node);
	}
	ccl::ImageTextureNode* trans_weight_map_node = nullptr;
	if (desc.trans_weight_map != nullptr) {
		trans_weight_map_node = texmap_cache.get_node_from_texmap(desc.trans_weight_map, scene);
		graph->add(trans_weight_map_node);
	}
	ccl::ImageTextureNode* trans_color_map_node = nullptr;
	if (desc.trans_color_map != nullptr) {
		trans_color_map_node = texmap_cache.get_node_from_texmap(desc.trans_color_map, scene);
		graph->add(trans_color_map_node);
	}
	ccl::ImageTextureNode* trans_roughness_map_node = nullptr;
	if (desc.trans_roughness_map != nullptr) {
		trans_roughness_map_node = texmap_cache.get_node_from_texmap(desc.trans_roughness_map, scene);
		graph->add(trans_roughness_map_node);
	}
	ccl::ImageTextureNode* cc_weight_map_node = nullptr;
	if (desc.cc_weight_map != nullptr) {
		cc_weight_map_node = texmap_cache.get_node_from_texmap(desc.cc_weight_map, scene);
		graph->add(cc_weight_map_node);
	}
	ccl::ImageTextureNode* cc_color_map_node = nullptr;
	if (desc.cc_color_map != nullptr) {
		cc_color_map_node = texmap_cache.get_node_from_texmap(desc.cc_color_map, scene);
		graph->add(cc_color_map_node);
	}
	ccl::ImageTextureNode* cc_roughness_map_node = nullptr;
	if (desc.cc_roughness_map != nullptr) {
		cc_roughness_map_node = texmap_cache.get_node_from_texmap(desc.cc_roughness_map, scene);
		graph->add(cc_roughness_map_node);
	}
	ccl::ImageTextureNode* emission_color_map_node = nullptr;
	if (desc.emission_color_map != nullptr) {
		emission_color_map_node = texmap_cache.get_node_from_texmap(desc.emission_color_map, scene);
		graph->add(emission_color_map_node);
	}
	ccl::ImageTextureNode* emission_weight_map_node = nullptr;
	if (desc.emission_weight_map != nullptr) {
		emission_weight_map_node = texmap_cache.get_node_from_texmap(desc.emission_weight_map, scene);
		graph->add(emission_weight_map_node);
	}
	ccl::ImageTextureNode* base_bump_map_node = nullptr;
	if (desc.base_bump_map != nullptr) {
		base_bump_map_node = texmap_cache.get_node_from_texmap(desc.base_bump_map, scene);
		graph->add(base_bump_map_node);
	}
	ccl::ImageTextureNode* cc_bump_map_node = nullptr;
	if (desc.cc_bump_map != nullptr) {
		cc_bump_map_node = texmap_cache.get_node_from_texmap(desc.cc_bump_map, scene);
		graph->add(cc_bump_map_node);
	}
	ccl::ImageTextureNode* cutout_map_node = nullptr;
	if (desc.cutout_map != nullptr) {
		cutout_map_node = texmap_cache.get_node_from_texmap(desc.cutout_map, scene);
		graph->add(cutout_map_node);
	}

	*logger << "making nodes..." << LogCtl::WRITE_LINE;

	// Create shader nodes

	// Common stuff
	ccl::MixNode* const base_weighted_color_node = new ccl::MixNode();
	base_weighted_color_node->set_mix_type(ccl::NodeMix::NODE_MIX_MUL);
	base_weighted_color_node->set_color1(ccl::make_float3(desc.base_weight, desc.base_weight, desc.base_weight));
	base_weighted_color_node->set_color2(desc.base_color);
	base_weighted_color_node->set_fac(1.0f);
	graph->add(base_weighted_color_node);
	
	ccl::MixNode* const base_color_with_cc_new_node = new ccl::MixNode();
	base_color_with_cc_new_node->set_mix_type(ccl::NodeMix::NODE_MIX_MUL);
	base_color_with_cc_new_node->set_color2(desc.cc_color);
	base_color_with_cc_new_node->set_fac(desc.cc_weight);
	graph->add(base_color_with_cc_new_node);

	// Base
	ccl::DiffuseBsdfNode* const base_color_diffuse_node = new ccl::DiffuseBsdfNode();
	base_color_diffuse_node->set_roughness(desc.base_roughness);
	graph->add(base_color_diffuse_node);

	// Nonmetal reflection
	ccl::InvertNode* const refl_roughness_inv_node = new ccl::InvertNode();
	refl_roughness_inv_node->set_color(ccl::make_float3(desc.refl_roughness, desc.refl_roughness, desc.refl_roughness));
	if (desc.refl_roughness_invert) {
		refl_roughness_inv_node->set_fac(1.0f);
	}
	else {
		refl_roughness_inv_node->set_fac(0.0f);
	}
	graph->add(refl_roughness_inv_node);

	ccl::MixNode* const refl_cc_color_node = new ccl::MixNode();
	refl_cc_color_node->set_mix_type(ccl::NodeMix::NODE_MIX_MUL);
	refl_cc_color_node->set_color1(desc.refl_color);
	refl_cc_color_node->set_color2(desc.cc_color);
	refl_cc_color_node->set_fac(1.0f);
	graph->add(refl_cc_color_node);

	ccl::MixNode* const refl_color_with_cc_node = new ccl::MixNode();
	refl_color_with_cc_node->set_mix_type(ccl::NodeMix::NODE_MIX_BLEND);
	refl_color_with_cc_node->set_color1(desc.refl_color);
	refl_color_with_cc_node->set_fac(desc.cc_weight);
	graph->add(refl_color_with_cc_node);

	ccl::GlossyBsdfNode* const refl_node = new ccl::GlossyBsdfNode();
	refl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_ID);
	graph->add(refl_node);

	ccl::FresnelNode* const refl_fresnel_node = new ccl::FresnelNode();
	refl_fresnel_node->set_IOR(desc.refl_ior);
	graph->add(refl_fresnel_node);

	ccl::MixClosureNode* const base_refl_mix_node = new ccl::MixClosureNode();
	graph->add(base_refl_mix_node);

	ccl::MathNode* const nonmetal_fac_node = new ccl::MathNode();
	nonmetal_fac_node->set_math_type(ccl::NODE_MATH_MULTIPLY);
	nonmetal_fac_node->set_value1(desc.refl_weight);
	graph->add(nonmetal_fac_node);

	// Metal Reflection
	ccl::MixNode* const metal_refl_color_node = new ccl::MixNode();
	metal_refl_color_node->set_mix_type(ccl::NodeMix::NODE_MIX_BLEND);
	graph->add(metal_refl_color_node);

	ccl::GlossyBsdfNode* const metal_refl_node = new ccl::GlossyBsdfNode();
	graph->add(metal_refl_node);

	ccl::MixClosureNode* const metal_mix_node = new ccl::MixClosureNode();
	metal_mix_node->set_fac(desc.refl_metalness);
	graph->add(metal_mix_node);

	// Transparency
	ccl::MixNode* const trans_cc_color_node = new ccl::MixNode();
	trans_cc_color_node->set_mix_type(ccl::NodeMix::NODE_MIX_MUL);
	trans_cc_color_node->set_color1(desc.trans_color);
	trans_cc_color_node->set_color2(desc.cc_color);
	trans_cc_color_node->set_fac(1.0f);
	graph->add(trans_cc_color_node);

	ccl::MixNode* const trans_color_with_cc_node = new ccl::MixNode();
	trans_color_with_cc_node->set_mix_type(ccl::NodeMix::NODE_MIX_BLEND);
	trans_color_with_cc_node->set_color1(desc.trans_color);
	trans_color_with_cc_node->set_fac(desc.cc_weight);
	graph->add(trans_color_with_cc_node);

	ccl::InvertNode* const trans_roughness_inv_node = new ccl::InvertNode();
	trans_roughness_inv_node->set_color(ccl::make_float3(desc.trans_roughness, desc.trans_roughness, desc.trans_roughness));
	if (desc.trans_roughness_invert) {
		trans_roughness_inv_node->set_fac(1.0f);
	}
	else {
		trans_roughness_inv_node->set_fac(0.0f);
	}
	graph->add(trans_roughness_inv_node);

	ccl::RefractionBsdfNode* const trans_node = new ccl::RefractionBsdfNode();
	trans_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_REFRACTION_ID);
	trans_node->set_IOR(desc.refl_ior);
	graph->add(trans_node);

	ccl::MixClosureNode* const trans_mix_node = new ccl::MixClosureNode();
	trans_mix_node->set_fac(desc.trans_weight);
	graph->add(trans_mix_node);

	// Clearcoat
	ccl::FresnelNode* const cc_mix_fresnel_node = new ccl::FresnelNode();
	cc_mix_fresnel_node->set_IOR(desc.cc_ior);
	graph->add(cc_mix_fresnel_node);

	ccl::MathNode* const cc_mix_weight_node = new ccl::MathNode();
	cc_mix_weight_node->set_math_type(ccl::NODE_MATH_MULTIPLY);
	cc_mix_weight_node->set_value1(desc.cc_weight);
	graph->add(cc_mix_weight_node);

	ccl::InvertNode* const cc_roughness_inv_node = new ccl::InvertNode();
	cc_roughness_inv_node->set_color(ccl::make_float3(desc.cc_roughness, desc.cc_roughness, desc.cc_roughness));
	if (desc.cc_roughness_invert) {
		cc_roughness_inv_node->set_fac(1.0f);
	}
	else {
		cc_roughness_inv_node->set_fac(0.0f);
	}
	graph->add(cc_roughness_inv_node);

	ccl::GlossyBsdfNode* const cc_refl_node = new ccl::GlossyBsdfNode();
	cc_refl_node->set_color(ccl::make_float3(1.0f, 1.0f, 1.0f));
	graph->add(cc_refl_node);

	ccl::MixClosureNode* const cc_mix_node = new ccl::MixClosureNode();
	graph->add(cc_mix_node);

	// Emission
	ccl::MixNode* const emission_color_mix_node = new ccl::MixNode();
	emission_color_mix_node->set_mix_type(ccl::NodeMix::NODE_MIX_MUL);
	emission_color_mix_node->set_fac(1.0f);
	emission_color_mix_node->set_color1(desc.emission_color);
	emission_color_mix_node->set_color2(rgb_from_temp(desc.emission_temp));
	graph->add(emission_color_mix_node);

	ccl::MathNode* const emission_strength_math_node = new ccl::MathNode();
	emission_strength_math_node->set_math_type(ccl::NodeMathType::NODE_MATH_MULTIPLY);
	emission_strength_math_node->set_value1(desc.emission_weight);
	{
		constexpr float DEFAULT_LUMINANCE_PHYSMAT = 1500.0f;
		constexpr float DEFAULT_LUMINANCE_CYCLES = 3.14159f;
		constexpr float PHYSMAT_TO_CYCLES = DEFAULT_LUMINANCE_CYCLES / DEFAULT_LUMINANCE_PHYSMAT;

		const float adjusted_luminance = desc.emission_luminance * PHYSMAT_TO_CYCLES;

		emission_strength_math_node->set_value2(adjusted_luminance);
	}
	graph->add(emission_strength_math_node);

	ccl::EmissionNode* const emission_node = new ccl::EmissionNode();
	graph->add(emission_node);

	ccl::AddClosureNode* const emission_add_node = new ccl::AddClosureNode();
	graph->add(emission_add_node);

	// Cutout
	ccl::TransparentBsdfNode* const transp_node = new ccl::TransparentBsdfNode();
	transp_node->set_color(ccl::make_float3(1.0f, 1.0f, 1.0f));
	graph->add(transp_node);

	ccl::MixClosureNode* const cutout_mix_node = new ccl::MixClosureNode();
	cutout_mix_node->set_fac(1.0f);
	graph->add(cutout_mix_node);

	// Bump maps
	constexpr float BUMP_SCALE_FACTOR = 0.3f;

	ccl::BumpNode* const base_bump_node = new ccl::BumpNode();
	base_bump_node->set_strength(0.8f);
	base_bump_node->set_distance(desc.base_bump_strength * BUMP_SCALE_FACTOR);
	graph->add(base_bump_node);

	ccl::BumpNode* const cc_bump_node = new ccl::BumpNode();
	cc_bump_node->set_strength(0.8f);
	cc_bump_node->set_distance(desc.cc_bump_strength * BUMP_SCALE_FACTOR);
	graph->add(cc_bump_node);

	*logger << "connecting texmaps..." << LogCtl::WRITE_LINE;

	// Connect all texmaps that exist
	if (base_color_map_node != nullptr) {
		graph->connect(base_color_map_node->output("Color"), base_weighted_color_node->input("Color2"));
	}
	if (base_roughness_map_node != nullptr) {
		graph->connect(base_roughness_map_node->output("Color"), base_color_diffuse_node->input("Roughness"));
	}
	if (base_weight_map_node != nullptr) {
		graph->connect(base_weight_map_node->output("Color"), base_weighted_color_node->input("Color1"));
	}
	if (refl_color_map_node != nullptr) {
		graph->connect(refl_color_map_node->output("Color"), refl_cc_color_node->input("Color1"));
		graph->connect(refl_color_map_node->output("Color"), refl_color_with_cc_node->input("Color1"));
	}
	if (refl_roughness_map_node != nullptr) {
		graph->connect(refl_roughness_map_node->output("Color"), refl_roughness_inv_node->input("Color"));
	}
	if (refl_ior_map_node != nullptr) {
		graph->connect(refl_ior_map_node->output("Color"), refl_fresnel_node->input("IOR"));
		graph->connect(refl_ior_map_node->output("Color"), trans_node->input("IOR"));
	}
	if (refl_weight_map_node != nullptr) {
		graph->connect(refl_weight_map_node->output("Color"), nonmetal_fac_node->input("Value1"));
	}
	if (refl_metalness_map_node != nullptr) {
		graph->connect(refl_metalness_map_node->output("Color"), metal_mix_node->input("Fac"));
	}
	if (trans_weight_map_node != nullptr) {
		graph->connect(trans_weight_map_node->output("Color"), trans_mix_node->input("Fac"));
	}
	if (trans_color_map_node != nullptr) {
		graph->connect(trans_color_map_node->output("Color"), trans_cc_color_node->input("Color1"));
		graph->connect(trans_color_map_node->output("Color"), trans_color_with_cc_node->input("Color1"));
	}
	if (trans_roughness_map_node != nullptr) {
		graph->connect(trans_roughness_map_node->output("Color"), trans_roughness_inv_node->input("Color"));
	}
	if (cc_weight_map_node != nullptr) {
		graph->connect(cc_weight_map_node->output("Color"), base_color_with_cc_new_node->input("Fac"));
		graph->connect(cc_weight_map_node->output("Color"), refl_color_with_cc_node->input("Fac"));
		graph->connect(cc_weight_map_node->output("Color"), trans_color_with_cc_node->input("Fac"));
		graph->connect(cc_weight_map_node->output("Color"), cc_mix_weight_node->input("Value1"));
	}
	if (cc_color_map_node != nullptr) {
		graph->connect(cc_color_map_node->output("Color"), base_color_with_cc_new_node->input("Color2"));
		graph->connect(cc_color_map_node->output("Color"), refl_cc_color_node->input("Color2"));
		graph->connect(cc_color_map_node->output("Color"), trans_cc_color_node->input("Color2"));
	}
	if (cc_roughness_map_node != nullptr) {
		graph->connect(cc_roughness_map_node->output("Color"), cc_roughness_inv_node->input("Color"));
	}
	if (emission_color_map_node != nullptr) {
		graph->connect(emission_color_map_node->output("Color"), emission_color_mix_node->input("Color1"));
	}
	if (emission_weight_map_node != nullptr) {
		graph->connect(emission_weight_map_node->output("Color"), emission_strength_math_node->input("Value1"));
	}
	if (cutout_map_node != nullptr) {
		graph->connect(cutout_map_node->output("Color"), cutout_mix_node->input("Fac"));
	}

	// Bump map
	if (base_bump_map_node != nullptr) {
		graph->connect(base_bump_map_node->output("Color"), base_bump_node->input("Height"));
		graph->connect(base_bump_node->output("Normal"), base_color_diffuse_node->input("Normal"));
		graph->connect(base_bump_node->output("Normal"), refl_fresnel_node->input("Normal"));
		graph->connect(base_bump_node->output("Normal"), refl_node->input("Normal"));
		graph->connect(base_bump_node->output("Normal"), metal_refl_node->input("Normal"));
		graph->connect(base_bump_node->output("Normal"), trans_node->input("Normal"));
	}
	if (cc_bump_map_node != nullptr) {
		graph->connect(cc_bump_map_node->output("Color"), cc_bump_node->input("Height"));
		graph->connect(cc_bump_node->output("Normal"), cc_mix_fresnel_node->input("Normal"));
		graph->connect(cc_bump_node->output("Normal"), cc_refl_node->input("Normal"));
	}

	// Connect all other nodes

	*logger << "connecting garbage..." << LogCtl::WRITE_LINE;

	// Base
	graph->connect(base_weighted_color_node->output("Color"), base_color_with_cc_new_node->input("Color1"));
	graph->connect(base_color_with_cc_new_node->output("Color"), base_color_diffuse_node->input("Color"));
	graph->connect(base_color_with_cc_new_node->output("Color"), metal_refl_color_node->input("Color1"));

	// Reflection
	graph->connect(refl_cc_color_node->output("Color"), refl_color_with_cc_node->input("Color2"));
	graph->connect(refl_color_with_cc_node->output("Color"), refl_node->input("Color"));
	graph->connect(refl_color_with_cc_node->output("Color"), metal_refl_color_node->input("Color2"));
	graph->connect(refl_roughness_inv_node->output("Color"), refl_node->input("Roughness"));
	graph->connect(base_color_diffuse_node->output("BSDF"), base_refl_mix_node->input("Closure1"));
	graph->connect(refl_node->output("BSDF"), base_refl_mix_node->input("Closure2"));
	graph->connect(refl_fresnel_node->output("Fac"), nonmetal_fac_node->input("Value2"));
	graph->connect(nonmetal_fac_node->output("Value"), base_refl_mix_node->input("Fac"));

	// Metal reflection
	graph->connect(refl_fresnel_node->output("Fac"), metal_refl_color_node->input("Fac"));
	graph->connect(metal_refl_color_node->output("Color"), metal_refl_node->input("Color"));
	graph->connect(refl_roughness_inv_node->output("Color"), metal_refl_node->input("Roughness"));
	graph->connect(base_refl_mix_node->output("Closure"), metal_mix_node->input("Closure1"));
	graph->connect(metal_refl_node->output("BSDF"), metal_mix_node->input("Closure2"));

	// Transparency
	graph->connect(metal_mix_node->output("Closure"), trans_mix_node->input("Closure1"));
	graph->connect(trans_roughness_inv_node->output("Color"), trans_node->input("Roughness"));
	graph->connect(trans_cc_color_node->output("Color"), trans_color_with_cc_node->input("Color2"));
	graph->connect(trans_color_with_cc_node->output("Color"), trans_node->input("Color"));
	graph->connect(trans_node->output("BSDF"), trans_mix_node->input("Closure2"));

	// Clearcoat
	graph->connect(trans_mix_node->output("Closure"), cc_mix_node->input("Closure1"));
	graph->connect(cc_mix_fresnel_node->output("Fac"), cc_mix_weight_node->input("Value2"));
	graph->connect(cc_mix_weight_node->output("Value"), cc_mix_node->input("Fac"));
	graph->connect(cc_roughness_inv_node->output("Color"), cc_refl_node->input("Roughness"));
	graph->connect(cc_refl_node->output("BSDF"), cc_mix_node->input("Closure2"));

	// Emission
	graph->connect(cc_mix_node->output("Closure"), emission_add_node->input("Closure1"));
	graph->connect(emission_color_mix_node->output("Color"), emission_node->input("Color"));
	graph->connect(emission_strength_math_node->output("Value"), emission_node->input("Strength"));
	graph->connect(emission_node->output("Emission"), emission_add_node->input("Closure2"));

	// Cutout
	graph->connect(transp_node->output("BSDF"), cutout_mix_node->input("Closure1"));
	graph->connect(emission_add_node->output("Closure"), cutout_mix_node->input("Closure2"));

	*logger << "done" << LogCtl::WRITE_LINE;

	return cutout_mix_node;
}

int MaxShaderManager::add_simple_color_shader(const ccl::float3 color)
{
	ccl::ShaderGraph* const graph = new ccl::ShaderGraph();
	ccl::ShaderNode* const bsdf_node = add_simple_color_shader_nodes(graph, color);
	graph->connect(bsdf_node->output("BSDF"), graph->output()->input("Surface"));

	ccl::Shader* const shader = new ccl::Shader();
	shader->set_graph(graph);
	shader->tag_update(scene);

	return add_shader_to_scene(shader);
}

ccl::ShaderNode* MaxShaderManager::add_simple_color_shader_nodes(ccl::ShaderGraph* const graph, const ccl::float3 color)
{
	ccl::DiffuseBsdfNode* const diffuse_node = new ccl::DiffuseBsdfNode();
	diffuse_node->set_color(color);
	diffuse_node->set_roughness(0.1f);
	graph->add(diffuse_node);

	return diffuse_node;
}

ccl::ShaderNode* MaxShaderManager::add_light_shader_nodes(ccl::ShaderGraph* const graph, const LightShaderDescriptor& desc)
{
	ccl::EmissionNode* const emission_node = new ccl::EmissionNode();
	emission_node->set_color(desc.color);
	emission_node->set_strength(desc.intensity);
	graph->add(emission_node);

	return emission_node;
}

ccl::ShaderNode* MaxShaderManager::add_nodes_for_mtl(ccl::ShaderGraph* const graph, Mtl* const mtl)
{
	Class_ID mtl_class = mtl->ClassID();

	if (mtl_class == CYCLES_MAT_ADD_CLASS) {
		AddShaderDescriptor desc(dynamic_cast<CyclesAddMat*>(mtl), frame_time);
		return add_add_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_ANISOTROPIC_CLASS) {
		AnisotropicShaderDescriptor desc(dynamic_cast<CyclesAnisotropicMat*>(mtl), frame_time);
		return add_anisotropic_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_DIFFUSE_CLASS) {
		DiffuseShaderDescriptor desc(dynamic_cast<CyclesDiffuseMat*>(mtl), frame_time);
		return add_diffuse_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_EMISSION_CLASS) {
		EmissionShaderDescriptor desc(dynamic_cast<CyclesEmissionMat*>(mtl), frame_time);
		return add_emission_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_GLASS_CLASS) {
		GlassShaderDescriptor desc(dynamic_cast<CyclesGlassMat*>(mtl), frame_time);
		return add_glass_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_GLOSSY_CLASS) {
		GlossyShaderDescriptor desc(dynamic_cast<CyclesGlossyMat*>(mtl), frame_time);
		return add_glossy_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_HOLDOUT_CLASS) {
		return add_holdout_shader_nodes(graph);
	}
	else if (mtl_class == CYCLES_MAT_MIX_CLASS) {
		MixShaderDescriptor desc(dynamic_cast<CyclesMixMat*>(mtl), frame_time);
		return add_mix_shader_nodes(graph, desc);
	}
	else if (mtl->ClassID() == CYCLES_MAT_PRINCIPLED_CLASS) {
		PrincipledBSDFShaderDescriptor desc(dynamic_cast<CyclesPrincipledBSDFMat*>(mtl), frame_time);
		return add_principled_bsdf_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_REFRACTION_CLASS) {
		RefractionShaderDescriptor desc(dynamic_cast<CyclesRefractionMat*>(mtl), frame_time);
		return add_refraction_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_SUBSURFACE_CLASS) {
		SubsurfaceScatterShaderDescriptor desc(dynamic_cast<CyclesSubsurfaceScatterMat*>(mtl), frame_time);
		return add_subsurface_scatter_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_TOON_CLASS) {
		ToonShaderDescriptor desc(dynamic_cast<CyclesToonMat*>(mtl), frame_time);
		return add_toon_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_TRANSLUCENT_CLASS) {
		TranslucentShaderDescriptor desc(dynamic_cast<CyclesTranslucentMat*>(mtl), frame_time);
		return add_translucent_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_TRANSPARENT_CLASS) {
		TransparentShaderDescriptor desc(dynamic_cast<CyclesTransparentMat*>(mtl), frame_time);
		return add_transparent_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_VELVET_CLASS) {
		VelvetShaderDescriptor desc(dynamic_cast<CyclesVelvetMat*>(mtl), frame_time);
		return add_velvet_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_VOL_ABSORPTION_CLASS) {
		VolAbsorbtionShaderDescriptor desc(dynamic_cast<CyclesVolAbsorptionMat*>(mtl), frame_time);
		return add_vol_absorption_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_VOL_ADD_CLASS) {
		AddShaderDescriptor desc(dynamic_cast<CyclesVolAddMat*>(mtl), frame_time);
		return add_add_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_VOL_MIX_CLASS) {
		MixShaderDescriptor desc(dynamic_cast<CyclesVolMixMat*>(mtl), frame_time);
		return add_mix_shader_nodes(graph, desc);
	}
	else if (mtl_class == CYCLES_MAT_VOL_SCATTER_CLASS) {
		VolScatterShaderDescriptor desc(dynamic_cast<CyclesVolScatterMat*>(mtl), frame_time);
		return add_vol_scatter_shader_nodes(graph, desc);
	}
	else if (mtl->ClassID() == PHYSICAL_MATERIAL_CLASS) {
		AdskPhysicalMaterialDescriptor desc(mtl, frame_time);
		return add_adsk_phys_shader_nodes(graph, desc);
	}
	else {
		return add_unsupported_shader_nodes(graph);
	}
}

ccl::NormalMapNode* MaxShaderManager::add_normal_map_to_graph(ccl::ShaderGraph* const graph, const NormalMapDescriptor& desc)
{
	ccl::NormalMapNode* normal_map_node = new ccl::NormalMapNode();
	graph->add(normal_map_node);

	if (desc.space == NormalMapTangentSpace::OBJECT) {
		normal_map_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_OBJECT);
	}
	else if (desc.space == NormalMapTangentSpace::WORLD) {
		normal_map_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_WORLD);
	}
	else {
		normal_map_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_TANGENT);
	}

	normal_map_node->set_attribute(ccl::ustring());
	
	ccl::ImageTextureNode* strength_map_node = nullptr;
	if (desc.strength_map == nullptr) {
		normal_map_node->set_strength(desc.strength);
	}
	else {
		strength_map_node = texmap_cache.get_node_from_texmap(desc.strength_map, scene);
		graph->add(strength_map_node);
	}

	ccl::ImageTextureNode* color_map_node = nullptr;
	if (desc.color_map == nullptr) {
		normal_map_node->set_color(desc.color);
	}
	else {
		color_map_node = texmap_cache.get_node_from_texmap(desc.color_map, scene);
		graph->add(color_map_node);
	}

	if (strength_map_node != nullptr) {
		ccl::MathNode* math_node = new ccl::MathNode();
		math_node->set_math_type(ccl::NodeMathType::NODE_MATH_MULTIPLY);
		math_node->set_value2(desc.strength);
		graph->add(math_node);

		graph->connect(strength_map_node->output("Color"), math_node->input("Value1"));
		graph->connect(math_node->output("Value"), normal_map_node->input("Strength"));
	}
	if (color_map_node != nullptr) {
		if (desc.invert_green) {
			ccl::SeparateRGBNode* separate_rgb_node = new ccl::SeparateRGBNode();
			graph->add(separate_rgb_node);
			ccl::InvertNode* invert_node = new ccl::InvertNode();
			graph->add(invert_node);
			ccl::CombineRGBNode* combine_rgb_node = new ccl::CombineRGBNode();
			graph->add(combine_rgb_node);

			graph->connect(color_map_node->output("Color"), separate_rgb_node->input("Image"));
			graph->connect(separate_rgb_node->output("R"), combine_rgb_node->input("R"));
			graph->connect(separate_rgb_node->output("B"), combine_rgb_node->input("B"));
			graph->connect(separate_rgb_node->output("G"), invert_node->input("Color"));
			graph->connect(invert_node->output("Color"), combine_rgb_node->input("G"));
			graph->connect(combine_rgb_node->output("Image"), normal_map_node->input("Color"));
		}
		else {
			graph->connect(color_map_node->output("Color"), normal_map_node->input("Color"));
		}
	}

	return normal_map_node;
}

int MaxShaderManager::add_shader_to_scene(ccl::Shader* const new_shader)
{
	const int result = static_cast<int>(scene->shaders.size());
	scene->shaders.push_back(new_shader);
	return result;
}
