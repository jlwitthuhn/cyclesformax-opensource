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
 
#include "util_cycles_integrator.h"

#include <render/integrator.h>

#include "rend_params.h"
#include "trans_output.h"

void apply_integrator_params(ccl::Integrator& integrator, const CyclesRenderParams& rend_params, const CyclesCameraParams& cam_params)
{
	if (rend_params.use_clamp_direct) {
		integrator.set_sample_clamp_direct(rend_params.clamp_direct);
	}

	if (rend_params.use_clamp_indirect) {
		integrator.set_sample_clamp_indirect(rend_params.clamp_indirect);
	}

	const int rand_seed = rend_params.rand_seed;
	if (rend_params.animate_rand_seed) {
		integrator.set_seed(rand_seed + rend_params.frame_t);
	}
	else {
		integrator.set_seed(rand_seed);
	}

	integrator.set_volume_max_steps(rend_params.vol_max_steps);
	integrator.set_volume_step_rate(rend_params.vol_step_rate);

	const int lp_max_bounce = rend_params.lp_max_bounce;
	if (lp_max_bounce >= 0) {
		integrator.set_max_bounce(lp_max_bounce);
	}

	const int lp_min_bounce = rend_params.lp_min_bounce;
	if (lp_min_bounce >= 0) {
		integrator.set_min_bounce(lp_min_bounce);
	}

	const int lp_diffuse_bounces = rend_params.lp_diffuse_bounce;
	if (lp_diffuse_bounces >= 0) {
		integrator.set_max_diffuse_bounce(lp_diffuse_bounces);
	}

	const int lp_glossy_bounces = rend_params.lp_glossy_bounce;
	if (lp_glossy_bounces >= 0) {
		integrator.set_max_glossy_bounce(lp_glossy_bounces);
	}

	const int lp_transmission_bounces = rend_params.lp_transmission_bounce;
	if (lp_transmission_bounces >= 0) {
		integrator.set_max_transmission_bounce(lp_transmission_bounces);
	}

	const int lp_transparent_bounces = rend_params.lp_transparent_bounce;
	if (lp_transparent_bounces >= 0) {
		integrator.set_transparent_max_bounce(lp_transparent_bounces);
	}

	const int lp_volume_bounces = rend_params.lp_volume_bounce;
	if (lp_volume_bounces >= 0) {
		integrator.set_max_volume_bounce(lp_volume_bounces);
	}

	integrator.set_motion_blur(cam_params.is_motion_blur_enabled());

	if (rend_params.use_adaptive_sampling) {
		integrator.set_sampling_pattern(ccl::SAMPLING_PATTERN_PMJ);
		integrator.set_adaptive_threshold(rend_params.adaptive_threshold * rend_params.adaptive_threshold);
		integrator.set_adaptive_min_samples(rend_params.adaptive_min_samples);
	}
	else {
		integrator.set_adaptive_threshold(0.0f);
		integrator.set_adaptive_min_samples(INT_MAX);
	}
}
