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
 * @brief Defines class OfflineTranslationManager.
 */

#include <atomic>
#include <map>
#include <memory>
#include <vector>

#include <maxtypes.h>

#include "extern_tyflow.h"

#include "rend_logger.h"
#include "rend_shader_manager.h"

class BakedTexmapCache;
class CyclesRenderParams;
class GeomObject;
class Mesh;
class Mtl;
class INode;
class IParticleObjectExt;
class Object;

namespace ccl {
	class Device;
	class DeviceInfo;
	class Mesh;
	class Scene;
}

namespace MaxSDK {
	namespace RenderingAPI {
		class IRenderSessionContext;
	}
}

/**
 * @brief Class responsible for extracting objects from a Max scene graph and putting them in a Cycles scene.
 */
class OfflineTranslationManager {
public:
	ccl::Scene* scene = nullptr;

	OfflineTranslationManager(
		MaxSDK::RenderingAPI::IRenderSessionContext& session_context,
		BakedTexmapCache& texmap_cache,
		CyclesRenderParams& rend_params
	);

	// Create the scene object to be populated
	void init(ccl::Device* device);

	// Copies elements from the max scene graph to the cycles scene, should be called once after camera has been set up
	void copy_scene(const std::vector<int>& mblur_sample_ticks);

	void end_render();

private:
	MaxSDK::RenderingAPI::IRenderSessionContext& session_context;
	CyclesRenderParams& rend_params;
	const TimeValue frame_t;

	std::unique_ptr<MaxShaderManager> shader_manager;
	BakedTexmapCache& texmap_cache;

	std::atomic<bool> stop_requested = false;

	void add_node_to_scene(INode* node, const std::vector<int>& mblur_sample_ticks);

	void add_mtl_preview_lights_new();
	void add_mtl_preview_lights();

	void process_geom_object(INode* node, GeomObject* obj, const std::vector<int>& mblur_sample_ticks);
	void process_light_object(INode* node, Object* obj);
	void process_particle_system(INode* node, IParticleObjectExt* particle_ext, const std::vector<int>& mblur_sample_ticks);
	void process_particle_system_ty(INode* node, tyParticleInterface* ty_ext, const std::vector<int>& mblur_sample_ticks);


	void refresh_ui();

	bool should_stop();

	class MeshDescriptor {
	public:
		MeshDescriptor(GeomObject* geom_object, Mtl* mtl) : geom_object(geom_object), mtl(mtl) {}

		bool operator<(const MeshDescriptor& other) const;

	private:
		GeomObject* geom_object;
		Mtl* mtl;
	};

	class RawMeshDescriptor {
	public:
		RawMeshDescriptor(Mesh* mesh, Mtl* mtl, int shader_index_override) : mesh{ mesh }, mtl{ mtl }, shader_index_override{ shader_index_override } {}

		bool operator<(const RawMeshDescriptor& other) const;

	private:
		Mesh* mesh;
		Mtl* mtl;
		int shader_index_override;
	};

	std::map<MeshDescriptor, ccl::Mesh*> mesh_cache;
	std::map<RawMeshDescriptor, ccl::Mesh*> raw_mesh_cache;

	const std::unique_ptr<LoggerInterface> logger;
};
