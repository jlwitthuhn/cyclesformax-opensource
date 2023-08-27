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
 
#include "plugin_urender_cycles.h"

#include <dllutilities.h>

#include "const_classid.h"
#include "max_rend_interactive_session.h"
#include "max_rend_offline_session.h"
#include "plugin_re_simple.h"
#include "rend_logger.h"
#include "ui_paramdialog.h"
#include "win_resource.h"

using MaxSDK::RenderingAPI::IInteractiveRenderSession;
using MaxSDK::RenderingAPI::IOfflineRenderSession;
using MaxSDK::RenderingAPI::IRenderSessionContext;
using MaxSDK::RenderingAPI::UnifiedRenderer;

extern HINSTANCE hInstance;

extern bool enable_plugin_debug;

CyclesRenderParams gui_render_params;

UnifiedRenderer* CyclesUnifiedRendererDescriptor::CreateRenderer(const bool loading)
{
	UNREFERENCED_PARAMETER(loading);
	return new CyclesUnifiedRenderer();
}

const TCHAR* CyclesUnifiedRendererDescriptor::InternalName()
{
	return _T("Cycles for Max Renderer");
}

HINSTANCE CyclesUnifiedRendererDescriptor::HInstance()
{
	return hInstance;
}

bool CyclesUnifiedRendererDescriptor::IsCompatibleWithMtlBase(ClassDesc& mtlBaseClassDesc)
{
	// Standard materials
	if (mtlBaseClassDesc.SuperClassID() == MATERIAL_CLASS_ID) {
		if (mtlBaseClassDesc.ClassID() == STANDARD_MATERIAL_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == MULTI_SUB_MATERIAL_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == PHYSICAL_MATERIAL_CLASS) {
			return true;
		}
	}

	// Standard maps
	if (mtlBaseClassDesc.SuperClassID() == TEXMAP_CLASS_ID) {
		if (mtlBaseClassDesc.ClassID() == BITMAP_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == CELLULAR_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == CHECKER_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == COLOR_CORRECTION_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == COMPOSITE_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == DENT_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == GRADIENT_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == GRADIENT_RAMP_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == MARBLE_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == MASK_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == MIX_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == NOISE_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == NORMALBUMP_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == OUTPUT_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == PERLIN_MARBLE_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == RGB_MULTIPLY_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == RGB_TINT_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == SHAPEMAP_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == SMOKE_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == SPECKLE_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == SPLAT_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == STUCCO_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == SWIRL_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == TEXTMAP_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == TILES_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == VECTORMAP_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == WAVES_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == WOOD_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == COLOR_MAP_CLASS) {
			return true;
		}
		else if (mtlBaseClassDesc.ClassID() == PHYS_SKY_MAP_CLASS) {
			return true;
		}
	}

	// Cycles materials
	if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_SHADER_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_SHADER_GRAPH_08_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_SHADER_GRAPH_16_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_SHADER_GRAPH_32_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_ADD_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_ANISOTROPIC_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_DIFFUSE_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_EMISSION_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_GLASS_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_GLOSSY_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_HAIR_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_HOLDOUT_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_MIX_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_PRINCIPLED_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_REFRACTION_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_SUBSURFACE_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_TOON_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_TRANSLUCENT_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_TRANSPARENT_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_VELVET_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_VOL_ABSORPTION_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_VOL_SCATTER_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_VOL_ADD_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_MAT_VOL_MIX_CLASS) {
		return true;
	}

	// Cycles maps
	if (mtlBaseClassDesc.ClassID() == CYCLES_TEXMAP_FILTER_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_TEXMAP_SKY_CLASS) {
		return true;
	}
	else if (mtlBaseClassDesc.ClassID() == CYCLES_TEXMAP_ENVIRONMENT_CLASS) {
		return true;
	}

	return false;
}

const MCHAR* CyclesUnifiedRendererDescriptor::ClassName()
{
	static const MSTR name = MaxSDK::GetResourceStringAsMSTR(IDS_CYCLES_UNIFIED_RENDERER);
	return name;
}

Class_ID CyclesUnifiedRendererDescriptor::ClassID()
{
	return CYCLES_UNIFIED_RENDERER_CLASS;
}

static CyclesUnifiedRendererDescriptor renderer_desc;

CyclesUnifiedRendererDescriptor& GetCyclesUnifiedRendererDescriptor()
{
	return renderer_desc;
}

//////
// Renderer begin
//////

CyclesUnifiedRenderer::CyclesUnifiedRenderer() : UnifiedRenderer()
{
	//global_log_manager.init();
}

UnifiedRenderer::ClassDescriptor& CyclesUnifiedRenderer::GetClassDescriptor() const
{
	return GetCyclesUnifiedRendererDescriptor();
}

std::unique_ptr<IOfflineRenderSession> CyclesUnifiedRenderer::CreateOfflineSession(IRenderSessionContext& sessionContext)
{
	global_log_manager.new_render_begin(LogLevel::DEBUG);
	return std::unique_ptr<IOfflineRenderSession>(new CyclesOfflineRenderSession(sessionContext, gui_render_params));
}

