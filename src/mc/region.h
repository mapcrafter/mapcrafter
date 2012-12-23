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

#ifndef REGION_H_
#define REGION_H_

#include "mc/pos.h"
#include "mc/chunk.h"

#include <fstream>
#include <string>
#include <set>

#define CHUNK_OK 1
#define CHUNK_DOES_NOT_EXISTS 2
#define CHUNK_INVALID 3
#define CHUNK_NBT_ERROR 4

namespace mapcrafter {
namespace mc {

class RegionFile {
private:
	std::string filename;
	RegionPos regionpos;

	std::set<ChunkPos> containing_chunks;

	int chunk_offsets[1024];
	int chunk_timestamps[1024];
	std::vector<uint8_t> regiondata;

	bool readHeaders(std::ifstream& file);
public:
	RegionFile();
	RegionFile(const std::string& filename);
	virtual ~RegionFile();

	bool loadAll();
	bool loadHeaders();

	const std::string& getFilename() const;
	const RegionPos& getPos() const;

	const std::set<ChunkPos>& getContainingChunks() const;
	bool hasChunk(const ChunkPos& chunk) const;

	int getChunkTimestamp(const ChunkPos& chunk) const;
	int loadChunk(const ChunkPos& pos, Chunk& chunk);
};

}
}

#endif /* REGION_H_ */
