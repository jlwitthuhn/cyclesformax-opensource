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
 
#include "util_translate_camera.h"

#include <cmath>

#include <render/camera.h>
#include <util/util_transform.h>

#include <RenderingAPI/Renderer/ICameraContainer.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/ISceneContainer.h>
#include <Scene/IPhysicalCamera.h>
#include <units.h>

#include "const_classid.h"
#include "plugin_camera_panoramic.h"
#include "rend_logger.h"
#include "rend_params.h"
#include "util_enums.h"
#include "util_matrix_max.h"
#include "util_resolution.h"
#include "util_stereo.h"

using MaxSDK::RenderingAPI::ICameraContainer;

static ccl::Transform get_camera_transform(const MaxSDK::RenderingAPI::IRenderSessionContext& session_context, TimeValue t)
{
	const std::unique_ptr<LoggerInterface> logger{ global_log_manager.new_logger(L"UtilCameraGetTransform", false, true) };

	if (t < 0) {
		t = 0;
	}
	Interval view_par_valid = FOREVER;
	const ViewParams view_params = session_context.GetCamera().GetViewParams(t, view_par_valid);

	// Find ortho backup distance
	float ortho_backup_dist = 1.0e5f;
	{
		Interval bbox_valid = FOREVER;
		Box3 scene_bound_box = session_context.GetScene().GetSceneBoundingBox(t, bbox_valid);

		Matrix3 inverted_tfm = view_params.affineTM;
		inverted_tfm.Invert();

		const Point3 cam_pos = inverted_tfm.GetTrans();

		scene_bound_box += cam_pos;

		float x_dist = scene_bound_box.pmax.x - scene_bound_box.pmin.x;
		float y_dist = scene_bound_box.pmax.y - scene_bound_box.pmin.y;
		float z_dist = scene_bound_box.pmax.z - scene_bound_box.pmin.z;

		ortho_backup_dist = sqrt(x_dist * x_dist + y_dist * y_dist + z_dist * z_dist);
	}

	Matrix3 affine_tfm = view_params.affineTM;

	int proj_type = view_params.projType;
	bool backup_ortho_cam = false;
	if (session_context.GetCamera().GetCameraNode() != nullptr) {
		*logger << "Camera node found" << LogCtl::WRITE_LINE;

		INode* const cam_node = session_context.GetCamera().GetCameraNode();

		Interval cam_tfm_valid = FOREVER;
		const Matrix3 cam_tfm = cam_node->GetObjTMAfterWSM(t, &cam_tfm_valid);

		*logger << "max cam_tfm:" << LogCtl::WRITE_LINE
			<< cam_tfm.GetRow(0).x << ", " << cam_tfm.GetRow(0).y << ", " << cam_tfm.GetRow(0).z << LogCtl::WRITE_LINE
			<< cam_tfm.GetRow(1).x << ", " << cam_tfm.GetRow(1).y << ", " << cam_tfm.GetRow(1).z << LogCtl::WRITE_LINE
			<< cam_tfm.GetRow(2).x << ", " << cam_tfm.GetRow(2).y << ", " << cam_tfm.GetRow(2).z << LogCtl::WRITE_LINE
			<< cam_tfm.GetRow(3).x << ", " << cam_tfm.GetRow(3).y << ", " << cam_tfm.GetRow(3).z << LogCtl::WRITE_LINE;

		*logger << "cam_tfm scale: "
			<< cam_tfm.GetRow(0).Length() << ", "
			<< cam_tfm.GetRow(1).Length() << ", "
			<< cam_tfm.GetRow(2).Length() << LogCtl::WRITE_LINE;

		affine_tfm = Inverse(cam_tfm);

		*logger << "affine_tfm scale: "
			<< affine_tfm.GetRow(0).Length() << ", "
			<< affine_tfm.GetRow(1).Length() << ", "
			<< affine_tfm.GetRow(2).Length() << LogCtl::WRITE_LINE;

		const ObjectState& os = cam_node->EvalWorldState(t);
		if (os.obj->SuperClassID() != CAMERA_CLASS_ID) {
			return ccl::transform_identity();
		}
		CameraObject* const cam = static_cast<CameraObject*>(os.obj);
		Interval cs_valid;
		CameraState cs;
		cam->EvalCameraState(t, cs_valid, &cs);

		if (cs.isOrtho) {
			proj_type = PROJ_PARALLEL;
		}
		else {
			proj_type = PROJ_PERSPECTIVE;
		}

		if (cs.manualClip == false) {
			backup_ortho_cam = true;
		}
	}
	else {
		*logger << "Camera node NOT found" << LogCtl::WRITE_LINE;

		Interval proj_valid = FOREVER;
		const ICameraContainer::ProjectionType proj = session_context.GetCamera().GetProjectionType(t, proj_valid);
		if (proj == ICameraContainer::ProjectionType::Orthographic) {
			backup_ortho_cam = true;
		}
	}

	ccl::Transform result = ccl::transform_identity();
	{
		const Matrix3 cam_tfm = Inverse(affine_tfm);
		result = cycles_transform_from_max_matrix(cam_tfm);

		*logger << "before tfm: " << LogCtl::WRITE_LINE
			<< result.x.x << ", " << result.x.y << ", " << result.x.z << ", " << result.x.w << LogCtl::WRITE_LINE
			<< result.y.x << ", " << result.y.y << ", " << result.y.z << ", " << result.y.w << LogCtl::WRITE_LINE
			<< result.z.x << ", " << result.z.y << ", " << result.z.z << ", " << result.z.w << LogCtl::WRITE_LINE;

		if (proj_type == PROJ_PERSPECTIVE) {
			IPoint2 resolution = session_context.GetCamera().GetResolution();
			const float scale_amount = 0.5f * (static_cast<float>(resolution.y) / static_cast<float>(resolution.x));
			result = result * ccl::transform_scale(scale_amount, scale_amount, 1.0f);
		}
		else {
			// Ortho
			const float zoom = view_params.zoom;
			result = result * ccl::transform_scale(zoom * 100.0f, zoom * 100.0f, 1.0f);
			// Translate camera forwards(backwards) along z axis so we get the whole scene, this is only done when no manual clip planes are set with an ortho camera
			if (backup_ortho_cam) {
				result = result * ccl::transform_translate(0.0f, 0.0f, 1.0f * ortho_backup_dist);
			}
		}

		*logger << "after tfm: " << LogCtl::WRITE_LINE
			<< result.x.x << ", " << result.x.y << ", " << result.x.z << ", " << result.x.w << LogCtl::WRITE_LINE
			<< result.y.x << ", " << result.y.y << ", " << result.y.z << ", " << result.y.w << LogCtl::WRITE_LINE
			<< result.z.x << ", " << result.z.y << ", " << result.z.z << ", " << result.z.w << LogCtl::WRITE_LINE;
	}

	result = result * ccl::transform_scale(1.0f, 1.0f, -1.0f);

	*logger << "backup_ortho_cam: " << backup_ortho_cam << LogCtl::WRITE_LINE;

	return result;
}

