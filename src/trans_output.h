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
 * @brief Defines the the types used as output in ActiveShade translators.
 */

#include <chrono>
#include <map>
#include <memory>
#include <vector>
#include <string>

#include <kernel/kernel_types.h>

#include "util_enums.h"
#include "util_simple_types.h"

class Mtl;
class Texmap;

class CyclesPerspOrthoCamParams {
public:
	CyclesPerspOrthoCamParams();

	float nearclip = 0.1f;
	float farclip = 1000.0f;

	float fov = 90.0f;

	ccl::Transform transform_pre;
	ccl::Transform transform;
	ccl::Transform transform_post;

	bool use_dof = false;
	float dof_aperturesize = 0.0f;
	float dof_focaldistance = 0.0f;
	float dof_aperture_ratio = 0.0f;

	bool use_motion_blur = false;
	float shutter_time = 0.5f;

	int get_mblur_full_offset() const;

	bool operator==(const CyclesPerspOrthoCamParams& other) const;
	bool operator!=(const CyclesPerspOrthoCamParams& other) const;
};

class CyclesPanoramaCamParams {
public:
	CyclesPanoramaCamParams();

	ccl::Transform matrix;

	CameraPanoramaType pano_type = CameraPanoramaType::EQUIRECTANGULAR;

	float nearclip = 0.1f;
	float farclip = 1000.0f;

	float min_lat = 0.0f;
	float max_lat = 0.0f;
	float min_long = 0.0f;
	float max_long = 0.0f;

	float fisheye_fov = M_PI_F;
	float fisheye_focal_length = 10.5f;

	float sensorwidth = 0.036f;
	float sensorheight = 0.024f;

	bool use_spherical_stereo = true;
	bool use_pole_merge = false;
	float pole_merge_from = static_cast<float>(60.0 * 3.14159 / 180.0);
	float pole_merge_to = static_cast<float>(75.0 * 3.14159 / 180.0);

	bool operator==(const CyclesPanoramaCamParams& other) const;
	bool operator!=(const CyclesPanoramaCamParams& other) const;
};

union CyclesCameraParamsUnion {
	CyclesCameraParamsUnion();
	CyclesPerspOrthoCamParams persp_ortho_params;
	CyclesPanoramaCamParams pano_params;
};

class CyclesCameraParams {
public:
	CyclesCameraParams();

	ccl::CameraType camera_type = ccl::CameraType::CAMERA_PERSPECTIVE;

	Int2 final_resolution;
	IntRect region;

	CyclesCameraParamsUnion params_union;

	bool is_motion_blur_enabled() const;
	int get_mblur_full_offset() const;

	bool operator==(const CyclesCameraParams& other) const;
	bool operator!=(const CyclesCameraParams& other) const;
};

class CyclesEnvironmentParams {
public:
	CyclesEnvironmentParams();
	CyclesEnvironmentParams(ccl::float3 bg_color);
	CyclesEnvironmentParams(Texmap* environment_map);

	ccl::float3 bg_color;
	Texmap* environment_map;

	bool operator==(const CyclesEnvironmentParams& other) const;
	bool operator!=(const CyclesEnvironmentParams& other) const;
};

class TriangleFace {
public:
	TriangleFace(int v0, int v1, int v2, int shader_mtl_index, bool smooth);
	const int v0;
	const int v1;
	const int v2;
	const int shader_mtl_index;
	const bool smooth;
};

class MeshGeometryObj {
public:
	std::vector<ccl::float3> verts;
	std::vector<ccl::float3> normals;
	std::vector<TriangleFace> faces;
	std::vector<ccl::float3> uvw_verts;
	std::vector<ccl::float3> uvw_tangents;
	std::vector<float> uvw_tangent_signs;
	std::vector<std::vector<ccl::float3>> motion_verts;
	std::vector<std::vector<ccl::float3>> motion_normals;
	bool use_mesh_motion_blur = false;

	// More info needed by ActiveShade only
	std::vector<unsigned int> uv_channels_present;
	std::vector<unsigned short> mtl_ids_present;
};

class CyclesGeomObject {
public:
	CyclesGeomObject();

	bool is_shadow_catcher = false;

	bool visible_to_camera = true;

	ccl::Transform tfm_pre;
	ccl::Transform tfm;
	ccl::Transform tfm_post;

	ccl::uint random_id;

	std::shared_ptr<MeshGeometryObj> mesh_geometry;
	Mtl* mtl = nullptr;
	ccl::float3 wire_color;
	bool hidden = false;
	bool use_object_motion_blur = false;

	bool operator==(const CyclesGeomObject& other) const;
	bool operator!=(const CyclesGeomObject& other) const;
};

class CyclesSceneGeometryList {
public:
	std::vector<CyclesGeomObject> geom_objects;

	bool operator==(const CyclesSceneGeometryList& other) const;
	bool operator!=(const CyclesSceneGeometryList& other) const;
};

class CyclesLightParams {
public:
	CyclesLightParams();

	bool active = false;

	ccl::Transform tfm;

	float spot_angle = 3.14159f / 4.0f;
	float spot_smooth = 0.0f;
	float size = 1.0f;
	CyclesLightType type = CyclesLightType::INVALID;
	float intensity = 1.0f;
	ccl::float3 color = ccl::make_float3(1.0f, 1.0f, 1.0f);
	bool shadows_enabled = true;

	bool errored = false;
	std::wstring error_string;

	bool operator==(const CyclesLightParams& other) const;
	bool operator!=(const CyclesLightParams& other) const;
};

class CyclesSceneLightList {
public:
	std::vector<CyclesLightParams> lights;

	bool operator==(const CyclesSceneLightList& other) const;
	bool operator!=(const CyclesSceneLightList& other) const;
};

class MaterialProperties {
public:
	std::chrono::steady_clock::time_point last_update_time;

	bool operator==(const MaterialProperties& other) const;
	bool operator!=(const MaterialProperties& other) const;
};

class TexmapUpdateTimes {
public:
	std::map<Texmap*, std::chrono::steady_clock::time_point> update_times;

	bool operator==(const TexmapUpdateTimes& other) const;
	bool operator!=(const TexmapUpdateTimes& other) const;
};

class CyclesSceneDescriptor {
public:
	CyclesCameraParams camera_params;
	CyclesEnvironmentParams env_params;
	CyclesSceneGeometryList scene_geometry;
	CyclesSceneLightList scene_lights;
	MaterialProperties mtl_properties;
	TexmapUpdateTimes texmap_times;

	bool operator==(const CyclesSceneDescriptor& other) const;
	bool operator!=(const CyclesSceneDescriptor& other) const;
};
