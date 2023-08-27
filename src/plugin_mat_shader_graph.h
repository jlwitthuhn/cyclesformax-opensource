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
 * @brief Defines common classes and functionality used by all Shader Graph materials.
 */

#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "plugin_mat_base.h"
#include "plugin_mat_shader_graph_ops.h"

class ShaderEditorSharedData;
class ShaderParamsDescriptor;

/*
 * @brief Base class for Shader Graph materials. This implements almost all functionality needed by Shader Graph materials
 */
class CyclesShaderGraphMatBase : public CyclesPluginMatBase {
public:
	CyclesShaderGraphMatBase();

	// Material-specific functions
	void OpenNodeEditor();
	std::string GetNodeGraph();

	virtual Texmap* GetNumberedTexmap(size_t number) = 0;

	// Shader params
	void PopulateShaderParamsDesc(ShaderParamsDescriptor* desc);

	// UI setup
	virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;

	// Max functions
	virtual IOResult Load(ILoad *iload) override;
	virtual IOResult Save(ISave *isave) override;

	virtual void Update(TimeValue t, Interval& valid) override;

	virtual BaseInterface* GetInterface(Interface_ID id) override;

	// Mixins
	virtual TSTR GetShaderGraph();
	virtual void SetShaderGraph(TSTR input);

	std::string loaded_graph;
	bool loaded_graph_set = false;

protected:
	virtual IParamBlock2* GetShaderParamsPblock() = 0;

	std::shared_ptr<ShaderEditorSharedData> shared_data;
	std::thread shader_editor_thread;
};
