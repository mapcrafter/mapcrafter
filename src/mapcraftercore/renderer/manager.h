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
#include "../config/webconfig.h"
#include "../mc/world.h"
#include "../mc/worldcache.h"

#include <ctime>
#include <map>
#include <set>
#include <vector>
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

/**
 * How to render a map? Skip the map, auto-render it (incremental rendering) or
 * force-render (complete re-rendering) it.
 */
enum class RenderBehavior {
	SKIP, AUTO, FORCE
};

/**
 * Manages how to render each map of a configuration.
 */
class RenderBehaviors {
public:
	/**
	 * Constructor. You can specify a default render behavior which is used if no
	 * render behavior is specified for a map. The default default render behavior is
	 * auto.
	 */
	RenderBehaviors(RenderBehavior default_behavior = RenderBehavior::AUTO);
	~RenderBehaviors();

	/**
	 * Returns the render behavior of a specific map and rotation of it.
	 */
	RenderBehavior getRenderBehavior(const std::string& map, int rotation) const;

	/**
	 * Sets the render behavior of a whole map.
	 */
	void setRenderBehavior(const std::string& map, RenderBehavior behavior);

	/**
	 * Sets the render behavior of a single rotation of a map.
	 */
	void setRenderBehavior(const std::string& map, int rotation, RenderBehavior behavior);

	/**
	 * Checks whether a map is completely to be skipped.
	 */
	bool isCompleteRenderSkip(const std::string& map) const;

	/**
	 * Parses the render behaviors of the maps from the command line arguments.
	 */
	static RenderBehaviors fromRenderOpts(const config::MapcrafterConfig& config,
			const RenderOpts& render_opts);

private:
	// default behavior for maps if nothing specified
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

	void setRenderBehaviors(const RenderBehaviors& render_behaviors);

	// either call those three or just run(threads, batch)...
	bool initialize();
	bool scanWorlds();
	void renderMap(const std::string& map, int rotation, int threads,
			util::IProgressHandler* progress);

	bool run(int threads, bool batch);

	const std::vector<std::pair<std::string, std::set<int> > >& getRequiredMaps();

private:
	bool copyTemplateFile(const std::string& filename,
			const std::map<std::string, std::string>& vars) const;
	bool copyTemplateFile(const std::string& filename) const;

	bool writeTemplateIndexHtml() const;
	void writeTemplates() const;

	void initializeMap(const std::string& map);
	void increaseMaxZoom(const fs::path& dir, std::string image_format,
			int jpeg_quality = 85) const;

	config::MapcrafterConfig config;
	config::WebConfig web_config;

	RenderBehaviors render_behaviors;

	// time when we started scanning the worlds, used as last last render time of the maps
	std::time_t time_started_scanning;
	// set of initialized maps, initializeMap-method must be called for each map,
	// this is automatically done by the renderMap-method
	std::set<std::string> map_initialized;

	// maps for world- and tile set objects
	std::map<std::string, std::array<mc::World, 4> > worlds;
	// (world, render view, rotation) -> tile set
	std::map<config::TileSetID, std::shared_ptr<TileSet> > tile_sets;

	// all required (= not skipped) maps and rotations
	// as pair (map name, required rotations)
	std::vector<std::pair<std::string, std::set<int> > > required_maps;
};

}
}

#endif /* MANAGER_H_ */
