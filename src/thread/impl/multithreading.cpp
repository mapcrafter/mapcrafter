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

#include <cstdlib>

namespace mapcrafter {
namespace thread {

ThreadManager::ThreadManager() {
	for (int i = 0; i < 100; i++)
		work_list.push_back({{ render::TilePos(i, 0) }});

	for (auto it = work_list.begin(); it != work_list.end(); ++it)
		work_queue.push(*it);
}

ThreadManager::~ThreadManager() {
}

bool ThreadManager::getWork(RenderWork& work) {
	std::unique_lock<std::mutex> lock(mutex);
	if (work_queue.empty())
		return false;
	work = work_queue.pop();
	//std::cout << "Start " << work.tile_pos << std::endl;
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
		condition_variable.notify_one();
	}
}

bool ThreadManager::getResult(RenderWorkResult& result) {
	std::unique_lock<std::mutex> lock(mutex);
	if (result_queue.empty() && work_list.size() == result_list.size())
		return false;
	while (result_queue.empty()) {
		condition_variable.wait(lock);
		if (result_queue.empty() && work_list.size() == result_list.size())
			return false;
	}
	result = result_queue.pop();
	return true;
}

ThreadWorker::ThreadWorker(const RenderWorkContext& context,
		WorkerManager<RenderWork, RenderWorkResult>& manager)
	: work_context(context), manager(manager) {
}

ThreadWorker::~ThreadWorker() {
}

void ThreadWorker::operator()() {
	RenderWork work;

	while (manager.getWork(work)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));

		RenderWorkResult result;
		result.tiles_rendered = work.tile_pos.getX();
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
	for (int i = 0; i < thread_count; i++)
		threads.push_back(std::thread(ThreadWorker(context, manager)));

	progress->setMax(100);
	RenderWorkResult result;
	while (manager.getResult(result)) {
		progress->setValue(progress->getValue()+1);
		//std::cout << "Finished " << result.tiles_rendered << std::endl;
	}

	for (int i = 0; i < thread_count; i++)
		threads[i].join();
}

} /* namespace thread */
} /* namespace mapcrafter */
