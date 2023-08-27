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
 
#include "plugin_mat_shader_graph_ops.h"

#include "max_classdesc_mtl.h"

static FPInterfaceDesc shader_graph_08_desc(SHADER_GRAPH_OPS_ID,
	_T("graphops"), 0,
	GetCyclesMaterialShaderGraph08ClassDesc(), FP_MIXIN,
	properties,
	static_cast<int>(ShaderGraphOpsEnum::GET), static_cast<int>(ShaderGraphOpsEnum::SET), _T("cycles_graph"), 0, TYPE_TSTR_BV,
	p_end
);

FPInterfaceDesc* ShaderGraph08Mixin::GetDesc()
{
	return &shader_graph_08_desc;
}

static FPInterfaceDesc shader_graph_16_desc(SHADER_GRAPH_OPS_ID,
	_T("graphops"), 0,
	GetCyclesMaterialShaderGraph16ClassDesc(), FP_MIXIN,
	properties,
	static_cast<int>(ShaderGraphOpsEnum::GET), static_cast<int>(ShaderGraphOpsEnum::SET), _T("cycles_graph"), 0, TYPE_TSTR_BV,
	p_end
);

FPInterfaceDesc* ShaderGraph16Mixin::GetDesc()
{
	return &shader_graph_16_desc;
}

static FPInterfaceDesc shader_graph_32_desc(SHADER_GRAPH_OPS_ID,
	_T("graphops"), 0,
	GetCyclesMaterialShaderGraph32ClassDesc(), FP_MIXIN,
	properties,
	static_cast<int>(ShaderGraphOpsEnum::GET), static_cast<int>(ShaderGraphOpsEnum::SET), _T("cycles_graph"), 0, TYPE_TSTR_BV,
	p_end
);

FPInterfaceDesc* ShaderGraph32Mixin::GetDesc()
{
	return &shader_graph_32_desc;
}
