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
 * @brief Defines various shader descriptors that can be used to build Cycles shaders.
 */

#include <string>
#include <vector>

#include <util/util_types.h>

#include <maxtypes.h>

#include "util_enums.h"

class CyclesAddMat;
class CyclesAnisotropicMat;
class CyclesDiffuseMat;
class CyclesEmissionMat;
class CyclesGlassMat;
class CyclesGlossyMat;
class CyclesHairMat;
class CyclesMixMat;
class CyclesPrincipledBSDFMat;
class CyclesRefractionMat;
class CyclesShaderGraphMat08;
class CyclesShaderGraphMat16;
class CyclesShaderGraphMat32;
class CyclesShaderMat;
class CyclesSubsurfaceScatterMat;
class CyclesToonMat;
class CyclesTranslucentMat;
class CyclesTransparentMat;
class CyclesVelvetMat;
class CyclesVolAbsorptionMat;
class CyclesVolAddMat;
class CyclesVolMixMat;
class CyclesVolScatterMat;
class MaxRenderManager;

class Mtl;
class Texmap;

class AdskPhysicalMaterialDescriptor {
public:
	std::string name;

	float base_weight = 0.0f;
	Texmap* base_weight_map = nullptr;
	ccl::float3 base_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
	Texmap* base_color_map = nullptr;
	float base_roughness = 0.0f;
	Texmap* base_roughness_map = nullptr;

	float refl_weight = 0.0f;
	Texmap* refl_weight_map = nullptr;
	ccl::float3 refl_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
	Texmap* refl_color_map = nullptr;
	float refl_ior = 0.0f;
	Texmap* refl_ior_map = nullptr;
	bool refl_roughness_invert = false;
	float refl_roughness = 0.0f;
	Texmap* refl_roughness_map = nullptr;
	float refl_metalness = 0.0f;
	Texmap* refl_metalness_map = nullptr;

	float trans_weight = 0.0f;
	Texmap* trans_weight_map = nullptr;
	ccl::float3 trans_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
	Texmap* trans_color_map = nullptr;
	bool trans_roughness_invert = false;
	float trans_roughness = 0.0f;
	Texmap* trans_roughness_map = nullptr;

	float cc_weight = 0.0f;
	Texmap* cc_weight_map = nullptr;
	ccl::float3 cc_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
	Texmap* cc_color_map = nullptr;
	float cc_ior = 0.0f;
	bool cc_roughness_invert = false;
	float cc_roughness = 0.0f;
	Texmap* cc_roughness_map = nullptr;
	
	float emission_weight = 0.0f;
	Texmap* emission_weight_map = nullptr;
	ccl::float3 emission_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
	Texmap* emission_color_map = nullptr;
	float emission_luminance = 0.0f;
	float emission_temp = 6500.0f;

	float base_bump_strength = 0.0f;
	Texmap* base_bump_map = nullptr;
	float cc_bump_strength = 0.0f;
	Texmap* cc_bump_map = nullptr;

	Texmap* cutout_map = nullptr;

	AdskPhysicalMaterialDescriptor(Mtl* mtl, TimeValue t);

	bool operator<(const AdskPhysicalMaterialDescriptor& other) const;
};

class NormalMapDescriptor {
public:
	bool enabled = false;
	NormalMapTangentSpace space = NormalMapTangentSpace::TANGENT;
	float strength = 0.0f;
	Texmap* strength_map = nullptr;
	ccl::float3 color = ccl::make_float3(0.0f);
	Texmap* color_map = nullptr;
	bool invert_green = false;

	bool operator<(const NormalMapDescriptor& other) const;
};

class ShaderParamsDescriptor {
public:
	bool use_mis = true;
	DisplacementMethod displacement = DisplacementMethod::BUMP_MAP;
	VolumeSamplingMethod vol_sampling = VolumeSamplingMethod::MULTIPLE_IMPORTANCE;
	VolumeInterpolationMethod vol_interp = VolumeInterpolationMethod::LINEAR;
	bool vol_is_homogeneous = false;

	bool operator<(const ShaderParamsDescriptor& other) const;
};

class ShaderGraphShaderDescriptor {
public:
	std::string name;

	ShaderParamsDescriptor shader_params;
	std::string encoded_graph;

	std::vector<Texmap*> texmaps;

	ShaderGraphShaderDescriptor(CyclesShaderGraphMat08* mtl, TimeValue t);
	ShaderGraphShaderDescriptor(CyclesShaderGraphMat16* mtl, TimeValue t);
	ShaderGraphShaderDescriptor(CyclesShaderGraphMat32* mtl, TimeValue t);

