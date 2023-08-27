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
 

#define WIN32_LEAN_AND_MEAN

#include "cache_baked_texmap.h"

#include <queue>

#include <boost/optional.hpp>

#include <render/nodes.h>
#include <render/scene.h>

#include <bitmap.h>
#include <Noncopyable.h>
#include <RenderingAPI/Renderer/IRenderingProcess.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>
#include <RenderingAPI/Renderer/IRenderSettingsContainer.h>
#include <stdmat.h>

#include "plugin_tex_bitmap_filter.h"
#include "plugin_tex_environment.h"
#include "rend_logger_ext.h"
#include "rend_texture_baker.h"
#include "const_classid.h"

#define PHYS_SKY_DEFAULT_WIDTH 3200;
#define PHYS_SKY_DEFAULT_HEIGHT 1600;

class BitmapSampling {
public:
	int width = 512;
	int height = 512;
	bool use_float = false;
};

static boost::optional<BitmapSampling> get_sampling_for_texmap(Texmap* const texmap_in, const TimeValue frame_t)
{
	assert(texmap_in != nullptr);

	int max_bitmap_width = 0;
	int max_bitmap_height = 0;

	std::queue<Texmap*> texmaps;
	texmaps.push(texmap_in);

	BitmapSampling result;

	while (!texmaps.empty()) {
		Texmap* const texmap = texmaps.front();
		texmaps.pop();

		if (texmap->ClassID() == CYCLES_TEXMAP_FILTER_CLASS) {
			BitmapFilterTexmap* bitmap_texmap = dynamic_cast<BitmapFilterTexmap*>(texmap);
			assert(bitmap_texmap != nullptr);
			result.width = bitmap_texmap->GetParamWidth(frame_t);
			result.height = bitmap_texmap->GetParamHeight(frame_t);
			result.use_float = bitmap_texmap->UseFloatPrecision(frame_t);
			return result;
		}
		else if (texmap->ClassID() == BITMAP_MAP_CLASS) {
			BitmapTex* bitmap_tex = dynamic_cast<BitmapTex*>(texmap);
			assert(bitmap_tex != nullptr);
			Bitmap* bitmap = bitmap_tex->GetBitmap(frame_t);
			if (bitmap == nullptr) {
				continue;
			}

			if (bitmap->Width() > max_bitmap_width) {
				max_bitmap_width = bitmap->Width();
			}
			if (bitmap->Height() > max_bitmap_height) {
				max_bitmap_height = bitmap->Height();
			}

			if (bitmap_tex->IsHighDynamicRange()) {
				result.use_float = true;
			}
		}

		for (int i = 0; i < texmap->NumSubTexmaps(); ++i) {
			Texmap* sub_texmap = texmap->GetSubTexmap(i);
			if (sub_texmap != nullptr) {
				texmaps.push(sub_texmap);
			}
		}
	}


	// Special case, for env maps use dimensions from PHYS_SKY_DEFAULT_ macros
	if (texmap_in->ClassID() == PHYS_SKY_MAP_CLASS) {
		result.use_float = true;
		result.width = PHYS_SKY_DEFAULT_WIDTH;
		result.height = PHYS_SKY_DEFAULT_HEIGHT;
		return result;
	}

	if (max_bitmap_width > 0 && max_bitmap_height > 0) {
		result.width = max_bitmap_width;
		result.height = max_bitmap_height;
		return result;
	}

	return boost::none;
}

bool SampledTexmapDescriptor::operator<(const SampledTexmapDescriptor& other) const
{
	if (width < other.width) return true;
	else if (other.width < width) return false;

	if (height < other.height) return true;
	else if (other.height < height) return false;

	if (use_float < other.use_float) return true;
	else if (other.use_float < use_float) return false;

	if (texmap < other.texmap) return true;
	else if (other.texmap < texmap) return false;

	return false;
}

BakedTexmapCache::BakedTexmapCache(const MaxSDK::RenderingAPI::IRenderSessionContext& session_context, const int default_bake_width, const int default_bake_height) :
	session_context{ session_context },
	default_bake_width{ default_bake_width },
	default_bake_height{ default_bake_height },
	logger{ global_log_manager.new_logger(L"BakedTexmapCache") }
{
	*logger << LogCtl::SEPARATOR;

	static_assert(sizeof(unsigned char) == sizeof(ccl::uchar), "TexmapHeapData requires ccl::uchar to be a typedef for unsigned char");

	frame_time = 0;
}

BakedTexmapCache::~BakedTexmapCache()
{
	*logger << "original_coord_mapping size: " << original_coord_mapping.size() << LogCtl::WRITE_LINE;
	for (std::pair<BitmapTex*, int> this_pair : original_coord_mapping) {
		*logger << "original mapping found: " << this_pair.second << LogCtl::WRITE_LINE;
		this_pair.first->GetUVGen()->SetCoordMapping(this_pair.second);
	}
	original_coord_mapping.clear();
}

