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
 * @brief Defines ClassDesc2s for texmaps included with the plugin.
 */

#include <imtl.h>
#include <iparamb2.h>

#include "const_classid.h"
#include "max_mat_browser.h"

extern HINSTANCE hInstance;

class CyclesTexmapBitmapFilterClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Bitmap Filter"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_TEXMAP_FILTER_CLASS; }
	virtual const MCHAR* Category() { return TEXMAP_CAT_2D;  }
	virtual const MCHAR* InternalName() { return L"texmap_cycles_bitmap_filter"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	TexmapBitmapFilterBrowserEntryInfo entry_info;
};

class CyclesTexmapSkyClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Sky"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_TEXMAP_SKY_CLASS; }
	virtual const MCHAR* Category() { return TEXMAP_CAT_ENV; }
	virtual const MCHAR* InternalName() { return L"texmap_cycles_sky"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	TexmapSkyBrowserEntryInfo entry_info;
};

class CyclesTexmapEnvironmentClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() { return 1; }
	virtual const MCHAR* ClassName() { return L"Cycles Environment"; }
	virtual const MCHAR* NonLocalizedClassName() { return ClassName(); }
	virtual SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
	virtual Class_ID ClassID() { return CYCLES_TEXMAP_ENVIRONMENT_CLASS; }
	virtual const MCHAR* Category() { return TEXMAP_CAT_ENV; }
	virtual const MCHAR* InternalName() { return L"texmap_cycles_environment"; }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual void* Create(BOOL loading = FALSE);
	virtual FPInterface* GetInterface(Interface_ID id);

private:
	TexmapEnvironmentBrowserEntryInfo entry_info;
};

CyclesTexmapBitmapFilterClassDesc* GetCyclesTexmapBitmapFilterClassDesc();
CyclesTexmapSkyClassDesc* GetCyclesTexmapSkyClassDesc();
CyclesTexmapEnvironmentClassDesc* GetCyclesTexmapEnvironmentClassDesc();
