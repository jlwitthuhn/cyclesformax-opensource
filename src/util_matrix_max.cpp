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
 
#include "util_matrix_max.h"

ccl::Transform cycles_transform_from_max_matrix(const Matrix3& max_matrix)
{
	const Point3 row0 = max_matrix.GetRow(0);
	const Point3 row1 = max_matrix.GetRow(1);
	const Point3 row2 = max_matrix.GetRow(2);
	const Point3 row3 = max_matrix.GetRow(3);

	ccl::Transform result = ccl::make_transform(
		row0.x, row1.x, row2.x, row3.x,
		row0.y, row1.y, row2.y, row3.y,
		row0.z, row1.z, row2.z, row3.z
	);

	return result;
}
