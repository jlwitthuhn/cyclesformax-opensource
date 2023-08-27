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
 
#include "ui_paramdialog.h"

#include <array>
#include <sstream>
#include <vector>

#include <custcont.h>
#include <3dsmaxdlport.h>
#include <Rendering/IRendParams.h>

#include "const_classid.h"
#include "const_tooltip.h"
#include "rend_params.h"
#include "util_cycles_device.h"
#include "win_resource.h"

extern HINSTANCE hInstance;

extern bool enable_plugin_debug;

// Do not write to this struct, this is just for reading params easily from a wndproc
extern CyclesRenderParams gui_render_params;

static std::array<wchar_t, 513> device_select_buffer;
static bool device_select_buffer_has_data = false;

static void fill_device_select_buffer(std::wstring string)
{
	device_select_buffer.fill(L'\0');
	const size_t usable_buffer_size = device_select_buffer.size() - 1;
	for (int i = 0; i < usable_buffer_size && i < string.size(); i++) {
		device_select_buffer[i] = string[i];
	}
}

static INT_PTR CALLBACK CudaMultiDeviceSelectProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM /*lParam*/)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		// Enable and set text for checkboxes here
		std::vector<int> checkbox_ids;
		checkbox_ids.push_back(IDC_BOOL_DEVICE_01);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_02);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_03);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_04);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_05);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_06);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_07);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_08);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_09);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_10);

		std::vector<HWND> checkboxes;
		for (size_t i = 0; i < checkbox_ids.size(); i++) {
			checkboxes.push_back(GetDlgItem(hWnd, checkbox_ids[i]));
		}

		RenderDeviceSelector device_selector(RenderDevice::CUDA, device_select_buffer.data());

		std::vector<CyclesDeviceDesc> cuda_desc_list = get_cuda_device_descs(gui_render_params.debug_multi_cuda);
		for (size_t i = 0; i < cuda_desc_list.size() && i < checkboxes.size(); i++) {
			CyclesDeviceDesc this_desc = cuda_desc_list[i];
			HWND this_checkbox = checkboxes[i];
			SetWindowText(this_checkbox, this_desc.desc.c_str());
			if (device_selector.use_device_id(this_desc.id)) {
				CheckDlgButton(hWnd, checkbox_ids[i], BST_CHECKED);
			}
			EnableWindow(this_checkbox, TRUE);
		}

		device_select_buffer_has_data = false;
		break;
	}

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_DEVICE_SINGLE:
			fill_device_select_buffer(L"SINGLE");
			device_select_buffer_has_data = true;
			EndDialog(hWnd, wParam);
			break;
		case IDC_BUTTON_DEVICE_ALL:
			fill_device_select_buffer(L"ALL");
			device_select_buffer_has_data = true;
			EndDialog(hWnd, wParam);
			break;
		case IDOK:
		{
			std::vector<CyclesDeviceDesc> cuda_desc_list = get_cuda_device_descs(gui_render_params.debug_multi_cuda);

			std::vector<int> checkbox_ids;
			checkbox_ids.push_back(IDC_BOOL_DEVICE_01);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_02);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_03);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_04);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_05);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_06);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_07);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_08);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_09);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_10);

			std::wstringstream out_stream;

			bool after_first_id = false;
			for (size_t i = 0; i < cuda_desc_list.size() && i < checkbox_ids.size(); i++) {
				if (IsDlgButtonChecked(hWnd, checkbox_ids[i])) {
					if (after_first_id == true) {
						out_stream << L',';
					}
					after_first_id = true;
					out_stream << cuda_desc_list[i].id << std::endl;
				}
			}

			fill_device_select_buffer(out_stream.str());
			device_select_buffer_has_data = true;
			EndDialog(hWnd, wParam);
			break;
		}
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			break;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK OptixMultiDeviceSelectProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM /*lParam*/)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		// Enable and set text for checkboxes here
		std::vector<int> checkbox_ids;
		checkbox_ids.push_back(IDC_BOOL_DEVICE_01);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_02);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_03);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_04);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_05);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_06);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_07);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_08);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_09);
		checkbox_ids.push_back(IDC_BOOL_DEVICE_10);

		std::vector<HWND> checkboxes;
		for (size_t i = 0; i < checkbox_ids.size(); i++) {
			checkboxes.push_back(GetDlgItem(hWnd, checkbox_ids[i]));
		}

		RenderDeviceSelector device_selector(RenderDevice::OPTIX, device_select_buffer.data());

		std::vector<CyclesDeviceDesc> optix_desc_list = get_optix_device_descs(gui_render_params.debug_multi_cuda);
		for (size_t i = 0; i < optix_desc_list.size() && i < checkboxes.size(); i++) {
			CyclesDeviceDesc this_desc = optix_desc_list[i];
			HWND this_checkbox = checkboxes[i];
			SetWindowText(this_checkbox, this_desc.desc.c_str());
			if (device_selector.use_device_id(this_desc.id)) {
				CheckDlgButton(hWnd, checkbox_ids[i], BST_CHECKED);
			}
			EnableWindow(this_checkbox, TRUE);
		}

		device_select_buffer_has_data = false;
		break;
	}

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_DEVICE_SINGLE:
			fill_device_select_buffer(L"SINGLE");
			device_select_buffer_has_data = true;
			EndDialog(hWnd, wParam);
			break;
		case IDC_BUTTON_DEVICE_ALL:
			fill_device_select_buffer(L"ALL");
			device_select_buffer_has_data = true;
			EndDialog(hWnd, wParam);
			break;
		case IDOK:
		{
			std::vector<CyclesDeviceDesc> optix_desc_list = get_optix_device_descs(gui_render_params.debug_multi_cuda);

			std::vector<int> checkbox_ids;
			checkbox_ids.push_back(IDC_BOOL_DEVICE_01);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_02);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_03);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_04);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_05);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_06);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_07);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_08);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_09);
			checkbox_ids.push_back(IDC_BOOL_DEVICE_10);

			std::wstringstream out_stream;

			bool after_first_id = false;
			for (size_t i = 0; i < optix_desc_list.size() && i < checkbox_ids.size(); i++) {
				if (IsDlgButtonChecked(hWnd, checkbox_ids[i])) {
					if (after_first_id == true) {
						out_stream << L',';
					}
					after_first_id = true;
					out_stream << optix_desc_list[i].id << std::endl;
				}
			}

			fill_device_select_buffer(out_stream.str());
			device_select_buffer_has_data = true;
			EndDialog(hWnd, wParam);
			break;
		}
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			break;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


static INT_PTR CALLBACK ProgressRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (CyclesRendParamDlg*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			if (dlg) {
				dlg->InitProgressRollup(hWnd);
			}
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
			break;

		default:
			return FALSE;
	} 
	return TRUE;
}

