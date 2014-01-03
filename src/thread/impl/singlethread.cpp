/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "singlethread.h"

#include "../../mc/worldcache.h"

#include <set>

namespace mapcrafter {
namespace thread {

SingleThreadDispatcher::SingleThreadDispatcher() {
}

SingleThreadDispatcher::~SingleThreadDispatcher() {
}

void SingleThreadDispatcher::dispatch(const renderer::RenderWorkContext& context,
		std::shared_ptr<util::IProgressHandler> progress) {
	int render_tiles = context.tileset->getRequiredRenderTilesCount();
	std::cout << "Single thread will render " << render_tiles;
	std::cout << " render tiles." << std::endl;

	renderer::TileRenderWorker worker;

	std::shared_ptr<mc::WorldCache> cache(new mc::WorldCache(context.world));
	worker.setMapConfig(context.blockimages, context.map_config, context.output_dir);
	worker.setWorld(cache, context.tileset);

	std::set<renderer::TilePath> tiles, tiles_skip;
	tiles.insert(renderer::TilePath());
	worker.setWork(tiles, tiles_skip);

	worker.setProgressHandler(progress);
	worker();
}

} /* namespace thread */
} /* namespace mapcrafter */
