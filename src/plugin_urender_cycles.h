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
 * @brief Defines renderer plugin class CyclesUnifiedRenderer.
 */

#include <RenderingAPI\Renderer\UnifiedRenderer.h>

/**
 * @brief ClassDesc for main renderer plugin class.
 */
class CyclesUnifiedRendererDescriptor : public MaxSDK::RenderingAPI::UnifiedRenderer::ClassDescriptor {
public:
	// from UnifiedRenderer::ClassDescriptor
	virtual MaxSDK::RenderingAPI::UnifiedRenderer* CreateRenderer(const bool loading) override;
	virtual	const MCHAR* InternalName() override;
	virtual	HINSTANCE HInstance() override;

	// from IMtlRender_Compatibility_Renderer
    virtual bool IsCompatibleWithMtlBase(ClassDesc& mtlBaseClassDesc) override;

	// from ClassDesc
	virtual const MCHAR* ClassName() override;
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual Class_ID ClassID() override;
};

CyclesUnifiedRendererDescriptor& GetCyclesUnifiedRendererDescriptor();

/**
* @brief Main Cycles Renderer plugin class implementing the new UnifiedRenderer interface.
*/
class CyclesUnifiedRenderer : public MaxSDK::RenderingAPI::UnifiedRenderer {
public:
	CyclesUnifiedRenderer();
	virtual ~CyclesUnifiedRenderer() override {}

	virtual UnifiedRenderer::ClassDescriptor& GetClassDescriptor() const override;

	// Render session creation
	virtual std::unique_ptr<MaxSDK::RenderingAPI::IOfflineRenderSession> CreateOfflineSession(MaxSDK::RenderingAPI::IRenderSessionContext& sessionContext) override;
	virtual std::unique_ptr<MaxSDK::RenderingAPI::IInteractiveRenderSession> CreateInteractiveSession(MaxSDK::RenderingAPI::IRenderSessionContext& sessionContext) override;
	virtual bool SupportsInteractiveRendering() const override;

	// From base Renderer
	virtual RendParamDlg* CreateParamDialog(IRendParams* ir, BOOL prog) override;
	virtual bool HasRequirement(Requirement requirement);
	virtual void GetVendorInformation(MSTR & info) const;
	virtual void GetPlatformInformation(MSTR & info) const;

	// IO
	virtual IOResult Load_UnifiedRenderer(ILoad& iload) override;
	virtual IOResult Save_UnifiedRenderer(ISave& isave) const override;

	// InterfaceServer stuff
	virtual void* GetInterface_UnifiedRenderer(ULONG id) override;
	virtual BaseInterface* GetInterface_UnifiedRenderer(Interface_ID id) override;

	// Scene options
	virtual bool MotionBlurIgnoresNodeProperties() const override;

	// Process control
    virtual bool IsStopSupported() const override;
	virtual PauseSupport IsPauseSupported() const override;

	// ParamBlock2 stuff (not used by this class)
	virtual int NumParamBlocks() override;
	virtual IParamBlock2* GetParamBlock(int i) override;
	virtual IParamBlock2* GetParamBlockByID(BlockID id) override;

	// Tabs
	virtual void AddTabToDialog(ITabbedDialog* dialog, ITabDialogPluginTab* tab) override;

	// Reference stuff
	virtual int NumRefs() override;
	virtual RefTargetHandle GetReference(int i) override;
	virtual void SetReference(int i, RefTargetHandle rtarg) override;

	// Other Max nonsense
	virtual RefTargetHandle Clone(RemapDir &remap) override;

protected:
	// From UnifiedRenderer
	virtual bool CompatibleWithAnyRenderElement() const override;
	virtual bool CompatibleWithRenderElement(IRenderElement& pIRenderElement) const override;
	virtual bool GetEnableInteractiveMEditSession() const override;

private:

};
