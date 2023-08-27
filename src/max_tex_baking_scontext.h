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
 * @brief Defines CyclesTexmapScontext.
 */

#include <vector>

#include <Rendering/ShadeContext.h>

/**
 * @brief Implementation of ShadeContext, this class is used for baking Max texmaps.
 */
class CyclesTexmapScontext: public ShadeContext {
public:
	CyclesTexmapScontext(TimeValue t, BOOL mtl_edit, int width, int height);

	// Set the next pixel to be sampled
	virtual void SetUV(Point2 uv_in, int x_pos, int y_pos);
	virtual void SetUVFromRadialPixel(int x, int y, int width, int height);

	// ShadeContext functions
	virtual Class_ID ClassID() override;

	virtual BOOL InMtlEditor() override;
	virtual int Antialias() override;
	virtual int ProjType() override;
	virtual LightDesc* Light(int n) override;
	virtual TimeValue CurTime() override;
	virtual int NodeID() override;
	virtual INode* Node() override;
	virtual Object* GetEvalObject() override;

	virtual Point3 BarycentricCoords() override;
	virtual int FaceNumber() override;
	virtual Point3 Normal() override;
	virtual Point3 OrigNormal() override;
	virtual Point3 GNormal() override;
	virtual float Curve() override;

	virtual Point3 V() override;
	virtual void SetView(Point3 p) override;
	virtual Point3 OrigView() override;
	
	virtual Point3 ReflectVector() override;
	virtual Point3 RefractVector(float ior) override;
	virtual float GetIOR() override;

	virtual Point3 CamPos() override;
	virtual Point3 P() override;
	virtual Point3 DP() override;
	virtual Point3 PObj() override;
	virtual Point3 DPObj() override;
	virtual Box3 ObjectBox() override;
	virtual Point3 PObjRelBox() override;
	virtual Point3 DPObjRelBox() override;

	virtual void ScreenUV(Point2& uv, Point2& duv) override;
	virtual IPoint2 ScreenCoord() override;
	virtual Point3 UVW(int channel=0) override;
	virtual Point3 DUVW(int channel=0) override;
	virtual void DPdUVW(Point3 dP[3], int channel=0) override;
	
	virtual float RayConeAngle() override;
	virtual float RayDiam() override;

	virtual void GetBGColor(Color& bgcol, Color& transp, BOOL fogBG=true) override;
	
	virtual Point3 PointTo(const Point3& p, RefFrame ito) override;
	virtual Point3 PointFrom(const Point3& p, RefFrame ifrom) override;
	virtual Point3 VectorTo(const Point3& p, RefFrame ito) override;
	virtual Point3 VectorFrom(const Point3& p, RefFrame ifrom) override;

private:
	bool in_mtl_edit;
	TimeValue curr_time;
	Point2 uv_point;
	IPoint2 screen_pos;
	int mat_width, mat_height;

	std::vector<double> radial_distance_powers;

	double GetAdjustedRadialDistance(double dist);
};
