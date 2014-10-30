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

#ifndef WORLDENTITIES_H_
#define WORLDENTITIES_H_

#include "nbt.h"
#include "pos.h"
#include "world.h"
#include "worldcrop.h"
#include "../util.h"

#include <array>
#include <map>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace mc {

class SignEntity {
public:
	typedef std::array<std::string, 4> Lines;

	SignEntity();
	SignEntity(const mc::BlockPos& pos, const Lines& lines);
	~SignEntity();

	/**
	 * Returns the position of the sign.
	 */
	const mc::BlockPos& getPos() const;

	/**
	 * Returns the four lines of the sign.
	 */
	const SignEntity::Lines& getLines() const;

	/**
	 * Returns the text of the sign -- the not-empty lines joined with a separative space.
	 */
	const std::string& getText() const;

private:
	mc::BlockPos pos;

	Lines lines;
	std::string text;
};

class WorldEntitiesCache {
public:
	WorldEntitiesCache(const World& world);
	~WorldEntitiesCache();

	/**
	 * Updates the entity cache.
	 */
	void update(bool verbose = false);

	std::vector<SignEntity> getSigns(WorldCrop crop = WorldCrop()) const;
private:
	World world;
	fs::path cache_file;

	std::map<RegionPos, std::map<ChunkPos, std::vector<nbt::TagCompound> > > entities;

	/**
	 * Reads the file with the cached entities and returns a timestamp when this cache
	 * was updated the last time.
	 */
	unsigned int readCacheFile();

	/**
	 * Writes the file with the cached entities.
	 */
	void writeCacheFile() const;
};

} /* namespace mc */
} /* namespace mapcrafter */

#endif /* WORLDENTITIES_H_ */
