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

#include "worldcache.h"

namespace mapcrafter {
namespace mc {

Block::Block()
//	: Block(mc::BlockPos(0, 0, 0), 0, 0) { /* gcc 4.4 being stupid :/ */
	: pos(mc::BlockPos(0, 0, 0)), id(0), data(0), biome(0),
	  block_light(0), sky_light(15), fields_set(0) {
}

Block::Block(const mc::BlockPos& pos, uint16_t id, uint16_t data)
	: pos(pos), id(id), data(data), biome(0),
	  block_light(0), sky_light(15), fields_set(GET_ID | GET_DATA) {
}

bool Block::isFullWater() const {
	return (id == 8 || id == 9) && (data & 0x7) == 0;
}

bool Block::isStairs() const {
	return id == 53 || id == 67 || id == 108 || id == 109 || id == 114 || id == 128 || id == 134 || id == 135 || id == 136 || id == 156 || id == 163 || id == 164 || id == 180 || id == 203;
}

WorldCache::WorldCache() {
	for (int i = 0; i < RSIZE; i++)
		regioncache[i].used = false;
	for (int i = 0; i < CSIZE; i++)
		chunkcache[i].used = false;
}

WorldCache::WorldCache(const World& world)
	: world(world) {
	for (int i = 0; i < RSIZE; i++)
		regioncache[i].used = false;
	for (int i = 0; i < CSIZE; i++)
		chunkcache[i].used = false;
}

const World& WorldCache::getWorld() const {
	return world;
}

/**
 * Calculates the position of a region position in the cache.
 */
int WorldCache::getRegionCacheIndex(const RegionPos& pos) const {
	return (((pos.x + 4096) & RMASK) * RWIDTH + (pos.z + 4096)) & RMASK;
}

/**
 * Calculates the position of a chunk position in the cache.
 */
int WorldCache::getChunkCacheIndex(const ChunkPos& pos) const {
	//                4096*32
	return (((pos.x + 131072) & CMASK) * CWIDTH + (pos.z + 131072)) & CMASK;
}

RegionFile* WorldCache::getRegion(const RegionPos& pos) {
	CacheEntry<RegionPos, RegionFile>& entry = regioncache[getRegionCacheIndex(pos)];

	// check if region is already in cache
	if (entry.used && entry.key == pos) {
		//regionstats.hits++;
		return &entry.value;
	}

	// if not try to load the region
	// but make sure we did not already try to load the region file and it was broken
	if (regions_broken.count(pos))
		return nullptr;

	// region does not exist, region in cache was not modified
	if (!world.getRegion(pos, entry.value))
		return nullptr;

	if (!entry.value.read()) {
		// the region is not valid, region in cache was probably modified
		entry.used = false;
		// remember this region as broken and do not try to load it again
		regions_broken.insert(pos);
		return nullptr;
	}

	entry.used = true;
	entry.key = pos;
	//regionstats.misses++;
	return &entry.value;
}

Chunk* WorldCache::getChunk(const ChunkPos& pos) {
	CacheEntry<ChunkPos, Chunk>& entry = chunkcache[getChunkCacheIndex(pos)];
	// check if chunk is already in cache
	if (entry.used && entry.key == pos) {
		//chunkstats.hits++;
		return &entry.value;
	}

	// if not try to get the region of the chunk from the cache
	RegionFile* region = getRegion(pos.getRegion());
	if (region == nullptr) {
		//chunkstats.unavailable++;
		return nullptr;
	}

	// then try to load the chunk
	// but make sure we did not already try to load the chunk and it was broken
	if (chunks_broken.count(pos))
		return nullptr;

	int status = region->loadChunk(pos, entry.value);
	// the chunk does not exist, chunk in cache was not modified
	if (status == RegionFile::CHUNK_DOES_NOT_EXIST)
		return nullptr;

	if (status != RegionFile::CHUNK_OK) {
		//chunkstats.unavailable++;
		// the chunk is not valid, chunk in cache was probably modified
		entry.used = false;
		// remember this chunk as broken and do not try to load it again
		chunks_broken.insert(pos);
		return nullptr;
	}

	entry.used = true;
	entry.key = pos;
	//chunkstats.misses++;
	return &entry.value;
}

Block WorldCache::getBlock(const mc::BlockPos& pos, const mc::Chunk* chunk, int get) {
	// this can happen when we check for the bottom block shadow edges
	if (pos.y < 0)
		return Block();

	mc::ChunkPos chunk_pos(pos);
	const mc::Chunk* mychunk = chunk;
	if (chunk == nullptr || chunk_pos != chunk->getPos())
		mychunk = getChunk(chunk_pos);
	// chunk may be nullptr
	if (mychunk == nullptr) {
		return Block();
	// otherwise get all required block data
	} else {
		mc::LocalBlockPos local(pos);
		Block block;
		block.pos = pos;
		if (get & GET_ID) {
			block.id = mychunk->getBlockID(local);
			block.fields_set |= GET_ID;
		} if (get & GET_DATA) {
			block.data = mychunk->getBlockData(local);
			block.fields_set |= GET_DATA;
		} if (get & GET_BIOME) {
			block.biome = mychunk->getBiomeAt(local);
			block.fields_set |= GET_BIOME;
		} if (get & GET_BLOCK_LIGHT) {
			block.block_light = mychunk->getBlockLight(local);
			block.fields_set |= GET_BLOCK_LIGHT;
		} if (get & GET_SKY_LIGHT) {
			block.sky_light = mychunk->getSkyLight(local);
			block.fields_set |= GET_SKY_LIGHT;
		}
		return block;
	}
}

const CacheStats& WorldCache::getRegionCacheStats() const {
	return regionstats;
}

const CacheStats& WorldCache::getChunkCacheStats() const {
	return chunkstats;
}

}
}