static boost::optional<CyclesPanoramaCamParams> get_panorama_cam_params(
	const MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
	const TimeValue t,
	const StereoscopyType stereo_type)
{
	CyclesPanoramaCamParams result;

	Interval view_params_valid = FOREVER;
	const ViewParams view_params = session_context.GetCamera().GetViewParams(t, view_params_valid);

	Matrix3 max_matrix = view_params.affineTM;
	max_matrix.NoScale();
	max_matrix.Invert();

	result.matrix = cycles_transform_from_max_matrix(max_matrix);

	CyclesPanoramaCamera* const pano_cam = dynamic_cast<CyclesPanoramaCamera*>(session_context.GetCamera().GetCameraNode()->GetObjectRef());
	if (pano_cam == nullptr) {
		return boost::none;
	}

	Interval type_valid = FOREVER;
	result.pano_type = pano_cam->GetPanoramaType(t, type_valid);

	Interval nearclip_valid = FOREVER;
	result.nearclip = pano_cam->GetNearClip(t, nearclip_valid);
	Interval farclip_valid = FOREVER;
	result.farclip = pano_cam->GetFarClip(t, farclip_valid);

	Interval min_lat_valid = FOREVER;
	result.min_lat = pano_cam->GetMinLattitude(t, min_lat_valid);
	Interval max_lat_valid = FOREVER;
	result.max_lat = pano_cam->GetMaxLattitude(t, max_lat_valid);
	Interval min_long_valid = FOREVER;
	result.min_long = pano_cam->GetMinLongitude(t, min_long_valid);
	Interval max_long_valid = FOREVER;
	result.max_long = pano_cam->GetMaxLongitude(t, max_long_valid);

	Interval fe_fov_valid = FOREVER;
	result.fisheye_fov = pano_cam->GetFisheyeFov(t, fe_fov_valid);
	Interval fe_focal_length_valid = FOREVER;
	result.fisheye_focal_length = pano_cam->GetFisheyeFocalLength(t, fe_focal_length_valid);

	// Calculate sensor size
	// This block is adapted from blender_camera_sync in blender/blender_camera.cpp
	if (result.pano_type == CameraPanoramaType::FISHEYE_EQUISOLID) {
		const IPoint2 max_res = session_context.GetCamera().GetResolution();
		const RenderResolutions resolutions(max_res.x, max_res.y, stereo_type);
		const Double2 render_res_double = cast_as_double2(resolutions.render_res());

		const bool horizontal_fit = render_res_double.x() > render_res_double.y();

		Interval sensor_size_valid = FOREVER;
		const float sensor_size = pano_cam->GetFisheyeSensorSize(t, sensor_size_valid);

		if (horizontal_fit) {
			result.sensorwidth = sensor_size;
			result.sensorheight = sensor_size * render_res_double.y() / render_res_double.x();
		}
		else {
			result.sensorwidth = sensor_size * render_res_double.x() / render_res_double.y();
			result.sensorheight = sensor_size;
		}
	}

	Interval use_spherical_stereo_valid = FOREVER;
	result.use_spherical_stereo = pano_cam->GetUseSphericalStereo(t, use_spherical_stereo_valid);
	Interval use_pole_merge_valid = FOREVER;
	result.use_pole_merge = pano_cam->GetUsePoleMerge(t, use_pole_merge_valid);
	Interval pole_merge_from_valid = FOREVER;
	result.pole_merge_from = pano_cam->GetPoleMergeFrom(t, pole_merge_from_valid) * M_PI / 180.0;
	Interval pole_merge_to_valid = FOREVER;
	result.pole_merge_to = pano_cam->GetPoleMergeTo(t, pole_merge_to_valid) * M_PI / 180.0;

	return result;
}