static INT_PTR CALLBACK UncategorizedConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (CyclesRendParamDlg*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			if (dlg) {
				dlg->InitUncategorizedConfig(hWnd);
			}
			break;

		case WM_DESTROY:
			ReleaseICustEdit(dlg->cudaGpuEdit);
			ReleaseICustEdit(dlg->optixGpuEdit);
			ReleaseISpinner(dlg->mtlPreviewSamplesSpinner);
			ReleaseICustEdit(dlg->mtlPreviewSamplesEdit);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_BUTTON_CUDA_GPU_SELECT) {
				TSTR original_device;
				dlg->cudaGpuEdit->GetText(original_device);
				fill_device_select_buffer(std::wstring(original_device.ToWStr()));
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CUDA_MULTI_DEVICE), hWnd, CudaMultiDeviceSelectProc);
				if (device_select_buffer_has_data) {
					dlg->cudaGpuEdit->SetText(device_select_buffer.data());
				}
				return TRUE;
			}
			else if (LOWORD(wParam) == IDC_BUTTON_OPTIX_GPU_SELECT) {
				TSTR original_device;
				dlg->optixGpuEdit->GetText(original_device);
				fill_device_select_buffer(std::wstring(original_device.ToWStr()));
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_OPTIX_MULTI_DEVICE), hWnd, OptixMultiDeviceSelectProc);
				if (device_select_buffer_has_data) {
					dlg->optixGpuEdit->SetText(device_select_buffer.data());
				}
				return TRUE;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
			break;

		default:
			return FALSE;
	}  
	return TRUE;
}

static INT_PTR CALLBACK SamplingConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (CyclesRendParamDlg*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			if (dlg) {
				dlg->InitSamplingConfig(hWnd);
			}
			break;

		case WM_DESTROY:
			ReleaseISpinner(dlg->samplesSpinner);
			ReleaseISpinner(dlg->adaptiveThresholdSpinner);
			ReleaseISpinner(dlg->adaptiveMinimumSpinner);
			ReleaseISpinner(dlg->clampDirectSpinner);
			ReleaseISpinner(dlg->clampIndirectSpinner);
			ReleaseISpinner(dlg->randSeedSpinner);
			ReleaseISpinner(dlg->volMaxStepsSpinner);
			ReleaseISpinner(dlg->volStepRateSpinner);
			ReleaseICustEdit(dlg->samplesEdit);
			ReleaseICustEdit(dlg->adaptiveThresholdEdit);
			ReleaseICustEdit(dlg->adaptiveMinimumEdit);
			ReleaseICustEdit(dlg->clampDirectEdit);
			ReleaseICustEdit(dlg->clampIndirectEdit);
			ReleaseICustEdit(dlg->randSeedEdit);
			ReleaseICustEdit(dlg->volMaxStepsEdit);
			ReleaseICustEdit(dlg->volStepRateEdit);
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


static INT_PTR CALLBACK TranslationConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
	case WM_INITDIALOG:
		dlg = (CyclesRendParamDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		if (dlg) {
			dlg->InitTranslationConfig(hWnd);
		}
		break;

	case WM_DESTROY:
		ReleaseISpinner(dlg->bgIntensityMulSpinner);
		ReleaseISpinner(dlg->misMapSizeSpinner);
		ReleaseISpinner(dlg->pointLightSizeSpinner);
		ReleaseISpinner(dlg->texmapBakeWidthSpinner);
		ReleaseISpinner(dlg->texmapBakeHeightSpinner);
		ReleaseISpinner(dlg->deformBlurSamplesSpinner);
		ReleaseICustEdit(dlg->bgIntensityMulEdit);
		ReleaseICustEdit(dlg->misMapSizeEdit);
		ReleaseICustEdit(dlg->pointLightSizeEdit);
		ReleaseICustEdit(dlg->texmapBakeWidthEdit);
		ReleaseICustEdit(dlg->texmapBakeHeightEdit);
		ReleaseICustEdit(dlg->deformBlurSamplesEdit);
		break;

	case WM_COMMAND:
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK LightPathConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (CyclesRendParamDlg*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			if (dlg) {
				dlg->InitLightPathConfig(hWnd);
			}
			break;

		case WM_DESTROY:
			ReleaseISpinner(dlg->lpMaxBounceSpinner);
			ReleaseISpinner(dlg->lpMinBounceSpinner);
			ReleaseISpinner(dlg->lpDiffuseBounceSpinner);
			ReleaseISpinner(dlg->lpGlossyBounceSpinner);
			ReleaseISpinner(dlg->lpTransmissionBounceSpinner);
			ReleaseISpinner(dlg->lpTransparentBounceSpinner);
			ReleaseISpinner(dlg->lpVolumeBounceSpinner);
			ReleaseICustEdit(dlg->lpMaxBounceEdit);
			ReleaseICustEdit(dlg->lpMinBounceEdit);
			ReleaseICustEdit(dlg->lpDiffuseBounceEdit);
			ReleaseICustEdit(dlg->lpGlossyBounceEdit);
			ReleaseICustEdit(dlg->lpTransmissionBounceEdit);
			ReleaseICustEdit(dlg->lpTransparentBounceEdit);
			ReleaseICustEdit(dlg->lpVolumeBounceEdit);
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK FilmConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (CyclesRendParamDlg*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			if (dlg) {
				dlg->InitFilmConfig(hWnd);
			}
			break;

		case WM_DESTROY:
			ReleaseISpinner(dlg->exposureSpinner);
			ReleaseISpinner(dlg->filterSizeSpinner);
			ReleaseICustEdit(dlg->exposureEdit);
			ReleaseICustEdit(dlg->filterSizeEdit);
			break;

		case WM_COMMAND:
			break;

		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
			break;

		default:
			return FALSE;
	}  
	return TRUE;
}

static INT_PTR CALLBACK PerformanceConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
	case WM_INITDIALOG:
		dlg = (CyclesRendParamDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		if (dlg) {
			dlg->InitPerformanceConfig(hWnd);
		}
		break;

	case WM_DESTROY:
		ReleaseISpinner(dlg->cpuThreadsSpinner);
		ReleaseICustEdit(dlg->cpuThreadsEdit);

		ReleaseISpinner(dlg->tileWidthSpinner);
		ReleaseISpinner(dlg->tileHeightSpinner);
		ReleaseICustEdit(dlg->tileWidthEdit);
		ReleaseICustEdit(dlg->tileHeightEdit);
		break;

	case WM_COMMAND:
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK StereoConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
	case WM_INITDIALOG:
		dlg = (CyclesRendParamDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		if (dlg) {
			dlg->InitStereoConfig(hWnd);
		}
		break;

	case WM_DESTROY:
		ReleaseISpinner(dlg->interocularDistSpinner);
		ReleaseISpinner(dlg->convergenceDistSpinner);
		ReleaseICustEdit(dlg->interocularDistEdit);
		ReleaseICustEdit(dlg->convergenceDistEdit);
		break;

	case WM_COMMAND:
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK PassesConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
	case WM_INITDIALOG:
		dlg = (CyclesRendParamDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		if (dlg) {
			dlg->InitPassesConfig(hWnd);
		}
		break;

	case WM_DESTROY:
		ReleaseISpinner(dlg->passesMistNearSpinner);
		ReleaseISpinner(dlg->passesMistDepthSpinner);
		ReleaseISpinner(dlg->passesMistExponentSpinner);
		ReleaseICustEdit(dlg->passesMistNearEdit);
		ReleaseICustEdit(dlg->passesMistDepthEdit);
		ReleaseICustEdit(dlg->passesMistExponentEdit);
		break;

	case WM_COMMAND:
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK AboutConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
	case WM_INITDIALOG:
		dlg = (CyclesRendParamDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON_WEBSITE) {
			ShellExecute(hWnd, L"open", L"https://cyclesformax.net/", NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		if (LOWORD(wParam) == IDC_BUTTON_MANUAL) {
			ShellExecute(hWnd, L"open", L"https://cyclesformax.net/manual/", NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		if (LOWORD(wParam) == IDC_BUTTON_GITHUB) {
			ShellExecute(hWnd, L"open", L"https://github.com/jlwitthuhn/cyclesformax-opensource", NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK DebugConfigRollupProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesRendParamDlg* dlg = DLGetWindowLongPtr<CyclesRendParamDlg*>(hWnd);
	switch (msg) {
	case WM_INITDIALOG:
		dlg = (CyclesRendParamDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		if (dlg) {
			dlg->InitDebugConfig(hWnd);
		}
		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		break;

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		dlg->ir->RollupMouseMessage(hWnd, msg, wParam, lParam);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

CyclesRendParamDlg::CyclesRendParamDlg(CyclesRenderParams* const cycles_rend_params, IRendParams* const i, const BOOL prog) :
	logger(global_log_manager.new_logger(L"RendParamDlg", true))
{
	*logger << LogCtl::SEPARATOR;

	rend_params = cycles_rend_params;
	ir = i;

	// Create the rollup panel
	if (prog) {
		progressPanel = ir->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(IDD_RENDER_PROG),
			ProgressRollupProc,
			_T("Important Information for Cool People"),
			(LPARAM)this);
	} else {
		// Main tab
		uncategorizedConfigPanel = ir->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG),
			UncategorizedConfigRollupProc,
			_T("Misc"),
			(LPARAM)this);
		samplingConfigPanel = ir->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_SAMPLING),
			SamplingConfigRollupProc,
			_T("Sampling"),
			(LPARAM)this);
		filmConfigPanel = ir->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_FILM),
			FilmConfigRollupProc,
			_T("Film"),
			(LPARAM)this);
		translationConfigPanel = ir->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_TRANSLATION),
			TranslationConfigRollupProc,
			_T("Translation"),
			(LPARAM)this);

		// Advanced tab
		performanceConfigPanel = ir->AddTabRollupPage(
			CYCLES_TAB_ADVANCED_CLASS,
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_PERF),
			PerformanceConfigRollupProc,
			_T("Performance"),
			(LPARAM)this);
		lightPathConfigPanel = ir->AddTabRollupPage(
			CYCLES_TAB_ADVANCED_CLASS,
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_LIGHT_PATH),
			LightPathConfigRollupProc,
			_T("Light Path"),
			(LPARAM)this);
		renderPassesConfigPanel = ir->AddTabRollupPage(
			CYCLES_TAB_ADVANCED_CLASS,
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_PASSES),
			PassesConfigRollupProc,
			_T("Render Passes"),
			(LPARAM)this);
		stereoConfigPanel = ir->AddTabRollupPage(
			CYCLES_TAB_ADVANCED_CLASS,
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_STEREO),
			StereoConfigRollupProc,
			_T("Stereoscopy"),
			(LPARAM)this);

		// About tab
		aboutConfigPanel = ir->AddTabRollupPage(
			CYCLES_TAB_ABOUT_CLASS,
			hInstance,
			MAKEINTRESOURCE(IDD_RENDER_CONFIG_VERSION),
			AboutConfigRollupProc,
			_T("About"),
			(LPARAM)this);

		if (enable_plugin_debug) {
			*logger << "Adding debug tab" << LogCtl::WRITE_LINE;
			// Debug tab
			debugConfigPanel = ir->AddTabRollupPage(
				CYCLES_TAB_DEBUG_CLASS,
				hInstance,
				MAKEINTRESOURCE(IDD_RENDER_CONFIG_DEBUG),
				DebugConfigRollupProc,
				_T("Debug"),
				(LPARAM)this);
		}
		else {
			*logger << "Skipping debug tab" << LogCtl::WRITE_LINE;
		}
	}
}

