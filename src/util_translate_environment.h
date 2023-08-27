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
 * @brief Defines functions used to translate an environment.
 */

#include <memory>

#include <maxtypes.h>

#include "trans_output.h"

class CyclesRenderParams;
class MaxShaderManager;

namespace MaxSDK {
	namespace RenderingAPI {
		class IRenderSessionContext;
	}
}

/**
 * @brief Returns a CyclesEnvironmentParams corresponding to the given scene.
 */
CyclesEnvironmentParams get_environment_params(MaxSDK::RenderingAPI::IRenderSessionContext& session_context, TimeValue t);

/**
 * @brief Use the given MaxShaderManager to create a shader matching the provided CyclesEnvironmentParams.
 */
void apply_environment_params(const CyclesEnvironmentParams& env_params, const CyclesRenderParams& rend_params, const std::unique_ptr<MaxShaderManager>& shader_manager);
