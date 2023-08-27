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

#include "ifnpub.h"
#include "maxscript/maxscript.h"

static const Interface_ID SHADER_GRAPH_OPS_ID{ 0x625e6e72, 0x13ad2f49 };

enum class ShaderGraphOpsEnum {
	GET,
	SET,
};

#pragma warning( push )
#pragma warning( disable: 4238 )

class ShaderGraphOpsMixin : public FPMixinInterface
{
	BEGIN_FUNCTION_MAP
		PROP_FNS(static_cast<int>(ShaderGraphOpsEnum::GET), GetShaderGraph, static_cast<int>(ShaderGraphOpsEnum::SET), SetShaderGraph, TYPE_TSTR_BV)
	END_FUNCTION_MAP

public:
	virtual FPInterfaceDesc* GetDesc() = 0;

	virtual MSTR GetShaderGraph() = 0;
	virtual void SetShaderGraph(MSTR input) = 0;
};

#pragma warning( pop )

class ShaderGraph08Mixin : public ShaderGraphOpsMixin
{
	virtual FPInterfaceDesc* GetDesc() override;
};

class ShaderGraph16Mixin : public ShaderGraphOpsMixin
{
	virtual FPInterfaceDesc* GetDesc() override;
};

class ShaderGraph32Mixin : public ShaderGraphOpsMixin
{
	virtual FPInterfaceDesc* GetDesc() override;
};