CyclesRendParamDlg::~CyclesRendParamDlg()
{
	if (progressPanel != nullptr) {
		ir->DeleteRollupPage(progressPanel);
	}

	if (uncategorizedConfigPanel != nullptr) {
		ir->DeleteRollupPage(uncategorizedConfigPanel);
	}
	if (samplingConfigPanel != nullptr) {
		ir->DeleteRollupPage(samplingConfigPanel);
	}
	if (translationConfigPanel != nullptr) {
		ir->DeleteRollupPage(translationConfigPanel);
	}
	if (filmConfigPanel != nullptr) {
		ir->DeleteRollupPage(filmConfigPanel);
	}

	if (performanceConfigPanel != nullptr) {
		ir->DeleteTabRollupPage(CYCLES_TAB_ADVANCED_CLASS, performanceConfigPanel);
	}
	if (lightPathConfigPanel != nullptr) {
		ir->DeleteTabRollupPage(CYCLES_TAB_ADVANCED_CLASS, lightPathConfigPanel);
	}
	if (renderPassesConfigPanel != nullptr) {
		ir->DeleteTabRollupPage(CYCLES_TAB_ADVANCED_CLASS, renderPassesConfigPanel);
	}
	if (stereoConfigPanel != nullptr) {
		ir->DeleteTabRollupPage(CYCLES_TAB_ADVANCED_CLASS, stereoConfigPanel);
	}

	if (aboutConfigPanel != nullptr) {
		ir->DeleteTabRollupPage(CYCLES_TAB_ABOUT_CLASS, aboutConfigPanel);
	}

	if (debugConfigPanel != nullptr) {
		ir->DeleteTabRollupPage(CYCLES_TAB_DEBUG_CLASS, debugConfigPanel);
	}
}

void CyclesRendParamDlg::InitProgressRollup(const HWND /*hWnd*/)
{
	// Do nothing
}

