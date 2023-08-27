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
 * @brief Defines functions used to translate a camera.
 */

#include <boost/optional.hpp>

#include <maxtypes.h>

#include "trans_output.h"

namespace ccl {
	class Camera;
}

namespace MaxSDK {
	namespace RenderingAPI {
		class IRenderSessionContext;
	}
}

class CyclesRenderParams;

/**
 * @brief Returns information about the given camera.
 */
boost::optional<CyclesCameraParams> get_camera_params(const MaxSDK::RenderingAPI::IRenderSessionContext& session_context, TimeValue t, StereoscopyType stereo_type);

/**
 * @brief Applies the given camera parameters to the ccl::Camera.
 */
void apply_camera_params(const CyclesCameraParams& camera_params, const CyclesRenderParams& rend_params, ccl::Camera& camera);
