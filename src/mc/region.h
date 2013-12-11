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

#include "pos.h"
#include "chunk.h"
#include "worldcrop.h"

#include <string>
#include <set>

namespace mapcrafter {
namespace mc {

/**
 * This class represents a Minecraft region file.
 */
class RegionFile {
public:
	typedef std::set<ChunkPos> ChunkMap;

	// status codes for loadChunk method
	static const int CHUNK_OK = 1;
	static const int CHUNK_DOES_NOT_EXIST = 2;
	static const int CHUNK_DATA_INVALID = 3;
	static const int CHUNK_NBT_ERROR = 4;

	RegionFile();
	RegionFile(const std::string& filename);
	~RegionFile();

	/**
	 * Sets the rotation of the world. You have to call this before loading a world.
	 */
	void setRotation(int rotation);

	/**
	 * Sets the boundaries of the world.
	 */
	void setWorldCrop(const WorldCrop& worldcrop);

	/**
	 * Reads the whole region file with the data of all chunks. Returns false if the
	 * region file is corrupted.
	 */
	bool read();

	/**
	 * Reads only the headers (timestamps and which chunks exist) of the region file.
	 * Returns false if the region header is corrupted (size < 8192).
	 */
	bool readOnlyHeaders();

	/**
	 * Returns the filename of the region file.
	 */
	const std::string& getFilename() const;

	/**
	 * Returns the region position of the region file.
	 */
	const RegionPos& getPos() const;

	/**
	 * Returns the count of containing chunks.
	 */
	int getContainingChunksCount() const;

	/**
	 * Returns a set of containing chunks.
	 */
	const RegionFile::ChunkMap& getContainingChunks() const;

	/**
	 * Returns whether a specific chunk is contained in the region file.
	 */
	bool hasChunk(const ChunkPos& chunk) const;

	/**
	 * Returns/Sets the timestamp of a specific chunk.
	 */
	int getChunkTimestamp(const ChunkPos& chunk) const;
	void setChunkTimestamp(const ChunkPos& chunk, int timestamp);

	/**
	 * Loads a specific chunk into the supplied Chunk-object.
	 * Returns as integer one of the RegionFile::CHUNK_* status codes.
	 */
	int loadChunk(const ChunkPos& pos, Chunk& chunk);

private:
	std::string filename;
	RegionPos regionpos, regionpos_original;

	// rotation of the region file
	int rotation;
	// and possible boundaries of the world
	WorldCrop worldcrop;

	// a set with all available chunks
	ChunkMap containing_chunks;

	// indexes of the following arrays are chunk coordinates: z*32 + x
	// where x and z are the original local chunk coordinates -- not the rotated ones

	// the offsets where the chunk data of each chunk starts
	int chunk_offsets[1024];
	// timestamps of the chunks
	int chunk_timestamps[1024];
	// actual chunk data with compression type
	uint8_t chunk_data_compression[1024];
	std::vector<uint8_t> chunk_data[1024];

	/**
	 * Reads the headers of a region file.
	 */
	bool readHeaders(std::ifstream& file);
};

}
}

#endif /* REGION_H_ */
