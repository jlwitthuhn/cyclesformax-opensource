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
 
#include "plugin_camera_panoramic.h"

#include <gfx.h>
#include <Graphics/CustomRenderItemHandle.h>
#include <Graphics/IMeshDisplay2.h>
#include <Graphics/Utilities/SplineRenderItem.h>
#include <mouseman.h>

#include "max_classdesc_cam.h"
#include "win_resource.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

using MaxSDK::Graphics::CustomRenderItemHandle;
using MaxSDK::Graphics::DrawContext;
using MaxSDK::Graphics::GenerateMeshRenderItemsContext;
using MaxSDK::Graphics::HitTestContext;
using MaxSDK::Graphics::IMeshDisplay2;
using MaxSDK::Graphics::Utilities::SplineRenderItem;

#define PBLOCK_REF_MAIN 0
#define PBLOCK_REF_CLIP 1
#define PBLOCK_REF_STEREO 2

#define PBLOCK_COUNT 3

// Pblock enum
enum { cam_pano_pblock_main, cam_pano_pblock_clip, cam_pano_pblock_stereo };

// Pblock params
enum {
	main_param_type,
	main_param_er_min_lat, main_param_er_max_lat,
	main_param_er_min_long, main_param_er_max_long,
	main_param_fe_fov,
	main_param_fe_focal_length,
	main_param_fe_sensor_size,
};
enum { clip_param_nearclip, clip_param_farclip };
enum {
	stereo_param_use_spherical_stereo,
	stereo_param_use_pole_merge,
	stereo_param_merge_from,
	stereo_param_merge_to,
};

// Radio button enums
enum { main_type_equirectangular, main_type_fisheye_equidistant, main_type_fisheye_equisolid };

