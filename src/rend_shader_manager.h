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
 * @brief Defines the class MaxShaderManager.
 */

#include <map>
#include <memory>

#include <kernel/svm/svm_types.h>

#include "rend_logger.h"
#include "rend_shader_desc.h"
#include "util_multi_shader_max.h"

class BakedTexmapCache;

namespace ccl {
	class NormalMapNode;
	class Scene;
	class Shader;
	class ShaderGraph;
	class ShaderNode;
}

/**
 * @brief Class responsible for creating Cycles shaders from Max materials.
 */
class MaxShaderManager {
public:
	MaxShaderManager(BakedTexmapCache& texmap_cache, ccl::Scene* scene, TimeValue t);
	~MaxShaderManager();

	ccl::Shader* get_ccl_shader(int index) const;

	void set_mis_map_size(int size);

	void set_sky_from_env_map(Texmap* env_map, float strength);
	void set_simple_sky_shader(const SimpleSkyDescriptor& in);
	void set_bitmap_sky_shader(
		float strength,
		Texmap* bg_map,
		EnvironmentProjection bg_proj = EnvironmentProjection::EQUIRECTANGULAR,
		Texmap* light_map = nullptr,
		EnvironmentProjection light_proj = EnvironmentProjection::EQUIRECTANGULAR
	);

	MaxMultiShaderHelper get_mtl_multishader(Mtl* mtl);
	int get_mtl_shader(Mtl* mtl);

	int get_unsupported_shader();
	int get_holdout_shader();

	int get_shader_graph_shader(const ShaderGraphShaderDescriptor& desc);
	int get_shader_shader(const ShaderShaderDescriptor& desc);

	int get_add_shader(const AddShaderDescriptor& desc);
	int get_anisotropic_shader(const AnisotropicShaderDescriptor& desc);
	int get_diffuse_shader(const DiffuseShaderDescriptor& desc);
	int get_emission_shader(const EmissionShaderDescriptor& desc);
	int get_glass_shader(const GlassShaderDescriptor& desc);
	int get_glossy_shader(const GlossyShaderDescriptor& desc);
	int get_hair_shader(const HairShaderDescriptor& desc);
	int get_mix_shader(const MixShaderDescriptor& desc);
	int get_principled_bsdf_shader(const PrincipledBSDFShaderDescriptor& desc);
	int get_refraction_shader(const RefractionShaderDescriptor& desc);
	int get_subsurface_scatter_shader(const SubsurfaceScatterShaderDescriptor& desc);
	int get_toon_shader(const ToonShaderDescriptor& desc);
	int get_translucent_shader(const TranslucentShaderDescriptor& desc);
	int get_transparent_shader(const TransparentShaderDescriptor& desc);
	int get_velvet_shader(const VelvetShaderDescriptor& desc);
	int get_vol_absorption_shader(const VolAbsorbtionShaderDescriptor& desc);
	int get_vol_add_shader(const AddShaderDescriptor& desc);
	int get_vol_mix_shader(const MixShaderDescriptor& desc);
	int get_vol_scatter_shader(const VolScatterShaderDescriptor& desc);

	int get_adsk_phys_shader(const AdskPhysicalMaterialDescriptor& desc);

	int get_simple_color_shader(ccl::float3 color);

	int get_light_shader(ccl::float3 color, float intensity);

	void log_shader_stats() const;

private:
	ccl::Scene* const scene;
	BakedTexmapCache& texmap_cache;

	TimeValue frame_time;

	int mis_map_size = 2048;

	// Variables to track existing shaders
	int unsupported_shader_index;
	int holdout_shader_index;
	std::map<ShaderGraphShaderDescriptor, int> shader_graph_shaders;
	std::map<ShaderShaderDescriptor, int> shader_shaders;
	std::map<AddShaderDescriptor, int> add_shaders;
	std::map<AnisotropicShaderDescriptor, int> anisotropic_shaders;
	std::map<DiffuseShaderDescriptor, int> diffuse_shaders;
	std::map<EmissionShaderDescriptor, int> emission_shaders;
	std::map<GlassShaderDescriptor, int> glass_shaders;
	std::map<GlossyShaderDescriptor, int> glossy_shaders;
	std::map<HairShaderDescriptor, int> hair_shaders;
	std::map<MixShaderDescriptor, int> mix_shaders;
	std::map<PrincipledBSDFShaderDescriptor, int> principled_bsdf_shaders;
	std::map<RefractionShaderDescriptor, int> refraction_shaders;
	std::map<SubsurfaceScatterShaderDescriptor, int> subsurface_scatter_shaders;
	std::map<ToonShaderDescriptor, int> toon_shaders;
	std::map<TranslucentShaderDescriptor, int> translucent_shaders;
	std::map<TransparentShaderDescriptor, int> transparent_shaders;
	std::map<VelvetShaderDescriptor, int> velvet_shaders;
	std::map<VolAbsorbtionShaderDescriptor, int> vol_absorption_shaders;
	std::map<AddShaderDescriptor, int> vol_add_shaders;
	std::map<MixShaderDescriptor, int> vol_mix_shaders;
	std::map<VolScatterShaderDescriptor, int> vol_scatter_shaders;

	std::map<AdskPhysicalMaterialDescriptor, int> adsk_phys_shaders;

	std::map<ComparableColor, int> simple_color_shaders;

	std::map<LightShaderDescriptor, int> light_shaders;