void CyclesRendParamDlg::InitUncategorizedConfig(const HWND hWnd)
{
	cudaGpuEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_CUDA_GPU));
	cudaGpuEdit->SetText(rend_params->cuda_device.c_str());
	cudaGpuEdit->SetTooltip(true, TOOLTIP_CUDA_DEVICE);

	optixGpuEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_OPTIX_GPU));
	optixGpuEdit->SetText(rend_params->optix_device.c_str());
	optixGpuEdit->SetTooltip(true, TOOLTIP_OPTIX_DEVICE);

	mtlPreviewSamplesEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_MAT_PREVIEW_SAMPLES));
	mtlPreviewSamplesEdit->SetTooltip(true, TOOLTIP_MTL_PREVIEW_SAMPLES);
	mtlPreviewSamplesSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_MAT_PREVIEW_SAMPLES));
	mtlPreviewSamplesSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_MAT_PREVIEW_SAMPLES), EDITTYPE_INT);
	mtlPreviewSamplesSpinner->SetLimits(1, 500, TRUE);
	mtlPreviewSamplesSpinner->SetValue(rend_params->mtl_preview_samples, FALSE);
	mtlPreviewSamplesSpinner->SetTooltip(true, TOOLTIP_MTL_PREVIEW_SAMPLES);

	if (rend_params->cuda_with_cpu) {
		CheckDlgButton(hWnd, IDC_BOOL_CUDA_CPU, BST_CHECKED);
	}

	if (rend_params->render_device == RenderDevice::CUDA) {
		CheckRadioButton(hWnd, IDC_RADIO_DEV_CPU, IDC_RADIO_DEV_OPTIX, IDC_RADIO_DEV_CUDA);
	}
	else if (rend_params->render_device == RenderDevice::OPTIX) {
		CheckRadioButton(hWnd, IDC_RADIO_DEV_CPU, IDC_RADIO_DEV_OPTIX, IDC_RADIO_DEV_OPTIX);
	}
	else {
		CheckRadioButton(hWnd, IDC_RADIO_DEV_CPU, IDC_RADIO_DEV_OPTIX, IDC_RADIO_DEV_CPU);
	}
}

void CyclesRendParamDlg::InitSamplingConfig(const HWND hWnd)
{
	samplesEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_SAMPLES));
	samplesEdit->SetTooltip(true, TOOLTIP_SAMPLING_SAMPLES);
	samplesSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_SAMPLES));
	samplesSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_SAMPLES), EDITTYPE_INT);
	samplesSpinner->SetLimits(1, 50000);
	samplesSpinner->SetValue(rend_params->samples, FALSE);
	samplesSpinner->SetTooltip(true, TOOLTIP_SAMPLING_SAMPLES);

	if (rend_params->use_progressive_refine) {
		CheckDlgButton(hWnd, IDC_BOOL_PROGRESSIVE_REFINE, BST_CHECKED);
	}

	if (rend_params->use_adaptive_sampling) {
		CheckDlgButton(hWnd, IDC_BOOL_ADAPTIVE_SAMPLING, BST_CHECKED);
	}

	adaptiveThresholdEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_ADAPTIVE_THRESHOLD));
	adaptiveThresholdEdit->SetTooltip(true, TOOLTIP_SAMPLING_ADAPTIVE_THRESHOLD);
	adaptiveThresholdSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_ADAPTIVE_THRESHOLD));
	adaptiveThresholdSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_ADAPTIVE_THRESHOLD), EDITTYPE_FLOAT);
	adaptiveThresholdSpinner->SetLimits(0.0f, 1.0f);
	adaptiveThresholdSpinner->SetScale(0.01f);
	adaptiveThresholdSpinner->SetValue(rend_params->adaptive_threshold, FALSE);
	adaptiveThresholdSpinner->SetTooltip(true, TOOLTIP_SAMPLING_ADAPTIVE_THRESHOLD);

	adaptiveMinimumEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_ADAPTIVE_MIN));
	adaptiveMinimumEdit->SetTooltip(true, TOOLTIP_SAMPLING_ADAPTIVE_MINIMUM);
	adaptiveMinimumSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_ADAPTIVE_MIN));
	adaptiveMinimumSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_ADAPTIVE_MIN), EDITTYPE_INT);
	adaptiveMinimumSpinner->SetLimits(0, 10000);
	adaptiveMinimumSpinner->SetValue(rend_params->adaptive_min_samples, FALSE);
	adaptiveMinimumSpinner->SetTooltip(true, TOOLTIP_SAMPLING_ADAPTIVE_MINIMUM);

	clampDirectEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_CLAMP_DIRECT));
	clampDirectEdit->SetTooltip(true, TOOLTIP_SAMPLING_CLAMP_DIRECT);
	clampDirectSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_CLAMP_DIRECT));
	clampDirectSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_CLAMP_DIRECT), EDITTYPE_FLOAT);
	clampDirectSpinner->SetLimits(0.0f, 10e9f);
	clampDirectSpinner->SetValue(rend_params->clamp_direct, FALSE);
	clampDirectSpinner->SetTooltip(true, TOOLTIP_SAMPLING_CLAMP_DIRECT);

	if (rend_params->use_clamp_direct) {
		CheckDlgButton(hWnd, IDC_BOOL_CLAMP_DIRECT, BST_CHECKED);
	}

	clampIndirectEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_CLAMP_INDIRECT));
	clampIndirectEdit->SetTooltip(true, TOOLTIP_SAMPLING_CLAMP_INDIRECT);
	clampIndirectSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_CLAMP_INDIRECT));
	clampIndirectSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_CLAMP_INDIRECT), EDITTYPE_FLOAT);
	clampIndirectSpinner->SetLimits(0.0f, 10e9f);
	clampIndirectSpinner->SetValue(rend_params->clamp_indirect, FALSE);
	clampIndirectSpinner->SetTooltip(true, TOOLTIP_SAMPLING_CLAMP_INDIRECT);

	if (rend_params->use_clamp_indirect) {
		CheckDlgButton(hWnd, IDC_BOOL_CLAMP_INDIRECT, BST_CHECKED);
	}

	randSeedEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_RAND_SEED));
	randSeedEdit->SetTooltip(true, TOOLTIP_SAMPLING_RAND_SEED);
	randSeedSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_RAND_SEED));
	randSeedSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_RAND_SEED), EDITTYPE_INT);
	randSeedSpinner->SetLimits(INT_MIN, INT_MAX);
	randSeedSpinner->SetValue(rend_params->rand_seed, FALSE);
	randSeedSpinner->SetTooltip(true, TOOLTIP_SAMPLING_RAND_SEED);

	if (rend_params->animate_rand_seed) {
		CheckDlgButton(hWnd, IDC_BOOL_ANIMATE_RAND_SEED, BST_CHECKED);
	}

	volStepRateEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_VOL_STEP_RATE));
	volStepRateEdit->SetTooltip(true, TOOLTIP_SAMPLING_VOL_STEP_RATE);
	volStepRateSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_VOL_STEP_RATE));
	volStepRateSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_VOL_STEP_RATE), EDITTYPE_FLOAT);
	volStepRateSpinner->SetLimits(0.001f, 10e6f);
	volStepRateSpinner->SetValue(rend_params->vol_step_rate, FALSE);
	volStepRateSpinner->SetTooltip(true, TOOLTIP_SAMPLING_VOL_STEP_RATE);

	volMaxStepsEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_VOL_MAX_STEPS));
	volMaxStepsEdit->SetTooltip(true, TOOLTIP_SAMPLING_VOL_MAX_STEPS);
	volMaxStepsSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_VOL_MAX_STEPS));
	volMaxStepsSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_VOL_MAX_STEPS), EDITTYPE_INT);
	volMaxStepsSpinner->SetLimits(1, 10000);
	volMaxStepsSpinner->SetValue(rend_params->vol_max_steps, FALSE);
	volMaxStepsSpinner->SetTooltip(true, TOOLTIP_SAMPLING_VOL_MAX_STEPS);
}

