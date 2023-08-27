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
 
#include "util_cycles_film.h"

#include <render/film.h>

#include "rend_params.h"

void apply_film_params(ccl::Film& film, const CyclesRenderParams& rend_params)
{
	film.set_exposure(rend_params.exposure_multiplier);
	film.set_mist_start(rend_params.mist_near);
	film.set_mist_depth(rend_params.mist_depth);
	film.set_mist_falloff(rend_params.mist_exponent);

	film.set_use_adaptive_sampling(rend_params.use_adaptive_sampling);

	if (rend_params.filter_type == FilmFilterType::BLACKMAN_HARRIS) {
		film.set_filter_type(ccl::FilterType::FILTER_BLACKMAN_HARRIS);
		film.set_filter_width(rend_params.filter_size);
	}
	else if (rend_params.filter_type == FilmFilterType::GAUSSIAN) {
		film.set_filter_type(ccl::FilterType::FILTER_GAUSSIAN);
		film.set_filter_width(rend_params.filter_size);
	}
	else {
		film.set_filter_type(ccl::FilterType::FILTER_BOX);
		film.set_filter_width(1.0f);
	}
}
