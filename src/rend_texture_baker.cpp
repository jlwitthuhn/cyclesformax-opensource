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
 
#include "rend_texture_baker.h"

#include <util/util_types.h>

#include <Materials/Texmap.h>

#include "max_tex_baking_scontext.h"
#include "const_classid.h"

#define DEFAULT_ROWS_PER_JOB 128

////////
// For a regular grid of dimensions x_size * y_size
// find the center of a square in that grid
// 0, 0 is the top left
static void find_grid_box_center(int x_pos, int y_pos, int x_size, int y_size, float& x, float& y)
{
	x = (static_cast<float>(x_pos) + 0.5) / x_size;
	y = (static_cast<float>(y_pos) + 0.5) / y_size;
}

static float get_scale_for_texmap(Class_ID id)
{
	if (id == PHYS_SKY_MAP_CLASS) {
		return 1.09f;
	}
	else {
		return 1.0f;
	}
}

////////
// Samples the given texmap and writes results to dest
// This uses one sample per pixel, sampled at the center of each pixel
static void simple_grid_sample(Texmap* texmap, int width, int height, int first_row, int total_rows, TimeValue t, ccl::uchar* dest)
{
	CyclesTexmapScontext sc(t, false, width, height);
	Point2 uv;

	assert(texmap != nullptr);
	const float scale = get_scale_for_texmap(texmap->ClassID());

	Interval texmap_valid = texmap->Validity(t);

	for (int y = first_row; y < height && y < first_row + total_rows; y++) {
		for (int x = 0; x < width; x++) {
			find_grid_box_center(x, y, width, height, uv.x, uv.y);
			sc.SetUV(uv, x, (width - 1 - y));
			AColor sample = texmap->EvalColor(sc);
			sample.r *= scale;
			sample.g *= scale;
			sample.b *= scale;

			int dest_index = (y * width + x) * 4;
			if (sample.r <= 1.0f) {
				dest[dest_index + 0] = sample.r * UCHAR_MAX;
			}
			else {
				dest[dest_index + 0] = UCHAR_MAX;
			}
			if (sample.g <= 1.0f) {
				dest[dest_index + 1] = sample.g * UCHAR_MAX;
			}
			else {
				dest[dest_index + 1] = UCHAR_MAX;
			}
			if (sample.b <= 1.0f) {
				dest[dest_index + 2] = sample.b * UCHAR_MAX;
			}
			else {
				dest[dest_index + 2] = UCHAR_MAX;
			}
			dest[dest_index + 3] = sample.a * UCHAR_MAX;
		}
	}
}

////////
// Samples the given texmap and writes results to dest
// This uses one sample per pixel, sampled at the center of each pixel
static void simple_grid_sample(Texmap* texmap, int width, int height, int first_row, int total_rows, TimeValue t, float* dest)
{
	CyclesTexmapScontext sc(t, false, width, height);
	Point2 uv;

	assert(texmap != nullptr);
	const float scale = get_scale_for_texmap(texmap->ClassID());

	Interval texmap_valid = texmap->Validity(t);

	for (int y = first_row; y < height && y < first_row + total_rows; y++) {
		for (int x = 0; x < width; x++) {
			find_grid_box_center(x, y, width, height, uv.x, uv.y);
			sc.SetUV(uv, x, (width - 1 - y));
			AColor sample = texmap->EvalColor(sc);
			sample.r *= scale;
			sample.g *= scale;
			sample.b *= scale;

			int dest_index = (y * width + x) * 4;
			dest[dest_index + 0] = sample.r;
			dest[dest_index + 1] = sample.g;
			dest[dest_index + 2] = sample.b;
			dest[dest_index + 3] = sample.a;
		}
	}
}

static void radial_sample_uchar(Texmap* texmap, int width, int height, int first_row, int total_rows, TimeValue t, ccl::uchar* dest)
{
	CyclesTexmapScontext sc(t, false, width, height);

	assert(texmap != nullptr);
	float scale = get_scale_for_texmap(texmap->ClassID());

	Interval texmap_valid = texmap->Validity(t);

	for (int y = first_row; y < height && y < first_row + total_rows; y++) {
		for (int x = 0; x < width; x++) {
			sc.SetUVFromRadialPixel(x, y, width, height);
			AColor sample = texmap->EvalColor(sc);
			sample.r *= scale;
			sample.g *= scale;
			sample.b *= scale;

			int dest_index = (y * width + x) * 4;
			dest[dest_index + 0] = sample.r * UCHAR_MAX;
			dest[dest_index + 1] = sample.g * UCHAR_MAX;
			dest[dest_index + 2] = sample.b * UCHAR_MAX;
			dest[dest_index + 3] = sample.a * UCHAR_MAX;
		}
	}
}

