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
 
#include "util_simple_types.h"

Int2::Int2(const int x, const int y) : _x(x), _y(y)
{

}

Int2 Int2::operator+(const Int2& other) const
{
	return Int2(_x + other._x, _y + other._y);
}

Int2 Int2::operator-(const Int2& other) const
{
	return Int2(_x - other._x, _y - other._y);
}

bool Int2::operator==(const Int2& other) const
{
	return (_x == other._x) && (_y == other._y);
}

bool Int2::operator!=(const Int2& other) const
{
	return !(operator==(other));
}

bool Int2::operator<(const Int2& other) const
{
	if (_x < other._x) {
		return true;
	}
	else if (other._x < _x) {
		return false;
	}

	if (_y < other._y) {
		return true;
	}
	else if (other._y < _y) {
		return false;
	}

	return false;
}

Double2::Double2(const double x, const double y) : _x(x), _y(y)
{

}

bool Double2::operator==(const Double2& other) const
{
	return (_x == other._x) && (_y == other._y);
}

bool Double2::operator!=(const Double2& other) const
{
	return !(operator==(other));
}

bool Double2::operator<(const Double2& other) const
{
	if (_x < other._x) {
		return true;
	}
	else if (other._x < _x) {
		return false;
	}

	if (_y < other._y) {
		return true;
	}
	else if (other._y < _y) {
		return false;
	}

	return false;
}

IntRect::IntRect() : _begin(0, 0), _end(0, 0)
{

}

IntRect::IntRect(const Int2 begin, const Int2 end) : _begin(0, 0), _end(0, 0)
{
	const int x1 = begin.x();
	const int x2 = end.x();

	const int y1 = begin.y();
	const int y2 = end.y();

	int x_begin, x_end;
	int y_begin, y_end;

	if (x1 < x2) {
		x_begin = x1;
		x_end = x2;
	}
	else {
		x_begin = x2;
		x_end = x1;
	}

	if (y1 < y2) {
		y_begin = y1;
		y_end = y2;
	}
	else {
		y_begin = y2;
		y_end = y1;
	}

	_begin = Int2(x_begin, y_begin);
	_end = Int2(x_end, y_end);
}

bool IntRect::contains(const Int2& point) const
{
	const bool inside_begin = (point.x() >= _begin.x()) && (point.y() >= _begin.y());
	const bool inside_end = (point.x() <= _end.x()) && (point.y() <= _end.y());
	return inside_begin && inside_end;
}

bool IntRect::contains(const IntRect& point) const
{
	const bool inside_begin = contains(point.begin());
	const bool inside_end = contains(point.end());
	return inside_begin && inside_end;
}

IntRect IntRect::move(const Int2& distance) const
{
	const Int2 new_begin = _begin + distance;
	const Int2 new_end = _end + distance;
	return IntRect(new_begin, new_end);
}

bool IntRect::operator==(const IntRect& other) const
{
	return (_begin == other._begin) && (_end == other._end);
}

bool IntRect::operator!=(const IntRect& other) const
{
	return !operator==(other);
}

BackplateBitmap::BackplateBitmap(Int2 resolution) : resolution(resolution)
{
	constexpr size_t CHANNELS = 3;
	pixels = new float[CHANNELS * resolution.x() * resolution.y()];
}

BackplateBitmap::~BackplateBitmap()
{
	delete[] pixels;
}

size_t BackplateBitmap::num_pixels() const
{
	return static_cast<size_t>(resolution.x()) * resolution.y();
}

Double2 cast_as_double2(const Int2& input)
{
	return Double2(static_cast<double>(input.x()), static_cast<double>(input.y()));
}
