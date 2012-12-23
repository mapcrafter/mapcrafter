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

#include "mc/cache.h"

namespace mapcrafter {
namespace mc {

WorldCache::WorldCache(World& world)
		: world(world) {
	for (int i = 0; i < RSIZE; i++) {
		regioncache[i].used = false;
	}
	for (int i = 0; i < CSIZE; i++) {
		chunkcache[i].used = false;
	}
}

int WorldCache::getRegionCacheIndex(const RegionPos& pos) {
	return ((pos.x + 4096) & RMASK) * RWIDTH + (pos.z + 4096) & RMASK;
}

int WorldCache::getChunkCacheIndex(const ChunkPos& pos) {
	//                4096*32
	return ((pos.x + 131072) & CMASK) * CWIDTH + (pos.z + 131072) & CMASK;
}

RegionFile* WorldCache::getRegion(const RegionPos& pos) {
	CacheEntry<RegionPos, RegionFile>& entry = regioncache[getRegionCacheIndex(pos)];
	if (entry.used && entry.key == pos) {
		//regionstats.hits++;
		return &entry.value;
	}

	//if (!world.hasRegion(pos)) {
	//	regionstats.unavailable++;
	//	return NULL;
	//}
	if (!world.getRegion(pos, entry.value) || !entry.value.loadAll()) {
		//regionstats.unavailable++;
		entry.used = false;
		return NULL;
	}

	entry.used = true;
	entry.key = pos;
	//regionstats.misses++;
	return &entry.value;
}

Chunk* WorldCache::getChunk(const ChunkPos& pos) {
	CacheEntry<ChunkPos, Chunk>& entry = chunkcache[getChunkCacheIndex(pos)];
	if (entry.used && entry.key == pos) {
		//chunkstats.hits++;
		return &entry.value;
	}

	RegionFile* region = getRegion(pos.getRegion());
	if (region == NULL) {
		//chunkstats.unavailable++;
		return NULL;
	}

	int status = region->loadChunk(pos, entry.value);
	if(status != CHUNK_OK) {
		//chunkstats.unavailable++;
		entry.used = false;
		return NULL;
	}

	entry.used = true;
	entry.key = pos;
	//chunkstats.misses++;
	return &entry.value;
}

const CacheStats& WorldCache::getRegionCacheStats() const {
	return regionstats;
}

const CacheStats& WorldCache::getChunkCacheStats() const {
	return chunkstats;
}

}
}