static boost::optional<CyclesPerspOrthoCamParams> get_persp_ortho_cam_params(
	const MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
	const TimeValue t)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilCameraGetPerspectiveParams", false, true);
	*logger << LogCtl::SEPARATOR;

	// Find ortho backup distance
	float ortho_backup_dist = 1.0e5f;
	{
		Interval bbox_valid = FOREVER;
		Box3 scene_bound_box = session_context.GetScene().GetSceneBoundingBox(t, bbox_valid);

		Interval view_params_valid = FOREVER;
		const ViewParams view_params = session_context.GetCamera().GetViewParams(t, view_params_valid);

		Matrix3 inverted_tfm = view_params.affineTM;
		inverted_tfm.Invert();

		Point3 cam_pos = inverted_tfm.GetTrans();

		scene_bound_box += cam_pos;

		float x_dist = scene_bound_box.pmax.x - scene_bound_box.pmin.x;
		float y_dist = scene_bound_box.pmax.y - scene_bound_box.pmin.y;
		float z_dist = scene_bound_box.pmax.z - scene_bound_box.pmin.z;

		ortho_backup_dist = sqrt(x_dist * x_dist + y_dist * y_dist + z_dist * z_dist);

		*logger << "ortho_backup_dist: " << ortho_backup_dist << LogCtl::WRITE_LINE;
	}

	CyclesPerspOrthoCamParams result;

	Interval view_par_valid = FOREVER;
	const ViewParams view_params = session_context.GetCamera().GetViewParams(t, view_par_valid);
	float hither = view_params.hither;
	float yon = view_params.yon;
	int proj_type = view_params.projType;
	float fov = view_params.fov;

	// This will be set to true if there are no manual clip planes set
	bool backup_ortho_cam = false;

	if (session_context.GetCamera().GetCameraNode() != nullptr) {
		INode* vnode = session_context.GetCamera().GetCameraNode();

		const ObjectState& os = vnode->EvalWorldState(t);
		if (os.obj->SuperClassID() != CAMERA_CLASS_ID) {
			return boost::none;
		}

		CameraObject* cam = static_cast<CameraObject*>(os.obj);

		Interval cs_valid;
		CameraState cs;
		cam->EvalCameraState(t, cs_valid, &cs);

		if (cs.manualClip) {
			hither = cs.hither;
			yon = cs.yon;
		}
		else {
			hither = 1.0f;
			yon = 1e24f;
			backup_ortho_cam = true;
		}

		if (cs.isOrtho) {
			proj_type = PROJ_PARALLEL;
		}
		else {
			proj_type = PROJ_PERSPECTIVE;
			fov = cs.fov;
		}


		if (cam->ClassID() == PHYS_CAMERA_CLASS) {
			MaxSDK::IPhysicalCamera* const phys_cam = dynamic_cast<MaxSDK::IPhysicalCamera*>(cam);
			assert(phys_cam != nullptr);

			Interval motion_blur_valid = FOREVER;
			const bool use_motion_blur = phys_cam->GetMotionBlurEnabled(t, motion_blur_valid);
			result.use_motion_blur = use_motion_blur;

			Interval shutter_time_valid = FOREVER;
			const float shutter_time = phys_cam->GetShutterDurationInFrames(t, shutter_time_valid);
			result.shutter_time = shutter_time;

			Interval dof_valid = FOREVER;
			if (phys_cam->GetDOFEnabled(t, dof_valid)) {
				Interval focal_length_valid = FOREVER;
				const float focal_length = phys_cam->GetEffectiveLensFocalLength(t, focal_length_valid);

				Interval aperture_ratio_valid = FOREVER;
				const float aperture_ratio = phys_cam->GetLensApertureFNumber(t, aperture_ratio_valid);

				Interval focus_distance_valid = FOREVER;
				const float focus_distance = phys_cam->GetFocusDistance(t, focus_distance_valid);

				result.use_dof = true;
				result.dof_aperturesize = 0.5f * focal_length / aperture_ratio;
				result.dof_focaldistance = focus_distance;
				result.dof_aperture_ratio = aperture_ratio;
			}
		}

	}
	else {
		*logger << "camera is nullptr" << LogCtl::WRITE_LINE;

		Interval proj_valid = FOREVER;
		ICameraContainer::ProjectionType proj = session_context.GetCamera().GetProjectionType(t, proj_valid);
		if (proj == ICameraContainer::ProjectionType::Orthographic) {
			hither = 1.0f;
			yon = 1e24f;
			backup_ortho_cam = true;
		}
	}

	result.nearclip = hither;
	result.farclip = yon;

	if (proj_type == PROJ_PERSPECTIVE) {
		result.fov = fov;
	}
	else {
		// Ortho
		if (backup_ortho_cam) {
			result.farclip += ortho_backup_dist;
		}
	}

	{
		const int ticks_offset = result.get_mblur_full_offset();
		result.transform_pre = get_camera_transform(session_context, t - ticks_offset);
		result.transform = get_camera_transform(session_context, t);
		result.transform_post = get_camera_transform(session_context, t + ticks_offset);
	}

	*logger << "complete" << LogCtl::WRITE_LINE;

	return result;
}

