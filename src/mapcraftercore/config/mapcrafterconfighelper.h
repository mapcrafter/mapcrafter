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

class MapcrafterConfig;

class MapcrafterConfigHelper {
public:
	MapcrafterConfigHelper();
	MapcrafterConfigHelper(const MapcrafterConfig& config);
	~MapcrafterConfigHelper();

	void readMapSettings();
	void writeMapSettings() const;

	std::set<int> getUsedRotations(const TileSetKey& tile_set) const;

	int getTileSetMaxZoom(const TileSetKey& tile_set) const;
	void setTileSetMaxZoom(const TileSetKey& tile_set, int zoomlevel);

	renderer::TilePos getWorldTileOffset(const TileSetKey& tile_set,
			int rotation) const;
	void setWorldTileOffset(const TileSetKey& tile_set, int rotation,
			const renderer::TilePos& tile_offset);

	int getMapTileSize(const std::string& map) const;
	void setMapTileSize(const std::string& map, int tile_size);

	int getMapMaxZoom(const std::string& map) const;
	void setMapMaxZoom(const std::string& map, int max_zoom);

	int getMapLastRendered(const std::string& map, int rotation) const;
	void setMapLastRendered(const std::string& map, int rotation, int last_rendered);

private:
	MapcrafterConfig config;

	// tile offset of world/view/tile_width/rotation
	std::map<TileSetKey, std::array<renderer::TilePos, 4> > world_tile_offset;
	// used rotations of world/view/tile_width
	std::map<TileSetKey, std::set<int> > world_rotations;
	// max max zoom of world/view/tile_width (iterate over rotations to calculate)
	std::map<TileSetKey, int> world_max_max_zoom;

	// tile size of map
	std::map<std::string, int> map_tile_size;
	// max zoom of map (= max max zoom level of the world at time of rendering)
	std::map<std::string, int> map_max_zoom;
	// last render time of map/rotation
	std::map<std::string, std::array<int, 4> > map_last_rendered;

	picojson::value getConfigJSON() const;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* MAPCRAFTERCONFIGHELPER_H_ */
