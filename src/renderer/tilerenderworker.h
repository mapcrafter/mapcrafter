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

#ifndef TILERENDERWORKER_H_
#define TILERENDERWORKER_H_

#include "blockimages.h"
#include "tileset.h"
#include "tilerenderer.h"
#include "../config/mapcrafter_config.h"
#include "../mc/world.h"
#include "../mc/worldcache.h"
#include "../renderer/blockimages.h"
#include "../renderer/tileset.h"
#include "../util.h"

#include <memory> // shared_ptr
#include <set>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace renderer {

struct RenderContext {
	fs::path output_dir;
	config::MapSection map_config;
	std::shared_ptr<renderer::BlockImages> blockimages;

	mc::World world;
	std::shared_ptr<renderer::TileSet> tileset;
};

struct RenderWork {
	std::set<renderer::TilePath> tiles, tiles_skip;
};

struct RenderWorkResult {
	RenderWorkResult() : tiles_rendered(0) {}

	RenderWork render_work;

	int tiles_rendered;
};

class TileRenderWorker {
public:
	TileRenderWorker();
	~TileRenderWorker();

	void setRenderContext(const RenderContext& context);
	void setRenderWork(const RenderWork& work);
	const RenderWorkResult& getRenderWorkResult() const;

	void setProgressHandler(std::shared_ptr<util::IProgressHandler> progress,
			std::shared_ptr<bool> finished = std::shared_ptr<bool>(new bool));

	void saveTile(const TilePath& tile, const Image& image);
	void renderRecursive(const TilePath& path, Image& image);

	void operator()();

private:
	RenderContext render_context;
	RenderWork render_work;
	RenderWorkResult render_work_result;

	// progress handler
	std::shared_ptr<util::IProgressHandler> progress;
	std::shared_ptr<bool> finished;

	TileRenderer renderer;
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* TILERENDERWORKER_H_ */
