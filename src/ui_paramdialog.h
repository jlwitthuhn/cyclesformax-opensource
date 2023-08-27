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
 * @brief Defines the class CyclesRendParamDlg.
 */

#include <memory>

#include <Windows.h>

#include <Rendering/RendParamDlg.h>

#include "rend_logger.h"

class CyclesRenderParams;
class ICustEdit;
class IRendParams;
class ISpinnerControl;

/**
 * @brief Class responsible for presenting and managing the render setup dialog.
 */
class CyclesRendParamDlg : public RendParamDlg {
public:
	CyclesRenderParams* rend_params = nullptr;
	IRendParams* ir = nullptr;

	// Rollup panels
	HWND progressPanel = nullptr;
	HWND uncategorizedConfigPanel = nullptr;
	HWND samplingConfigPanel = nullptr;
	HWND translationConfigPanel = nullptr;
	HWND filmConfigPanel = nullptr;
	HWND performanceConfigPanel = nullptr;
	HWND lightPathConfigPanel = nullptr;
	HWND renderPassesConfigPanel = nullptr;
	HWND stereoConfigPanel = nullptr;
	HWND aboutConfigPanel = nullptr;
	HWND debugConfigPanel = nullptr;

	// Misc
	ICustEdit* cudaGpuEdit = nullptr;
	ICustEdit* optixGpuEdit = nullptr;
	ICustEdit* mtlPreviewSamplesEdit = nullptr;
	ISpinnerControl* mtlPreviewSamplesSpinner = nullptr;

	// Sampling
	ICustEdit* samplesEdit = nullptr;
	ICustEdit* adaptiveThresholdEdit = nullptr;
	ICustEdit* adaptiveMinimumEdit = nullptr;
	ICustEdit* clampDirectEdit = nullptr;
	ICustEdit* clampIndirectEdit = nullptr;
	ICustEdit* randSeedEdit = nullptr;
	ICustEdit* volStepRateEdit = nullptr;
	ICustEdit* volMaxStepsEdit = nullptr;
	ISpinnerControl* samplesSpinner = nullptr;
	ISpinnerControl* adaptiveThresholdSpinner = nullptr;
	ISpinnerControl* adaptiveMinimumSpinner = nullptr;
	ISpinnerControl* clampDirectSpinner = nullptr;
	ISpinnerControl* clampIndirectSpinner = nullptr;
	ISpinnerControl* randSeedSpinner = nullptr;
	ISpinnerControl* volStepRateSpinner = nullptr;
	ISpinnerControl* volMaxStepsSpinner = nullptr;

	// Translation
	ICustEdit* bgIntensityMulEdit = nullptr;
	ICustEdit* misMapSizeEdit = nullptr;
	ICustEdit* pointLightSizeEdit = nullptr;
	ICustEdit* texmapBakeWidthEdit = nullptr;
	ICustEdit* texmapBakeHeightEdit = nullptr;
	ICustEdit* deformBlurSamplesEdit = nullptr;
	ISpinnerControl* bgIntensityMulSpinner = nullptr;
	ISpinnerControl* misMapSizeSpinner = nullptr;
	ISpinnerControl* pointLightSizeSpinner = nullptr;
	ISpinnerControl* texmapBakeWidthSpinner = nullptr;
	ISpinnerControl* texmapBakeHeightSpinner = nullptr;
	ISpinnerControl* deformBlurSamplesSpinner = nullptr;

	// Light path
	ICustEdit* lpMaxBounceEdit = nullptr;
	ICustEdit* lpMinBounceEdit = nullptr;
	ICustEdit* lpDiffuseBounceEdit = nullptr;
	ICustEdit* lpGlossyBounceEdit = nullptr;
	ICustEdit* lpTransmissionBounceEdit = nullptr;
	ICustEdit* lpTransparentBounceEdit = nullptr;
	ICustEdit* lpVolumeBounceEdit = nullptr;
	ISpinnerControl* lpMaxBounceSpinner = nullptr;
	ISpinnerControl* lpMinBounceSpinner = nullptr;
	ISpinnerControl* lpDiffuseBounceSpinner = nullptr;
	ISpinnerControl* lpGlossyBounceSpinner = nullptr;
	ISpinnerControl* lpTransmissionBounceSpinner = nullptr;
	ISpinnerControl* lpTransparentBounceSpinner = nullptr;
	ISpinnerControl* lpVolumeBounceSpinner = nullptr;

	// Film
	ICustEdit* exposureEdit = nullptr;
	ICustEdit* filterSizeEdit = nullptr;
	ISpinnerControl* exposureSpinner = nullptr;
	ISpinnerControl* filterSizeSpinner = nullptr;

	// Performance
	ICustEdit* cpuThreadsEdit = nullptr;
	ISpinnerControl* cpuThreadsSpinner = nullptr;
	ICustEdit* tileWidthEdit = nullptr;
	ICustEdit* tileHeightEdit = nullptr;
	ISpinnerControl* tileWidthSpinner = nullptr;
	ISpinnerControl* tileHeightSpinner = nullptr;

	// Stereoscopy
	ICustEdit* interocularDistEdit = nullptr;
	ICustEdit* convergenceDistEdit = nullptr;
	ISpinnerControl* interocularDistSpinner = nullptr;
	ISpinnerControl* convergenceDistSpinner = nullptr;

	// Render passes
	ICustEdit* passesMistNearEdit = nullptr;
	ICustEdit* passesMistDepthEdit = nullptr;
	ICustEdit* passesMistExponentEdit = nullptr;
	ISpinnerControl* passesMistNearSpinner = nullptr;
	ISpinnerControl* passesMistDepthSpinner = nullptr;
	ISpinnerControl* passesMistExponentSpinner = nullptr;

	CyclesRendParamDlg(CyclesRenderParams* cycles_rend_params, IRendParams* i, BOOL prog);
	~CyclesRendParamDlg();

	void AcceptParams();
	void RejectParams();
	void DeleteThis() { delete this; }
	
	void InitProgressRollup(HWND hWnd);

	void InitUncategorizedConfig(HWND hWnd);
	void InitSamplingConfig(HWND hWnd);
	void InitTranslationConfig(HWND hWnd);
	void InitLightPathConfig(HWND hWnd);
	void InitFilmConfig(HWND hWnd);
	void InitPerformanceConfig(HWND hWnd);
	void InitStereoConfig(HWND hWnd);
	void InitPassesConfig(HWND hWnd);
	void InitDebugConfig(HWND hWnd);

private:
	const std::unique_ptr<LoggerInterface> logger;
};
