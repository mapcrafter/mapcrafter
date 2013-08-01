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

#include "region.h"

#include <fstream>
#include <cstdlib>

namespace mapcrafter {
namespace mc {

RegionFile::RegionFile()
		: rotation(0) {
}

RegionFile::RegionFile(const std::string& filename, int rotation)
		: filename(filename), rotation(rotation) {
	regionpos = RegionPos::byFilename(filename);
	if (rotation)
		regionpos.rotate(rotation);
}

RegionFile::~RegionFile() {
}

/**
 * Reads the headers of a region file: chunk offsets/timestamps
 */
bool RegionFile::readHeaders(std::ifstream& file) {
	if (!file)
		return false;
	containing_chunks.clear();

	for (int i = 0; i < 1024; i++) {
		chunk_offsets[i] = 0;
		chunk_timestamps[i] = 0;
	}

	for (int x = 0; x < 32; x++) {
		for (int z = 0; z < 32; z++) {
			file.seekg(4 * (x + z * 32), std::ios::beg);
			int tmp;
			file.read(reinterpret_cast<char*>(&tmp), 4);
			if (tmp == 0)
				continue;
			int offset = bigEndian32(tmp << 8) * 4096;
			//uint8_t sectors = ((uint8_t*) &tmp)[3];

			file.seekg(4096, std::ios::cur);
			int timestamp;
			file.read(reinterpret_cast<char*>(&timestamp), 4);
			timestamp = bigEndian32(timestamp);

			ChunkPos pos(x + regionpos.x * 32, z + regionpos.z * 32);
			if (rotation)
				pos.rotate(rotation);

			containing_chunks.insert(pos);

			chunk_offsets[z * 32 + x] = offset;
			chunk_timestamps[z * 32 + x] = timestamp;
		}
	}
	return true;
}

/**
 * Reads the whole region file.
 */
bool RegionFile::loadAll() {
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	if (!readHeaders(file))
		return false;
	file.seekg(0, std::ios::end);
	int filesize = file.tellg();
	file.seekg(0, std::ios::beg);

	regiondata.resize(filesize);
	file.read(reinterpret_cast<char*>(&regiondata[0]), filesize);

	return true;
}

bool RegionFile::loadHeaders() {
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	return readHeaders(file);
}

const std::string& RegionFile::getFilename() const {
	return filename;
}

const RegionPos& RegionFile::getPos() const {
	return regionpos;
}

const std::set<ChunkPos>& RegionFile::getContainingChunks() const {
	return containing_chunks;
}

bool RegionFile::hasChunk(const ChunkPos& chunk) const {
	ChunkPos unrotated = chunk;
	if (rotation)
		unrotated.rotate(4 - rotation);
	return chunk_offsets[unrotated.getLocalZ() * 32 + unrotated.getLocalX()] != 0;
}

int RegionFile::getChunkTimestamp(const ChunkPos& chunk) const {
	ChunkPos unrotated = chunk;
	if (rotation)
		unrotated.rotate(4 - rotation);
	return chunk_timestamps[unrotated.getLocalZ() * 32 + unrotated.getLocalX()];
}

/**
 * This method tries to load a chunk from the region data and returns a status.
 */
int RegionFile::loadChunk(const ChunkPos& pos, Chunk& chunk) {
	// unrotate the chunk position, because the chunks are stored interna with their
	// original positions
	ChunkPos unrotated = pos;
	if (rotation)
		unrotated.rotate(4 - rotation);

	if (!hasChunk(pos))
		return CHUNK_DOES_NOT_EXIST;

	// get the offsets, where the chunk data starts
	int offset = chunk_offsets[unrotated.getLocalZ() * 32 + unrotated.getLocalX()];

	// get data size and compression type
	int size = *(reinterpret_cast<int*>(&regiondata[offset]));
	uint8_t compression = regiondata[offset + 4];
	nbt::Compression comp = nbt::Compression::NO_COMPRESSION;
	if (compression == 1)
		comp = nbt::Compression::GZIP;
	else if (compression == 2)
		comp = nbt::Compression::ZLIB;

	size = be32toh(size) - 1;

	// set the chunk rotation
	chunk.setRotation(rotation);
	// try to load the chunk
	try {
		if (!chunk.readNBT(reinterpret_cast<char*>(&regiondata[offset + 5]), size, comp))
			return CHUNK_DATA_INVALID;
	} catch (const nbt::NBTError& err) {
		std::cout << "Error: Unable to read chunk at " << pos.x << ":" << pos.z
		        << " : " << err.what() << std::endl;
		return CHUNK_NBT_ERROR;
	}
	return CHUNK_OK;
}

}
}
