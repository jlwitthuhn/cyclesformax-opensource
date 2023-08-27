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
 
#include "util_color_temp.h"

static double polynomial_8(const double x, const double c0, const double c1, const double c2, const double c3, const double c4, const double c5, const double c6, const double c7, const double c8)
{
	double acc = 0.0;
	double after_power = 1.0;
	
	acc += c0 * after_power;
	after_power *= x;
	acc += c1 * after_power;
	after_power *= x;
	acc += c2 * after_power;
	after_power *= x;
	acc += c3 * after_power;
	after_power *= x;
	acc += c4 * after_power;
	after_power *= x;
	acc += c5 * after_power;
	after_power *= x;
	acc += c6 * after_power;
	after_power *= x;
	acc += c7 * after_power;
	after_power *= x;
	acc += c8 * after_power;

	return acc;
}

// The coefficients here are from three 8th degree polynomials for red, green, and blue.
// The polynomials were generated using linear regression on a number of RGB values produced by ART to copy the temparature curve from ART as closely as possible.
// Note that this produces slightly different values than the Cycles Blackbody Node.
ccl::float3 rgb_from_temp(float color_temp)
{
	// The offsets on the C0 terms are to make 6500 = (1, 1, 1) exactly
	// The left side is the value from the equation and the right side is the offset
	constexpr double R_C0 =  5.6911798 + (-0.000526);
	constexpr double R_C1 = -2.8151440e-3;
	constexpr double R_C2 =  7.9372248e-7;
	constexpr double R_C3 = -1.3407811e-10;
	constexpr double R_C4 =  1.4294266e-14;
	constexpr double R_C5 = -9.6808701e-19;
	constexpr double R_C6 =  4.0341431e-23;
	constexpr double R_C7 = -9.4218432e-28;
	constexpr double R_C8 =  9.4295793e-33;

	constexpr double G_C0 = -0.2819545 + 0.000163;
	constexpr double G_C1 =  8.5404165e-4;
	constexpr double G_C2 = -2.5510779e-7;
	constexpr double G_C3 =  4.4093968e-11;
	constexpr double G_C4 = -4.7478376e-15;
	constexpr double G_C5 =  3.2292891e-19;
	constexpr double G_C6 = -1.3478480e-23;
	constexpr double G_C7 =  3.1488023e-28;
	constexpr double G_C8 = -3.1501398e-33;

	constexpr double B_C0 = -0.3682470 + (-0.000156);
	constexpr double B_C1 =  1.8201210e-5;
	constexpr double B_C2 =  1.3061578e-7;
	constexpr double B_C3 = -3.1691709e-11;
	constexpr double B_C4 =  3.8566286e-15;
	constexpr double B_C5 = -2.7738087e-19;
	constexpr double B_C6 =  1.1914009e-23;
	constexpr double B_C7 = -2.8277884e-28;
	constexpr double B_C8 =  2.8552937e-33;

	double double_temp = static_cast<double>(color_temp);
	double r = polynomial_8(double_temp, R_C0, R_C1, R_C2, R_C3, R_C4, R_C5, R_C6, R_C7, R_C8);
	double g = polynomial_8(double_temp, G_C0, G_C1, G_C2, G_C3, G_C4, G_C5, G_C6, G_C7, G_C8);
	double b = polynomial_8(double_temp, B_C0, B_C1, B_C2, B_C3, B_C4, B_C5, B_C6, B_C7, B_C8);

	return ccl::make_float3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}
