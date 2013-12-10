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

#include "world.h"

#include "../util.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace mc {

World::World()
	: rotation(0) {
}

World::~World() {
}

bool World::readRegions(const std::string& path) {
	fs::path region_dir(path);
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

void World::setRotation(int rotation) {
	this->rotation = rotation;
}

void World::setWorldCrop(const WorldCrop& worldcrop) {
	this->worldcrop = worldcrop;
}

bool World::load(const std::string& dir) {
	fs::path world_dir(dir);
	fs::path region_dir = world_dir / "region";
	if(!fs::exists(world_dir)) {
		std::cerr << "Error: World directory " << world_dir << " does not exist!" << std::endl;
	} else if(!fs::exists(region_dir)) {
		std::cerr << "Error: Region directory " << region_dir << " does not exist!" << std::endl;
	} else {
		return readRegions(region_dir.string());
	}
	return false;
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
