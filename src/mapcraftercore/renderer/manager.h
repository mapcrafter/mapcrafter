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

#ifndef MANAGER_H_
#define MANAGER_H_

#include "tilerenderer.h"
#include "tileset.h"
#include "../config/mapcrafterconfig.h"
#include "../config/mapcrafterconfighelper.h"
#include "../mc/world.h"
#include "../mc/worldcache.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {

namespace util {
class IProgressHandler;
}

namespace renderer {

/**
 * This are the render options from the command line.
 */
struct RenderOpts {
	fs::path logging_config;
	bool batch;

	fs::path config;
	std::vector<std::string> render_skip, render_auto, render_force;
	bool skip_all;
	int jobs;
};

enum class RenderBehavior {
	SKIP, AUTO, FORCE
};

// TODO a better name?
class RenderBehaviorMap {
public:
	RenderBehaviorMap(RenderBehavior default_behavior = RenderBehavior::AUTO);
	~RenderBehaviorMap();

	RenderBehavior getRenderBehavior(const std::string& map, int rotation) const;
	void setRenderBehavior(const std::string& map, RenderBehavior behavior);
	void setRenderBehavior(const std::string& map, int rotation, RenderBehavior behavior);

	bool isCompleteRenderSkip(const std::string& map) const;

	static RenderBehaviorMap fromRenderOpts(const config::MapcrafterConfig& config,
			const RenderOpts& render_opts);

private:
	RenderBehavior default_behavior;

	// render behavior of each map: map -> (rotation -> render behavior)
	std::map<std::string, std::array<RenderBehavior, 4> > render_behaviors;
};

/**
 * This does the whole rendering process.
 */
class RenderManager {
public:
	RenderManager(const config::MapcrafterConfig& config);

	void setThreadCount(int thread_count);
	void setRenderBehaviors(const RenderBehaviorMap& render_behaviors);

	void initialize();
	void scanWorlds();

	void initializeMap(const std::string& map);
	void renderMap(const std::string& map, int rotation, util::IProgressHandler* progress);

	void run(bool batch);

	const std::vector<std::pair<std::string, std::set<int> > >& getRequiredMaps();

private:
	bool copyTemplateFile(const std::string& filename,
			const std::map<std::string, std::string>& vars) const;
	bool copyTemplateFile(const std::string& filename) const;

	bool writeTemplateIndexHtml() const;
	void writeTemplates() const;

	void increaseMaxZoom(const fs::path& dir, std::string image_format,
			int jpeg_quality = 85) const;

	config::MapcrafterConfig config;
	config::MapcrafterConfigHelper confighelper;

	int thread_count;
	RenderBehaviorMap render_behaviors;

	std::time_t time_started_scanning;

	// maps for world- and tile set objects
	std::map<std::string, std::array<mc::World, 4> > worlds;
	// (world, render view, rotation) -> tile set
	std::map<config::TileSetKey, std::array<std::shared_ptr<TileSet>, 4> > tile_sets;

	// all required (= not skipped) maps and rotations
	// as pair (map name, required rotations)
	std::vector<std::pair<std::string, std::set<int> > > required_maps;
};

}
}

#endif /* MANAGER_H_ */
