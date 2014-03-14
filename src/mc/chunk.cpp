/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "chunk.h"

#include <cmath>
#include <iostream>

namespace mapcrafter {
namespace mc {

const uint8_t* ChunkSection::getArray(int i) const {
	if (i == 0)
		return data;
	else if (i == 1)
		return block_light;
	else
		return sky_light;
}

Chunk::Chunk()
	: chunkpos(42, 42), rotation(0) {
	clear();
}

Chunk::~Chunk() {
}

void Chunk::setRotation(int rotation) {
	this->rotation = rotation;
}

void Chunk::setWorldCrop(const WorldCrop& worldcrop) {
	this->worldcrop = worldcrop;
}

bool Chunk::readNBT(const char* data, size_t len, nbt::Compression compression) {
	clear();

	nbt::NBTFile nbt;
	nbt.readNBT(data, len, compression);

	// find "level" tag
	if (!nbt.hasTag<nbt::TagCompound>("Level")) {
		std::cerr << "Warning: Corrupt chunk (No level tag)!" << std::endl;
		return false;
	}
	const nbt::TagCompound& level = nbt.findTag<nbt::TagCompound>("Level");

	// then find x/z pos of the chunk
	if (!level.hasTag<nbt::TagInt>("xPos") || !level.hasTag<nbt::TagInt>("zPos")) {
		std::cerr << "Warning: Corrupt chunk (No x/z position found)!" << std::endl;
		return false;
	}
	chunkpos_original = ChunkPos(level.findTag<nbt::TagInt>("xPos").payload,
	                             level.findTag<nbt::TagInt>("zPos").payload);
	chunkpos = chunkpos_original;
	if (rotation)
		chunkpos.rotate(rotation);

	// now we have the original chunk position:
	// check whether this chunk is completely contained within the cropped world
	chunk_completely_contained = worldcrop.isChunkCompletelyContained(chunkpos_original);

	if (level.hasArray<nbt::TagByteArray>("Biomes", 256)) {
		const nbt::TagByteArray& biomes_tag = level.findTag<nbt::TagByteArray>("Biomes");
		std::copy(biomes_tag.payload.begin(), biomes_tag.payload.end(), biomes);
	} else
		std::cerr << "Warning: Corrupt chunk at " << chunkpos.x << ":" << chunkpos.z
				<< " (No biome data found)!" << std::endl;

	// find sections list
	// ignore it if section list does not exist, can happen sometimes with the empty
	// chunks of the end
	if (!level.hasList<nbt::TagCompound>("Sections"))
		return true;
	
	const nbt::TagList& sections_tag = level.findTag<nbt::TagList>("Sections");
	if (sections_tag.tag_type != nbt::TagCompound::TAG_TYPE)
		return true;

	// go through all sections
	for (auto it = sections_tag.payload.begin(); it != sections_tag.payload.end(); ++it) {
		const nbt::TagCompound& section_tag = (*it)->cast<nbt::TagCompound>();
		
		// make sure section is valid
		if (!section_tag.hasTag<nbt::TagByte>("Y")
				|| !section_tag.hasArray<nbt::TagByteArray>("Blocks", 4096)
				|| !section_tag.hasArray<nbt::TagByteArray>("Data", 2048)
				|| !section_tag.hasArray<nbt::TagByteArray>("BlockLight", 2048)
				|| !section_tag.hasArray<nbt::TagByteArray>("SkyLight", 2048))
			continue;
		
		const nbt::TagByte& y = section_tag.findTag<nbt::TagByte>("Y");
		if (y.payload >= CHUNK_HEIGHT)
			continue;
		const nbt::TagByteArray& blocks = section_tag.findTag<nbt::TagByteArray>("Blocks");
		const nbt::TagByteArray& data = section_tag.findTag<nbt::TagByteArray>("Data");

		const nbt::TagByteArray& block_light = section_tag.findTag<nbt::TagByteArray>("BlockLight");
		const nbt::TagByteArray& sky_light = section_tag.findTag<nbt::TagByteArray>("SkyLight");

		// create a ChunkSection-object
		ChunkSection section;
		section.y = y.payload;
		std::copy(blocks.payload.begin(), blocks.payload.end(), section.blocks);
		if (!section_tag.hasArray<nbt::TagByteArray>("Add", 2048))
			std::fill(&section.add[0], &section.add[2048], 0);
		else {
			const nbt::TagByteArray& add = section_tag.findTag<nbt::TagByteArray>("Add");
			std::copy(add.payload.begin(), add.payload.end(), section.add);
		}
		std::copy(data.payload.begin(), data.payload.end(), section.data);
		std::copy(block_light.payload.begin(), block_light.payload.end(), section.block_light);
		std::copy(sky_light.payload.begin(), sky_light.payload.end(), section.sky_light);

		// add this section to the section list
		section_offsets[section.y] = sections.size();
		sections.push_back(section);
	}

	return true;
}

void Chunk::clear() {
	sections.clear();
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		section_offsets[i] = -1;
}

bool Chunk::hasSection(int section) const {
	return section < CHUNK_HEIGHT && section_offsets[section] != -1;
}

void rotateBlockPos(int& x, int& z, int rotation) {
	int nx = x, nz = z;
	for (int i = 0; i < rotation; i++) {
		nx = z;
		nz = 15 - x;
		x = nx;
		z = nz;
	}
}

uint16_t Chunk::getBlockID(const LocalBlockPos& pos) const {
	// at first find out the section and check if it's valid and contained
	int section = pos.y / 16;
	if (section >= CHUNK_HEIGHT || section_offsets[section] == -1)
		return 0;
	// FIXME sometimes this happens, fix this
	//if (sections.size() > 16 || sections.size() <= (unsigned) section_offsets[section]) {
	//	return 0;
	//}

	// if rotated: rotate position to position with original rotation
	int x = pos.x;
	int z = pos.z;
	if (rotation)
		rotateBlockPos(x, z, rotation);

	// check whether this block is really rendered
	if (!checkBlockWorldCrop(x, z, pos.y))
		return 0;

	// calculate the offset and get the block ID
	// and don't forget the add data
	int offset = ((pos.y % 16) * 16 + z) * 16 + x;
	uint16_t add = 0;
	if ((offset % 2) == 0)
		add = sections[section_offsets[section]].add[offset / 2] & 0xf;
	else
		add = (sections[section_offsets[section]].add[offset / 2] >> 4) & 0x0f;
	return sections[section_offsets[section]].blocks[offset] + (add << 8);
}

bool Chunk::checkBlockWorldCrop(int x, int z, int y) const {
	// at first get the global position of the block, with the original world rotation
	BlockPos global_pos = LocalBlockPos(x, z, y).toGlobalPos(chunkpos_original);
	// check whether the block is contained in the y-bounds.
	if (!worldcrop.isBlockContainedY(global_pos))
		return false;
	// only check x/z-bounds if the chunk is not completely contained
	if (!chunk_completely_contained && !worldcrop.isBlockContainedXZ(global_pos))
		return false;
	return true;
}

uint8_t Chunk::getData(const LocalBlockPos& pos, int array) const {
	// at first find out the section and check if it's valid and contained
	int section = pos.y / 16;
	if (section >= CHUNK_HEIGHT || section_offsets[section] == -1)
		// not existing sections should always have skylight
		return array == 2 ? 15 : 0;

	// if rotated: rotate position to position with original rotation
	int x = pos.x;
	int z = pos.z;
	if (rotation)
		rotateBlockPos(x, z, rotation);

	// check whether this block is really rendered
	if (!checkBlockWorldCrop(x, z, pos.y))
		return array == 2 ? 15 : 0;

	// calculate the offset and get the block data
	int offset = ((pos.y % 16) * 16 + z) * 16 + x;
	// handle bottom/top nibble
	if ((offset % 2) == 0)
		return sections[section_offsets[section]].getArray(array)[offset / 2] & 0xf;
	return (sections[section_offsets[section]].getArray(array)[offset / 2] >> 4) & 0x0f;
}

uint8_t Chunk::getBlockData(const LocalBlockPos& pos) const {
	return getData(pos, 0);
}

uint8_t Chunk::getBlockLight(const LocalBlockPos& pos) const {
	return getData(pos, 1);
}

uint8_t Chunk::getSkyLight(const LocalBlockPos& pos) const {
	return getData(pos, 2);
}

uint8_t Chunk::getBiomeAt(const LocalBlockPos& pos) const {
	int x = pos.x;
	int z = pos.z;
	if (rotation)
		rotateBlockPos(x, z, rotation);

	return biomes[z * 16 + x];
}

const ChunkPos& Chunk::getPos() const {
	return chunkpos;
}

}
}