boost::optional<CyclesCameraParams> get_camera_params(
	const MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
	const TimeValue t,
	const StereoscopyType stereo_type)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilCameraGetParams", false, true);
	*logger << LogCtl::SEPARATOR;

	CyclesCameraParams result;
	
	const bool is_pano_cam = session_context.GetCamera().GetCameraNode() != nullptr && session_context.GetCamera().GetCameraNode()->GetObjectRef()->ClassID() == CYCLES_CAMERA_PANORAMIC_CLASS;

	Interval view_par_valid = FOREVER;
	const ViewParams view_params = session_context.GetCamera().GetViewParams(t, view_par_valid);

	if (is_pano_cam) {
		result.camera_type = ccl::CAMERA_PANORAMA;
	}
	else if (view_params.projType == PROJ_PERSPECTIVE) {
		result.camera_type = ccl::CAMERA_PERSPECTIVE;
	}
	else {
		result.camera_type = ccl::CAMERA_ORTHOGRAPHIC;
	}

	if (is_pano_cam) {
		const boost::optional<CyclesPanoramaCamParams> pano_params{ get_panorama_cam_params(session_context, t, stereo_type) };
		if (pano_params) {
			result.params_union.pano_params = *pano_params;
		}
		else {
			return boost::none;
		}
	}
	else {
		const boost::optional<CyclesPerspOrthoCamParams> perp_ortho_params{ get_persp_ortho_cam_params(session_context, t) };
		if (perp_ortho_params) {
			result.params_union.persp_ortho_params = *perp_ortho_params;
		}
		else {
			return boost::none;
		}
	}

	result.final_resolution = Int2(session_context.GetCamera().GetResolution().x, session_context.GetCamera().GetResolution().y);

	const Box2 box_region = session_context.GetCamera().GetRegion();
	const Int2 region_begin(box_region.x(), box_region.y());
	const Int2 region_end(box_region.x() + box_region.w(), box_region.y() + box_region.h());
	result.region = IntRect(region_begin, region_end);

	const RenderResolutions resolutions(result.final_resolution.x(), result.final_resolution.y(), stereo_type);
	const IntRect render_res_box(Int2(0, 0), resolutions.render_res());
	if (render_res_box.contains(result.region) == false) {
		result.region = render_res_box;
	}

	*logger << "Region: " << result.region << LogCtl::WRITE_LINE;

	return result;
}