static void radial_sample_float(Texmap* texmap, int width, int height, int first_row, int total_rows, TimeValue t, float* dest)
{
	CyclesTexmapScontext sc(t, false, width, height);

	assert(texmap != nullptr);
	float scale = get_scale_for_texmap(texmap->ClassID());

	Interval texmap_valid = texmap->Validity(t);

	for (int y = first_row; y < height && y < first_row + total_rows; y++) {
		for (int x = 0; x < width; x++) {
			sc.SetUVFromRadialPixel(x, y, width, height);
			AColor sample = texmap->EvalColor(sc);
			sample.r *= scale;
			sample.g *= scale;
			sample.b *= scale;

			int dest_index = (y * width + x) * 4;
			dest[dest_index + 0] = sample.r;
			dest[dest_index + 1] = sample.g;
			dest[dest_index + 2] = sample.b;
			dest[dest_index + 3] = sample.a;
		}
	}
}

void run_baking_job(TexmapBakingJob job)
{
	Texmap* const texmap = job.texmap;

	if (job.use_radial_sampling) {
		if (job.is_float) {
			radial_sample_float(texmap, job.width, job.height, job.first_row, job.total_rows, job.frame_t, static_cast<float*>(job.rgba_ptr));
		}
		else {
			radial_sample_uchar(texmap, job.width, job.height, job.first_row, job.total_rows, job.frame_t, static_cast<ccl::uchar*>(job.rgba_ptr));
		}
	}
	else {
		if (job.is_float) {
			simple_grid_sample(texmap, job.width, job.height, job.first_row, job.total_rows, job.frame_t, static_cast<float*>(job.rgba_ptr));
		}
		else {
			simple_grid_sample(texmap, job.width, job.height, job.first_row, job.total_rows, job.frame_t, static_cast<ccl::uchar*>(job.rgba_ptr));
		}
	}

	job.baker->decrement_pending_jobs();
}

MaxTextureBaker::MaxTextureBaker() :
	logger(global_log_manager.new_logger(L"MaxTextureBaker"))
{
	*logger << LogCtl::SEPARATOR;

	service_work = new boost::asio::io_service::work(io_service);

	const int thread_count = get_processor_count();

	boost::asio::io_service* const service_ptr = &io_service;

	// Initialize all threads for the service
	thread_pool.clear();
	thread_pool.reserve(thread_count);
	for (int i = 0; i < thread_count; ++i) {
		thread_pool.push_back(std::thread([service_ptr] { service_ptr->run(); }));
	}
}

MaxTextureBaker::~MaxTextureBaker()
{
	io_service.stop();

	for (auto& this_thread : thread_pool) {
		if (this_thread.joinable()) {
			this_thread.join();
		}
	}

	delete service_work;
}

void MaxTextureBaker::queue_texmap(SampledTexmapDescriptor desc, void* rgba_ptr, TimeValue frame_t)
{
	*logger << "queue_texmap called..." << LogCtl::WRITE_LINE;
	*logger << "width: " << desc.width << LogCtl::WRITE_LINE;
	*logger << "height: " << desc.height << LogCtl::WRITE_LINE;
	*logger << "use_float: " << desc.use_float << LogCtl::WRITE_LINE;

	TexmapBakingJob this_job;

	this_job.texmap = desc.texmap;
	this_job.width = desc.width;
	this_job.height = desc.height;
	this_job.is_float = desc.use_float;
	this_job.use_radial_sampling = desc.use_radial_sampling;
	this_job.rgba_ptr = rgba_ptr;

	this_job.frame_t = frame_t;

	this_job.baker = this;

	this_job.first_row = 0;
	this_job.total_rows = DEFAULT_ROWS_PER_JOB;

	if (this_job.texmap->ClassID() == PHYS_SKY_MAP_CLASS) {
		// This sometimes crashes with multithreaded rendering, whole texmap should be one job
		this_job.total_rows = this_job.height;
		job_queue.push(this_job);
		return;
	}

	job_queue.push(this_job);

	while (this_job.first_row + this_job.total_rows < this_job.height) {
		this_job.first_row += this_job.total_rows;
		job_queue.push(this_job);
	}
}

bool MaxTextureBaker::run_queue_jobs()
{
	while (get_pending_jobs() < thread_pool.size() && job_queue.empty() == false) {
		TexmapBakingJob this_job = job_queue.front();
		job_queue.pop();
		increment_pending_jobs();
		io_service.post(std::bind(run_baking_job, this_job));
	}

	if (pending_jobs == 0 && job_queue.empty()) {
		return true;
	}

	return false;
}

int MaxTextureBaker::get_pending_jobs()
{
	return pending_jobs;
}

void MaxTextureBaker::increment_pending_jobs()
{
	++pending_jobs;
}

void MaxTextureBaker::decrement_pending_jobs()
{
	--pending_jobs;
}

int MaxTextureBaker::get_processor_count()
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	*logger << "Found processors: " << sys_info.dwNumberOfProcessors << LogCtl::WRITE_LINE;

	if (sys_info.dwNumberOfProcessors == 0) {
		return 4;
	}

	return sys_info.dwNumberOfProcessors;
}