void CyclesRendParamDlg::InitTranslationConfig(const HWND hWnd)
{
	bgIntensityMulEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_INTENSITY_MUL));
	bgIntensityMulEdit->SetTooltip(true, TOOLTIP_TRANS_BG_INTENSITY_MUL);
	bgIntensityMulSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_INTENSITY_MUL));
	bgIntensityMulSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_INTENSITY_MUL), EDITTYPE_FLOAT);
	bgIntensityMulSpinner->SetLimits(0.0f, 100.0f);
	bgIntensityMulSpinner->SetValue(rend_params->bg_intensity, FALSE);
	bgIntensityMulSpinner->SetTooltip(true, TOOLTIP_TRANS_BG_INTENSITY_MUL);

	misMapSizeEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_MIS_MAP_SIZE));
	misMapSizeEdit->SetTooltip(true, TOOLTIP_TRANS_MIS_MAP_SIZE);
	misMapSizeSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_MIS_MAP_SIZE));
	misMapSizeSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_MIS_MAP_SIZE), EDITTYPE_INT);
	misMapSizeSpinner->SetLimits(1, 16384);
	misMapSizeSpinner->SetValue(rend_params->mis_map_size, FALSE);
	misMapSizeSpinner->SetTooltip(true, TOOLTIP_TRANS_MIS_MAP_SIZE);

	pointLightSizeEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_POINT_LIGHT_SIZE));
	pointLightSizeEdit->SetTooltip(true, TOOLTIP_TRANS_POINT_LIGHT_SIZE);
	pointLightSizeSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_POINT_LIGHT_SIZE));
	pointLightSizeSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_POINT_LIGHT_SIZE), EDITTYPE_FLOAT);
	pointLightSizeSpinner->SetLimits(0.01f, 5000.0f);
	pointLightSizeSpinner->SetValue(rend_params->point_light_size, FALSE);
	pointLightSizeSpinner->SetTooltip(true, TOOLTIP_TRANS_POINT_LIGHT_SIZE);

	texmapBakeWidthEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_BITMAP_WIDTH));
	texmapBakeWidthEdit->SetTooltip(true, TOOLTIP_TRANS_BITMAP_WIDTH);
	texmapBakeWidthSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_BITMAP_WIDTH));
	texmapBakeWidthSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_BITMAP_WIDTH), EDITTYPE_INT);
	texmapBakeWidthSpinner->SetLimits(1, 16384);
	texmapBakeWidthSpinner->SetValue(rend_params->texmap_bake_width, FALSE);
	texmapBakeWidthSpinner->SetTooltip(true, TOOLTIP_TRANS_BITMAP_WIDTH);

	texmapBakeHeightEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_BITMAP_HEIGHT));
	texmapBakeHeightEdit->SetTooltip(true, TOOLTIP_TRANS_BITMAP_HEIGHT);
	texmapBakeHeightSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_BITMAP_HEIGHT));
	texmapBakeHeightSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_BITMAP_HEIGHT), EDITTYPE_INT);
	texmapBakeHeightSpinner->SetLimits(1, 16384);
	texmapBakeHeightSpinner->SetValue(rend_params->texmap_bake_height, FALSE);
	texmapBakeHeightSpinner->SetTooltip(true, TOOLTIP_TRANS_BITMAP_HEIGHT);

	deformBlurSamplesEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_DEFORM_BLUR_SAMPLES));
	deformBlurSamplesEdit->SetTooltip(true, TOOLTIP_TRANS_DEFORM_BLUR_SAMPLES);
	deformBlurSamplesSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_DEFORM_BLUR_SAMPLES));
	deformBlurSamplesSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_DEFORM_BLUR_SAMPLES), EDITTYPE_INT);
	deformBlurSamplesSpinner->SetLimits(1, 32);
	deformBlurSamplesSpinner->SetValue(rend_params->deform_blur_samples, FALSE);
	deformBlurSamplesSpinner->SetTooltip(true, TOOLTIP_TRANS_DEFORM_BLUR_SAMPLES);
}