void BakedTexmapCache::new_frame(TimeValue t)
{
	frame_time = t;
}

ccl::ImageTextureNode* BakedTexmapCache::get_node_from_texmap(Texmap* const texmap, ccl::Scene* const scene, const int width, const int height, const bool sample_as_float)
{
	*logger << "get_node_from_texmap called..." << LogCtl::WRITE_LINE;
	const Class_ID class_id = texmap->ClassID();
	*logger << "class id: " << class_id << LogCtl::WRITE_LINE;

	ccl::ImageTextureNode* const result = new ccl::ImageTextureNode();

	const SampledImage sampled_image = make_sampled_image(texmap, true, width, height, sample_as_float);
	result->set_filename(ccl::ustring{ sampled_image.filename });
	result->handle = scene->image_manager->add_image(new CyclesPluginImageLoader(sampled_image), result->image_params());

	result->set_colorspace(ccl::u_colorspace_raw);

	return result;
}

ccl::ImageTextureNode* BakedTexmapCache::get_node_from_texmap(Texmap* const texmap, ccl::Scene* const scene)
{
	*logger << "get_node_from_texmap called..." << LogCtl::WRITE_LINE;
	const Class_ID class_id = texmap->ClassID();
	*logger << "class id: " << class_id << LogCtl::WRITE_LINE;

	ccl::ImageTextureNode* const result = new ccl::ImageTextureNode();

	const SampledImage sampled_image = make_sampled_image(texmap);
	result->set_filename(ccl::ustring{ sampled_image.filename });
	result->handle = scene->image_manager->add_image(new CyclesPluginImageLoader(sampled_image), result->image_params());

	result->set_colorspace(ccl::u_colorspace_raw);

	return result;
}

ccl::EnvironmentTextureNode* BakedTexmapCache::get_env_node_from_texmap(Texmap* const texmap, ccl::Scene* const scene)
{
	*logger << "get_env_node_from_texmap called..." << LogCtl::WRITE_LINE;

	// Special case to handle environment image textures
	// For these, we want to get a texmap from the environment map's input, rather than the environment map itself
	// This is because the input can be a filter containing information about baking the texmap
	if (texmap->ClassID() == CYCLES_TEXMAP_ENVIRONMENT_CLASS) {
		EnvironmentTexmap* env_map = dynamic_cast<EnvironmentTexmap*>(texmap);
		if (env_map != nullptr && env_map->GetBGMap() != nullptr) {
			return get_env_node_from_texmap(env_map->GetBGMap(), scene);
		}
	}

	ccl::EnvironmentTextureNode* const result = new ccl::EnvironmentTextureNode();

	const SampledImage sampled_image = make_sampled_image(texmap);
	result->set_filename(ccl::ustring{ sampled_image.filename });
	result->handle = scene->image_manager->add_image(new CyclesPluginImageLoader(sampled_image), result->image_params());

	result->set_colorspace(ccl::u_colorspace_raw);

	return result;
}

void BakedTexmapCache::set_backplate_texmap(Texmap* const texmap)
{
	backplate_texmap = texmap;
}

std::shared_ptr<BackplateBitmap> BakedTexmapCache::get_backplate_bitmap(Int2 resolution)
{
	*logger << "get_backplate_bitmap called..." << LogCtl::WRITE_LINE;
	*logger << "resolution: " << resolution << LogCtl::WRITE_LINE;
	if (backplate_texmap == nullptr) {
		*logger << "no backplate set, returning early" << LogCtl::WRITE_LINE;
		return std::shared_ptr<BackplateBitmap>(nullptr);
	}

	auto result = std::make_shared<BackplateBitmap>(resolution);

	BitmapInfo bi;
	bi.SetType(BMM_TRUE_32);
	bi.SetWidth(resolution.x());
	bi.SetHeight(resolution.y());

	Bitmap* this_bitmap = TheManager->Create(&bi);

	if (this_bitmap == nullptr) {
		*logger << "failed to create bitmap, returning early" << LogCtl::WRITE_LINE;
		return std::shared_ptr<BackplateBitmap>(nullptr);
	}

	backplate_texmap->RenderBitmap(frame_time, this_bitmap, 1.0f, TRUE);

	BMM_Color_fl* const temp_row = new BMM_Color_fl[resolution.x()];
	for (int y = 0; y < resolution.y(); y++) {
		const int max_y = (resolution.y() - 1) - y;
		this_bitmap->GetLinearPixels(0, max_y, resolution.x(), temp_row);
		float* const dest_begin = result->pixels + static_cast<size_t>(y) * resolution.x() * 3;
		for (int x = 0; x < resolution.x(); x++) {
			dest_begin[x * 3 + 0] = temp_row[x].r;
			dest_begin[x * 3 + 1] = temp_row[x].g;
			dest_begin[x * 3 + 2] = temp_row[x].b;
		}
	}

	this_bitmap->DeleteThis();

	*logger << "get_backplate_bitmap complete" << LogCtl::WRITE_LINE;
	return result;
}

