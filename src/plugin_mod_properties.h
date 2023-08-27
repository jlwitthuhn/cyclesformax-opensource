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
 * @brief Defines modifier plugin CyclesPropertiesMod.
 */

#include <object.h>

/**
 * @brief Modifier plugin class that allows the object-level properties to be set.
 */
class CyclesPropertiesMod : public OSModifier {
public:
	CyclesPropertiesMod();
	
	// Access to properties
	bool GetIsShadowCatcher(TimeValue t);

	// From Modifier
	virtual ChannelMask ChannelsUsed() override;
	virtual ChannelMask ChannelsChanged() override;
	virtual Class_ID InputType() override;
	virtual void ModifyObject(TimeValue t, ModContext& mc, ObjectState* os, INode* node) override;

	// From Animatable
	virtual void DeleteThis() override;
#if PLUGIN_SDK_VERSION < 2022
	virtual void GetClassName(MSTR& s) override;
#else
	virtual void GetClassName(MSTR& s, bool localized) const override;
#endif
	virtual SClass_ID SuperClassID() override;
	virtual Class_ID ClassID() override;

	// From BaseObject
	virtual CreateMouseCallBack* GetCreateMouseCallBack() override;
#if PLUGIN_SDK_VERSION < 2022
	virtual const MCHAR* GetObjectName() override;
#else
	virtual const MCHAR* GetObjectName(bool localized) const override;
#endif
	virtual void NotifyPostCollapse(INode* node, Object* obj, IDerivedObject* derObj, int index) override;

	// From ReferenceTarget
	virtual RefTargetHandle Clone(RemapDir& remap) override;

	// Pblock management
	virtual int NumSubs() override;
	virtual Animatable* SubAnim(int i) override;
#if PLUGIN_SDK_VERSION < 2022
	virtual TSTR SubAnimName(int i) override;
#else
	virtual TSTR SubAnimName(int i, bool localized) override;
#endif
	virtual int SubNumToRefNum(int subNum) override;
	virtual int NumRefs() override;
	virtual RefTargetHandle GetReference(int i) override;
	virtual void SetReference(int i, RefTargetHandle rtarg) override;
	virtual RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;
	virtual int NumParamBlocks() override;
	virtual IParamBlock2* GetParamBlock(int i) override;
	virtual IParamBlock2* GetParamBlockByID(BlockID id) override;

	// UI
	virtual void BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev) override;
	virtual void EndEditParams(IObjParam* ip, ULONG flags, Animatable* next) override;

private:
	IParamBlock2* pblock_general = nullptr;
};
