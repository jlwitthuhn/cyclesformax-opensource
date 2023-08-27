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
 
#include "trans_output.h"

#include <units.h>

CyclesPerspOrthoCamParams::CyclesPerspOrthoCamParams()
{
	transform_pre = ccl::transform_identity();
	transform = ccl::transform_identity();
	transform_post = ccl::transform_identity();
}

int CyclesPerspOrthoCamParams::get_mblur_full_offset() const
{
	if (use_motion_blur == false) {
		return 0;
	}
	return static_cast<int>(GetTicksPerFrame() / 2.0f * shutter_time);
}

bool CyclesPerspOrthoCamParams::operator==(const CyclesPerspOrthoCamParams& other) const
{
	return (
		nearclip == other.nearclip &&
		farclip == other.farclip &&
		fov == other.fov &&
		transform_pre == other.transform_pre &&
		transform == other.transform &&
		transform_post == other.transform_post &&
		use_dof == other.use_dof &&
		dof_aperturesize == other.dof_aperturesize &&
		dof_focaldistance == other.dof_focaldistance &&
		dof_aperture_ratio == other.dof_aperture_ratio &&
		use_motion_blur == other.use_motion_blur &&
		shutter_time == other.shutter_time
		);
}

bool CyclesPerspOrthoCamParams::operator!=(const CyclesPerspOrthoCamParams& other) const
{
	return !(operator==(other));
}

CyclesPanoramaCamParams::CyclesPanoramaCamParams()
{
	matrix = ccl::transform_identity();
}

bool CyclesPanoramaCamParams::operator==(const CyclesPanoramaCamParams& other) const
{
	return (
		matrix == other.matrix &&
		pano_type == other.pano_type &&
		nearclip == other.nearclip &&
		farclip == other.farclip &&
		min_lat == other.min_lat &&
		max_lat == other.max_lat &&
		min_long == other.min_long &&
		max_long == other.max_long &&
		fisheye_fov == other.fisheye_fov &&
		fisheye_focal_length == other.fisheye_focal_length &&
		sensorwidth == other.sensorwidth &&
		sensorheight == other.sensorheight &&
		use_spherical_stereo == other.use_spherical_stereo &&
		use_pole_merge == other.use_pole_merge &&
		pole_merge_from == other.pole_merge_from &&
		pole_merge_to == other.pole_merge_to
		);
}

bool CyclesPanoramaCamParams::operator!=(const CyclesPanoramaCamParams& other) const
{
	return !(operator==(other));
}

CyclesCameraParamsUnion::CyclesCameraParamsUnion() : persp_ortho_params()
{

}

bool CyclesCameraParams::is_motion_blur_enabled() const
{
	if (camera_type == ccl::CameraType::CAMERA_PERSPECTIVE || camera_type == ccl::CameraType::CAMERA_ORTHOGRAPHIC) {
		return params_union.persp_ortho_params.use_motion_blur;
	}
	return false;
}

int CyclesCameraParams::get_mblur_full_offset() const
{
	if (is_motion_blur_enabled()) {
		return params_union.persp_ortho_params.get_mblur_full_offset();
	}
	return 0;
}

bool CyclesCameraParams::operator==(const CyclesCameraParams& other) const
{
	if (camera_type != other.camera_type) {
		return false;
	}

	if (final_resolution != other.final_resolution) {
		return false;
	}

	if (region != other.region) {
		return false;
	}

	if (camera_type == ccl::CameraType::CAMERA_PERSPECTIVE || camera_type == ccl::CameraType::CAMERA_ORTHOGRAPHIC) {
		return params_union.persp_ortho_params == other.params_union.persp_ortho_params;
	}
	else if (camera_type == ccl::CameraType::CAMERA_PANORAMA) {
		return params_union.pano_params == other.params_union.pano_params;
	}

	return true;
}

CyclesCameraParams::CyclesCameraParams() : final_resolution(1, 1)
{

}

bool CyclesCameraParams::operator!=(const CyclesCameraParams& other) const
{
	return !(operator==(other));
}

CyclesEnvironmentParams::CyclesEnvironmentParams() :
	bg_color(ccl::make_float3(0.0f, 0.0f, 0.0f)),
	environment_map(nullptr)
{

}

CyclesEnvironmentParams::CyclesEnvironmentParams(const ccl::float3 bg_color) :
	bg_color(bg_color),
	environment_map(nullptr)
{

}

