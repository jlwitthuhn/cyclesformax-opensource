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
 * @brief Defines classes used to organize plugin materials and texmaps in the material browser.
 */

#include <IMaterialBrowserEntryInfo.h>

class TexmapBitmapFilterBrowserEntryInfo: public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class TexmapSkyBrowserEntryInfo: public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class TexmapEnvironmentBrowserEntryInfo: public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatShaderBroswerEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatShaderGraph08BrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatShaderGraph16BrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatShaderGraph32BrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatAddBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatAnisotropicBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatDiffuseBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatEmissionBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatGlassBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatGlossyBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatHairBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatHoldoutBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatMixBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatPrincipledBsdfBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatRefractionBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatSubsurfaceScatterBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatToonBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatTranslucentBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatTransparentBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatVelvetBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
    virtual const MCHAR* GetEntryName() const;
    virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatVolAbsorptionBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatVolScatterBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatVolAddBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};

class MatVolMixBrowserEntryInfo : public IMaterialBrowserEntryInfo
{
	virtual const MCHAR* GetEntryName() const;
	virtual const MCHAR* GetEntryCategory() const;
	virtual Bitmap* GetEntryThumbnail() const { return nullptr; }
};