static ParamBlockDesc2 pano_cam_pblock_desc(
	// Pblock data
	cam_pano_pblock_main,
	_T("pblock_main"),
	0,
	GetCyclesCameraPanoramaClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF_MAIN,
	// Interface stuff
	IDD_PANEL_CAM_PANO_PARAM,
	IDS_PARAMS,
	0,
	0,
	NULL,
		// Panorama type
		main_param_type,
		_T("type"),
		TYPE_RADIOBTN_INDEX,
		P_ANIMATABLE,
		IDS_PANORAMA_TYPE,
		p_default, main_type_equirectangular,
		p_range, main_type_equirectangular, main_type_fisheye_equisolid,
		p_ui, TYPE_RADIO, 3, IDC_RADIO_CAMTYPE_EQUIRECTANGULAR, IDC_RADIO_CAMTYPE_FE_EQUIDISTANT, IDC_RADIO_CAMTYPE_FE_EQUISOLID, // Order here must match enum
		p_end,
		// Equirectangular - min lattitude
		main_param_er_min_lat,
		_T("min_lattitude"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_MIN_LATTITUDE,
		p_default, -90.0f,
		p_range, -90.0f, 90.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_MIN_LAT, IDC_SPIN_MIN_LAT, 1.0f,
		p_end,
		// Equirectangular - max lattitude
		main_param_er_max_lat,
		_T("max_lattitude"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_MAX_LATTITUDE,
		p_default, 90.0f,
		p_range, -90.0f, 90.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_MAX_LAT, IDC_SPIN_MAX_LAT, 1.0f,
		p_end,
		// Equirectangular - min longitude
		main_param_er_min_long,
		_T("min_longitude"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_MIN_LONGITUDE,
		p_default, -180.0f,
		p_range, -180.0f, 180.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_MIN_LONG, IDC_SPIN_MIN_LONG, 1.0f,
		p_end,
		// Equirectangular - max longitude
		main_param_er_max_long,
		_T("max_longitude"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_MAX_LONGITUDE,
		p_default, 180.0f,
		p_range, -180.0f, 180.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_MAX_LONG, IDC_SPIN_MAX_LONG, 1.0f,
		p_end,
		// Fisheye - fov
		main_param_fe_fov,
		_T("fisheye_fov"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_FOV,
		p_default, 180.0f,
		p_range, 5.0f, 180.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_FOV, IDC_SPIN_FOV, 1.0f,
		p_end,
		// Fisheye - focal length
		main_param_fe_focal_length,
		_T("fisheye_focal_length"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_FOCAL_LENGTH,
		p_default, 10.5f,
		p_range, 0.01f, 50.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_FOCAL_LENGTH, IDC_SPIN_FOCAL_LENGTH, 0.5f,
		p_end,
		// Fisheye - sensor size
		main_param_fe_sensor_size,
		_T("fisheye_sensor_size"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_SENSOR_SIZE,
		p_default, 32.0f,
		p_range, 0.1f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_SENSOR_SIZE, IDC_SPIN_SENSOR_SIZE, 1.0f,
		p_end,
	p_end
	);


static ParamBlockDesc2 pano_cam_clip_pblock_desc(
	// Pblock data
	cam_pano_pblock_clip,
	_T("pblock_clip"),
	0,
	GetCyclesCameraPanoramaClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF_CLIP,
	// Interface stuff
	IDD_PANEL_CAM_PANO_CLIP,
	IDS_CLIPPING,
	0,
	0,
	NULL,
		// Nearclip
		clip_param_nearclip,
		_T("nearclip"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_NEARCLIP,
		p_default, 0.1f,
		p_range, 0.001f, 1.0e12f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_NEARCLIP, IDC_SPIN_NEARCLIP, 0.1f,
		p_end,
		// Farclip
		clip_param_farclip,
		_T("farclip"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_FARCLIP,
		p_default, 2000.0f,
		p_range, 0.001f, 1.0e12f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_FARCLIP, IDC_SPIN_FARCLIP, 0.1f,
		p_end,
	p_end
);

static ParamBlockDesc2 pano_cam_stereo_pblock_desc(
	// Pblock data
	cam_pano_pblock_stereo,
	_T("pblock_stereo"),
	0,
	GetCyclesCameraPanoramaClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF_STEREO,
	// Interface stuff
	IDD_PANEL_CAM_PANO_STEREO,
	IDS_STEREOSCOPY,
	0,
	0,
	NULL,
		// Spherical stereo
		stereo_param_use_spherical_stereo,
		_T("use_spherical_stereo"),
		TYPE_BOOL,
		P_ANIMATABLE,
		IDS_USE_SPHERICAL_STEREO,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHECKBOX, IDC_BOOL_SPHERICAL_STEREO,
		p_end,
		// Pole merge
		stereo_param_use_pole_merge,
		_T("use_pole_merge"),
		TYPE_BOOL,
		P_ANIMATABLE,
		IDS_USE_POLE_MERGE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHECKBOX, IDC_BOOL_POLE_MERGE,
		p_end,
		// Merge from
		stereo_param_merge_from,
		_T("pole_merge_from"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_POLE_MERGE_FROM,
		p_default, 60.0f,
		p_range, 0.0f, 90.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_MERGE_BEGIN, IDC_SPIN_MERGE_BEGIN, 1.0f,
		p_end,
		// Merge to
		stereo_param_merge_to,
		_T("pole_merge_to"),
		TYPE_FLOAT,
		P_ANIMATABLE,
		IDS_POLE_MERGE_TO,
		p_default, 75.0f,
		p_range, 0.0f, 90.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_MERGE_END, IDC_SPIN_MERGE_END, 1.0f,
		p_end,
	p_end
);

class CyclesPanoramaCreateMouseCallback : public CreateMouseCallBack
{
public:
	virtual int proc(ViewExp* vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat) override;
	void set_camera(CyclesPanoramaCamera* camera_in);

private:
	CyclesPanoramaCamera* camera = nullptr;
};

int CyclesPanoramaCreateMouseCallback::proc(ViewExp* const vpt, const int msg, const int /*point*/, const int /*flags*/, IPoint2 m, Matrix3& mat)
{
	if (vpt == nullptr || vpt->IsAlive() == false) {
		return FALSE;
	}

	if (msg == MOUSE_ABORT) {
		return CREATE_ABORT;
	}
	else if (msg == MOUSE_MOVE) {
		mat.SetTrans(vpt->SnapPoint(m, m, nullptr, SNAP_IN_3D));
		return TRUE;
	}
	else if (msg == MOUSE_POINT) {
		mat.SetTrans(vpt->SnapPoint(m, m, nullptr, SNAP_IN_3D));
		if (camera != nullptr) {
			camera->Enable(1);
		}
		return FALSE;
	}

	return TRUE;
}


void CyclesPanoramaCreateMouseCallback::set_camera(CyclesPanoramaCamera* const camera_in)
{
	camera = camera_in;
}

class PanoCameraSplineItem : public SplineRenderItem
{
public:
	PanoCameraSplineItem();

	virtual void Realize(DrawContext& draw_context) override;
};

PanoCameraSplineItem::PanoCameraSplineItem() : SplineRenderItem()
{

}

void PanoCameraSplineItem::Realize(DrawContext& draw_context)
{
	ViewExp* const vpt = const_cast<ViewExp*>(draw_context.GetViewExp());
	if (vpt == nullptr || !vpt->IsAlive())
	{
		return;
	}

	INode* const inode = draw_context.GetCurrentNode();
	if (inode == nullptr) {
		return;
	}

	Color line_color(0.0, 0.0, 0.0);
	if (inode->Selected()) {
		line_color = Color(GetSelColor());
	}
	else if (inode->IsFrozen()) {
		line_color = Color(GetFreezeColor());
	}
	else {
		line_color = Color(inode->GetWireColor());
	}

	ClearLines();

	// Vertical line
	{
		const size_t vert_count = 2;
		Point3 verts[vert_count] = {
			Point3(0.0f, 0.0f, -4.0f),
			Point3(0.0f, 0.0f,  4.0f)
		};

		AddLineStrip(verts, line_color, vert_count, false, false);
	}

	// Top square
	{
		const size_t vert_count = 4;
		Point3 verts[vert_count] = {
			Point3( 2.0f,  2.0f, 2.0f),
			Point3( 2.0f, -2.0f, 2.0f),
			Point3(-2.0f, -2.0f, 2.0f),
			Point3(-2.0f,  2.0f, 2.0f)
		};

		AddLineStrip(verts, line_color, vert_count, true, false);
	}

	// Bottom square
	{
		const size_t vert_count = 4;
		Point3 verts[vert_count] = {
			Point3( 2.0f,  2.0f, -2.0f),
			Point3( 2.0f, -2.0f, -2.0f),
			Point3(-2.0f, -2.0f, -2.0f),
			Point3(-2.0f,  2.0f, -2.0f)
		};

		AddLineStrip(verts, line_color, vert_count, true, false);
	}

	// Vertical lines of cube
	{
		const size_t vert_count = 8;
		Point3 verts[vert_count] = {
			Point3( 2.0f,  2.0f, -2.0f),
			Point3( 2.0f,  2.0f,  2.0f),
			Point3( 2.0f, -2.0f,  2.0f),
			Point3( 2.0f, -2.0f, -2.0f),
			Point3(-2.0f, -2.0f, -2.0f),
			Point3(-2.0f, -2.0f,  2.0f),
			Point3(-2.0f,  2.0f,  2.0f),
			Point3(-2.0f,  2.0f, -2.0f)
		};

		AddLineStrip(verts, line_color, vert_count, true, false);
	}

	// Forward line
	{
		const size_t vert_count = 2;
		Point3 verts[vert_count] = {
			Point3(0.0f, 0.0f, 0.0f),
			Point3(4.0f, 0.0f, 0.0f)
		};

		AddLineStrip(verts, line_color, vert_count, false, false);
	}

	// Forward arrow
	{
		const size_t vert_count = 3;
		Point3 verts[vert_count] = {
			Point3(3.5f,  0.5f, 0.0f),
			Point3(4.0f,  0.0f, 0.0f),
			Point3(3.5f, -0.5f, 0.0f)
		};

		AddLineStrip(verts, line_color, vert_count, false, false);
	}
	{
		const size_t vert_count = 3;
		Point3 verts[vert_count] = {
			Point3(3.5f, 0.0f,  0.5f),
			Point3(4.0f, 0.0f,  0.0f),
			Point3(3.5f, 0.0f, -0.5f)
		};

		AddLineStrip(verts, line_color, vert_count, false, false);
	}


	// Bottom cross
	{
		const size_t vert_count = 2;
		Point3 verts[vert_count] = {
			Point3(-2.0f, 0.0f, -4.0f),
			Point3( 2.0f, 0.0f, -4.0f)
		};

		AddLineStrip(verts, line_color, vert_count, false, false);
	}
	{
		const size_t vert_count = 2;
		Point3 verts[vert_count] = {
			Point3(0.0f, -2.0f, -4.0f),
			Point3(0.0f,  2.0f, -4.0f)
		};

		AddLineStrip(verts, line_color, vert_count, false, false);
	}

	SplineRenderItem::Realize(draw_context);
}

CyclesPanoramaCamera::CyclesPanoramaCamera() : GenCamera()
{
	Reset();
	BuildMesh();
}

void CyclesPanoramaCamera::Reset()
{
	DeleteReference(PBLOCK_REF_MAIN);
	DeleteReference(PBLOCK_REF_CLIP);
	DeleteReference(PBLOCK_REF_STEREO);

	GetCyclesCameraPanoramaClassDesc()->MakeAutoParamBlocks(this);
}

CameraPanoramaType CyclesPanoramaCamera::GetPanoramaType(const TimeValue t, Interval& valid)
{
	const int int_val = pblock_main->GetInt(main_param_type, t, valid);
	if (int_val == main_type_fisheye_equidistant) {
		return CameraPanoramaType::FISHEYE_EQUIDISTANT;
	}
	else if (int_val == main_type_fisheye_equisolid) {
		return CameraPanoramaType::FISHEYE_EQUISOLID;
	}
	else {
		return CameraPanoramaType::EQUIRECTANGULAR;
	}
}

float CyclesPanoramaCamera::GetMinLattitude(const TimeValue t, Interval& valid)
{
	const float degrees = pblock_main->GetFloat(main_param_er_min_lat, t, valid);
	return degrees * M_PI / 180.0f;
}


float CyclesPanoramaCamera::GetMaxLattitude(const TimeValue t, Interval& valid)
{
	const float degrees = pblock_main->GetFloat(main_param_er_max_lat, t, valid);
	return degrees * M_PI / 180.0f;
}

float CyclesPanoramaCamera::GetMinLongitude(const TimeValue t, Interval& valid)
{
	const float degrees = pblock_main->GetFloat(main_param_er_min_long, t, valid);
	return degrees * M_PI / 180.0f;
}


float CyclesPanoramaCamera::GetMaxLongitude(const TimeValue t, Interval& valid)
{
	const float degrees = pblock_main->GetFloat(main_param_er_max_long, t, valid);
	return degrees * M_PI / 180.0f;
}

float CyclesPanoramaCamera::GetFisheyeFov(const TimeValue t, Interval& valid)
{
	const float degrees = pblock_main->GetFloat(main_param_fe_fov, t, valid);
	return degrees * M_PI / 180.0f;
}

float CyclesPanoramaCamera::GetFisheyeFocalLength(const TimeValue t, Interval& valid)
{
	return pblock_main->GetFloat(main_param_fe_focal_length, t, valid);
}

float CyclesPanoramaCamera::GetFisheyeSensorSize(const TimeValue t, Interval& valid)
{
	return pblock_main->GetFloat(main_param_fe_sensor_size, t, valid);
}

float CyclesPanoramaCamera::GetNearClip(const TimeValue t, Interval& valid)
{
	return pblock_clip->GetFloat(clip_param_nearclip, t, valid);
}

float CyclesPanoramaCamera::GetFarClip(const TimeValue t, Interval& valid)
{
	return pblock_clip->GetFloat(clip_param_farclip, t, valid);
}

bool CyclesPanoramaCamera::GetUseSphericalStereo(const TimeValue t, Interval& valid)
{
	const BOOL result = pblock_stereo->GetInt(stereo_param_use_spherical_stereo, t, valid);
	if (result == FALSE) {
		return false;
	}
	return true;
}

bool CyclesPanoramaCamera::GetUsePoleMerge(const TimeValue t, Interval& valid)
{
	const BOOL result = pblock_stereo->GetInt(stereo_param_use_pole_merge, t, valid);
	if (result == FALSE) {
		return false;
	}
	return true;
}

float CyclesPanoramaCamera::GetPoleMergeFrom(const TimeValue t, Interval& valid)
{
	return pblock_stereo->GetFloat(stereo_param_merge_from, t, valid);
}

float CyclesPanoramaCamera::GetPoleMergeTo(const TimeValue t, Interval& valid)
{
	return pblock_stereo->GetFloat(stereo_param_merge_to, t, valid);
}

GenCamera* CyclesPanoramaCamera::NewCamera(const int /*type*/)
{
	return new CyclesPanoramaCamera();
}

void CyclesPanoramaCamera::SetConeState(const int /*state*/)
{
	// Do nothing
}

int CyclesPanoramaCamera::GetConeState()
{
	return 0;
}

void CyclesPanoramaCamera::SetHorzLineState(const int /*state*/)
{
	// Do nothing
}

int CyclesPanoramaCamera::GetHorzLineState()
{
	return FALSE;
}

BOOL CyclesPanoramaCamera::SetFOVControl(Control* const /*control*/)
{
	return FALSE;
}

Control* CyclesPanoramaCamera::GetFOVControl()
{
	return nullptr;
}

void CyclesPanoramaCamera::SetFOVType(const int /*type*/)
{
	// Do nothing
}

int CyclesPanoramaCamera::GetFOVType()
{
	return 1; // Height-based FOV
}

void CyclesPanoramaCamera::SetType(const int /*type*/)
{
	// Do nothing
}

int CyclesPanoramaCamera::Type()
{
	return FREE_CAMERA;
}

void CyclesPanoramaCamera::Enable(const int enab)
{
	if (enab != 0) {
		enabled = true;
	}
	else {
		enabled = false;
	}

	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

int CyclesPanoramaCamera::UsesWireColor()
{
	return TRUE;
}

RefResult CyclesPanoramaCamera::EvalCameraState(const TimeValue t, Interval& valid, CameraState* const cs)
{
	if (cs == nullptr) {
		return REF_FAIL;
	}

	valid = FOREVER;

	cs->isOrtho = this->IsOrtho();
	cs->fov = this->GetFOV(t, valid);
	cs->tdist = this->GetTDist(t, valid);
	cs->horzLine = this->GetHorzLineState();
	cs->manualClip = this->GetManualClip();
	cs->hither = 0.1f;
	cs->yon = 1.0f;
	cs->nearRange = 0.1f;
	cs->farRange = 1.0f;

	return REF_SUCCEED;
}

void CyclesPanoramaCamera::RenderApertureChanged(const TimeValue /*t*/)
{
	// Do nothing
}

void CyclesPanoramaCamera::SetOrtho(BOOL /*ortho*/)
{
	// Do nothing
}

BOOL CyclesPanoramaCamera::IsOrtho()
{
	return FALSE;
}

void CyclesPanoramaCamera::SetFOV(const TimeValue /*t*/, const float /*fov*/)
{
	// Do nothing
}

float CyclesPanoramaCamera::GetFOV(const TimeValue /*t*/, Interval& valid)
{
	valid &= FOREVER;
	return 45.0f;
}

void CyclesPanoramaCamera::SetTDist(const TimeValue /*t*/, const float /*t_dist*/)
{
	// Do nothing
}

float CyclesPanoramaCamera::GetTDist(const TimeValue /*t*/, Interval& valid)
{
	valid &= FOREVER;
	return 1.0f;
}

void CyclesPanoramaCamera::SetManualClip(const int /*clip*/)
{
	// Do nothing
}

int CyclesPanoramaCamera::GetManualClip()
{
	return 0;
}

void CyclesPanoramaCamera::SetClipDist(const TimeValue /*t*/, const int /*which*/, const float /*val*/)
{
	// Do nothing
}

float CyclesPanoramaCamera::GetClipDist(const TimeValue /*t*/, const int /*which*/, Interval& valid)
{
	valid &= FOREVER;
	return 1.0f;
}

void CyclesPanoramaCamera::SetEnvRange(const TimeValue /*t*/, const int /*which*/, const float /*val*/)
{
	// Do nothing
}

float  CyclesPanoramaCamera::GetEnvRange(const TimeValue /*t*/, const int /*which*/, Interval& valid)
{
	valid &= FOREVER;
	return 1.0f;
}

void CyclesPanoramaCamera::SetEnvDisplay(const BOOL /*b*/, const int /*notify*/)
{
	// Do nothing
}

BOOL CyclesPanoramaCamera::GetEnvDisplay()
{
	return TRUE;
}

static CyclesPanoramaCreateMouseCallback create_mouse_callback;
CreateMouseCallBack* CyclesPanoramaCamera::GetCreateMouseCallBack()
{
	create_mouse_callback.set_camera(this);
	return &create_mouse_callback;
}

int CyclesPanoramaCamera::HitTest(
	const TimeValue t,
	INode* const inode,
	const int type,
	const int crossing,
	const int flags,
	IPoint2* const p,
	ViewExp* const vpt
	)
{
	if (vpt == nullptr || vpt->IsAlive() == false) {
		return 0;
	}

	if (enabled == false) {
		return 0;
	}

	HitRegion hit_region;
	MakeHitRegion(hit_region, type, crossing, 4, p);

	GraphicsWindow* const gw = vpt->getGW();
	const DWORD rlim = gw->getRndLimits();
	gw->setRndLimits((rlim | GW_PICK) & ~GW_ILLUM);
	const Matrix3 m = inode->GetObjectTM(t);
	gw->setTransform(m);
	gw->clearHitCode();

	const int result = mesh.select(gw, gw->getMaterial(), &hit_region, flags & HIT_ABORTONHIT);

	gw->setRndLimits(rlim);

	return result;
}

void CyclesPanoramaCamera::GetLocalBoundBox(const TimeValue /*t*/, INode* const /*mat*/, ViewExp* const /*vpt*/, Box3& box)
{
	box = mesh.getBoundingBox();
	box.Scale(1.5f);
}

void CyclesPanoramaCamera::GetWorldBoundBox(const TimeValue t, INode* const mat, ViewExp* const /*vpt*/, Box3& box)
{
	Matrix3 matrix = mat->GetObjectTM(t);
	box = mesh.getBoundingBox(&matrix);
	box.Scale(1.5f);
}

bool CyclesPanoramaCamera::PrepareDisplay(const MaxSDK::Graphics::UpdateDisplayContext& update_display_context)
{
	IMeshDisplay2* const md2 = static_cast<IMeshDisplay2*>(mesh.GetInterface(IMesh_DISPLAY2_INTERFACE_ID));
	if (md2 == nullptr) {
		return false;
	}

	GenerateMeshRenderItemsContext gmric;
	gmric.GenerateDefaultContext(update_display_context);
	md2->PrepareDisplay(gmric);

	return true;
}

bool CyclesPanoramaCamera::UpdatePerNodeItems(
	const MaxSDK::Graphics::UpdateDisplayContext& /*update_display_context*/,
	MaxSDK::Graphics::UpdateNodeContext& /*node_context*/,
	MaxSDK::Graphics::IRenderItemContainer& render_item_container
	)
{
	{
		PanoCameraSplineItem* spline_item = new PanoCameraSplineItem();

		CustomRenderItemHandle render_item_handle;
		render_item_handle.Initialize();
		render_item_handle.SetVisibilityGroup(MaxSDK::Graphics::RenderItemVisible_Gizmo);
		render_item_handle.SetCustomImplementation(spline_item);

		render_item_container.AddRenderItem(render_item_handle);
	}

	return true;
}

ObjectState CyclesPanoramaCamera::Eval(TimeValue /*t*/)
{
	return ObjectState(this);
}

SClass_ID CyclesPanoramaCamera::SuperClassID()
{
	return CAMERA_CLASS_ID;
}

Class_ID CyclesPanoramaCamera::ClassID()
{
	return CYCLES_CAMERA_PANORAMIC_CLASS;
}

int CyclesPanoramaCamera::NumSubs()
{
	return PBLOCK_COUNT;
}

Animatable* CyclesPanoramaCamera::SubAnim(const int i)
{
	return GetReference(i);
}

#if PLUGIN_SDK_VERSION < 2022
MSTR CyclesPanoramaCamera::SubAnimName(const int i)
#else
MSTR CyclesPanoramaCamera::SubAnimName(const int i, const bool)
#endif
{
	if (i == PBLOCK_REF_MAIN) {
		return MSTR(L"pblock_main");
	}
	else if (i == PBLOCK_REF_CLIP) {
		return MSTR(L"pblock_clip");
	}
	else if (i == PBLOCK_REF_STEREO) {
		return MSTR(L"pblock_stereo");
	}

	return MSTR(L"");
}

int CyclesPanoramaCamera::NumRefs()
{
	return PBLOCK_COUNT;
}

RefTargetHandle CyclesPanoramaCamera::GetReference(const int i)
{
	return GetParamBlock(i);
}

int CyclesPanoramaCamera::NumParamBlocks()
{
	return PBLOCK_COUNT;
}

IParamBlock2* CyclesPanoramaCamera::GetParamBlock(const int i)
{
	if (i == PBLOCK_REF_MAIN) {
		return pblock_main;
	}
	else if (i == PBLOCK_REF_CLIP) {
		return pblock_clip;
	}
	else if (i == PBLOCK_REF_STEREO) {
		return pblock_stereo;
	}

	return nullptr;
}

IParamBlock2* CyclesPanoramaCamera::GetParamBlockByID(const BlockID id)
{
	if (pblock_main != nullptr && pblock_main->ID() == id) {
		return pblock_main;
	}

	if (pblock_clip != nullptr && pblock_clip->ID() == id) {
		return pblock_clip;
	}

	if (pblock_stereo != nullptr && pblock_stereo->ID() == id) {
		return pblock_stereo;
	}

	return nullptr;
}

RefTargetHandle CyclesPanoramaCamera::Clone(RemapDir &remap)
{
	CyclesPanoramaCamera* const new_cam = static_cast<CyclesPanoramaCamera*>(GetCyclesCameraPanoramaClassDesc()->Create());

	BaseClone(this, new_cam, remap);
	new_cam->ReplaceReference(PBLOCK_REF_MAIN, remap.CloneRef(pblock_main));
	new_cam->ReplaceReference(PBLOCK_REF_CLIP, remap.CloneRef(pblock_clip));
	new_cam->ReplaceReference(PBLOCK_REF_STEREO, remap.CloneRef(pblock_stereo));

	return new_cam;
}

void CyclesPanoramaCamera::DeleteThis()
{
	delete this;
}

void CyclesPanoramaCamera::BeginEditParams(IObjParam* const ip, const ULONG flags, Animatable* const prev)
{
	GetCyclesCameraPanoramaClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void CyclesPanoramaCamera::EndEditParams(IObjParam* const ip, const ULONG flags, Animatable* const next)
{
	GetCyclesCameraPanoramaClassDesc()->EndEditParams(ip, this, flags, next);
}

RefResult CyclesPanoramaCamera::NotifyRefChanged(const Interval& /*change_int*/, RefTargetHandle h_target, PartID& /*part_id*/, RefMessage message, BOOL /*propagate*/)
{
	Interval update_valid = FOREVER;

	switch (message)
	{
		case REFMSG_CHANGE:
			if (h_target == pblock_main) {
				const ParamID changing_param = pblock_main->LastNotifyParamID();
				pano_cam_pblock_desc.InvalidateUI(changing_param);
			}
			else if (h_target == pblock_clip) {
				const ParamID changing_param = pblock_clip->LastNotifyParamID();
				pano_cam_clip_pblock_desc.InvalidateUI(changing_param);
			}
			else if (h_target == pblock_stereo) {
				const ParamID changing_param = pblock_stereo->LastNotifyParamID();
				pano_cam_stereo_pblock_desc.InvalidateUI(changing_param);
			}
			break;

		case REFMSG_TARGET_DELETED:
			if (h_target == pblock_main) {
				pblock_main = nullptr;
			}
			else if (h_target == pblock_clip) {
				pblock_clip = nullptr;
			}
			else if (h_target == pblock_stereo) {
				pblock_stereo = nullptr;
			}
			break;

		default:
			break;
	}

	return REF_SUCCEED;
}

void CyclesPanoramaCamera::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF_MAIN) {
		pblock_main = static_cast<IParamBlock2*>(rtarg);
	}
	else if (i == PBLOCK_REF_CLIP) {
		pblock_clip = static_cast<IParamBlock2*>(rtarg);
	}
	else if (i == PBLOCK_REF_STEREO) {
		pblock_stereo = static_cast<IParamBlock2*>(rtarg);
	}
}

static void SetupQuad(Face* face1, Face* face2, DWORD v0, DWORD v1, DWORD v2, DWORD v3)
{
	face1->v[0] = v0;
	face1->v[1] = v1;
	face1->v[2] = v2;
	face1->setEdgeVis(0, EDGE_VIS);
	face1->setEdgeVis(1, EDGE_VIS);
	face1->setEdgeVis(2, EDGE_INVIS);

	face2->v[0] = v2;
	face2->v[1] = v3;
	face2->v[2] = v0;
	face2->setEdgeVis(0, EDGE_VIS);
	face2->setEdgeVis(1, EDGE_VIS);
	face2->setEdgeVis(2, EDGE_INVIS);
}

void CyclesPanoramaCamera::BuildMesh()
{
	const int VERT_COUNT = 8;
	const int FACE_COUNT = 12;

	mesh.setNumVerts(VERT_COUNT);
	mesh.setNumFaces(FACE_COUNT);

	mesh.setVert(0, -2.0f, -2.0f,  2.0f); // Cube top
	mesh.setVert(1,  2.0f, -2.0f,  2.0f);
	mesh.setVert(2,  2.0f,  2.0f,  2.0f);
	mesh.setVert(3, -2.0f,  2.0f,  2.0f);

	mesh.setVert(4, -2.0f, -2.0f, -2.0f); // Cube bottom
	mesh.setVert(5,  2.0f, -2.0f, -2.0f);
	mesh.setVert(6,  2.0f,  2.0f, -2.0f);
	mesh.setVert(7, -2.0f,  2.0f, -2.0f);

	Face* const face_px1 = mesh.faces + 0;
	Face* const face_px2 = mesh.faces + 1;
	Face* const face_nx1 = mesh.faces + 2;
	Face* const face_nx2 = mesh.faces + 3;

	SetupQuad(face_px1, face_px2, 5, 6, 2, 1);
	SetupQuad(face_nx1, face_nx2, 7, 4, 0, 3);

	Face* const face_py1 = mesh.faces + 4;
	Face* const face_py2 = mesh.faces + 5;
	Face* const face_ny1 = mesh.faces + 6;
	Face* const face_ny2 = mesh.faces + 7;

	SetupQuad(face_py1, face_py2, 6, 7, 3, 2);
	SetupQuad(face_ny1, face_ny2, 4, 5, 1, 0);

	Face* const face_pz1 = mesh.faces + 8;
	Face* const face_pz2 = mesh.faces + 9;
	Face* const face_nz1 = mesh.faces + 10;
	Face* const face_nz2 = mesh.faces + 11;

	SetupQuad(face_pz1, face_pz2, 0, 1, 2, 3);
	SetupQuad(face_nz1, face_nz2, 7, 6, 5, 4);

	mesh.buildNormals();
	mesh.EnableEdgeList(1);
}
