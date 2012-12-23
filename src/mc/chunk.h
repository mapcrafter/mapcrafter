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

#ifndef CHUNK_H_
#define CHUNK_H_

#include "mc/nbt.h"
#include "mc/pos.h"

#include <stdint.h>

namespace mapcrafter {
namespace mc {

struct ChunkSection {
	uint8_t y;
	bool used;
	uint8_t blocks[16 * 16 * 16];
	uint8_t data[16 * 16 * 8];
};

class Chunk {
private:
	ChunkPos pos;

	int section_offsets[16];
	std::vector<ChunkSection> sections;
public:
	Chunk();
	virtual ~Chunk();

	bool readNBT(const char* data, size_t len, nbt::CompressionType compression =
	        nbt::ZLIB);
	void clear();

	bool hasSection(int section) const;
	uint8_t getBlockID(const LocalBlockPos& pos) const;
	uint8_t getBlockData(const LocalBlockPos& pos) const;

	const ChunkPos& getPos() const;
};

}
}

#endif /* CHUNK_H_ */
