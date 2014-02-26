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

#include "world.h"

#include "../util.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace mapcrafter {
namespace mc {

World::World(std::string world_dir, Dimension dimension)
	: world_dir(world_dir), dimension(dimension), rotation(0) {
}

World::~World() {
}

bool World::readRegions(const fs::path& region_dir) {
	if(!fs::exists(region_dir))
		return false;
	std::string ending = ".mca";
	for(fs::directory_iterator it(region_dir); it != fs::directory_iterator(); ++it) {
		std::string region_file = (*it).path().string();
		std::string filename = BOOST_FS_FILENAME((*it).path());

		if(!std::equal(ending.rbegin(), ending.rend(), filename.rbegin()))
			continue;
		int x = 0;
		int z = 0;
		if(sscanf(filename.c_str(), "r.%d.%d.mca", &x, &z) != 2)
			continue;
		RegionPos pos(x, z);
		// check if we should not crop this region
		if (!worldcrop.isRegionContained(pos))
			continue;
		if (rotation)
			pos.rotate(rotation);
		available_regions.insert(pos);
		region_files[pos] = it->path().string();
	}
	return true;
}

fs::path World::getWorldDir() const {
	return world_dir;
}

fs::path World::getRegionDir() const {
	return region_dir;
}

Dimension World::getDimension() const {
	return dimension;
}

int World::getRotation() const {
	return rotation;
}


void World::setRotation(int rotation) {
	this->rotation = rotation;
}

WorldCrop World::getWorldCrop() const {
	return worldcrop;
}

void World::setWorldCrop(const WorldCrop& worldcrop) {
	this->worldcrop = worldcrop;
}

bool World::load() {
	if(!fs::exists(world_dir)) {
		std::cerr << "Error: World directory " << world_dir;
		std::cerr << " does not exist!" << std::endl;
		return false;
	}

	std::string world_name = BOOST_FS_FILENAME(world_dir);

	// try to find the region directory
	if (dimension == Dimension::OVERWORLD) {
		 region_dir = world_dir / "region";
	} else if (dimension == Dimension::NETHER) {
		// try the extra bukkit nether directory at first, then the normal directory
		region_dir = world_dir.parent_path() / (world_name + "_nether") / "DIM-1" / "region";
		if (!fs::exists(region_dir))
			region_dir = world_dir / "DIM-1" / "region";
	} else if (dimension == Dimension::END) {
		// same here
		region_dir = world_dir.parent_path() / (world_name + "_the_end") / "DIM1" / "region";
		if (!fs::exists(region_dir))
			region_dir = world_dir / "DIM1" / "region";
	}

	if(!fs::exists(region_dir)) {
		std::cerr << "Error: Region directory " << region_dir << " does not exist!" << std::endl;
		return false;
	}

	return readRegions(region_dir.string());
}

int World::getAvailableRegionCount() const {
	return available_regions.size();
}

const World::RegionSet& World::getAvailableRegions() const {
	return available_regions;
}

bool World::hasRegion(const RegionPos& pos) const {
	return available_regions.count(pos) != 0;
}

bool World::getRegion(const RegionPos& pos, RegionFile& region) const {
	RegionMap::const_iterator it = region_files.find(pos);
	if (it == region_files.end())
		return false;
	region = RegionFile(it->second);
	region.setRotation(rotation);
	region.setWorldCrop(worldcrop);
	return true;
}

}
}
