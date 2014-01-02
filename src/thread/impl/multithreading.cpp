/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "multithreading.h"

#include "../../mc/cache.h"
#include "../../render/tileset.h"

#include <cstdlib>

namespace mapcrafter {
namespace thread {

ThreadManager::ThreadManager()
	: finished(false) {
}

ThreadManager::~ThreadManager() {
}

void ThreadManager::setWork(const std::vector<RenderWork>& work) {
	work_list = work;
	for (auto work_it = work_list.begin(); work_it != work_list.end(); ++work_it)
		work_queue.push(*work_it);
}

void ThreadManager::addExtraWork(const RenderWork& work) {
	std::unique_lock<std::mutex> lock(mutex);
	work_list.push_back(work);
	work_extra_queue.push(work);
	condition_wait_jobs.notify_one();
}

void ThreadManager::setFinished() {
	std::unique_lock<std::mutex> lock(mutex);
	this->finished = true;
	condition_wait_jobs.notify_all();
	condition_wait_results.notify_all();
}

bool ThreadManager::getWork(RenderWork& work) {
	std::unique_lock<std::mutex> lock(mutex);
	while (!finished && (work_queue.empty() && work_extra_queue.empty()))
		condition_wait_jobs.wait(lock);
	if (finished)
		return false;
	if (!work_queue.empty())
		work = work_queue.pop();
	else if (!work_extra_queue.empty())
		work = work_extra_queue.pop();
	return true;
}

void ThreadManager::workFinished(const RenderWork& work,
		const RenderWorkResult& result) {
	std::unique_lock<std::mutex> lock(mutex);
	result_list.push_back(result);
	if (!result_queue.empty())
		result_queue.push(result);
	else {
		result_queue.push(result);
		condition_wait_results.notify_one();
	}
}

bool ThreadManager::getResult(RenderWorkResult& result) {
	std::unique_lock<std::mutex> lock(mutex);
	while (!finished && result_queue.empty())
		condition_wait_results.wait(lock);
	if (finished)
		return false;
	result = result_queue.pop();
	return true;
}

ThreadWorker::ThreadWorker(const RenderWorkContext& context,
		WorkerManager<RenderWork, RenderWorkResult>& manager)
	: render_context(context), manager(manager) {
	std::shared_ptr<mc::WorldCache> cache(new mc::WorldCache(context.world));
	render_worker.setWorld(cache, context.tileset);
	render_worker.setMapConfig(context.blockimages, context.map_config, context.output_dir);
}

ThreadWorker::~ThreadWorker() {
}

void ThreadWorker::operator()() {
	RenderWork work;

	while (manager.getWork(work)) {
		std::set<render::TilePath> tiles, tiles_skip;
		tiles.insert(work.tile_path);
		if (work.skip_childs)
			for (int i = 1; i <= 4; i++)
				tiles_skip.insert(work.tile_path + i);
		render_worker.setWork(tiles, tiles_skip);
		render_worker();

		RenderWorkResult result;
		result.tile_path = work.tile_path;
		result.tiles_rendered = render_context.tileset->getContainingRenderTiles(work.tile_path);
		if (work.skip_childs)
			result.tiles_rendered = 0;
		manager.workFinished(work, result);
	}
}

MultiThreadingDispatcher::MultiThreadingDispatcher(int threads)
	: thread_count(threads) {
}

MultiThreadingDispatcher::~MultiThreadingDispatcher() {
}

void MultiThreadingDispatcher::dispatch(const RenderWorkContext& context,
		std::shared_ptr<util::IProgressHandler> progress) {
	auto tiles = context.tileset->getRequiredCompositeTiles();
	std::vector<RenderWork> work;
	for (auto tile_it = tiles.begin(); tile_it != tiles.end(); ++tile_it)
		if (tile_it->getDepth() == context.tileset->getDepth() - 2) {
			RenderWork job;
			job.tile_path = *tile_it;
			job.skip_childs = false;
			work.push_back(job);
		}

	manager.setWork(work);
	std::cout << work.size() << " jobs" << std::endl;

	for (int i = 0; i < thread_count; i++)
		threads.push_back(std::thread(ThreadWorker(context, manager)));

	progress->setMax(context.tileset->getRequiredRenderTilesCount());
	RenderWorkResult result;
	while (manager.getResult(result)) {
		progress->setValue(progress->getValue() + result.tiles_rendered);
		rendered_tiles.insert(result.tile_path);
		if (result.tile_path == render::TilePath()) {
			manager.setFinished();
			continue;
		}

		render::TilePath parent = result.tile_path.parent();
		bool childs_rendered = true;
		for (int i = 1; i <= 4; i++)
			if (context.tileset->isTileRequired(parent + i) && !rendered_tiles.count(parent + i)) {
				childs_rendered = false;
			}

		if (childs_rendered) {
			RenderWork job;
			job.tile_path = parent;
			job.skip_childs = true;
			manager.addExtraWork(job);
		}
	}

	for (int i = 0; i < thread_count; i++)
		threads[i].join();
}

} /* namespace thread */
} /* namespace mapcrafter */
