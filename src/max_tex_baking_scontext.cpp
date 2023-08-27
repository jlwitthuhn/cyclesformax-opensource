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
 
#include "max_tex_baking_scontext.h"

#include <cmath>

#include "const_classid.h"

#define INVERSE_OF_INT(x) 1.0f / static_cast<float>(x)

// Some parameters are multiplied by 6
// I do not know why, such is life
#define MAGIC_SCALAR 6

#define DEFAULT_MULTIPLIER 1.0f

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

CyclesTexmapScontext::CyclesTexmapScontext(const TimeValue t, const BOOL mtl_edit, const int width, const int height)
	: ShadeContext()
{
	curr_time = t;
	in_mtl_edit = (mtl_edit != 0);
	uv_point = Point2(0, 0);
	mat_width = width;
	mat_height = height;

	// Public ShadeContext members
	ambientLight = Color(1.0f, 1.0f, 1.0f);
	backFace = 0;
	doMaps = 1;
	filterMaps = 1;
	globContext = nullptr;
	mode = SCMode::SCMODE_TEXMAP;
	mtlNum = 0;
	nLights = 0;
	rayLevel = 0;
	shadow = 0;
	xshadeID = 0;

	radial_distance_powers.push_back(1.05);
	radial_distance_powers.push_back(1.1);
	radial_distance_powers.push_back(1.15);
	radial_distance_powers.push_back(1.2);
	radial_distance_powers.push_back(1.25);
	radial_distance_powers.push_back(1.3);
	radial_distance_powers.push_back(1.35);
	radial_distance_powers.push_back(1.4);
	radial_distance_powers.push_back(1.45);
	radial_distance_powers.push_back(1.5);
}

void CyclesTexmapScontext::SetUV(const Point2 uv_in, const int x_pos, const int y_pos)
{
	uv_point = uv_in;
	screen_pos = IPoint2(x_pos, y_pos);
}

constexpr double PI = 3.14159;

void CyclesTexmapScontext::SetUVFromRadialPixel(const int x, const int y, const int width, const int height)
{
	// Find the horizontal angle represented by this point
	// Horizontal angle can be in range [pi, 3*pi] ([0, 2*pi] + pi)
	const double angle = (1.0 - x / static_cast<double>(width)) * 2 * PI + PI;

	// Find vertical distance, this will be in the range [0, 1]
	double distance = distance = 1.0 - (y / static_cast<double>(height));;

	const bool use_pow = false;
	const bool use_special_pow = true;
	if (use_pow) {
		// The projection of this map is not quite linear
		// We only care about adjusting the inner half, technically this should also apply to the other half as well but the physical sky has a bottom half of a constant color so it can be ignored
		if (distance > 0.0 && distance < 0.5) {
			const double scaled_distance = distance * 2;
			const double pow_low = scaled_distance; // pow(scaled_distance, 1.00);
			const double pow_high = pow(scaled_distance, 1.35);
			const double weight_factor = scaled_distance;
			const double result = weight_factor * pow_low + (1.0 - weight_factor) * pow_high;
			distance = result / 2.0;
		}
	}
	if (use_special_pow) {
		distance = GetAdjustedRadialDistance(distance);
	}

	// Find the point represented by the given angle and distance, offset by (0.5, 0.5) to center it on UV map
	const float x_pos = cos(angle) * distance + 0.5f;
	const float y_pos = sin(angle) * distance + 0.5f;
	uv_point.x = x_pos;
	uv_point.y = y_pos;

	screen_pos = IPoint2(x_pos * width, y_pos * height);
}

Class_ID CyclesTexmapScontext::ClassID()
{
	return CYCLES_TEXMAP_SCONTEXT_CLASS;
}

BOOL CyclesTexmapScontext::InMtlEditor()
{
	return in_mtl_edit;
}

int CyclesTexmapScontext::Antialias()
{
	return 0;
}

int CyclesTexmapScontext::ProjType()
{
	return 1; // 1 for parallel, 0 for perspective
}

LightDesc* CyclesTexmapScontext::Light(const int /*n*/)
{
	return nullptr;
}

TimeValue CyclesTexmapScontext::CurTime()
{
	return curr_time;
}

int CyclesTexmapScontext::NodeID()
{
	return -1;
}

INode* CyclesTexmapScontext::Node()
{
	return nullptr;
}

Object* CyclesTexmapScontext::GetEvalObject()
{
	return nullptr;
}

Point3 CyclesTexmapScontext::BarycentricCoords()
{
	return Point3(0, 0, 0);
}

int CyclesTexmapScontext::FaceNumber()
{
	return 0;
}

Point3 CyclesTexmapScontext::Normal()
{
	// Copied from the shadecontext max used to render preview texmaps
	return Point3(0, 0, 1);
}

Point3 CyclesTexmapScontext::OrigNormal()
{
	return Normal();
}

Point3 CyclesTexmapScontext::GNormal()
{
	return Normal();
}

float CyclesTexmapScontext::Curve()
{
	return 0.0f;
}

