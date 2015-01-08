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

#include "singlethread.h"

#include "../../mc/worldcache.h"
#include "../../util.h"

#include <set>

namespace mapcrafter {
namespace thread {

SingleThreadDispatcher::SingleThreadDispatcher() {
}

SingleThreadDispatcher::~SingleThreadDispatcher() {
}

void SingleThreadDispatcher::dispatch(const renderer::RenderContext& context,
		util::IProgressHandler* progress) {
	int render_tiles = context.tile_set->getRequiredRenderTilesCount();
	if (render_tiles == 0)
		return;

	LOG(INFO) << "Single thread will render " << render_tiles << " render tiles.";

	renderer::RenderWork work;
	work.tiles.insert(renderer::TilePath());

	renderer::TileRenderWorker worker;
	worker.setRenderContext(context);
	worker.setRenderWork(work);
	worker.setProgressHandler(progress);
	worker();
}

} /* namespace thread */
} /* namespace mapcrafter */
