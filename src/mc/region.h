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
	RegionFile(const std::string& filename, int rotation = 0);
	~RegionFile();

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
	 * Writes the region to a file. You can also specify a different filename to write
	 * the region file to.
	 */
	bool write(std::string filename = "") const;

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
	void setChunkTimestamp(const ChunkPos& chunk, uint32_t timestamp);

	/**
	 * Returns the raw (compressed) data of a specific chunk. Returns an empty array if
	 * the chunk does not exist.
	 */
	const std::vector<uint8_t>& getChunkData(const ChunkPos& chunk) const;

	/**
	 * Returns the type of the compressed chunk data (one byte, see specification of
	 * region format).
	 */
	uint8_t getChunkDataCompression(const ChunkPos& chunk) const;

	/**
	 * Sets the raw (compressed) data of a specific chunk. You also need to specify
	 * a compression type (one byte, see specification of region format).
	 * You can remove a chunk by setting its chunk data to an empty array.
	 */
	void setChunkData(const ChunkPos& chunk, const std::vector<uint8_t>& data,
			uint8_t compression);

	/**
	 * Loads a specific chunk into the supplied Chunk-object.
	 * Returns as integer one of the RegionFile::CHUNK_* status codes.
	 */
	int loadChunk(const ChunkPos& pos, Chunk& chunk);

private:
	std::string filename;
	RegionPos regionpos;

	// rotation of the region file
	int rotation;

	// a set with all available chunks
	ChunkMap containing_chunks;

	// the offsets where the chunk data of each chunk starts
	uint32_t chunk_offsets[1024];
	// timestamps of the chunks
	uint32_t chunk_timestamps[1024];
	// actual chunk data
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
