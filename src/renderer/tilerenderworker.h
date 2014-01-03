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

#ifndef TILERENDERWORKER_H_
#define TILERENDERWORKER_H_

#include "../util.h"

#include "../config/mapcrafter_config.h"

#include "../mc/cache.h"

#include "blockimages.h"
#include "tileset.h"
#include "tilerenderer.h"

#include <memory>
#include <set>

namespace mapcrafter {
namespace renderer {

class TileRenderWorker {
private:
	// cache and tileset of the map to render
	std::shared_ptr<mc::WorldCache> worldcache;
	std::shared_ptr<TileSet> tileset;
	
	// configuration of the map to render
	std::shared_ptr<BlockImages> blockimages;
	config::MapSection map_config;
	fs::path map_output_dir;
	// and a TileRenderer instance
	TileRenderer renderer;

	// information about the work to do:
	// tiles to render recursively
	// and tiles we can skip and just read from disk
	std::set<TilePath> tiles, tiles_skip;
	
	// progress handler
	std::shared_ptr<util::IProgressHandler> progress;
	std::shared_ptr<bool> finished;
public:
	TileRenderWorker();
	~TileRenderWorker();

	void setWorld(std::shared_ptr<mc::WorldCache> worldcache,
			std::shared_ptr<TileSet> tileset);
	
	void setMapConfig(std::shared_ptr<BlockImages> blockimages,
			const config::MapSection map_config,
			const fs::path& map_output_dir);

	void setWork(const std::set<TilePath>& tiles, const std::set<TilePath>& tiles_skip);

	void setProgressHandler(std::shared_ptr<util::IProgressHandler> progress,
			std::shared_ptr<bool> finished = std::shared_ptr<bool>(new bool));

	void saveTile(const TilePath& tile, const Image& image);
	void renderRecursive(const TilePath& path, Image& image);

	void operator()();
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* TILERENDERWORKER_H_ */
