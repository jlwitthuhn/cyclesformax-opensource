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
 
#include "max_classdesc_tex.h"

#include "plugin_tex_bitmap_filter.h"
#include "plugin_tex_environment.h"
#include "plugin_tex_sky.h"

void* CyclesTexmapBitmapFilterClassDesc::Create(const BOOL /*loading*/)
{
	return new BitmapFilterTexmap();
}

FPInterface* CyclesTexmapBitmapFilterClassDesc::GetInterface(const Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesTexmapSkyClassDesc::Create(const BOOL /*loading*/)
{
	return new CyclesSkyEnvTexmap();
}

FPInterface* CyclesTexmapSkyClassDesc::GetInterface(const Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

void* CyclesTexmapEnvironmentClassDesc::Create(const BOOL /*loading*/)
{
	return new EnvironmentTexmap();
}

FPInterface* CyclesTexmapEnvironmentClassDesc::GetInterface(const Interface_ID id)
{
	if (id == IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE) {
		return &entry_info;
	}
	return ClassDesc2::GetInterface(id);
}

CyclesTexmapBitmapFilterClassDesc* GetCyclesTexmapBitmapFilterClassDesc()
{
	static CyclesTexmapBitmapFilterClassDesc class_desc;
	return &class_desc;
}

CyclesTexmapSkyClassDesc* GetCyclesTexmapSkyClassDesc()
{
	static CyclesTexmapSkyClassDesc class_desc;
	return &class_desc;
}

CyclesTexmapEnvironmentClassDesc* GetCyclesTexmapEnvironmentClassDesc()
{
	static CyclesTexmapEnvironmentClassDesc class_desc;
	return &class_desc;
}