void apply_camera_params(const CyclesCameraParams& camera_params, const CyclesRenderParams& rend_params, ccl::Camera& camera)
{
	const RenderResolutions resolutions(camera_params.final_resolution.x(), camera_params.final_resolution.y(), rend_params.stereo_type);

	// TODO: Maybe a problem here, was width/height
	camera.set_full_width(resolutions.render_res().x());
	camera.set_full_height(resolutions.render_res().y());
	camera.set_camera_type(camera_params.camera_type);

	camera.compute_auto_viewplane();

	camera.set_interocular_distance(rend_params.interocular_distance);
	camera.set_convergence_distance(rend_params.convergence_distance);

	if (camera.get_camera_type() == ccl::CameraType::CAMERA_PERSPECTIVE || camera.get_camera_type() == ccl::CameraType::CAMERA_ORTHOGRAPHIC) {
		if (camera_params.params_union.persp_ortho_params.nearclip > 0.0f) {
			camera.set_nearclip(camera_params.params_union.persp_ortho_params.nearclip);
		}
		camera.set_farclip(camera_params.params_union.persp_ortho_params.farclip);

		if (camera.get_camera_type() == ccl::CameraType::CAMERA_PERSPECTIVE) {
			// TODO: animated fov should work here
			camera.set_fov(camera_params.params_union.persp_ortho_params.fov);
			camera.set_fov_pre(camera.get_fov());
			camera.set_fov_post(camera.get_fov());;
		}

		camera.set_matrix(camera_params.params_union.persp_ortho_params.transform);

		if (camera_params.params_union.persp_ortho_params.use_dof) {
			camera.set_aperturesize(camera_params.params_union.persp_ortho_params.dof_aperturesize);
			camera.set_focaldistance(camera_params.params_union.persp_ortho_params.dof_focaldistance);
			camera.set_aperture_ratio(camera_params.params_union.persp_ortho_params.dof_aperture_ratio);
		}


		{
			// Correct viewplane size
			// I'm not sure why this is needed, but this math makes the camera exactly match ART
			const double render_height = camera.get_full_width();
			const double render_width = camera.get_full_height();
			double scale_factor = (render_width / render_height);
			if (scale_factor < 1.0) {
				scale_factor = 1.0;
			}
			const double viewplane_scale = (16.0 / 9.0) / scale_factor;
			camera.viewplane = camera.viewplane * static_cast<float>(1.125 * viewplane_scale);
		}
	}
	else if (camera.get_camera_type() == ccl::CameraType::CAMERA_PANORAMA) {
		if (camera_params.params_union.pano_params.pano_type == CameraPanoramaType::EQUIRECTANGULAR) {
			camera.set_panorama_type(ccl::PanoramaType::PANORAMA_EQUIRECTANGULAR);
		}
		else if (camera_params.params_union.pano_params.pano_type == CameraPanoramaType::FISHEYE_EQUIDISTANT) {
			camera.set_panorama_type(ccl::PanoramaType::PANORAMA_FISHEYE_EQUIDISTANT);
		}
		else if (camera_params.params_union.pano_params.pano_type == CameraPanoramaType::FISHEYE_EQUISOLID) {
			camera.set_panorama_type(ccl::PanoramaType::PANORAMA_FISHEYE_EQUISOLID);
		}

		camera.set_matrix(camera_params.params_union.pano_params.matrix);

		camera.set_nearclip(camera_params.params_union.pano_params.nearclip);
		camera.set_farclip(camera_params.params_union.pano_params.farclip);

		camera.set_latitude_min(camera_params.params_union.pano_params.min_lat);
		camera.set_latitude_max(camera_params.params_union.pano_params.max_lat);
		camera.set_longitude_min(camera_params.params_union.pano_params.min_long);
		camera.set_longitude_max(camera_params.params_union.pano_params.max_long);

		camera.set_fisheye_fov(camera_params.params_union.pano_params.fisheye_fov);
		camera.set_fisheye_lens(camera_params.params_union.pano_params.fisheye_focal_length);

		camera.set_sensorwidth(camera_params.params_union.pano_params.sensorwidth);
		camera.set_sensorheight(camera_params.params_union.pano_params.sensorheight);

		camera.set_stereo_eye(ccl::Camera::StereoEye::STEREO_NONE);

		camera.set_use_spherical_stereo(camera_params.params_union.pano_params.use_spherical_stereo);
		camera.set_use_pole_merge(camera_params.params_union.pano_params.use_pole_merge);
		camera.set_pole_merge_angle_from(camera_params.params_union.pano_params.pole_merge_from);
		camera.set_pole_merge_angle_to(camera_params.params_union.pano_params.pole_merge_to);
	}

	if (camera_params.is_motion_blur_enabled()) {
		// TODO: Make this use an arbitrary number of samples
		const ccl::Transform transform_pre = camera_params.params_union.persp_ortho_params.transform_pre;
		const ccl::Transform transform = camera_params.params_union.persp_ortho_params.transform;
		const ccl::Transform transform_post = camera_params.params_union.persp_ortho_params.transform_post;

		if (transform_pre != transform || transform_post != transform) {
			camera.set_shuttertime(camera_params.params_union.persp_ortho_params.shutter_time);
			camera.set_use_perspective_motion(false);
			camera.get_motion().resize(3);
			camera.get_motion()[0] = transform_pre;
			camera.get_motion()[1] = transform;
			camera.get_motion()[2] = transform_post;
		}
	}

	// For region renders, adjust the viewplane
	{
		const Int2 render_res = resolutions.render_res();
		const IntRect& region_rect = camera_params.region;
		const Double2 region_begin = cast_as_double2(region_rect.begin());
		const Double2 region_end = cast_as_double2(region_rect.end());

		ccl::BoundBox2D sub_box;
		sub_box.left = static_cast<float>(region_begin.x() / render_res.x());
		sub_box.right = static_cast<float>(region_end.x() / render_res.x());

		// Top/bottom are flipped between max and cycles
		sub_box.bottom = 1.0f - static_cast<float>(region_end.y() / render_res.y());
		sub_box.top = 1.0f - static_cast<float>(region_begin.y() / render_res.y());

		camera.viewplane = camera.viewplane.subset(sub_box);
	}

	camera.set_motion_position(ccl::Camera::MotionPosition::MOTION_POSITION_CENTER);
	camera.set_rolling_shutter_type(ccl::Camera::RollingShutterType::ROLLING_SHUTTER_NONE);
}
