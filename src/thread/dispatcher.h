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

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <memory> // shared_ptr

#include "../util.h"

#include "render_work.h"

namespace mapcrafter {
namespace thread {

/**
 * This is an interface for a class responsible for managing and distributing the render
 * work.
 */
class Dispatcher {
public:
	virtual ~Dispatcher() {};

	virtual void dispatch(const RenderWorkContext& context,
			std::shared_ptr<util::IProgressHandler> progress) = 0;
};

} /* namespace thread */
} /* namespace mapcrafter */

#endif /* DISPATCHER_H_ */