void CyclesRendParamDlg::InitLightPathConfig(const HWND hWnd)
{
	lpMaxBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_MAX_BOUNCE));
	lpMaxBounceEdit->SetTooltip(true, TOOLTIP_LP_MAX_BOUNCE);
	lpMaxBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_MAX_BOUNCE));
	lpMaxBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_MAX_BOUNCE), EDITTYPE_INT);
	lpMaxBounceSpinner->SetLimits(0, 128);
	lpMaxBounceSpinner->SetValue(rend_params->lp_max_bounce, FALSE);
	lpMaxBounceSpinner->SetTooltip(true, TOOLTIP_LP_MAX_BOUNCE);

	lpMinBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_MIN_BOUNCE));
	lpMinBounceEdit->SetTooltip(true, TOOLTIP_LP_MIN_BOUNCE);
	lpMinBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_MIN_BOUNCE));
	lpMinBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_MIN_BOUNCE), EDITTYPE_INT);
	lpMinBounceSpinner->SetLimits(0, 128);
	lpMinBounceSpinner->SetValue(rend_params->lp_min_bounce, FALSE);
	lpMinBounceSpinner->SetTooltip(true, TOOLTIP_LP_MIN_BOUNCE);

	lpDiffuseBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_DIFFUSE_BOUNCE));
	lpDiffuseBounceEdit->SetTooltip(true, TOOLTIP_LP_DIFFUSE_BOUNCE);
	lpDiffuseBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_DIFFUSE_BOUNCE));
	lpDiffuseBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_DIFFUSE_BOUNCE), EDITTYPE_INT);
	lpDiffuseBounceSpinner->SetLimits(0, 128);
	lpDiffuseBounceSpinner->SetValue(rend_params->lp_diffuse_bounce, FALSE);
	lpDiffuseBounceSpinner->SetTooltip(true, TOOLTIP_LP_DIFFUSE_BOUNCE);

	lpGlossyBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_GLOSSY_BOUNCE));
	lpGlossyBounceEdit->SetTooltip(true, TOOLTIP_LP_GLOSSY_BOUNCE);
	lpGlossyBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_GLOSSY_BOUNCE));
	lpGlossyBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_GLOSSY_BOUNCE), EDITTYPE_INT);
	lpGlossyBounceSpinner->SetLimits(0, 128);
	lpGlossyBounceSpinner->SetValue(rend_params->lp_glossy_bounce, FALSE);
	lpGlossyBounceSpinner->SetTooltip(true, TOOLTIP_LP_GLOSSY_BOUNCE);

	lpTransmissionBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_TRANS_BOUNCE));
	lpTransmissionBounceEdit->SetTooltip(true, TOOLTIP_LP_TRANSMISSION_BOUNCE);
	lpTransmissionBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_TRANS_BOUNCE));
	lpTransmissionBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_TRANS_BOUNCE), EDITTYPE_INT);
	lpTransmissionBounceSpinner->SetLimits(0, 128);
	lpTransmissionBounceSpinner->SetValue(rend_params->lp_transmission_bounce, FALSE);
	lpTransmissionBounceSpinner->SetTooltip(true, TOOLTIP_LP_TRANSMISSION_BOUNCE);

	lpTransparentBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_TRANSP_BOUNCE));
	lpTransparentBounceEdit->SetTooltip(true, TOOLTIP_LP_TRANSPARENT_BOUNCE);
	lpTransparentBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_TRANSP_BOUNCE));
	lpTransparentBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_TRANSP_BOUNCE), EDITTYPE_INT);
	lpTransparentBounceSpinner->SetLimits(0, 128);
	lpTransparentBounceSpinner->SetValue(rend_params->lp_transparent_bounce, FALSE);
	lpTransparentBounceSpinner->SetTooltip(true, TOOLTIP_LP_TRANSPARENT_BOUNCE);

	lpVolumeBounceEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_LP_VOL_BOUNCE));
	lpVolumeBounceEdit->SetTooltip(true, TOOLTIP_LP_VOLUME_BOUNCE);
	lpVolumeBounceSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_LP_VOL_BOUNCE));
	lpVolumeBounceSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_LP_VOL_BOUNCE), EDITTYPE_INT);
	lpVolumeBounceSpinner->SetLimits(0, 128);
	lpVolumeBounceSpinner->SetValue(rend_params->lp_volume_bounce, FALSE);
	lpVolumeBounceSpinner->SetTooltip(true, TOOLTIP_LP_VOLUME_BOUNCE);
}

void CyclesRendParamDlg::InitFilmConfig(const HWND hWnd)
{
	if (rend_params->use_transparent_sky) {
		CheckDlgButton(hWnd, IDC_BOOL_TRANSPARENT_SKY, BST_CHECKED);
	}

	exposureEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_EXPOSURE));
	exposureEdit->SetTooltip(true, TOOLTIP_FILM_EXPOSURE);
	exposureSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_EXPOSURE));
	exposureSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_EXPOSURE), EDITTYPE_FLOAT);
	exposureSpinner->SetLimits(0.0f, 1000.0f);
	exposureSpinner->SetScale(0.1f);
	exposureSpinner->SetValue(rend_params->exposure_multiplier, FALSE);
	exposureSpinner->SetTooltip(true, TOOLTIP_FILM_EXPOSURE);

	if (rend_params->filter_type == FilmFilterType::BLACKMAN_HARRIS) {
		CheckRadioButton(hWnd, IDC_RADIO_FILTER_BOX, IDC_RADIO_FILTER_BLACKMAN_HARRIS, IDC_RADIO_FILTER_BLACKMAN_HARRIS);
	}
	else if (rend_params->filter_type == FilmFilterType::GAUSSIAN) {
		CheckRadioButton(hWnd, IDC_RADIO_FILTER_BOX, IDC_RADIO_FILTER_BLACKMAN_HARRIS, IDC_RADIO_FILTER_GAUSSIAN);
	}
	else {
		CheckRadioButton(hWnd, IDC_RADIO_FILTER_BOX, IDC_RADIO_FILTER_BLACKMAN_HARRIS, IDC_RADIO_FILTER_BOX);
	}

	filterSizeEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_FILTER_SIZE));
	filterSizeEdit->SetTooltip(true, TOOLTIP_FILTER_SIZE);
	filterSizeSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_FILTER_SIZE));
	filterSizeSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_FILTER_SIZE), EDITTYPE_FLOAT);
	filterSizeSpinner->SetLimits(0.0f, 100.0f);
	filterSizeSpinner->SetScale(0.1f);
	filterSizeSpinner->SetValue(rend_params->filter_size, FALSE);
	filterSizeSpinner->SetTooltip(true, TOOLTIP_FILTER_SIZE);
}

void CyclesRendParamDlg::InitPerformanceConfig(const HWND hWnd)
{
	cpuThreadsEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_CPU_THREADS));
	cpuThreadsEdit->SetTooltip(true, TOOLTIP_PERF_CPU_THREADS);
	cpuThreadsSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_CPU_THREADS));
	cpuThreadsSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_CPU_THREADS), EDITTYPE_INT);
	cpuThreadsSpinner->SetLimits(0, 128, TRUE);
	cpuThreadsSpinner->SetValue(rend_params->cpu_threads, FALSE);
	cpuThreadsSpinner->SetTooltip(true, TOOLTIP_PERF_CPU_THREADS);

	tileWidthEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_TILE_WIDTH));
	tileWidthEdit->SetTooltip(true, TOOLTIP_PERF_TILE_WIDTH);
	tileWidthSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_TILE_WIDTH));
	tileWidthSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_TILE_WIDTH), EDITTYPE_INT);
	tileWidthSpinner->SetLimits(16, 1024, TRUE);
	tileWidthSpinner->SetValue(rend_params->tile_width, FALSE);
	tileWidthSpinner->SetTooltip(true, TOOLTIP_PERF_TILE_WIDTH);

	tileHeightEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_TILE_HEIGHT));
	tileHeightEdit->SetTooltip(true, TOOLTIP_PERF_TILE_HEIGHT);
	tileHeightSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_TILE_HEIGHT));
	tileHeightSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_TILE_HEIGHT), EDITTYPE_INT);
	tileHeightSpinner->SetLimits(16, 1024, TRUE);
	tileHeightSpinner->SetValue(rend_params->tile_height, FALSE);
	tileHeightSpinner->SetTooltip(true, TOOLTIP_PERF_TILE_HEIGHT);
}

