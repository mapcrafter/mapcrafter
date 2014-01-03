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

#ifndef MULTITHREADING_H_
#define MULTITHREADING_H_

#include "concurrentqueue.h"
#include "../dispatcher.h"
#include "../renderwork.h"
#include "../workermanager.h"
#include "../../renderer/tilerenderworker.h"

#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace mapcrafter {
namespace thread {

class ThreadManager : public WorkerManager<RenderWork, RenderWorkResult> {
public:
	ThreadManager();
	virtual ~ThreadManager();

	void addWork(const RenderWork& work);
	void addExtraWork(const RenderWork& work);
	void setFinished();

	virtual bool getWork(RenderWork& work);
	virtual void workFinished(const RenderWork& work, const RenderWorkResult& result);

	bool getResult(RenderWorkResult& result);
private:
	ConcurrentQueue<RenderWork> work_queue, work_extra_queue;
	ConcurrentQueue<RenderWorkResult> result_queue;

	bool finished;
	std::mutex mutex;
	std::condition_variable condition_wait_jobs, condition_wait_results;
};

class ThreadWorker {
public:
	ThreadWorker(WorkerManager<RenderWork, RenderWorkResult>& manager,
			const RenderWorkContext& context);
	~ThreadWorker();

	void operator()();
private:
	WorkerManager<RenderWork, RenderWorkResult>& manager;

	RenderWorkContext render_context;
	render::TileRenderWorker render_worker;
};

class MultiThreadingDispatcher : public Dispatcher {
public:
	MultiThreadingDispatcher(int threads);
	virtual ~MultiThreadingDispatcher();

	virtual void dispatch(const RenderWorkContext& context,
			std::shared_ptr<util::IProgressHandler> progress);
private:
	int thread_count;

	ThreadManager manager;
	std::vector<std::thread> threads;

	std::set<render::TilePath> rendered_tiles;
};

} /* namespace thread */
} /* namespace mapcrafter */

#endif /* MULTITHREADING_H_ */