Point3 CyclesTexmapScontext::V()
{
	const float x_dist = -0.5f + (1.0f - uv_point.x); // Tracks inverse of U coord
	const float y_dist = -0.5f;
	const float z_dist = -0.5f + uv_point.y; // Tracks V coord

	const Point3 dir_vec(x_dist, y_dist, z_dist);

	return dir_vec.Normalize();
}

void CyclesTexmapScontext::SetView(const Point3 /*p*/)
{
	// Do nothing
}

Point3 CyclesTexmapScontext::OrigView()
{
	return V();
}

Point3 CyclesTexmapScontext::ReflectVector()
{
	return Point3(0, 0, 1);
}

Point3 CyclesTexmapScontext::RefractVector(const float /*ior*/)
{
	return Point3(0, 0, 1);
}

float CyclesTexmapScontext::GetIOR()
{
	return 1.0f;
}

Point3 CyclesTexmapScontext::CamPos()
{
	return Point3(0, 0, 0);
}

Point3 CyclesTexmapScontext::P()
{
	return Point3(uv_point.x * MAGIC_SCALAR, uv_point.y * MAGIC_SCALAR, 0.0f);
}

Point3 CyclesTexmapScontext::DP()
{
	// Copied from the shadecontext max used to render preview texmaps
	//return Point3(MAGIC_SCALAR * INVERSE_OF_INT(mat_width), MAGIC_SCALAR * INVERSE_OF_INT(mat_height), 0.0f);
	return Point3(0.0f, 0.0f, 0.0f);
}

Point3 CyclesTexmapScontext::PObj()
{
	return P();
}

Point3 CyclesTexmapScontext::DPObj()
{
	return DP();
}

Box3 CyclesTexmapScontext::ObjectBox()
{
	// Copied from the shadecontext max used to render preview texmaps
	return Box3(Point3(0, 0, 0), Point3(MAGIC_SCALAR, MAGIC_SCALAR, MAGIC_SCALAR));
}

Point3 CyclesTexmapScontext::PObjRelBox()
{
	// Copied from the shadecontext max used to render preview texmaps
	// Scale U and V from [0,1] to [-1,1]
	const float x_coord = 2 * uv_point.x - 1;
	const float y_coord = 2 * uv_point.y - 1;
	return Point3(x_coord, y_coord, -1.0f);
}

Point3 CyclesTexmapScontext::DPObjRelBox()
{
	const Point3 dp = DP();
	return Point3(dp.x * 2 - 1, dp.y * 2 - 1, dp.z * 2 - 1);
}

void CyclesTexmapScontext::ScreenUV(Point2& uv, Point2& duv)
{
	uv = uv_point;
	const Point3 duvw = DUVW();
	duv.x = duvw.x;
	duv.y = duvw.y;
}

IPoint2 CyclesTexmapScontext::ScreenCoord()
{
	return screen_pos;
}

Point3 CyclesTexmapScontext::UVW(const int /*channel*/)
{
	return Point3(uv_point.x, uv_point.y, 0.0f);
}

Point3 CyclesTexmapScontext::DUVW(const int /*channel*/)
{
	return Point3(INVERSE_OF_INT(mat_width), INVERSE_OF_INT(mat_height), 0.0f);
}

void CyclesTexmapScontext::DPdUVW(Point3 /*dP*/[3], const int /*channel*/)
{
	// Do nothing
}

float CyclesTexmapScontext::RayConeAngle()
{
	return INVERSE_OF_INT(mat_width);
}

float CyclesTexmapScontext::RayDiam()
{
	return 8.5f * INVERSE_OF_INT(mat_width);
}

void CyclesTexmapScontext::GetBGColor(Color& /*bgcol*/, Color& /*transp*/, const BOOL /*fogBG*/)
{
	// Do nothing
}

Point3 CyclesTexmapScontext::PointTo(const Point3& p, const RefFrame /*ito*/)
{
	return p;
}

Point3 CyclesTexmapScontext::PointFrom(const Point3& p, const RefFrame /*ifrom*/)
{
	return p;
}

Point3 CyclesTexmapScontext::VectorTo(const Point3& p, const RefFrame /*ito*/)
{
	return p;
}

Point3 CyclesTexmapScontext::VectorFrom(const Point3& p, const RefFrame /*ifrom*/)
{
	return p;
}

double CyclesTexmapScontext::GetAdjustedRadialDistance(const double dist)
{
	assert(radial_distance_powers.size() > 1);

	if (dist >= 0.4999) {
		return dist;
	}

	const size_t section_count = radial_distance_powers.size() - 1;
	const double section_size = 1.0 / static_cast<double>(section_count);

	const double normalized_dist = dist * 2.0;

	const size_t section_index = normalized_dist / section_size;
	assert(section_index < section_count);

	const double section_weight = (normalized_dist - section_index * section_size) / section_size;

	const double lo_pow = pow(normalized_dist, radial_distance_powers[section_index]);
	const double hi_pow = pow(normalized_dist, radial_distance_powers[section_index + 1]);

	const double result = section_weight * hi_pow + (1.0 - section_weight) * lo_pow;

	return result / 2.0;
}