void CyclesRendParamDlg::InitStereoConfig(const HWND hWnd)
{
	if (rend_params->stereo_type == StereoscopyType::NONE) {
		CheckRadioButton(hWnd, IDC_RADIO_STEREO_MODE_NONE, IDC_RADIO_STEREO_MODE_TB, IDC_RADIO_STEREO_MODE_NONE);
	}
	else if (rend_params->stereo_type == StereoscopyType::LEFT_EYE) {
		CheckRadioButton(hWnd, IDC_RADIO_STEREO_MODE_NONE, IDC_RADIO_STEREO_MODE_TB, IDC_RADIO_STEREO_MODE_LEFT);
	}
	else if (rend_params->stereo_type == StereoscopyType::RIGHT_EYE) {
		CheckRadioButton(hWnd, IDC_RADIO_STEREO_MODE_NONE, IDC_RADIO_STEREO_MODE_TB, IDC_RADIO_STEREO_MODE_RIGHT);
	}
	else if (rend_params->stereo_type == StereoscopyType::ANAGLYPH) {
		CheckRadioButton(hWnd, IDC_RADIO_STEREO_MODE_NONE, IDC_RADIO_STEREO_MODE_TB, IDC_RADIO_STEREO_MODE_ANAGLYPH);
	}
	else if (rend_params->stereo_type == StereoscopyType::SPLIT_LEFT_RIGHT) {
		CheckRadioButton(hWnd, IDC_RADIO_STEREO_MODE_NONE, IDC_RADIO_STEREO_MODE_TB, IDC_RADIO_STEREO_MODE_LR);
	}
	else if (rend_params->stereo_type == StereoscopyType::SPLIT_TOP_BOTTOM) {
		CheckRadioButton(hWnd, IDC_RADIO_STEREO_MODE_NONE, IDC_RADIO_STEREO_MODE_TB, IDC_RADIO_STEREO_MODE_TB);
	}

	interocularDistEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_INTEROCULAR_DISTANCE));
	interocularDistEdit->SetTooltip(true, TOOLTIP_STEREO_INTEROCULAR_DISTANCE);
	interocularDistSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_INTEROCULAR_DISTANCE));
	interocularDistSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_INTEROCULAR_DISTANCE), EDITTYPE_FLOAT);
	interocularDistSpinner->SetLimits(0.001f, 1e6f, TRUE);
	interocularDistSpinner->SetValue(rend_params->interocular_distance, FALSE);
	interocularDistSpinner->SetTooltip(true, TOOLTIP_STEREO_INTEROCULAR_DISTANCE);

	convergenceDistEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_CONVERGENCE_DIST));
	convergenceDistEdit->SetTooltip(true, TOOLTIP_STEREO_CONVERGENCE_DISTANCE);
	convergenceDistSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_CONVERGENCE_DIST));
	convergenceDistSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_CONVERGENCE_DIST), EDITTYPE_FLOAT);
	convergenceDistSpinner->SetLimits(0.001f, 1e6f, TRUE);
	convergenceDistSpinner->SetValue(rend_params->convergence_distance, FALSE);
	convergenceDistSpinner->SetTooltip(true, TOOLTIP_STEREO_CONVERGENCE_DISTANCE);

	if (rend_params->stereo_swap_eyes) {
		CheckDlgButton(hWnd, IDC_BOOL_STEREO_SWAP_EYES, BST_CHECKED);
	}

	if (rend_params->anaglyph_type == AnaglyphType::RED_CYAN) {
		CheckRadioButton(hWnd, IDC_RADIO_ANAGLYPH_RC, IDC_RADIO_ANAGLYPH_YB, IDC_RADIO_ANAGLYPH_RC);
	}
	else if (rend_params->anaglyph_type == AnaglyphType::GREEN_MAGENTA) {
		CheckRadioButton(hWnd, IDC_RADIO_ANAGLYPH_RC, IDC_RADIO_ANAGLYPH_YB, IDC_RADIO_ANAGLYPH_GM);
	}
	else if (rend_params->anaglyph_type == AnaglyphType::YELLOW_BLUE) {
		CheckRadioButton(hWnd, IDC_RADIO_ANAGLYPH_RC, IDC_RADIO_ANAGLYPH_YB, IDC_RADIO_ANAGLYPH_YB);
	}
}

void CyclesRendParamDlg::InitPassesConfig(const HWND hWnd)
{
	passesMistNearEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_MIST_NEAR));
	passesMistNearEdit->SetTooltip(true, TOOLTIP_PASSES_MIST_NEAR);
	passesMistNearSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_MIST_NEAR));
	passesMistNearSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_MIST_NEAR), EDITTYPE_FLOAT);
	passesMistNearSpinner->SetLimits(0.001f, 1e6f, TRUE);
	passesMistNearSpinner->SetValue(rend_params->mist_near, FALSE);
	passesMistNearSpinner->SetTooltip(true, TOOLTIP_PASSES_MIST_NEAR);

	passesMistDepthEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_MIST_DEPTH));
	passesMistDepthEdit->SetTooltip(true, TOOLTIP_PASSES_MIST_DEPTH);
	passesMistDepthSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_MIST_DEPTH));
	passesMistDepthSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_MIST_DEPTH), EDITTYPE_FLOAT);
	passesMistDepthSpinner->SetLimits(0.001f, 1e6f, TRUE);
	passesMistDepthSpinner->SetValue(rend_params->mist_depth, FALSE);
	passesMistDepthSpinner->SetTooltip(true, TOOLTIP_PASSES_MIST_DEPTH);

	passesMistExponentEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_MIST_EXP));
	passesMistExponentEdit->SetTooltip(true, TOOLTIP_PASSES_MIST_EXPONENT);
	passesMistExponentSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_MIST_EXP));
	passesMistExponentSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDIT_MIST_EXP), EDITTYPE_FLOAT);
	passesMistExponentSpinner->SetLimits(0.01f, 10.0f, TRUE);
	passesMistExponentSpinner->SetValue(rend_params->mist_exponent, FALSE);
	passesMistExponentSpinner->SetTooltip(true, TOOLTIP_PASSES_MIST_EXPONENT);
}

void CyclesRendParamDlg::InitDebugConfig(const HWND hWnd)
{
	if (rend_params->debug_multi_cuda) {
		CheckDlgButton(hWnd, IDC_BOOL_DEBUG_MULTI_CUDA, BST_CHECKED);
	}
}

