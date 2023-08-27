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
 
#include "util_translate_environment.h"

#include <RenderingAPI/Renderer/IEnvironmentContainer.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "rend_params.h"
#include "rend_shader_manager.h"

using MaxSDK::RenderingAPI::IEnvironmentContainer;

CyclesEnvironmentParams get_environment_params(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, const TimeValue t)
{
	CyclesEnvironmentParams result;

	if (session_context.GetEnvironment()->GetEnvironmentMode() == IEnvironmentContainer::EnvironmentMode::Texture && session_context.GetEnvironment()->GetEnvironmentTexture() != nullptr) {
		result.environment_map = session_context.GetEnvironment()->GetEnvironmentTexture();
		session_context.CallRenderBegin(*(result.environment_map), t);
	}
	else {
		Interval color_valid = FOREVER;
		Color bg_max_color = session_context.GetEnvironment()->GetBackgroundColor(t, color_valid);
		result.bg_color = ccl::make_float3(bg_max_color.r, bg_max_color.g, bg_max_color.b);
	}

	return result;
}

void apply_environment_params(const CyclesEnvironmentParams& env_params, const CyclesRenderParams& rend_params, const std::unique_ptr<MaxShaderManager>& shader_manager)
{
	if (env_params.environment_map != nullptr) {
		shader_manager->set_sky_from_env_map(env_params.environment_map, rend_params.bg_intensity);
	}
	else {
		SimpleSkyDescriptor sky_desc;
		sky_desc.color = env_params.bg_color;

		if (rend_params.in_mtl_edit) {
			sky_desc.strength = 1.0f;
		}
		else {
			sky_desc.strength = rend_params.bg_intensity;
		}

		shader_manager->set_simple_sky_shader(sky_desc);
	}
}
