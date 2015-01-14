/*
 * Copyright 2012-2015 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "multithreading.h"

#include "../../mc/worldcache.h"
#include "../../renderer/tileset.h"

#include <cstdlib>

namespace mapcrafter {
namespace thread {

ThreadManager::ThreadManager()
	: finished(false) {
}

ThreadManager::~ThreadManager() {
}

void ThreadManager::addWork(const renderer::RenderWork& work) {
	thread_ns::unique_lock<thread_ns::mutex> lock(mutex);
	work_queue.push(work);
}

void ThreadManager::addExtraWork(const renderer::RenderWork& work) {
	thread_ns::unique_lock<thread_ns::mutex> lock(mutex);
	work_extra_queue.push(work);
	condition_wait_jobs.notify_one();
}

void ThreadManager::setFinished() {
	thread_ns::unique_lock<thread_ns::mutex> lock(mutex);
	this->finished = true;
	condition_wait_jobs.notify_all();
	condition_wait_results.notify_all();
}

bool ThreadManager::getWork(renderer::RenderWork& work) {
	thread_ns::unique_lock<thread_ns::mutex> lock(mutex);
	while (!finished && (work_queue.empty() && work_extra_queue.empty()))
		condition_wait_jobs.wait(lock);
	if (finished)
		return false;
	if (!work_extra_queue.empty())
		work = work_extra_queue.pop();
	else if (!work_queue.empty())
		work = work_queue.pop();
	return true;
}

void ThreadManager::workFinished(const renderer::RenderWork& work,
		const renderer::RenderWorkResult& result) {
	thread_ns::unique_lock<thread_ns::mutex> lock(mutex);
	if (!result_queue.empty())
		result_queue.push(result);
	else {
		result_queue.push(result);
		condition_wait_results.notify_one();
	}
}

bool ThreadManager::getResult(renderer::RenderWorkResult& result) {
	thread_ns::unique_lock<thread_ns::mutex> lock(mutex);
	while (!finished && result_queue.empty())
		condition_wait_results.wait(lock);
	if (finished)
		return false;
	result = result_queue.pop();
	return true;
}

ThreadWorker::ThreadWorker(WorkerManager<renderer::RenderWork, renderer::RenderWorkResult>& manager,
		const renderer::RenderContext& context)
	: manager(manager), render_context(context) {
	render_worker.setRenderContext(context);
}

ThreadWorker::~ThreadWorker() {
}

void ThreadWorker::operator()() {
	renderer::RenderWork work;

	while (manager.getWork(work)) {
		render_worker.setRenderWork(work);
		render_worker();

		manager.workFinished(work, render_worker.getRenderWorkResult());
	}
}

MultiThreadingDispatcher::MultiThreadingDispatcher(int threads)
	: thread_count(threads) {
}

MultiThreadingDispatcher::~MultiThreadingDispatcher() {
}

void MultiThreadingDispatcher::dispatch(const renderer::RenderContext& context,
		util::IProgressHandler* progress) {
	auto tiles = context.tile_set->getRequiredCompositeTiles();
	if (tiles.size() == 0)
		return;

	int jobs = 0;
	for (auto tile_it = tiles.begin(); tile_it != tiles.end(); ++tile_it)
		if (tile_it->getDepth() == context.tile_set->getDepth() - 2) {
			renderer::RenderWork work;
			work.tiles.insert(*tile_it);
			manager.addWork(work);
			jobs++;
		}

	int render_tiles = context.tile_set->getRequiredRenderTilesCount();
	LOG(INFO) << thread_count << " threads will render " << render_tiles << " render tiles.";

	for (int i = 0; i < thread_count; i++) {
		renderer::RenderContext thread_context = context;
		thread_context.initializeTileRenderer();
		threads.push_back(thread_ns::thread(ThreadWorker(manager, thread_context)));
	}

	progress->setMax(context.tile_set->getRequiredRenderTilesCount());
	renderer::RenderWorkResult result;
	while (manager.getResult(result)) {
		progress->setValue(progress->getValue() + result.tiles_rendered);
		for (auto tile_it = result.render_work.tiles.begin();
				tile_it != result.render_work.tiles.end(); ++tile_it) {
			rendered_tiles.insert(*tile_it);
			if (*tile_it == renderer::TilePath()) {
				manager.setFinished();
				continue;
			}

			renderer::TilePath parent = tile_it->parent();
			bool childs_rendered = true;
			for (int i = 1; i <= 4; i++)
				if (context.tile_set->isTileRequired(parent + i)
						&& !rendered_tiles.count(parent + i)) {
					childs_rendered = false;
				}

			if (childs_rendered) {
				renderer::RenderWork work;
				work.tiles.insert(parent);
				for (int i = 1; i <= 4; i++)
					if (context.tile_set->hasTile(parent + i))
						work.tiles_skip.insert(parent + i);
				manager.addExtraWork(work);
			}
		}
	}

	for (int i = 0; i < thread_count; i++)
		threads[i].join();
}

} /* namespace thread */
} /* namespace mapcrafter */