void BakedTexmapCache::queue_dirty_texmaps()
{
	*logger << "queue_dirty_texmaps called..." << LogCtl::WRITE_LINE;
	*logger << "dirty texmap count: " << dirty_texmaps.size() << LogCtl::WRITE_LINE;

	// Form a set of all descriptors that are not used this frame and release their resources
	std::set<SampledTexmapDescriptor> to_remove;
	for (std::pair<SampledTexmapDescriptor, SampledImage> this_pair : sampled_images) {
		if (this_frame_sampled_maps.count(this_pair.first) == 0) {
			to_remove.insert(this_pair.first);
		}
	}

	// Remove above-identified descriptors from texmap_heap_data
	for (SampledTexmapDescriptor desc : to_remove) {
		sampled_images.erase(desc);
	}

	for (std::pair<SampledTexmapDescriptor, SampledImage> this_pair : sampled_images) {
		if (dirty_texmaps.count(this_pair.first.texmap) == 1) {
			prepare_texmap(this_pair.first.texmap, frame_time);

			if (texture_baker == nullptr) {
				texture_baker = new MaxTextureBaker();
			}

			// TODO: Clean this up to not require a void cast
			void* bitmap_ptr = nullptr;
			if (this_pair.second.float_pixels.use_count() > 0) {
				bitmap_ptr = static_cast<void*>(this_pair.second.float_pixels.get());
			}
			if (this_pair.second.char_pixels.use_count() > 0) {
				bitmap_ptr = static_cast<void*>(this_pair.second.char_pixels.get());
			}

			texture_baker->queue_texmap(this_pair.first, bitmap_ptr, frame_time);

			*logger << "texmap queued" << LogCtl::WRITE_LINE;
		}
	}

	dirty_texmaps.clear();
	this_frame_sampled_maps.clear();

	last_bake_begin_time = std::chrono::steady_clock::now();
}

bool BakedTexmapCache::bake_texmaps_iteration()
{
	if (texture_baker == nullptr) {
		return true;
	}

	bool done = texture_baker->run_queue_jobs();

	if (done) {
		delete texture_baker;
		texture_baker = nullptr;
	}

	return done;
}

void BakedTexmapCache::set_texmap_times(std::map<Texmap*, std::chrono::steady_clock::time_point>& times_in)
{
	texmap_update_times = times_in;
}

void BakedTexmapCache::bake_all_texmaps()
{
	*logger << "bake_all_texmaps called..." << LogCtl::WRITE_LINE;

	if (dirty_texmaps.size() == 0) {
		return;
	}
	else {
		*logger << "found "  << dirty_texmaps.size() << " dirty texmaps to bake" << LogCtl::WRITE_LINE;
	}

	*logger << "Begin baking" << LogCtl::WRITE_LINE;

	session_context.GetRenderingProcess().SetRenderingProgressTitle(L"Baking textures...");

	queue_dirty_texmaps();

	*logger << "waiting to complete..." << LogCtl::WRITE_LINE;

	while (bake_texmaps_iteration() == false) {
		update_status();
		std::this_thread::yield();
	}

	*logger << "End baking, all threads done" << LogCtl::WRITE_LINE;
}

