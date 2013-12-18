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

bool RegionFile::readHeaders(std::ifstream& file) {
	if (!file)
		return false;
	containing_chunks.clear();

	for (int i = 0; i < 1024; i++) {
		chunk_offsets[i] = 0;
		chunk_timestamps[i] = 0;
		chunk_data_compression[i] = 0;
	}

	file.seekg(0, std::ios::end);
	int filesize = file.tellg();
	file.seekg(0, std::ios::beg);
	// make sure the region file has a header
	if (filesize < 8192) {
		return false;
	}

	for (int x = 0; x < 32; x++) {
		for (int z = 0; z < 32; z++) {
			file.seekg(4 * (x + z * 32), std::ios::beg);
			int tmp;
			file.read(reinterpret_cast<char*>(&tmp), 4);
			if (tmp == 0)
				continue;
			int offset = util::bigEndian32(tmp << 8) * 4096;
			//uint8_t sectors = ((uint8_t*) &tmp)[3];

			file.seekg(4096, std::ios::cur);
			int timestamp;
			file.read(reinterpret_cast<char*>(&timestamp), 4);
			timestamp = util::bigEndian32(timestamp);

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

bool RegionFile::read() {
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	if (!readHeaders(file))
		return false;
	file.seekg(0, std::ios::end);
	int filesize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> regiondata(filesize);
	file.read(reinterpret_cast<char*>(&regiondata[0]), filesize);

	for (int i = 0; i < 1024; i++) {
		// get the offsets, where the chunk data starts
		int offset = chunk_offsets[i];
		if (offset == 0)
			continue;

		// get data size and compression type
		int size = *(reinterpret_cast<int*>(&regiondata[offset]));
		size = util::bigEndian32(size) - 1;
		uint8_t compression = regiondata[offset + 4];

		chunk_data_compression[i] = compression;
		chunk_data[i].resize(size);
		std::copy(&regiondata[offset+5], &regiondata[offset+5+size], chunk_data[i].begin());
	}

	return true;
}

bool RegionFile::readOnlyHeaders() {
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	return readHeaders(file);
}

bool RegionFile::write(std::string filename) const {
	if (filename.empty())
		filename = this->filename;
	if (filename.empty())
		throw std::invalid_argument("You have to specify a filename!");

	uint32_t offsets[1024];
	for (int i = 0; i < 1024; i++)
		offsets[i] = 0;

	std::stringstream out_data, out_header;

	// write chunk data to a temporary string stream
	int position = 8192;
	for (int i = 0; i < 1024; i++) {
		if (chunk_data[i].size() == 0)
			continue;
		// pad every chunk data with zeros to the next n*4096 bytes
		if (position % 4096 != 0) {
			int append = 4096 - position % 4096;
			position += append;
			for (int j = 0; j < append; j++)
				out_data.put(0);
		}

		// calculate the offset, the chunk starts at 4096*offset bytes
		offsets[i] = position / 4096;

		// get chunk data, size and compression type
		const std::vector<uint8_t>& data = chunk_data[i];
		uint32_t size = data.size();
		size = util::bigEndian32(size + 1);
		uint8_t compression = chunk_data_compression[i];

		// append everything to the data
		out_data.write(reinterpret_cast<char*>(&size), 4);
		out_data.write(reinterpret_cast<char*>(&compression), 1);
		out_data.write(reinterpret_cast<const char*>(&data[0]), data.size());
		position += data.size() + 5;
	}

	// create the header with offsets and timestamps
	for (int i = 0; i < 1024; i++) {
		int offset_big_endian = util::bigEndian32(offsets[i]) >> 8;
		out_header.write(reinterpret_cast<char*>(&offset_big_endian), 4);
	}

	for (int i = 0; i < 1024; i++) {
		int timestamp_big_endian = util::bigEndian32(chunk_timestamps[i]);
		out_header.write(reinterpret_cast<char*>(&timestamp_big_endian), 4);
	}

	// write complete region file
	std::ofstream out(filename, std::ios::binary);
	if (!out)
		return false;
	out << out_header.rdbuf() << out_data.rdbuf();
	out.close();
	return !out.fail();
}

const std::string& RegionFile::getFilename() const {
	return filename;
}

const RegionPos& RegionFile::getPos() const {
	return regionpos;
}

int RegionFile::getContainingChunksCount() const {
	return containing_chunks.size();
}

const RegionFile::ChunkMap& RegionFile::getContainingChunks() const {
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

void RegionFile::setChunkTimestamp(const ChunkPos& chunk, uint32_t timestamp) {
	ChunkPos unrotated = chunk;
	if (rotation)
		unrotated.rotate(4 - rotation);
	chunk_timestamps[unrotated.getLocalZ() * 32 + unrotated.getLocalX()] = timestamp;
}

const std::vector<uint8_t>& RegionFile::getChunkData(const ChunkPos& chunk) const {
	ChunkPos unrotated = chunk;
	if (rotation)
		unrotated.rotate(4 - rotation);
	return chunk_data[unrotated.getLocalZ() * 32 + unrotated.getLocalX()];
}

uint8_t RegionFile::getChunkDataCompression(const ChunkPos& chunk) const {
	ChunkPos unrotated = chunk;
	if (rotation)
		unrotated.rotate(4 - rotation);
	return chunk_data_compression[unrotated.getLocalZ() * 32 + unrotated.getLocalX()];
}

void RegionFile::setChunkData(const ChunkPos& chunk, const std::vector<uint8_t>& data,
		uint8_t compression) {
	ChunkPos unrotated = chunk;
	if (rotation)
		unrotated.rotate(4 - rotation);
	int x = unrotated.getLocalX();
	int z = unrotated.getLocalZ();
	chunk_data[z * 32 + x] = data;
	chunk_data_compression[z * 32 + x] = compression;
}

/**
 * This method tries to load a chunk from the region data and returns a status.
 */
int RegionFile::loadChunk(const ChunkPos& pos, Chunk& chunk) {
	// unrotate the chunk position,
	// because the chunks are stored internally with their original positions
	ChunkPos unrotated = pos;
	if (rotation)
		unrotated.rotate(4 - rotation);

	int x = unrotated.getLocalX();
	int z = unrotated.getLocalZ();

	// check if the chunk exists
	if (chunk_data[z*32 + x].size() == 0)
		return CHUNK_DOES_NOT_EXIST;

	// get compression type and size of the data
	uint8_t compression = chunk_data_compression[z*32 + x];
	nbt::Compression comp = nbt::Compression::NO_COMPRESSION;
	if (compression == 1)
		comp = nbt::Compression::GZIP;
	else if (compression == 2)
		comp = nbt::Compression::ZLIB;
	int size = chunk_data[z*32 + x].size();

	// set the chunk rotation
	chunk.setRotation(rotation);
	// try to load the chunk
	try {
		if (!chunk.readNBT(reinterpret_cast<char*>(&chunk_data[z*32 + x][0]), size, comp))
			return CHUNK_DATA_INVALID;
	} catch (const nbt::NBTError& err) {
		std::cout << "Error: Unable to read chunk at " << pos << " : " << err.what() << std::endl;
		return CHUNK_NBT_ERROR;
	}
	return CHUNK_OK;
}

}
}
