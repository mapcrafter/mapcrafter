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

#include "chunk.h"

#include <iostream>
#include <cmath>

namespace mapcrafter {
namespace mc {

Chunk::Chunk()
		: pos(42, 42), rotation(0) {
	clear();
}

Chunk::~Chunk() {
}

void Chunk::setRotation(int rotation) {
	this->rotation = rotation;
}

/**
 * Reads the chunk from (compressed) nbt data.
 */
bool Chunk::readNBT(const char* data, size_t len, nbt::Compression compression) {
	clear();

	nbt::NBTFile nbt;
	nbt.readNBT(data, len, compression);

	// find "level" tag
	if (!nbt.hasTag<nbt::TagCompound>("Level")) {
		std::cerr << "Warning: Corrupt chunk (No level tag)!" << std::endl;
		return false;
	}
	nbt::TagCompound& level = nbt.findTag<nbt::TagCompound>("Level");

	// then find x/z pos of the chunk
	if (!level.hasTag<nbt::TagInt>("xPos") || !level.hasTag<nbt::TagInt>("zPos")) {
		std::cerr << "Warning: Corrupt chunk (No x/z position found)!" << std::endl;
		return false;
	}
	pos = ChunkPos(level.findTag<nbt::TagInt>("xPos").payload,
				   level.findTag<nbt::TagInt>("zPos").payload);
	if (rotation)
		pos.rotate(rotation);

	if (level.hasArray<nbt::TagByteArray>("Biomes", 256)) {
		nbt::TagByteArray& biomes_tag = level.findTag<nbt::TagByteArray>("Biomes");
		std::copy(biomes_tag.payload.begin(), biomes_tag.payload.end(), biomes);
	} else
		std::cerr << "Warning: Corrupt chunk at " << pos.x << ":" << pos.z
				<< " (No biome data found)!" << std::endl;

	// find sections list
	// I already saw (empty) chunks from the end with TagByte instead of TagCompound
	// in this list, ignore them, they are empty
	if (!level.hasList<nbt::TagCompound>("Sections")
			&& !level.hasList<nbt::TagByte>("Sections", 0)) {
		std::cerr << "Warning: Corrupt chunk at " << pos.x << ":" << pos.z
			<< " (No valid sections list found)!" << std::endl;
		return false;
	}
	
	nbt::TagList& sections_tag = level.findTag<nbt::TagList>("Sections");
	if (sections_tag.tag_type != nbt::TagCompound::TAG_TYPE)
		return true;

	// go through all sections
	for (auto it = sections_tag.payload.begin(); it != sections_tag.payload.end(); ++it) {
		nbt::TagCompound& section_tag = (*it)->cast<nbt::TagCompound>();
		
		// make sure section is valid
		if (!section_tag.hasTag<nbt::TagByte>("Y")
				|| !section_tag.hasArray<nbt::TagByteArray>("Blocks", 4096)
				|| !section_tag.hasArray<nbt::TagByteArray>("Data", 2048)
				|| !section_tag.hasArray<nbt::TagByteArray>("BlockLight", 2048)
				|| !section_tag.hasArray<nbt::TagByteArray>("SkyLight", 2048))
			continue;
		
		nbt::TagByte& y = section_tag.findTag<nbt::TagByte>("Y");
		nbt::TagByteArray& blocks = section_tag.findTag<nbt::TagByteArray>("Blocks");
		nbt::TagByteArray& data = section_tag.findTag<nbt::TagByteArray>("Data");

		nbt::TagByteArray& block_light = section_tag.findTag<nbt::TagByteArray>("BlockLight");
		nbt::TagByteArray& sky_light = section_tag.findTag<nbt::TagByteArray>("SkyLight");

		// add it
		ChunkSection section;
		section.y = y.payload;
		std::copy(blocks.payload.begin(), blocks.payload.end(), section.blocks);
		if (!section_tag.hasArray<nbt::TagByteArray>("Add", 2048))
			std::fill(&section.add[0], &section.add[2048], 0);
		else {
			nbt::TagByteArray& add = section_tag.findTag<nbt::TagByteArray>("Add");
			std::copy(add.payload.begin(), add.payload.end(), section.add);
		}
		std::copy(data.payload.begin(), data.payload.end(), section.data);

		std::copy(block_light.payload.begin(), block_light.payload.end(), section.block_light);
		std::copy(sky_light.payload.begin(), sky_light.payload.end(), section.sky_light);
		// set the position of this section
		section_offsets[section.y] = sections.size();
		sections.push_back(section);
	}

	return true;
}

/**
 * Clears the whole chunk data.
 */
void Chunk::clear() {
	sections.clear();
	for (int i = 0; i < 16; i++)
		section_offsets[i] = -1;
}

bool Chunk::hasSection(int section) const {
	return section_offsets[section] != -1;
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

/**
 * Returns the block id at a position.
 */
uint16_t Chunk::getBlockID(const LocalBlockPos& pos) const {
	int section = pos.y / 16;
	if (section_offsets[section] == -1)
		return 0;
	// FIXME sometimes this happens, fix this
	if (sections.size() > 16 || sections.size() <= (unsigned) section_offsets[section]) {
		return 0;
	}

	int x = pos.x;
	int z = pos.z;
	if (rotation)
		rotateBlockPos(x, z, rotation);

	int offset = ((pos.y % 16) * 16 + z) * 16 + x;
	uint16_t add = 0;
	if ((offset % 2) == 0)
		add = sections[section_offsets[section]].add[offset / 2] & 0xf;
	else
		add = (sections[section_offsets[section]].add[offset / 2] >> 4) & 0x0f;
	return sections[section_offsets[section]].blocks[offset] + (add << 8);
}

uint8_t Chunk::getData(const LocalBlockPos& pos, int array) const {
	int section = pos.y / 16;
	if (section_offsets[section] == -1) {
		if (array == 2)
			return 15;
		return 0;
	}

	int x = pos.x;
	int z = pos.z;
	if (rotation)
		rotateBlockPos(x, z, rotation);

	int offset = ((pos.y % 16) * 16 + z) * 16 + x;
	if ((offset % 2) == 0)
		return sections[section_offsets[section]].getArray(array)[offset / 2] & 0xf;
	return (sections[section_offsets[section]].getArray(array)[offset / 2] >> 4) & 0x0f;
}

/**
 * Returns the block data at a position.
 */
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
	return pos;
}

}
}
