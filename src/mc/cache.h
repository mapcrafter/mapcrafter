/*
 * Copyright 2012 Moritz Hilscher
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

#ifndef CACHE_H_
#define CACHE_H_

#include "mc/pos.h"
#include "mc/chunk.h"
#include "mc/region.h"
#include "mc/world.h"

#include <functional>
#include <boost/unordered_map.hpp>
#include <boost/functional.hpp>

namespace mapcrafter {
namespace mc {

struct CacheStats {
	CacheStats()
			: hits(0), misses(0), unavailable(0) {
	}

	void print(const std::string& name) const {
		double efficiency = ((double) hits / (hits + misses)) * 10000;
		efficiency = (double) efficiency / 100;
		std::cout << name << ": hits:" << hits << " misses:" << misses << " unavailable:"
		        << unavailable << std::endl;
		std::cout << "   efficiency: " << efficiency << "% hits (hits+misses)"
		        << std::endl;
	}

	int hits;
	int misses;
	int unavailable;
};

#define RBITS 3
#define RWIDTH (1 << RBITS)
#define RSIZE (RWIDTH*RWIDTH)
#define RMASK (RSIZE-1)

#define CBITS 5
#define CWIDTH (1 << CBITS)
#define CSIZE (CWIDTH*CWIDTH)
#define CMASK (CSIZE-1)

template<typename Key, typename Value>
struct CacheEntry {
	Key key;
	Value value;
	bool used;
};

class WorldCache {
private:
	World& world;

	CacheEntry<RegionPos, RegionFile> regioncache[RSIZE];
	CacheEntry<ChunkPos, Chunk> chunkcache[CSIZE];

	CacheStats regionstats;
	CacheStats chunkstats;

	int getRegionCacheIndex(const RegionPos& pos);
	int getChunkCacheIndex(const ChunkPos& pos);

public:
	WorldCache(World& world);

	RegionFile* getRegion(const RegionPos& pos);
	Chunk* getChunk(const ChunkPos& pos);

	const CacheStats& getRegionCacheStats() const;
	const CacheStats& getChunkCacheStats() const;
};

}
}

#endif /* CACHE_H_ */
