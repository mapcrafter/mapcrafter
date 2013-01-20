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

#ifndef WORLD_H_
#define WORLD_H_

#include "mc/pos.h"
#include "mc/chunk.h"
#include "mc/region.h"

#include <string>
#include <unordered_set>
#include <unordered_map>

namespace mapcrafter {
namespace mc {

/**
 * Simple hash function to use regions in unordered_set/map.
 * This just assumes that there are maximal 8096 regions on x/z axis, this are
 * all in all 8096^2=67108864 regions. I think this should be enough.
 */
struct hash_function {
	long operator()(const RegionPos& region) const {
		return (region.x+4096) * 2048 + region.z + 4096;
	}
};

/**
 * This class represents a Minecraft world. It manages only the available region files.
 * Access to the chunks is with the region files possible. If you want full reading
 * access to the world, use the WorldCache class.
 */
class World {
private:
	int rotation;

	std::unordered_set<RegionPos, hash_function> available_regions;
	std::unordered_map<RegionPos, std::string, hash_function> region_files;

	bool readRegions(const std::string& path);
public:
	World();
	~World();

	bool load(const std::string& dir, int rotation = 0);

	int getRegionCount() const;

	bool hasRegion(const RegionPos& pos) const;
	const std::unordered_set<RegionPos, hash_function>& getAvailableRegions() const;

	bool getRegion(const RegionPos& pos, RegionFile& region) const;

};

}
}

#endif /* WORLD_H_ */
