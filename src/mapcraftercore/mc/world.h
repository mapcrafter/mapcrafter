/*
 * Copyright 2012-2014 Moritz Hilscher
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

#ifndef WORLD_H_
#define WORLD_H_

#include "chunk.h"
#include "pos.h"
#include "region.h"
#include "worldcrop.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace mc {

/**
 * Dimension of the Minecraft world.
 * The Nether, normal Overworld or The End.
 */
enum class Dimension {
	NETHER,
	OVERWORLD,
	END,
};

/**
 * Simple hash function to use regions in unordered_set/map.
 * This just assumes that there are maximal 8096 regions on x/z axis, this are
 * all in all 8096^2=67108864 regions. I think this should be enough for now.
 */
struct hash_function {
	long operator()(const RegionPos& region) const {
		return (region.x+4096) * 2048 + region.z + 4096;
	}
};

/**
 * This class represents a Minecraft World.
 *
 * It manages only the available region files. Access to the chunks is with the region
 * files possible. If you want full reading access to the world, use the WorldCache class.
 */
class World {
public:
	typedef std::unordered_set<RegionPos, hash_function> RegionSet;
	typedef std::unordered_map<RegionPos, std::string, hash_function> RegionMap;

	/**
	 * Constructor. You should specify a world directory and you can specify a dimension
	 * of the world (Nether, Overworld per default, End). Mapcrafter will automagically
	 * try to find the right region directory.
	 */
	World(std::string world_dir = "", Dimension dimension = Dimension::OVERWORLD);
	~World();

	/**
	 * Returns the directory of the world.
	 */
	fs::path getWorldDir() const;

	/**
	 * Returns the region directory of the world.
	 */
	fs::path getRegionDir() const;

	/**
	 * Returns the used dimension of the world.
	 */
	Dimension getDimension() const;

	/**
	 * Returns/Sets the rotation of the world. You set this before loading the world.
	 */
	int getRotation() const;
	void setRotation(int rotation);

	/**
	 * Returns/Sets the boundaries of the world. You also have to set this before
	 * loading the world.
	 */
	WorldCrop getWorldCrop() const;
	void setWorldCrop(const WorldCrop& world_crop);

	/**
	 * Loads a world from the specified directory. Returns false if the world- or region
	 * directory does not exist.
	 */
	bool load();

	/**
	 * Returns the count of available region files.
	 */
	int getAvailableRegionCount() const;

	/**
	 * Returns the positions of all available regions.
	 */
	const World::RegionSet& getAvailableRegions() const;

	/**
	 * Returns whether a specific region exists.
	 */
	bool hasRegion(const RegionPos& pos) const;

	/**
	 * Returns the path of a region file. Returns an empty path if the region does
	 * not exist.
	 */
	fs::path getRegionPath(const RegionPos& pos) const;

	/**
	 * Creates the Region-object for a specific region and assigns the supplied reference
	 * 'region' to it. Returns false if the region does not exist.
	 */
	bool getRegion(const RegionPos& pos, RegionFile& region) const;

private:
	// world directory, region directory
	fs::path world_dir, region_dir;
	// used dimension of the world
	Dimension dimension;

	// rotation and possible boundaries of the world
	int rotation;
	WorldCrop world_crop;

	// (hash-) set containing positions of available region files
	RegionSet available_regions;
	// (hash-) map containing positions of available region files and their file paths
	RegionMap region_files;

	/**
	 * Scans a directory for Anvil *.mca region files and adds them to the available
	 * region files. Returns false if the directory does not exist.
	 */
	bool readRegions(const fs::path& region_dir);
};

}
}

#endif /* WORLD_H_ */
