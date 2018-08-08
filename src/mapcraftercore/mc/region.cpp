/*
 * Copyright 2012-2016 Moritz Hilscher
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

#include "region.h"

#include "blockstate.h"

#include <cstdlib>
#include <fstream>

namespace mapcrafter {
namespace mc {

RegionFile::RegionFile()
	: rotation(0) {
}

RegionFile::RegionFile(const std::string& filename)
	: filename(filename), rotation(0) {
	regionpos_original = RegionPos::byFilename(filename);
	regionpos = regionpos_original;
}

RegionFile::~RegionFile() {
}

bool RegionFile::readHeaders(std::ifstream& file, uint32_t chunk_offsets[1024]) {
	if (!file)
		return false;

	containing_chunks.clear();
	for (int i = 0; i < 1024; i++) {
		chunk_offsets[i] = 0;
		chunk_exists[i] = false;
		chunk_timestamps[i] = 0;
		chunk_data_compression[i] = 0;
	}

	file.seekg(0, std::ios::end);
	size_t filesize = file.tellg();
	file.seekg(0, std::ios::beg);
	// make sure the region file has a header
	if (filesize < 8192) {
		LOG(ERROR) << "Corrupt region '" << filename << "': Header is too short.";
		return false;
	}

	for (int x = 0; x < 32; x++) {
		for (int z = 0; z < 32; z++) {
			file.seekg(4 * (x + z * 32), std::ios::beg);
			int tmp;
			file.read(reinterpret_cast<char*>(&tmp), 4);
			if (tmp == 0)
				continue;
			uint32_t offset = util::bigEndian32(tmp << 8) * 4096;
			if (filesize < offset + 5) {
				LOG(ERROR) << "Corrupt region '" << filename << "': Invalid offset of chunk "
						<< x << ":" << z << ".";
				return false;
			}
			//uint8_t sectors = ((uint8_t*) &tmp)[3];

			file.seekg(4096, std::ios::cur);
			uint32_t timestamp;
			file.read(reinterpret_cast<char*>(&timestamp), 4);
			timestamp = util::bigEndian32(timestamp);

			// get the original (not rotated) position of the chunk
			ChunkPos chunkpos(x + regionpos_original.x * 32, z + regionpos_original.z * 32);
			// check if this chunk is not cropped
			if (!world_crop.isChunkContained(chunkpos))
				continue;

			// now rotate this chunk position for the public set with available chunks
			if (rotation)
				chunkpos.rotate(rotation);
			
			chunk_exists[z * 32 + x] = true;
			containing_chunks.insert(chunkpos);

			// set offset and timestamp of this chunk
			// now with the original coordinates again
			chunk_offsets[z * 32 + x] = offset;
			chunk_timestamps[z * 32 + x] = timestamp;
		}
	}
	return true;
}

size_t RegionFile::getChunkIndex(const mc::ChunkPos& chunkpos) const {
	ChunkPos unrotated = chunkpos;
	if (rotation)
		unrotated.rotate(4 - rotation);
	return unrotated.getLocalZ() * 32 + unrotated.getLocalX();
}

void RegionFile::setRotation(int rotation) {
	this->rotation = rotation;

	// TODO properly handle this
	if (rotation) {
		regionpos = regionpos_original;
		regionpos.rotate(rotation);
	}
}

void RegionFile::setWorldCrop(const WorldCrop& world_crop) {
	this->world_crop = world_crop;
}

bool RegionFile::read() {
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	uint32_t chunk_offsets[1024];
	if (!readHeaders(file, chunk_offsets))
		return false;
	file.seekg(0, std::ios::end);
	size_t filesize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> regiondata(filesize);
	file.read(reinterpret_cast<char*>(&regiondata[0]), filesize);

	for (int i = 0; i < 1024; i++) {
		// get the offsets, where the chunk data starts
		int offset = chunk_offsets[i];
		if (offset == 0)
			continue;

		// i = x + z * 32
		int x = i % 32;
		int z = (i - x) / 32;

		// get data size and compression type
		uint32_t size = *(reinterpret_cast<uint32_t*>(&regiondata[offset]));
		if (size == 0) {
			LOG(ERROR)  << "Corrupt region '" << filename << "': Size of chunk "
				<< x << ":" << z << " is zero.";
			return false;
		}
		size = util::bigEndian32(size) - 1;
		uint8_t compression = regiondata[offset + 4];
		if (filesize < offset + 5 + size) {
			LOG(ERROR) << "Corrupt region '" << filename << "': Invalid size of chunk "
				<< x << ":" << z << ".";
			return false;
		}

		chunk_data_compression[i] = compression;
		chunk_data[i].resize(size);
		std::copy(&regiondata[offset+5], &regiondata[offset+5+size], chunk_data[i].begin());
	}

	return true;
}

bool RegionFile::readOnlyHeaders() {
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	uint32_t chunk_offsets[1024];
	return readHeaders(file, chunk_offsets);
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
		uint32_t offset_big_endian = util::bigEndian32(offsets[i]) >> 8;
		out_header.write(reinterpret_cast<char*>(&offset_big_endian), 4);
	}

	for (int i = 0; i < 1024; i++) {
		uint32_t timestamp_big_endian = util::bigEndian32(chunk_timestamps[i]);
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
	return chunk_exists[getChunkIndex(chunk)];
}

uint32_t RegionFile::getChunkTimestamp(const ChunkPos& chunk) const {
	return chunk_timestamps[getChunkIndex(chunk)];
}

void RegionFile::setChunkTimestamp(const ChunkPos& chunk, uint32_t timestamp) {
	chunk_timestamps[getChunkIndex(chunk)] = timestamp;
}

const std::vector<uint8_t>& RegionFile::getChunkData(const ChunkPos& chunk) const {
	return chunk_data[getChunkIndex(chunk)];
}

uint8_t RegionFile::getChunkDataCompression(const ChunkPos& chunk) const {
	return chunk_data_compression[getChunkIndex(chunk)];
}

void RegionFile::setChunkData(const ChunkPos& chunk, const std::vector<uint8_t>& data,
		uint8_t compression) {
	size_t index = getChunkIndex(chunk);
	chunk_data[index] = data;
	chunk_data_compression[index] = compression;

	if (data.size() == 0) {
		chunk_exists[index] = false;
		containing_chunks.erase(chunk);
	} else {
		chunk_exists[index] = true;
		containing_chunks.insert(chunk);
	}
}

/**
 * This method tries to load a chunk from the region data and returns a status.
 */
int RegionFile::loadChunk(const ChunkPos& pos, BlockStateRegistry& block_registry, Chunk& chunk) {
	int index = getChunkIndex(pos);

	// check if the chunk exists
	if (chunk_data[index].size() == 0)
		return CHUNK_DOES_NOT_EXIST;

	// get compression type and size of the data
	uint8_t compression = chunk_data_compression[index];
	nbt::Compression comp = nbt::Compression::NO_COMPRESSION;
	if (compression == 1)
		comp = nbt::Compression::GZIP;
	else if (compression == 2)
		comp = nbt::Compression::ZLIB;
	size_t size = chunk_data[index].size();

	// set the chunk rotation
	chunk.setRotation(rotation);
	chunk.setWorldCrop(world_crop);
	// try to load the chunk
	try {
		if (!chunk.readNBT(block_registry, reinterpret_cast<char*>(&chunk_data[index][0]), size, comp))
			return CHUNK_DATA_INVALID;
	} catch (const nbt::NBTError& err) {
		LOG(ERROR) << "Unable to read chunk at " << pos << " : " << err.what();
		return CHUNK_NBT_ERROR;
	}
	return CHUNK_OK;
}

}
}