CyclesEnvironmentParams::CyclesEnvironmentParams(Texmap* const environment_map) :
	bg_color(ccl::make_float3(0.0f, 0.0f, 0.0f)),
	environment_map(environment_map)
{

}

bool CyclesEnvironmentParams::operator==(const CyclesEnvironmentParams& other) const
{
	if (environment_map == nullptr && other.environment_map == nullptr) {
		return (
			bg_color.x == other.bg_color.x &&
			bg_color.y == other.bg_color.y &&
			bg_color.z == other.bg_color.z
			);
	}

	return (environment_map == other.environment_map);
}

bool CyclesEnvironmentParams::operator!=(const CyclesEnvironmentParams& other) const
{
	return !(operator==(other));
}

TriangleFace::TriangleFace(const int v0, const int v1, const int v2, const int shader_mtl_index, const bool smooth) :
	v0(v0),
	v1(v1),
	v2(v2),
	shader_mtl_index(shader_mtl_index),
	smooth(smooth)
{

}

CyclesGeomObject::CyclesGeomObject()
{
	wire_color = ccl::make_float3(0.0f, 0.0f, 0.0f);
}

bool CyclesGeomObject::operator==(const CyclesGeomObject& other) const
{
	return(
		is_shadow_catcher == other.is_shadow_catcher &&
		visible_to_camera == other.visible_to_camera &&
		tfm_pre == other.tfm_pre &&
		tfm == other.tfm &&
		tfm_post == other.tfm_post &&
		random_id == other.random_id &&
		mesh_geometry == other.mesh_geometry &&
		mtl == other.mtl &&
		wire_color == other.wire_color &&
		hidden == other.hidden &&
		use_object_motion_blur == other.use_object_motion_blur
		);
}

bool CyclesGeomObject::operator!=(const CyclesGeomObject& other) const
{
	return !(operator==(other));
}

bool CyclesSceneGeometryList::operator==(const CyclesSceneGeometryList& other) const
{
	if (geom_objects.size() != other.geom_objects.size()) {
		return false;
	}

	for (size_t i = 0; i < geom_objects.size(); ++i) {
		const CyclesGeomObject& this_obj = geom_objects[i];
		const CyclesGeomObject& other_obj = other.geom_objects[i];

		if (this_obj != other_obj) {
			return false;
		}
	}

	return true;
}

bool CyclesSceneGeometryList::operator!=(const CyclesSceneGeometryList& other) const
{
	return !(operator==(other));
}

CyclesLightParams::CyclesLightParams()
{
	tfm = ccl::transform_identity();
}

bool CyclesLightParams::operator==(const CyclesLightParams& other) const
{
	return active == other.active &&
		tfm == other.tfm &&
		spot_angle == other.spot_angle &&
		spot_smooth == other.spot_smooth &&
		size == other.size &&
		type == other.type &&
		intensity == other.intensity &&
		color == other.color &&
		shadows_enabled == other.shadows_enabled &&
		errored == other.errored;
}

bool CyclesLightParams::operator!=(const CyclesLightParams& other) const
{
	return !(operator==(other));
}

bool CyclesSceneLightList::operator==(const CyclesSceneLightList& other) const
{
	if (lights.size() != other.lights.size()) {
		return false;
	}

	for (int i = 0; i < lights.size(); ++i) {
		CyclesLightParams this_obj = lights[i];
		CyclesLightParams other_obj = other.lights[i];

		if (this_obj != other_obj) {
			return false;
		}
	}

	return true;
}

bool CyclesSceneLightList::operator!=(const CyclesSceneLightList& other) const
{
	return !(operator==(other));
}

bool MaterialProperties::operator==(const MaterialProperties& other) const
{
	return last_update_time == other.last_update_time;
}

bool MaterialProperties::operator!=(const MaterialProperties& other) const
{
	return !operator==(other);
}

bool TexmapUpdateTimes::operator==(const TexmapUpdateTimes& other) const
{
	return update_times == other.update_times;
}

bool TexmapUpdateTimes::operator!=(const TexmapUpdateTimes& other) const
{
	return !operator==(other);
}

bool CyclesSceneDescriptor::operator==(const CyclesSceneDescriptor& other) const
{
	return(
		camera_params == other.camera_params &&
		env_params == other.env_params &&
		scene_geometry == other.scene_geometry &&
		scene_lights == other.scene_lights &&
		mtl_properties == other.mtl_properties &&
		texmap_times == other.texmap_times
		);
}

bool CyclesSceneDescriptor::operator!=(const CyclesSceneDescriptor& other) const
{
	return !(operator==(other));
}
