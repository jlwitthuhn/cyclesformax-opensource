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
 * @brief Defines ClassDesc2s for modifiers included with the plugin.
 */

#include <iparamb2.h>

#include "const_classid.h"

extern HINSTANCE hInstance;

class CyclesPropertiesModifierClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Properties"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return OSM_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_MOD_PROPERTIES_CLASS; }
	virtual const MCHAR* Category() { return L"Cycles"; }
	virtual const MCHAR* InternalName() { return L"mod_cycles_properties"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
};

CyclesPropertiesModifierClassDesc* GetCyclesPropertiesModifierClassDesc();