void CyclesRendParamDlg::AcceptParams()
{
	rend_params->mtl_preview_samples = mtlPreviewSamplesSpinner->GetIVal();

	if (IsDlgButtonChecked(uncategorizedConfigPanel, IDC_RADIO_DEV_CUDA)) {
		rend_params->render_device = RenderDevice::CUDA;
	}
	else if (IsDlgButtonChecked(uncategorizedConfigPanel, IDC_RADIO_DEV_OPTIX)) {
		rend_params->render_device = RenderDevice::OPTIX;
	}
	else {
		rend_params->render_device = RenderDevice::CPU;
	}

	if (IsDlgButtonChecked(uncategorizedConfigPanel, IDC_BOOL_CUDA_CPU)) {
		rend_params->cuda_with_cpu = true;
	}
	else {
		rend_params->cuda_with_cpu = false;
	}

	{
		MSTR device_str;
		cudaGpuEdit->GetText(device_str);
		rend_params->cuda_device = std::wstring(device_str.ToWStr());
	}

	{
		MSTR device_str;
		optixGpuEdit->GetText(device_str);
		rend_params->optix_device = std::wstring(device_str.ToWStr());
	}

	rend_params->samples = samplesSpinner->GetIVal();

	if (IsDlgButtonChecked(samplingConfigPanel, IDC_BOOL_PROGRESSIVE_REFINE)) {
		rend_params->use_progressive_refine = true;
	}
	else {
		rend_params->use_progressive_refine = false;
	}

	if (IsDlgButtonChecked(samplingConfigPanel, IDC_BOOL_ADAPTIVE_SAMPLING)) {
		rend_params->use_adaptive_sampling = true;
	}
	else {
		rend_params->use_adaptive_sampling = false;
	}

	rend_params->adaptive_threshold = adaptiveThresholdSpinner->GetFVal();
	rend_params->adaptive_min_samples = adaptiveMinimumSpinner->GetIVal();

	rend_params->clamp_direct = clampDirectSpinner->GetFVal();
	rend_params->clamp_indirect = clampIndirectSpinner->GetFVal();
	rend_params->rand_seed = randSeedSpinner->GetIVal();

	if (IsDlgButtonChecked(samplingConfigPanel, IDC_BOOL_CLAMP_DIRECT)) {
		rend_params->use_clamp_direct = true;
	}
	else {
		rend_params->use_clamp_direct = false;
	}

	if (IsDlgButtonChecked(samplingConfigPanel, IDC_BOOL_CLAMP_INDIRECT)) {
		rend_params->use_clamp_indirect = true;
	}
	else {
		rend_params->use_clamp_indirect = false;
	}

	if (IsDlgButtonChecked(samplingConfigPanel, IDC_BOOL_ANIMATE_RAND_SEED)) {
		rend_params->animate_rand_seed = true;
	}
	else {
		rend_params->animate_rand_seed = false;
	}

	rend_params->vol_step_rate = volStepRateSpinner->GetFVal();
	rend_params->vol_max_steps = volMaxStepsSpinner->GetIVal();

	rend_params->bg_intensity = bgIntensityMulSpinner->GetFVal();
	rend_params->mis_map_size = misMapSizeSpinner->GetIVal();
	rend_params->point_light_size = pointLightSizeSpinner->GetFVal();
	rend_params->texmap_bake_width = texmapBakeWidthSpinner->GetIVal();
	rend_params->texmap_bake_height = texmapBakeHeightSpinner->GetIVal();
	rend_params->deform_blur_samples = deformBlurSamplesSpinner->GetIVal();

	rend_params->lp_max_bounce = lpMaxBounceSpinner->GetIVal();
	rend_params->lp_min_bounce = lpMinBounceSpinner->GetIVal();
	rend_params->lp_diffuse_bounce = lpDiffuseBounceSpinner->GetIVal();
	rend_params->lp_glossy_bounce = lpGlossyBounceSpinner->GetIVal();
	rend_params->lp_transmission_bounce = lpTransmissionBounceSpinner->GetIVal();
	rend_params->lp_transparent_bounce = lpTransparentBounceSpinner->GetIVal();
	rend_params->lp_volume_bounce = lpVolumeBounceSpinner->GetIVal();

	if (IsDlgButtonChecked(filmConfigPanel, IDC_BOOL_TRANSPARENT_SKY)) {
		rend_params->use_transparent_sky = true;
	}
	else {
		rend_params->use_transparent_sky = false;
	}

	rend_params->exposure_multiplier = exposureSpinner->GetFVal();

	if (IsDlgButtonChecked(filmConfigPanel, IDC_RADIO_FILTER_BLACKMAN_HARRIS)) {
		rend_params->filter_type = FilmFilterType::BLACKMAN_HARRIS;
	}
	else if (IsDlgButtonChecked(filmConfigPanel, IDC_RADIO_FILTER_GAUSSIAN)) {
		rend_params->filter_type = FilmFilterType::GAUSSIAN;
	}
	else {
		rend_params->filter_type = FilmFilterType::BOX;
	}
	rend_params->filter_size = filterSizeSpinner->GetFVal();

	rend_params->cpu_threads = cpuThreadsSpinner->GetIVal();
	rend_params->tile_width = tileWidthSpinner->GetIVal();
	rend_params->tile_height = tileHeightSpinner->GetIVal();


	rend_params->mist_near = passesMistNearSpinner->GetFVal();
	rend_params->mist_depth = passesMistDepthSpinner->GetFVal();
	rend_params->mist_exponent = passesMistExponentSpinner->GetFVal();

	if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_STEREO_MODE_ANAGLYPH)) {
		rend_params->stereo_type = StereoscopyType::ANAGLYPH;
	}
	else if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_STEREO_MODE_LEFT)) {
		rend_params->stereo_type = StereoscopyType::LEFT_EYE;
	}
	else if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_STEREO_MODE_RIGHT)) {
		rend_params->stereo_type = StereoscopyType::RIGHT_EYE;
	}
	else if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_STEREO_MODE_LR)) {
		rend_params->stereo_type = StereoscopyType::SPLIT_LEFT_RIGHT;
	}
	else if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_STEREO_MODE_TB)) {
		rend_params->stereo_type = StereoscopyType::SPLIT_TOP_BOTTOM;
	}
	else {
		rend_params->stereo_type = StereoscopyType::NONE;
	}

	rend_params->interocular_distance = interocularDistSpinner->GetFVal();
	rend_params->convergence_distance = convergenceDistSpinner->GetFVal();

	if (IsDlgButtonChecked(stereoConfigPanel, IDC_BOOL_STEREO_SWAP_EYES)) {
		rend_params->stereo_swap_eyes = true;
	}
	else {
		rend_params->stereo_swap_eyes = false;
	}

	if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_ANAGLYPH_RC)) {
		rend_params->anaglyph_type = AnaglyphType::RED_CYAN;
	}
	else if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_ANAGLYPH_GM)) {
		rend_params->anaglyph_type = AnaglyphType::GREEN_MAGENTA;
	}
	else if (IsDlgButtonChecked(stereoConfigPanel, IDC_RADIO_ANAGLYPH_YB)) {
		rend_params->anaglyph_type = AnaglyphType::YELLOW_BLUE;
	}

	if (enable_plugin_debug) {
		if (IsDlgButtonChecked(debugConfigPanel, IDC_BOOL_DEBUG_MULTI_CUDA)) {
			rend_params->debug_multi_cuda = true;
		}
		else {
			rend_params->debug_multi_cuda = false;
		}
	}
}

void CyclesRendParamDlg::RejectParams()
{

}
