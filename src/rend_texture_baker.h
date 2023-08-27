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
 * @brief Defines classes used to bake Max Texmaps into plain bitmaps.
 */

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <boost/asio/io_service.hpp>

#include <maxtypes.h>

#include "cache_baked_texmap.h"
#include "rend_logger.h"

class MaxTextureBaker;
class Texmap;

/**
 * @brief Class to hold all information needed for a single baking job.
 */
class TexmapBakingJob {
public:
	Texmap* texmap;
	int width;
	int height;
	bool is_float;
	bool use_radial_sampling;
	void* rgba_ptr;

	TimeValue frame_t;

	MaxTextureBaker* baker;

	int first_row;
	int total_rows;
};

/**
 * @brief Class responsible for managing and running texmap baking jobs.
 */
class MaxTextureBaker {
public:
	MaxTextureBaker();
	~MaxTextureBaker();

	void queue_texmap(SampledTexmapDescriptor desc, void* rgba_ptr, TimeValue frame_t);
	bool run_queue_jobs();

	int get_pending_jobs();
	void increment_pending_jobs();
	void decrement_pending_jobs();

private:
	int get_processor_count();

	std::queue<TexmapBakingJob> job_queue;

	std::atomic<int> pending_jobs = 0;

	boost::asio::io_service io_service;
	std::vector<std::thread> thread_pool;

	boost::asio::io_service::work* service_work = nullptr;

	const std::unique_ptr<LoggerInterface> logger;
};
