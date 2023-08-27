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
 
#include "rend_logger_ext.h"

LoggerInterface& operator<<(LoggerInterface& lhs, const Class_ID& rhs)
{
	const LogNumberFormat initial_format = lhs.number_format(LogNumberFormat::HEX);
	lhs << "Class_ID(0x" << rhs.PartA() << ", 0x" << rhs.PartB() << ")";
	lhs.number_format(initial_format);
	return lhs;
}

LoggerInterface& operator<<(LoggerInterface& lhs, const ccl::float3& rhs)
{
	lhs << "ccl::float3(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
	return lhs;
}

LoggerInterface& operator<<(LoggerInterface& lhs, const ccl::float4& rhs)
{
	lhs << "ccl::float4(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ", " << rhs.w << ")";
	return lhs;
}