	void clear_array();

	bool operator<(const ShaderGraphShaderDescriptor& other) const;
};

class ShaderShaderDescriptor {
public:
	std::string name;

	ShaderParamsDescriptor shader_params;

	Mtl* surface_mtl = nullptr;
	Mtl* volume_mtl = nullptr;

	ShaderShaderDescriptor(CyclesShaderMat* mtl, TimeValue t);

	bool operator<(const ShaderShaderDescriptor& other) const;
};

class AddShaderDescriptor {
public:
	std::string name;

	Mtl* mtl_a = nullptr;
	Mtl* mtl_b = nullptr;

	AddShaderDescriptor(CyclesAddMat* mtl, TimeValue t);
	AddShaderDescriptor(CyclesVolAddMat* mtl, TimeValue t);

	bool operator<(const AddShaderDescriptor& other) const;
};

class AnisotropicShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	AnisotropicDistribution distribution;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float roughness;
	Texmap* roughness_map = nullptr;
	float anisotropy;
	Texmap* anisotropy_map = nullptr;
	float rotation;
	Texmap* rotation_map = nullptr;

	AnisotropicShaderDescriptor(CyclesAnisotropicMat* mtl, TimeValue t);

	bool operator<(const AnisotropicShaderDescriptor& other) const;
};

class DiffuseShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	ccl::float3 color;
	Texmap* color_map = nullptr;
	float roughness;
	Texmap* roughness_map = nullptr;
	
	DiffuseShaderDescriptor(float roughness_in, ccl::float3 color_in);
	DiffuseShaderDescriptor(CyclesDiffuseMat* mtl, TimeValue t);
	
	bool operator<(const DiffuseShaderDescriptor& other) const;
};

class EmissionShaderDescriptor {
public:
	std::string name;

	ccl::float3 color;
	Texmap* color_map = nullptr;
	float strength;
	Texmap* strength_map = nullptr;

	EmissionShaderDescriptor() : color(ccl::make_float3(0.0f, 0.0f, 0.0f)), color_map(nullptr), strength(1.0f), strength_map(nullptr) {} // Used internally to set up material preview lights
	EmissionShaderDescriptor(CyclesEmissionMat* mtl, TimeValue t);

	bool operator<(const EmissionShaderDescriptor& other) const;
};

class GlassShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	GlassDistribution distribution;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float roughness;
	Texmap* roughness_map = nullptr;
	float ior;

	GlassShaderDescriptor(CyclesGlassMat* mtl, TimeValue t);

	bool operator<(const GlassShaderDescriptor& other) const;
};

class GlossyShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	GlossyDistribution distribution;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float roughness;
	Texmap* roughness_map = nullptr;
	
	GlossyShaderDescriptor(CyclesGlossyMat* mtl, TimeValue t);

	bool operator<(const GlossyShaderDescriptor& other) const;
};

class HairShaderDescriptor {
public:
	std::string name;
	
	HairComponent component;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float offset;
	Texmap* offset_map = nullptr;
	float roughness_u;
	Texmap* roughness_u_map = nullptr;
	float roughness_v;
	Texmap* roughness_v_map = nullptr;

	HairShaderDescriptor(CyclesHairMat* mtl, TimeValue t);

	bool operator<(const HairShaderDescriptor& other) const;
};

class MixShaderDescriptor {
public:
	std::string name;

	float fac;
	Texmap* fac_map = nullptr;
	Mtl* mtl_a = nullptr;
	Mtl* mtl_b = nullptr;
	bool use_fresnel_blending = false;

	MixShaderDescriptor(CyclesMixMat* mtl, TimeValue t);
	MixShaderDescriptor(CyclesVolMixMat* mtl, TimeValue t);

	bool operator<(const MixShaderDescriptor& other) const;
};

class PrincipledBSDFShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;
	NormalMapDescriptor cc_normal_desc;

	// General
	PrincipledBsdfDistribution distribution;
	float ior;
	ccl::float3 base_color;
	Texmap* base_color_map = nullptr;
	float metallic;
	Texmap* metallic_map = nullptr;
	float roughness;
	Texmap* roughness_map = nullptr;
	float sheen;
	Texmap* sheen_map = nullptr;
	float sheen_tint;
	Texmap* sheen_tint_map = nullptr;
	float transmission;
	Texmap* transmission_map = nullptr;
	float transmission_roughness;
	Texmap* transmission_roughness_map = nullptr;

	// Specular
	float specular;
	Texmap* specular_map = nullptr;
	float specular_tint;
	Texmap* specular_tint_map = nullptr;
	float anisotropic;
	Texmap* anisotropic_map = nullptr;
	float anisotropic_rotation;
	Texmap* anisotropic_rotation_map = nullptr;

	// Subsurface
	PrincipledBsdfSSSMethod subsurface_method;
	float subsurface;
	Texmap* subsurface_map = nullptr;
	ccl::float3 subsurface_color;
	Texmap* subsurface_color_map = nullptr;
	ccl::float3 subsurface_radius;

	// Clearcoat
	float clearcoat;
	Texmap* clearcoat_map = nullptr;
	float clearcoat_roughness;
	Texmap* clearcoat_roughness_map = nullptr;

	// Other
	ccl::float3 emission_color;
	Texmap* emission_color_map = nullptr;
	float alpha;
	Texmap* alpha_map = nullptr;

	PrincipledBSDFShaderDescriptor(CyclesPrincipledBSDFMat* mtl, TimeValue t);

	bool operator<(const PrincipledBSDFShaderDescriptor& other) const;
};

class RefractionShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	RefractionDistribution distribution;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float roughness;
	Texmap* roughness_map = nullptr;
	float ior;

	RefractionShaderDescriptor(CyclesRefractionMat* mtl, TimeValue t);

	bool operator<(const RefractionShaderDescriptor& other) const;
};

class SubsurfaceScatterShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	SubsurfaceFalloff falloff;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float scale;
	Texmap* scale_map = nullptr;
	ccl::float3 radius;
	float tex_blur;
	Texmap* tex_blur_map = nullptr;

	SubsurfaceScatterShaderDescriptor(CyclesSubsurfaceScatterMat* mtl, TimeValue t);

	bool operator<(const SubsurfaceScatterShaderDescriptor& other) const;
};

class ToonShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	ToonComponent component;
	ccl::float3 color;
	Texmap* color_map = nullptr;
	float size;
	Texmap* size_map = nullptr;
	float smooth;
	Texmap* smooth_map = nullptr;

	ToonShaderDescriptor(CyclesToonMat* mtl, TimeValue t);

	bool operator<(const ToonShaderDescriptor& other) const;
};

class TranslucentShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	ccl::float3 color;
	Texmap* color_map = nullptr;

	TranslucentShaderDescriptor(CyclesTranslucentMat* mtl, TimeValue t);

	bool operator<(const TranslucentShaderDescriptor& other) const;
};

class TransparentShaderDescriptor {
public:
	std::string name;

	ccl::float3 color;
	Texmap* color_map = nullptr;

	TransparentShaderDescriptor(CyclesTransparentMat* mtl, TimeValue t);

	bool operator<(const TransparentShaderDescriptor& other) const;
};

class VelvetShaderDescriptor {
public:
	std::string name;

	NormalMapDescriptor normal_desc;

	ccl::float3 color;
	Texmap* color_map = nullptr;
	float sigma;
	Texmap* sigma_map = nullptr;
	
	VelvetShaderDescriptor(CyclesVelvetMat* mtl, TimeValue t);
	
	bool operator<(const VelvetShaderDescriptor& other) const;
};

class VolAbsorbtionShaderDescriptor {
public:
	std::string name;

	ccl::float3 color;
	float density;

	VolAbsorbtionShaderDescriptor(CyclesVolAbsorptionMat* mtl, TimeValue t);

	bool operator<(const VolAbsorbtionShaderDescriptor& other) const;
};

class VolScatterShaderDescriptor {
public:
	std::string name;

	ccl::float3 color;
	float density;
	float anisotropy;

	VolScatterShaderDescriptor(CyclesVolScatterMat* mtl, TimeValue t);

	bool operator<(const VolScatterShaderDescriptor& other) const;
};

class LightShaderDescriptor {
public:
	ccl::float3 color;
	float intensity;

	LightShaderDescriptor(ccl::float3 color_in, float intensity_in);

	bool operator<(const LightShaderDescriptor& other) const;
};

class ComparableColor {
public:
	float r;
	float g;
	float b;

	ComparableColor(ccl::float3 color);

	bool operator<(const ComparableColor& other) const;
};

class SimpleSkyDescriptor {
public:
	ccl::float3 color;
	float strength;
};
