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

#ifndef MANAGER_H_
#define MANAGER_H_

#include "mc/cache.h"
#include "mc/world.h"

#include "render/tile.h"

#include "util.h"

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <pthread.h>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace render {

/**
 * This are the render options from the command line.
 */
struct RenderOpts {
	fs::path input_dir;
	fs::path output_dir;
	fs::path data_dir;

	bool incremental;
	int jobs;
	bool batch;

	int texture_size;
	bool render_unknown_blocks;
	bool render_leaves_transparent;

	std::string dataPath(const std::string& path) const;
	std::string outputPath(const std::string& path) const;
};

/**
 * This informations are stored with the rendered map and used for incremental rendering.
 */
struct MapSettings {
	int texture_size;
	int tile_size;
	int max_zoom;

	bool render_unknown_blocks;
	bool render_leaves_transparent;

	int last_render;

	bool read(const std::string& filename);
	bool write(const std::string& filename) const;
};

/**
 * This are the options for a render worker.
 */
struct RenderWorkerSettings {

	RenderWorkerSettings()
			: worldcache(NULL), textures(NULL), depth(0), progress(0), finished(false) {
	}

	int thread;
	mc::WorldCache* worldcache;
	const BlockTextures* textures;

	int depth;
	fs::path output_dir;
	std::set<TilePos> render_tiles;

	int progress;
	bool finished;
};

/**
 * This renders the whole rendering process.
 */
class RenderManager {
private:
	RenderOpts opts;
	MapSettings settings;

	mc::World world;
	BlockTextures textures;

	bool copyTemplateFile(const std::string& filename,
	        std::map<std::string, std::string> vars =
	                std::map<std::string, std::string>());

	void writeTemplates(const TileSet& tiles);
	void writeStats(int time_took);
	void increaseMaxZoom();

	void render(const TileSet& tiles);
	void renderBaseTiles(const TileSet& tiles);
	void renderCompositeTiles(const TileSet& tiles);
	void renderCompositeTile(const TileSet& tiles, const Path& path, Image& tile,
	        ProgressBar& progress, int& current_progress);
	void saveTile(const Path& path, Image& tile) const;
public:
	RenderManager(const RenderOpts& opts);

	bool run();
};

}
}

#endif /* MANAGER_H_ */
