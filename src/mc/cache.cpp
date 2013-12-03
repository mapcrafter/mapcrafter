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

#include "cache.h"

#include "../util.h"

namespace mapcrafter {
namespace mc {

Block::Block(uint16_t id, uint16_t data, uint8_t biome,
		uint8_t block_light, uint8_t sky_light)
		: id(id), data(data), biome(biome),
		  block_light(block_light), sky_light(sky_light) {
}

bool Block::isFullWater() const {
	return (id == 8 || id == 9) && data == 0;
}

WorldCache::WorldCache(const World& world)
		: world(world) {
	for (int i = 0; i < RSIZE; i++) {
		regioncache[i].used = false;
	}
	for (int i = 0; i < CSIZE; i++) {
		chunkcache[i].used = false;
	}
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

	// region does not exist, region in cache was not modified
	if (!world.getRegion(pos, entry.value))
		return nullptr;

	if (!entry.value.read()) {
		// the region is not valid, region in cache was probably modified
		entry.used = false;
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
	int status = region->loadChunk(pos, entry.value);
	// the chunk does not exist, chunk in cache was not modified
	if (status == RegionFile::CHUNK_DOES_NOT_EXIST)
		return nullptr;

	if (status != RegionFile::CHUNK_OK) {
		//chunkstats.unavailable++;
		// the chunk is not valid, chunk in cache was probably modified
		entry.used = false;
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
		if (get & GET_ID)
			block.id = mychunk->getBlockID(local);
		if (get & GET_DATA)
			block.data = mychunk->getBlockData(local);
		if (get & GET_BIOME)
			block.biome = mychunk->getBiomeAt(local);
		if (get & GET_BLOCK_LIGHT)
			block.block_light = mychunk->getBlockLight(local);
		if (get & GET_SKY_LIGHT)
			block.sky_light = mychunk->getSkyLight(local);
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
