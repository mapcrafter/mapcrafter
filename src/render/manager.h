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

#include "../config/mapcrafter_config.h"

#include "../mc/cache.h"
#include "../mc/world.h"

#include "tileset.h"
#include "render.h"

#include "../util.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace config = mapcrafter::config;

namespace mapcrafter {
namespace render {

/**
 * This are the render options from the command line.
 */
struct RenderOpts {
	std::string config_file;
	std::string render_skip, render_auto, render_force;
	bool skip_all;

	int jobs;
	bool batch;
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
	bool render_biomes;

	std::vector<bool> rotations;
	std::vector<int> last_render;

	MapSettings();

	bool read(const std::string& filename);
	bool write(const std::string& filename) const;

	bool equalsMapConfig(const config::MapSection& map) const;
	static MapSettings byMapConfig(const config::MapSection& map);
};

/**
 * This does the whole rendering process.
 */
class RenderManager {
private:
	RenderOpts opts;
	config::MapcrafterConfigFile config;
	config::MapcrafterConfigHelper confighelper;

	bool copyTemplateFile(const std::string& filename,
	        const std::map<std::string, std::string>& vars) const;
	bool copyTemplateFile(const std::string& filename) const;

	bool writeTemplateIndexHtml() const;
	void writeTemplates() const;

	void increaseMaxZoom(const fs::path& dir) const;

	void render(const config::MapSection& map_config, const std::string& output_dir,
			const mc::World& world, std::shared_ptr<TileSet> tileset,
			std::shared_ptr<BlockImages> blockimages);
	void renderMultithreaded(const config::MapSection& map_config, const std::string& output_dir,
			const mc::World& world, std::shared_ptr<TileSet> tileset,
			std::shared_ptr<BlockImages> blockimages);
public:
	RenderManager(const RenderOpts& opts);

	bool run();
};

}
}

#endif /* MANAGER_H_ */