static std::string generate_texture_filename()
{
	static const char chars[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	static const int charset_size = sizeof(chars) - 1;
	static const size_t  suffix_size = 10;

	std::string beginning("cycles_tex_");

	char suffix[suffix_size];

	for (size_t i = 0; i < suffix_size; ++i) {
		suffix[i] = chars[rand() % charset_size];
	}

	return beginning + suffix;
}

SampledImage BakedTexmapCache::make_sampled_image(Texmap* texmap, bool use_arguments, int width_in, int height_in, bool sample_as_float)
{
	*logger << "get_callback_data called..." << LogCtl::WRITE_LINE;

	BitmapSampling sampling;
	if (use_arguments) {
		sampling.width = width_in;
		sampling.height = height_in;
		sampling.use_float = sample_as_float;
	}
	else {
		const boost::optional<BitmapSampling> opt_sampling{ get_sampling_for_texmap(texmap, frame_time) };
		if (opt_sampling) {
			sampling = *opt_sampling;
		}
		else {
			*logger << "Unable to find dimensions, using default" << LogCtl::WRITE_LINE;
			sampling.width = default_bake_width;
			sampling.height = default_bake_height;
		}
	}

	*logger << "Sampling resolution: " << sampling.width << ' ' << sampling.height << LogCtl::WRITE_LINE;
	*logger << "use_float: " << sampling.use_float << LogCtl::WRITE_LINE;

	// Special case for mtl edit renders, cap to 512x512
	if (session_context.GetRenderSettings().GetIsMEditRender()) {
		if (sampling.width > 512) {
			sampling.width = 512;
		}
		if (sampling.height > 512) {
			sampling.height = 512;
		}
	}

	*logger << "Post-clamp resolution: " << sampling.width << ' ' << sampling.height << LogCtl::WRITE_LINE;

	// If there is a texmap filter, get the input texmap for sampling instead of the filter
	Texmap* render_texmap = texmap;
	if (texmap->ClassID() == CYCLES_TEXMAP_FILTER_CLASS) {
		Texmap* maybe_texmap = dynamic_cast<BitmapFilterTexmap*>(texmap)->GetSubTexmap(0);
		if (maybe_texmap != nullptr) {
			render_texmap = maybe_texmap;
		}
	}

	SampledTexmapDescriptor desc;
	desc.width = sampling.width;
	desc.height = sampling.height;
	desc.use_float = sampling.use_float;
	desc.use_radial_sampling = false;
	desc.texmap = render_texmap;

	if (render_texmap->ClassID() == PHYS_SKY_MAP_CLASS) {
		desc.use_radial_sampling = true;
	}

	if (sampled_images.count(desc) == 1) {
		this_frame_sampled_maps.insert(desc);
		if (texmap_update_times[desc.texmap] >= last_bake_begin_time) {
			dirty_texmaps.insert(desc.texmap);
		}
		return sampled_images[desc];
	} 

	SampledImage image;

	image.width = desc.width;
	image.height = desc.height;
	image.filename = generate_texture_filename();

	const size_t TEXTURE_CHANNELS = 4;
	if (desc.use_float) {
		image.float_pixels = std::shared_ptr<float>(new float[TEXTURE_CHANNELS * desc.width * desc.height], std::default_delete<float[]>{});
	}
	else {
		image.char_pixels = std::shared_ptr<unsigned char>(new unsigned char[TEXTURE_CHANNELS * desc.width * desc.height], std::default_delete<unsigned char[]>{});
	}

	sampled_images[desc] = image;

	dirty_texmaps.insert(desc.texmap);
	this_frame_sampled_maps.insert(desc);

	return image;
}

////////
// Hacky fix for a new issue in Max 2017
// If this is not called on a texmap prior to EvalColor, EvalColor may only return black
void BakedTexmapCache::prepare_texmap(Texmap* const texmap, const TimeValue t)
{
	if (texmap->ClassID() == BITMAP_MAP_CLASS) {
		BitmapTex* const bitmap_tex = dynamic_cast<BitmapTex*>(texmap);
		if (bitmap_tex != nullptr) {
			Interval uv_gen_valid = FOREVER;
			StdUVGen* uv_gen = bitmap_tex->GetUVGen();
			uv_gen->Update(t, uv_gen_valid);
			int current_coord_mapping = uv_gen->GetCoordMapping(0);

			*logger << "texture started with mapping: " << current_coord_mapping << LogCtl::WRITE_LINE;

			if (current_coord_mapping != UVMAP_EXPLICIT && current_coord_mapping != UVMAP_SCREEN_ENV) {
				if (original_coord_mapping.count(bitmap_tex) == 0) {
					original_coord_mapping[bitmap_tex] = current_coord_mapping;
				}
				uv_gen->SetCoordMapping(UVMAP_EXPLICIT);

				Interval update_valid = FOREVER;
				bitmap_tex->Update(t, update_valid);
			}
		}
	}

	const int RENDER_BM_WIDTH = 64;
	const int RENDER_BM_HEIGHT = 64;

	BitmapInfo bi;
	bi.SetType(BMM_TRUE_32);
	bi.SetWidth(RENDER_BM_WIDTH);
	bi.SetHeight(RENDER_BM_HEIGHT);
	bi.SetFlags(MAP_HAS_ALPHA);
	bi.SetCustomFlag(0);

	Bitmap* const bitmap = TheManager->Create(&bi);

	if (bitmap == nullptr) {
		return;
	}

	texmap->RenderBitmap(t, bitmap, FALSE);

	bitmap->DeleteThis();
}

void BakedTexmapCache::update_status()
{
	session_context.GetRenderingProcess().SetRenderingProgress(0, 10, MaxSDK::RenderingAPI::IRenderingProcess::ProgressType::Translation);
}
