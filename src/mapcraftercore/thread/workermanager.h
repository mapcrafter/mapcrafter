/*
 * Copyright 2012-2016 Moritz Hilscher
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

#ifndef WORKER_MANAGER_H_
#define WORKER_MANAGER_H_

namespace mapcrafter {
namespace thread {

/**
 * This is an interface for a class managing the work of render workers.
 */
template <typename Work, typename WorkResult>
class WorkerManager {
public:
	virtual ~WorkerManager() {};

	virtual bool getWork(Work& work) = 0;
	virtual void workFinished(const Work& work, const WorkResult& result) = 0;
};

} /* namespace thread */
} /* namespace mapcrafter */

#endif /* WORKER_MANAGER_H_ */
