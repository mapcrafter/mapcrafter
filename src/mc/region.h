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

#ifndef REGION_H_
#define REGION_H_

#include "mc/pos.h"
#include "mc/chunk.h"

#include <string>
#include <set>

namespace mapcrafter {
namespace mc {

/**
 * This class represents a Minecraft region file.
 */
class RegionFile {
private:
	std::string filename;
	RegionPos regionpos;

	int rotation;

	// a set with all available chunks
	std::set<ChunkPos> containing_chunks;

	// the offsets, where the chunkdata starts
	int chunk_offsets[1024];
	// timestamps of the chunks
	int chunk_timestamps[1024];
	// data from the region file
	std::vector<uint8_t> regiondata;

	bool readHeaders(std::ifstream& file);
public:
	RegionFile();
	RegionFile(const std::string& filename, int rotation = 0);
	virtual ~RegionFile();

	bool loadAll();
	bool loadHeaders();

	const std::string& getFilename() const;
	const RegionPos& getPos() const;

	const std::set<ChunkPos>& getContainingChunks() const;
	bool hasChunk(const ChunkPos& chunk) const;

	int getChunkTimestamp(const ChunkPos& chunk) const;
	int loadChunk(const ChunkPos& pos, Chunk& chunk);

	static const int CHUNK_OK = 1;
	static const int CHUNK_DOES_NOT_EXIST = 2;
	static const int CHUNK_DATA_INVALID = 3;
	static const int CHUNK_NBT_ERROR = 4;
};

}
}

#endif /* REGION_H_ */
