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
 * @brief Defines classes used to convert serialized shader graphs to ccl::Shaders.
 */

#include <map>
#include <memory>

#include <util/util_types.h>

#include "rend_logger.h"

class BakedTexmapCache;
class Texmap;

namespace ccl {
	class Scene;
	class ShaderGraph;
	class ShaderNode;
}

/**
 * @brief Class to convert serialized graphs to ccl::Shader objects.
 */
class ShaderGraphConverter {
public:
	ShaderGraphConverter(BakedTexmapCache& texmap_cache);
	void set_texmap_slot(Texmap* texmap, size_t slot);
	ccl::ShaderGraph* get_shader_graph(std::string encoded_graph, ccl::Scene* scene) const;

private:
	std::map<size_t, Texmap*> texmap_slots;

	BakedTexmapCache& texmap_cache;

	const std::unique_ptr<LoggerInterface> logger;
};
