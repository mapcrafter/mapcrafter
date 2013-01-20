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

#include "mc/world.h"

#include "util.h"
#include "mc/chunk.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace mc {

World::World()
	: rotation(0) {
}

World::~World() {
}

/**
 * Loads a world from a directory.
 */
bool World::load(const std::string& dir, int rotation) {
	this->rotation = rotation;
	fs::path world_dir(dir);
	fs::path region_dir = world_dir / "region";
	if(!fs::exists(world_dir)) {
		std::cerr << "World directory " << world_dir << " does not exists!" << std::endl;
	} else if(!fs::exists(region_dir)) {
		std::cerr << "Region directory " << region_dir << " does not exists!" << std::endl;
	} else {
		return readRegions(region_dir.string());
	}
	return false;
}

/**
 * Scans the region directory for available Anvil region files.
 */
bool World::readRegions(const std::string& path) {
	fs::path region_dir(path);
	if(!fs::exists(region_dir))
		return false;
	std::string ending = ".mca";
	for(fs::directory_iterator it(region_dir); it != fs::directory_iterator();
			++it) {
		std::string region_file = (*it).path().string();
		std::string filename = (*it).path().filename().string();
		if(!std::equal(ending.rbegin(), ending.rend(), filename.rbegin()))
			continue;
		int x = 0;
		int z = 0;
		if(sscanf(filename.c_str(), "r.%d.%d.mca", &x, &z) != 2)
			continue;
		RegionPos pos(x, z);
		if (rotation)
			pos.rotate(rotation);
		available_regions.insert(pos);
		region_files[pos] = it->path().string();
	}
	return true;
}

int World::getRegionCount() const {
	return available_regions.size();
}

bool World::hasRegion(const RegionPos& pos) const {
	return available_regions.count(pos) != 0;
}

const std::unordered_set<RegionPos, hash_function>& World::getAvailableRegions() const {
	return available_regions;
}

/**
 * Returns a region file with a specific position.
 */
bool World::getRegion(const RegionPos& pos, RegionFile& region) const {
	if(available_regions.count(pos) == 0)
		return false;
	region = RegionFile(region_files.at(pos), rotation);
	return true;
}

}
}
