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

#ifndef MAPCRAFTERCONFIGHELPER_H_
#define MAPCRAFTERCONFIGHELPER_H_

#include "mapcrafterconfig.h"
#include "../renderer/tileset.h"
#include "../util/picojson.h"

#include <array>
#include <map>
#include <set>
#include <tuple>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

struct TileSetKey {
	std::string map_name;
	std::string render_view;
	int tile_width;
	int rotation;

	TileSetKey(const std::string& map_name, const std::string render_view,
			int tile_width, int rotation);

	bool operator<(const TileSetKey& other) const;
};

class MapcrafterConfigHelper {
public:
	MapcrafterConfigHelper();
	MapcrafterConfigHelper(const MapcrafterConfig& config);
	~MapcrafterConfigHelper();

	void readMapSettings();
	void writeMapSettings() const;

	std::set<int> getUsedRotations(const std::string& world,
			const std::string& render_view) const;
	void addUsedRotations(const std::string& world,
			const std::string& render_view, const std::set<int>& rotations);

	int getTileSetMaxZoom(const std::string& world,
			const std::string& render_view) const;
	void setTileSetMaxZoom(const std::string& world,
			const std::string& render_view, int zoomlevel);

	renderer::TilePos getWorldTileOffset(const std::string& world,
			const std::string& render_view, int rotation) const;
	void setWorldTileOffset(const std::string& world,
			const std::string& render_view, int rotation,
			const renderer::TilePos& tile_offset);

	int getMapTileSize(const std::string& map) const;
	void setMapTileSize(const std::string& map, int tile_size);

	int getMapMaxZoom(const std::string& map) const;
	void setMapMaxZoom(const std::string& map, int max_zoom);

	int getMapLastRendered(const std::string& map, int rotation) const;
	void setMapLastRendered(const std::string& map, int rotation, int last_rendered);

	int getRenderBehavior(const std::string& map, int rotation) const;
	void setRenderBehavior(const std::string& map, int rotation, int behavior);

	bool isCompleteRenderSkip(const std::string& map) const;
	bool isCompleteRenderForce(const std::string& map) const;

	void parseRenderBehaviors(bool skip_all,
			std::vector<std::string>,
			std::vector<std::string> render_auto,
			std::vector<std::string> render_force);

	static const int RENDER_SKIP = 0;
	static const int RENDER_AUTO = 1;
	static const int RENDER_FORCE = 2;

private:
	MapcrafterConfig config;

	typedef std::tuple<std::string, std::string> WorldRenderView;

	// tile offset of world/view/tile_width/rotation
	std::map<WorldRenderView, std::array<renderer::TilePos, 4> > world_tile_offset;
	// used rotations of world/view/tile_width
	std::map<WorldRenderView, std::set<int> > world_rotations;
	// max max zoom of world/view/tile_width (iterate over rotations to calculate)
	std::map<WorldRenderView, int> world_max_max_zoom;

	// tile size of map
	std::map<std::string, int> map_tile_size;
	// max zoom of map (= max max zoom level of the world at time of rendering)
	std::map<std::string, int> map_max_zoom;
	// last render time of map/rotation
	std::map<std::string, std::array<int, 4> > map_last_rendered;
	// how to render each map/rotation (render-auto/skip/force)
	std::map<std::string, std::array<int, 4> > render_behaviors;

	picojson::value getConfigJSON() const;

	void setRenderBehaviors(std::vector<std::string> maps, int behavior);
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* MAPCRAFTERCONFIGHELPER_H_ */
