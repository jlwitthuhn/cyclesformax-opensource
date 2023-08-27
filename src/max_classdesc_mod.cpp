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
 
#include "max_classdesc_mod.h"

#include "plugin_mod_properties.h"

void* CyclesPropertiesModifierClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesPropertiesMod();
}

CyclesPropertiesModifierClassDesc* GetCyclesPropertiesModifierClassDesc()
{
	static CyclesPropertiesModifierClassDesc class_desc;
	return &class_desc;
}
