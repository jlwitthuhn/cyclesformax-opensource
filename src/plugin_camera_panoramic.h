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
 * @brief Defines all plugin camera classes provided by this plugin.
 */

#include <gencam.h>

#include "util_enums.h"

/**
 * @brief The plugin class used to provide a panoramic camera.
 */
class CyclesPanoramaCamera : public GenCamera
{
public:
	CyclesPanoramaCamera();

	void Reset();

	// Get configurable parameters
	CameraPanoramaType GetPanoramaType(TimeValue t, Interval& valid);

	float GetMinLattitude(TimeValue t, Interval& valid);
	float GetMaxLattitude(TimeValue t, Interval& valid);
	float GetMinLongitude(TimeValue t, Interval& valid);
	float GetMaxLongitude(TimeValue t, Interval& valid);

	float GetFisheyeFov(TimeValue t, Interval& valid);
	float GetFisheyeFocalLength(TimeValue t, Interval& valid);
	float GetFisheyeSensorSize(TimeValue t, Interval& valid);

	float GetNearClip(TimeValue t, Interval& valid);
	float GetFarClip(TimeValue t, Interval& valid);

	bool GetUseSphericalStereo(TimeValue t, Interval& valid);
	bool GetUsePoleMerge(TimeValue t, Interval& valid);
	float GetPoleMergeFrom(TimeValue t, Interval& valid);
	float GetPoleMergeTo(TimeValue t, Interval& valid);

	// From GenCamera
	virtual GenCamera* NewCamera(int type) override;

	virtual void SetConeState(int state) override;
	virtual int GetConeState() override;
	virtual void SetHorzLineState(int state) override;
	virtual int GetHorzLineState() override;
	virtual BOOL SetFOVControl(Control* c) override;
	virtual Control* GetFOVControl() override;
	virtual void SetFOVType(int type) override;
	virtual int GetFOVType() override;
	virtual void SetType(int type) override;
	virtual int Type() override;

	virtual void Enable(int enab) override;

	// From CameraObject
	virtual int UsesWireColor() override;
	virtual RefResult EvalCameraState(TimeValue t, Interval& valid, CameraState* cs) override;
	virtual void RenderApertureChanged(TimeValue t) override;

	virtual void SetOrtho(BOOL ortho) override;
	virtual BOOL IsOrtho() override;
	virtual void SetFOV(TimeValue t, float fov) override;
	virtual float GetFOV(TimeValue t, Interval& valid) override;
	virtual void SetTDist(TimeValue t, float t_dist) override;
	virtual float GetTDist(TimeValue t, Interval& valid) override;
	virtual void SetManualClip(int clip) override;
	virtual int GetManualClip() override;
	virtual void SetClipDist(TimeValue t, int which, float val) override;
	virtual float GetClipDist(TimeValue t, int which, Interval& valid) override;
	virtual void SetEnvRange(TimeValue t, int which, float val) override;
	virtual float GetEnvRange(TimeValue t, int which, Interval& valid) override;
	virtual void SetEnvDisplay(BOOL b, int notify) override;
	virtual BOOL GetEnvDisplay() override;

	// From BaseObject
	virtual CreateMouseCallBack* GetCreateMouseCallBack() override;

	virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2* p, ViewExp* vpt) override;
	virtual void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp* vpt, Box3& box) override;
	virtual void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp* vpt, Box3& box) override;

	virtual bool PrepareDisplay(const MaxSDK::Graphics::UpdateDisplayContext& context) override;
	virtual bool UpdatePerNodeItems(
		const MaxSDK::Graphics::UpdateDisplayContext& update_display_context,
		MaxSDK::Graphics::UpdateNodeContext& node_context,
		MaxSDK::Graphics::IRenderItemContainer& render_item_container ) override;

	// From Object
	virtual ObjectState Eval(TimeValue t) override;

	// From Animatable
	virtual SClass_ID SuperClassID() override;
	virtual Class_ID ClassID() override;

	virtual int NumSubs() override;
	virtual Animatable* SubAnim(int i) override;
#if PLUGIN_SDK_VERSION < 2022
	virtual MSTR SubAnimName(int i) override;
#else
	virtual MSTR SubAnimName(int i, bool localized) override;
#endif

	virtual int NumRefs() override;
	virtual RefTargetHandle GetReference(int i) override;

	virtual int NumParamBlocks() override;
	virtual IParamBlock2* GetParamBlock(int i) override;
	virtual IParamBlock2* GetParamBlockByID(BlockID id) override;

	virtual RefTargetHandle Clone(RemapDir& remap) override;

	virtual void DeleteThis() override;

	virtual void BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev) override;
	virtual void EndEditParams(IObjParam* ip, ULONG flags, Animatable* next) override;

protected:
	virtual RefResult NotifyRefChanged(const Interval& change_int, RefTargetHandle h_target, PartID& part_id, RefMessage message, BOOL propagate) override;
	virtual void SetReference(int i, RefTargetHandle rtarg) override;

private:
	void BuildMesh();

	bool enabled = true;
	Mesh mesh;

	IParamBlock2* pblock_main = nullptr;
	IParamBlock2* pblock_clip = nullptr;
	IParamBlock2* pblock_stereo = nullptr;
};