	// Each type of shader has 2 functions for creation
	// add_[type]_shader creates a shader and adds it to the scene
	// add_[type]_shader_nodes adds the nodes needed for the shader to the given graph and returns the output node
	// The second family of functions is used for building composite shaders
	int add_unsupported_shader();
	int add_holdout_shader();
	int add_shader_graph_shader(const ShaderGraphShaderDescriptor& desc);
	int add_shader_shader(const ShaderShaderDescriptor& desc);
	int add_add_shader(const AddShaderDescriptor& desc);
	int add_anisotropic_shader(const AnisotropicShaderDescriptor& desc);
	int add_diffuse_shader(const DiffuseShaderDescriptor& desc);
	int add_emission_shader(const EmissionShaderDescriptor& desc);
	int add_glass_shader(const GlassShaderDescriptor& desc);
	int add_glossy_shader(const GlossyShaderDescriptor& desc);
	int add_hair_shader(const HairShaderDescriptor& desc);
	int add_mix_shader(const MixShaderDescriptor& desc);
	int add_principled_bsdf_shader(const PrincipledBSDFShaderDescriptor& desc);
	int add_refraction_shader(const RefractionShaderDescriptor& desc);
	int add_subsurface_scatter_shader(const SubsurfaceScatterShaderDescriptor& desc);
	int add_toon_shader(const ToonShaderDescriptor& desc);
	int add_translucent_shader(const TranslucentShaderDescriptor& desc);
	int add_transparent_shader(const TransparentShaderDescriptor& desc);
	int add_velvet_shader(const VelvetShaderDescriptor& desc);
	int add_vol_absorption_shader(const VolAbsorbtionShaderDescriptor& desc);
	int add_vol_add_shader(const AddShaderDescriptor& desc);
	int add_vol_mix_shader(const MixShaderDescriptor& desc);
	int add_vol_scatter_shader(const VolScatterShaderDescriptor& desc);
	int add_adsk_phys_shader(const AdskPhysicalMaterialDescriptor& desc);

	int add_light_shader(const LightShaderDescriptor& desc);

	ccl::ShaderNode* add_unsupported_shader_nodes(ccl::ShaderGraph* graph);
	ccl::ShaderNode* add_holdout_shader_nodes(ccl::ShaderGraph* graph);
	ccl::ShaderNode* add_add_shader_nodes(ccl::ShaderGraph* graph, const AddShaderDescriptor& desc);
	ccl::ShaderNode* add_anisotropic_shader_nodes(ccl::ShaderGraph* graph, const AnisotropicShaderDescriptor& desc);
	ccl::ShaderNode* add_diffuse_shader_nodes(ccl::ShaderGraph* graph, const DiffuseShaderDescriptor& desc);
	ccl::ShaderNode* add_emission_shader_nodes(ccl::ShaderGraph* graph, const EmissionShaderDescriptor& desc);
	ccl::ShaderNode* add_glass_shader_nodes(ccl::ShaderGraph* graph, const GlassShaderDescriptor& desc);
	ccl::ShaderNode* add_glossy_shader_nodes(ccl::ShaderGraph* graph, const GlossyShaderDescriptor& desc);
	ccl::ShaderNode* add_hair_shader_nodes(ccl::ShaderGraph* graph, const HairShaderDescriptor& desc);
	ccl::ShaderNode* add_mix_shader_nodes(ccl::ShaderGraph* graph, const MixShaderDescriptor& desc);
	ccl::ShaderNode* add_principled_bsdf_shader_nodes(ccl::ShaderGraph* graph, const PrincipledBSDFShaderDescriptor& desc);
	ccl::ShaderNode* add_refraction_shader_nodes(ccl::ShaderGraph* graph, const RefractionShaderDescriptor& desc);
	ccl::ShaderNode* add_subsurface_scatter_shader_nodes(ccl::ShaderGraph* graph, const SubsurfaceScatterShaderDescriptor& desc);
	ccl::ShaderNode* add_toon_shader_nodes(ccl::ShaderGraph* graph, const ToonShaderDescriptor& desc);
	ccl::ShaderNode* add_translucent_shader_nodes(ccl::ShaderGraph* graph, const TranslucentShaderDescriptor& desc);
	ccl::ShaderNode* add_transparent_shader_nodes(ccl::ShaderGraph* graph, const TransparentShaderDescriptor& desc);
	ccl::ShaderNode* add_velvet_shader_nodes(ccl::ShaderGraph* graph, const VelvetShaderDescriptor& desc);
	ccl::ShaderNode* add_vol_absorption_shader_nodes(ccl::ShaderGraph* graph, const VolAbsorbtionShaderDescriptor& desc);
	ccl::ShaderNode* add_vol_scatter_shader_nodes(ccl::ShaderGraph* graph, const VolScatterShaderDescriptor& desc);
	ccl::ShaderNode* add_adsk_phys_shader_nodes(ccl::ShaderGraph* graph, const AdskPhysicalMaterialDescriptor& desc);

	int add_simple_color_shader(ccl::float3 color);
	ccl::ShaderNode* add_simple_color_shader_nodes(ccl::ShaderGraph* graph, ccl::float3 color);

	ccl::ShaderNode* add_light_shader_nodes(ccl::ShaderGraph* graph, const LightShaderDescriptor& desc);

	ccl::ShaderNode* add_nodes_for_mtl(ccl::ShaderGraph* graph, Mtl* mtl);

	ccl::NormalMapNode* add_normal_map_to_graph(ccl::ShaderGraph* graph, const NormalMapDescriptor& desc);

	int add_shader_to_scene(ccl::Shader* new_shader);

	const std::unique_ptr<LoggerInterface> logger;
};
