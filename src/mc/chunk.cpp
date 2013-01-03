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

#include "mc/chunk.h"

#include "mc/nbt.h"

#include <iostream>
#include <math.h>

namespace mapcrafter {
namespace mc {

Chunk::Chunk()
		: pos(42, 42) {
	clear();
}

Chunk::~Chunk() {
}

/**
 * Reads the chunk from (compressed) nbt data.
 */
bool Chunk::readNBT(const char* data, size_t len, nbt::CompressionType compression) {
	clear();

	nbt::NBTFile nbt;
	nbt.readNBT(data, len, compression);

	// find "level" tag
	nbt::TagCompound* level = nbt.findTag<nbt::TagCompound>("Level", nbt::TAG_COMPOUND);
	if (level == NULL) {
		std::cerr << "Warning: Corrupt chunk (No level tag)!" << std::endl;
		return false;
	}

	// then find x/z pos of the chunk
	nbt::TagInt* xpos = level->findTag<nbt::TagInt>("xPos", nbt::TAG_INT);
	nbt::TagInt* zpos = level->findTag<nbt::TagInt>("zPos", nbt::TAG_INT);
	if (xpos == NULL || zpos == NULL) {
		std::cerr << "Warning: Corrupt chunk (No x/z position found)!" << std::endl;
		return false;
	}
	pos = ChunkPos(xpos->payload, zpos->payload);

	// find sections list
	nbt::TagList* tagSections = level->findTag<nbt::TagList>("Sections", nbt::TAG_LIST);
	if (tagSections == NULL || tagSections->tag_type != nbt::TAG_COMPOUND) {
		std::cerr << "Warning: Corrupt chunk at " << pos.x << ":" << pos.z
		        << " (No valid sections list found)!" << std::endl;
		return false;
	}

	// go through all sections
	for (std::vector<nbt::NBTTag*>::const_iterator it = tagSections->payload.begin();
	        it != tagSections->payload.end(); ++it) {
		nbt::TagCompound* tagSection = (nbt::TagCompound*) *it;
		nbt::TagByte* y = tagSection->findTag<nbt::TagByte>("Y", nbt::TAG_BYTE);
		nbt::TagByteArray* blocks = tagSection->findTag<nbt::TagByteArray>("Blocks",
		        nbt::TAG_BYTE_ARRAY);
		nbt::TagByteArray* data = tagSection->findTag<nbt::TagByteArray>("Data",
		        nbt::TAG_BYTE_ARRAY);
		// make sure section is valid
		if (y == NULL || blocks == NULL || data == NULL || blocks->payload.size() != 4096
		        || data->payload.size() != 2048)
			continue;

		// add it
		ChunkSection section;
		section.y = y->payload;
		std::copy(blocks->payload.begin(), blocks->payload.end(), section.blocks);
		std::copy(data->payload.begin(), data->payload.end(), section.data);
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

/**
 * Returns the block id at a position.
 */
uint8_t Chunk::getBlockID(const LocalBlockPos& pos) const {
	int section = pos.y / 16;
	if (section_offsets[section] == -1)
		return 0;
	// FIXME sometimes this happens, fix this
	if (sections.size() > 16 || sections.size() <= section_offsets[section]) {
		return 0;
	}
	int offset = ((pos.y % 16) * 16 + pos.z) * 16 + pos.x;
	return sections[section_offsets[section]].blocks[offset];
}

/**
 * Returns the block data at a position.
 */
uint8_t Chunk::getBlockData(const LocalBlockPos& pos) const {
	int section = pos.y / 16;
	if (section_offsets[section] == -1)
		return 0;
	int offset = ((pos.y % 16) * 16 + pos.z) * 16 + pos.x;
	if ((offset % 2) == 0)
		return sections[section_offsets[section]].data[offset / 2] & 0xf;
	return (sections[section_offsets[section]].data[offset / 2] >> 4) & 0x0f;
}

const ChunkPos& Chunk::getPos() const {
	return pos;
}

}
}
