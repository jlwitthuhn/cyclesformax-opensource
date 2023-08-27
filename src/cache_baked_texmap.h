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
 * @brief Defines BakedTexmapCache and supporting classes.
 */

#include <chrono>
#include <map>
#include <memory>
#include <set>

#include <maxtypes.h>

#include "cycles_image.h"
#include "rend_logger.h"
#include "util_simple_types.h"

namespace ccl {
	class EnvironmentTextureNode;
	class ImageTextureNode;
	class Scene;
}

namespace MaxSDK {
	namespace RenderingAPI {
		class IRenderSessionContext;
	}
}

class BitmapTex;
class MaxRenderManager;
class MaxTextureBaker;
class Texmap;

/**
 * @brief Class to describe the properties of a sampled Max Texmap.
 */
class SampledTexmapDescriptor {
public:
	int width = 0;
	int height = 0;
	bool use_float = false;
	bool use_radial_sampling = false;
	Texmap* texmap = nullptr;

	bool operator<(const SampledTexmapDescriptor& other) const;
};

/**
 * @brief Class responsible for creating and storing ImageTextureNodes from Max Texmaps.
 */
class BakedTexmapCache {
public:
	BakedTexmapCache(const MaxSDK::RenderingAPI::IRenderSessionContext& session_context, int default_bake_width, int default_bake_height);
	~BakedTexmapCache();

	void new_frame(TimeValue t);

	ccl::ImageTextureNode* get_node_from_texmap(Texmap* texmap, ccl::Scene* scene, int width, int height, bool sample_as_float);
	ccl::ImageTextureNode* get_node_from_texmap(Texmap* texmap, ccl::Scene* scene);
	ccl::EnvironmentTextureNode* get_env_node_from_texmap(Texmap* texmap, ccl::Scene* scene);

	// Functions to handle backplate texture for compositing
	void set_backplate_texmap(Texmap* texmap);
	std::shared_ptr<BackplateBitmap> get_backplate_bitmap(Int2 resoltuion);

	// Enqueues work for all texmaps that need to be updated
	void queue_dirty_texmaps();

	// Runs jobs from baking queue on available threads, this must be called continuously until it returns true
	bool bake_texmaps_iteration();

	void set_texmap_times(std::map<Texmap*, std::chrono::steady_clock::time_point>& times_in);

	// To be removed
	void bake_all_texmaps();

private:
	const MaxSDK::RenderingAPI::IRenderSessionContext& session_context;

	const int default_bake_width;
	const int default_bake_height;
	TimeValue frame_time;

	std::map<SampledTexmapDescriptor, SampledImage> sampled_images;

	std::set<SampledTexmapDescriptor> this_frame_sampled_maps;
	std::set<Texmap*> dirty_texmaps;

	std::map<BitmapTex*, int> original_coord_mapping;

	MaxTextureBaker* texture_baker = nullptr;

	Texmap* backplate_texmap = nullptr;

	// These are used in ActiveShade renders to keep track of when a texture changed last
	// If a texture has changed since the last bake, it must be rebaked
	std::chrono::steady_clock::time_point last_bake_begin_time;
	std::map<Texmap*, std::chrono::steady_clock::time_point> texmap_update_times;

	SampledImage make_sampled_image(Texmap* texmap, bool use_arguments = false, int width_in = 512, int height_in = 512, bool sample_as_float = false);

	void prepare_texmap(Texmap* texmap, TimeValue t);

	void update_status();

	const std::unique_ptr<LoggerInterface> logger;
};
