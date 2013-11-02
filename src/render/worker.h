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

#ifndef WORKER_H_
#define WORKER_H_

#include "../util.h"

#include "../config/mapcrafter_config.h"

#include "../mc/cache.h"

#include "blockimages.h"
#include "tileset.h"
#include "render.h"

#include <memory>
#include <set>

namespace mapcrafter {
namespace render {

class RenderWorker {
private:
	std::shared_ptr<mc::WorldCache> world;
	std::shared_ptr<TileSet> tileset;
	std::shared_ptr<BlockImages> blockimages;
	TileRenderer renderer;

	fs::path output_dir;
	config2::MapSection map_config;
	std::set<TilePath> tiles, tiles_skip;
public:
	RenderWorker();
	~RenderWorker();

	void setWorld(std::shared_ptr<mc::WorldCache> world,
			std::shared_ptr<TileSet> tileset,
			std::shared_ptr<BlockImages> blockimages);

	void setWork(const fs::path& output_dir,
			const config2::MapSection& map_config,
			const std::set<TilePath>& tiles, const std::set<TilePath>& tiles_skip);

	void saveTile(const TilePath& tile, const Image& image);
	void renderRecursive(const TilePath& path, Image& image);

	void operator()();
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* WORKER_H_ */
