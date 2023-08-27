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
 * @brief Defines simple types to be used throughout the plugin.
 */

/**
 * @brief A pair of integers.
 */
class Int2 {
public:
	Int2(int x, int y);

	inline int x() const { return _x; }
	inline int y() const { return _y; }

	Int2 operator+(const Int2& other) const;
	Int2 operator-(const Int2& other) const;

	bool operator==(const Int2& other) const;
	bool operator!=(const Int2& other) const;
	bool operator<(const Int2& other) const;

private:
	int _x;
	int _y;
};

/**
 * @brief A pair of doubles.
 */
class Double2 {
public:
	Double2(double x, double y);

	inline double x() const { return _x; }
	inline double y() const { return _y; }

	bool operator==(const Double2& other) const;
	bool operator!=(const Double2& other) const;
	bool operator<(const Double2& other) const;

private:
	double _x;
	double _y;
};

/**
 * @brief A rectangle with integer vertices
 */
class IntRect {
public:
	IntRect();
	IntRect(Int2 begin, Int2 end);

	inline Int2 begin() const { return _begin; }
	inline Int2 end() const { return _end; }
	inline Int2 size() const { return _end - _begin; }

	bool contains(const Int2& point) const;
	bool contains(const IntRect& other) const;

	IntRect move(const Int2& distance) const;

	bool operator==(const IntRect& other) const;
	bool operator!=(const IntRect& other) const;

private:
	Int2 _begin;
	Int2 _end;
};


/**
 * @brief Class to hold a bitmap that is used for backplate compositing.
 */
class BackplateBitmap {
public:
	BackplateBitmap(Int2 resolution);
	~BackplateBitmap();

	size_t num_pixels() const;

	Int2 resolution;
	float* pixels = nullptr;
};

// Functions to convert between types
Double2 cast_as_double2(const Int2& input);