std::unique_ptr<IInteractiveRenderSession> CyclesUnifiedRenderer::CreateInteractiveSession(IRenderSessionContext& sessionContext)
{
	global_log_manager.new_render_begin(LogLevel::DEBUG);
	return std::unique_ptr<IInteractiveRenderSession>(new CyclesInteractiveRenderSession(sessionContext, gui_render_params));
}

bool CyclesUnifiedRenderer::SupportsInteractiveRendering() const
{
	return true;
}

RendParamDlg* CyclesUnifiedRenderer::CreateParamDialog(IRendParams* const ir, const BOOL prog)
{
	return new CyclesRendParamDlg(&gui_render_params, ir, prog);
}

bool CyclesUnifiedRenderer::HasRequirement(const Requirement requirement)
{
	if (requirement == Requirement::kRequirement_Wants32bitFPOutput) {
		return true;
	}
	// Default to false for unlisted requirements
	return false;
}


void CyclesUnifiedRenderer::GetVendorInformation(MSTR& info) const
{
	info = _T("Cycles for Max Renderer");
}

void CyclesUnifiedRenderer::GetPlatformInformation(MSTR& info) const
{
	info = _T("3ds Max 2017-2023");
}

IOResult CyclesUnifiedRenderer::Load_UnifiedRenderer(ILoad& iload)
{
	gui_render_params.SetDefaults();
	return gui_render_params.LoadChunks(iload);
}

IOResult CyclesUnifiedRenderer::Save_UnifiedRenderer(ISave& isave) const
{
	return gui_render_params.SaveChunks(isave);
}

void* CyclesUnifiedRenderer::GetInterface_UnifiedRenderer(const ULONG /*id*/)
{
	return nullptr;
}

BaseInterface* CyclesUnifiedRenderer::GetInterface_UnifiedRenderer(Interface_ID /*id*/)
{
	return nullptr;
}

bool CyclesUnifiedRenderer::MotionBlurIgnoresNodeProperties() const
{
	return true;
}

bool CyclesUnifiedRenderer::IsStopSupported() const
{
	return true;
}

Renderer::PauseSupport CyclesUnifiedRenderer::IsPauseSupported() const
{
	return PauseSupport::None;
}

int CyclesUnifiedRenderer::NumParamBlocks()
{
	return 0;
}

IParamBlock2* CyclesUnifiedRenderer::GetParamBlock(const int /*i*/)
{
	return nullptr;
}

IParamBlock2* CyclesUnifiedRenderer::GetParamBlockByID(const BlockID /*id*/)
{
	return nullptr;
}

void CyclesUnifiedRenderer::AddTabToDialog(ITabbedDialog* const dialog, ITabDialogPluginTab* const tab)
{
	dialog->AddRollout(
		_T("Cycles"),
		NULL,
		CYCLES_TAB_MAIN_CLASS,
		tab,
		-1,
		222,
		0,
		0,
		ITabbedDialog::kSystemPage
	);
	dialog->AddRollout(
		_T("Advanced"),
		NULL,
		CYCLES_TAB_ADVANCED_CLASS,
		tab,
		-1,
		222,
		0,
		0,
		ITabbedDialog::kSystemPage
	);
	dialog->AddRollout(
		_T("About"),
		NULL,
		CYCLES_TAB_ABOUT_CLASS,
		tab,
		-1,
		222,
		0,
		0,
		ITabbedDialog::kSystemPage
	);
	if (enable_plugin_debug) {
		dialog->AddRollout(
			_T("Debug"),
			NULL,
			CYCLES_TAB_DEBUG_CLASS,
			tab,
			-1,
			222,
			0,
			0,
			ITabbedDialog::kSystemPage
		);
	}
}

int CyclesUnifiedRenderer::NumRefs()
{
	return 0;
}

RefTargetHandle CyclesUnifiedRenderer::GetReference(const int /*i*/)
{
	return nullptr;
}

void CyclesUnifiedRenderer::SetReference(const int /*i*/, const RefTargetHandle /*rtarg*/)
{
	// Do nothing
}

RefTargetHandle CyclesUnifiedRenderer::Clone(RemapDir &remap)
{
	return UnifiedRenderer::Clone(remap);
}

bool CyclesUnifiedRenderer::CompatibleWithAnyRenderElement() const
{
	return true;
}

bool CyclesUnifiedRenderer::CompatibleWithRenderElement(IRenderElement& plRenderElement) const
{
	IRenderElement* const ptr{ &plRenderElement };
	CyclesRenderElement* const cycles_ptr{ dynamic_cast<CyclesRenderElement*>(ptr) };
	if (cycles_ptr != nullptr) {
		return true;
	}
	return false;
}

bool CyclesUnifiedRenderer::GetEnableInteractiveMEditSession() const
{
	return true;
}
