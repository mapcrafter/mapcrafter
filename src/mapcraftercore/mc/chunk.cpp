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

#include "chunk.h"

#include "blockstate.h"

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
	: chunkpos(42, 42), rotation(0), terrain_populated(false) {
	clear();
}

Chunk::~Chunk() {
}

void Chunk::setRotation(int rotation) {
	this->rotation = rotation;
}

void Chunk::setWorldCrop(const WorldCrop& world_crop) {
	this->world_crop = world_crop;
}

int Chunk::positionToKey(int x, int z, int y) const {
	return y + 256 * (x + 16 * z);
}

bool Chunk::readNBT(mc::BlockStateRegistry& block_registry, const char* data, size_t len,
		nbt::Compression compression) {
	clear();

	nbt::NBTFile nbt;
	nbt.readNBT(data, len, compression);

	// find "level" tag
	if (!nbt.hasTag<nbt::TagCompound>("Level")) {
		LOG(ERROR) << "Corrupt chunk: No level tag found!";
		return false;
	}
	const nbt::TagCompound& level = nbt.findTag<nbt::TagCompound>("Level");

	// then find x/z pos of the chunk
	if (!level.hasTag<nbt::TagInt>("xPos") || !level.hasTag<nbt::TagInt>("zPos")) {
		LOG(ERROR) << "Corrupt chunk: No x/z position found!";
		return false;
	}
	chunkpos_original = ChunkPos(level.findTag<nbt::TagInt>("xPos").payload,
	                             level.findTag<nbt::TagInt>("zPos").payload);
	chunkpos = chunkpos_original;
	if (rotation)
		chunkpos.rotate(rotation);

	// now we have the original chunk position:
	// check whether this chunk is completely contained within the cropped world
	chunk_completely_contained = world_crop.isChunkCompletelyContained(chunkpos_original);

	/*
	if (level.hasTag<nbt::TagByte>("TerrainPopulated"))
		terrain_populated = level.findTag<nbt::TagByte>("TerrainPopulated").payload;
	else
		LOG(ERROR) << "Corrupt chunk " << chunkpos << ": No terrain populated tag found!";
	*/

	if (level.hasArray<nbt::TagIntArray>("Biomes", 256)) {
		const nbt::TagIntArray& biomes_tag = level.findTag<nbt::TagIntArray>("Biomes");
		std::copy(biomes_tag.payload.begin(), biomes_tag.payload.end(), biomes);
	} else {
		LOG(ERROR) << "Corrupt chunk " << chunkpos << ": No biome data found!";
	}

	const nbt::TagList& tile_entities_tag = level.findTag<nbt::TagList>("TileEntities");

	if (tile_entities_tag.tag_type == nbt::TagCompound::TAG_TYPE) {
		// go through all entities
		for (auto it = tile_entities_tag.payload.begin(); it != tile_entities_tag.payload.end(); ++it) {
			const nbt::TagCompound &entity = (*it)->cast<nbt::TagCompound>();
			std::string id = entity.findTag<nbt::TagString>("id").payload; // Not an integer, e.g. for beds: 'minecraft:bed'
			mc::BlockPos pos(
					entity.findTag<nbt::TagInt>("x").payload,
					entity.findTag<nbt::TagInt>("z").payload,
					entity.findTag<nbt::TagInt>("y").payload
			);

			if (id == "minecraft:bed") { // bed, stored as a string here
				uint16_t color = (uint16_t) entity.findTag<nbt::TagInt>("color").payload;
				insertExtraData(pos, color);
			}
		}
	}

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
		//		|| !section_tag.hasArray<nbt::TagByteArray>("Blocks", 4096)
		//		|| !section_tag.hasArray<nbt::TagByteArray>("Data", 2048)
				|| !section_tag.hasArray<nbt::TagByteArray>("BlockLight", 2048)
				|| !section_tag.hasArray<nbt::TagByteArray>("SkyLight", 2048)
				|| !section_tag.hasArray<nbt::TagLongArray>("BlockStates")
				|| !section_tag.hasTag<nbt::TagList>("Palette"))
			continue;
		
		const nbt::TagByte& y = section_tag.findTag<nbt::TagByte>("Y");
		if (y.payload >= CHUNK_HEIGHT)
			continue;
		//const nbt::TagByteArray& blocks = section_tag.findTag<nbt::TagByteArray>("Blocks");
		//const nbt::TagByteArray& data = section_tag.findTag<nbt::TagByteArray>("Data");
	
		const nbt::TagLongArray& blockstates = section_tag.findTag<nbt::TagLongArray>("BlockStates");
	
		int bits_per_block = blockstates.payload.size() * 64 / (16*16*16);
		int blocks_per_long = 64 / bits_per_block;
		//if (bits_per_block == 4 || bits_per_block == 5 || bits_per_block == 6) {
		//	continue;
		//}
		//LOG(INFO) << blockstates.payload.size() << " => " << bits_per_block;
		
		const nbt::TagList& palette = section_tag.findTag<nbt::TagList>("Palette");
		std::vector<mc::BlockState> palette_blockstates(palette.payload.size());
		std::vector<uint16_t> palette_lookup(palette.payload.size());

		size_t i = 0;
		for (auto it2 = palette.payload.begin(); it2 != palette.payload.end(); ++it2, ++i) {
			const nbt::TagCompound& entry = (*it2)->cast<nbt::TagCompound>();
			const nbt::TagString& name = entry.findTag<nbt::TagString>("Name");
			
			mc::BlockState block(name.payload);
			if (entry.hasTag<nbt::TagCompound>("Properties")) {
				const nbt::TagCompound& properties = entry.findTag<nbt::TagCompound>("Properties");
				for (auto it3 = properties.payload.begin(); it3 != properties.payload.end(); ++it3) {
					std::string key = it3->first;
					std::string value = it3->second->cast<nbt::TagString>().payload;
					block.setProperty(key, value);
				}
			}
			palette_blockstates[i] = block;
			palette_lookup[i] = block_registry.getBlockID(block);
			//LOG(INFO) << i << ": " << block.getName() << " " << block.getVariantDescription();
		}

		const nbt::TagByteArray& block_light = section_tag.findTag<nbt::TagByteArray>("BlockLight");
		const nbt::TagByteArray& sky_light = section_tag.findTag<nbt::TagByteArray>("SkyLight");

		// create a ChunkSection-object
		ChunkSection section;
		section.y = y.payload;

		for (size_t i = 0; i < 16*16*16; i++) {
			uint64_t long_index = i / blocks_per_long;
			uint64_t bit_index = (i % blocks_per_long) * bits_per_block;
			if (bit_index + bits_per_block > 64) {
				LOG(WARNING) << "blah";
				continue;
			}

			uint64_t value = blockstates.payload[long_index];
			uint64_t mask = ((1LL << (uint64_t) bits_per_block) - 1LL) << bit_index;
			uint64_t palette_index = (value & mask) >> bit_index;
			if (palette_index >= palette_blockstates.size()) {
				LOG(ERROR) << "Incorrectly parsed block ID at bit index " << bit_index << ": "
					<< value << " (max is " << palette_blockstates.size()-1
					<< " with " << bits_per_block << " bits per block)";
				LOG(ERROR) << "value: " << value << "   mask: " << mask << "  value&mask: " << (value&mask);
				break;
			}
			assert(palette_index < palette_blockstates.size());
			section.block_ids[i] = palette_lookup[palette_index];
		}

		//std::copy(blocks.payload.begin(), blocks.payload.end(), section.blocks);
		/*
		if (!section_tag.hasArray<nbt::TagByteArray>("Add", 2048))
			std::fill(&section.add[0], &section.add[2048], 0);
		else {
			const nbt::TagByteArray& add = section_tag.findTag<nbt::TagByteArray>("Add");
			std::copy(add.payload.begin(), add.payload.end(), section.add);
		}
		std::copy(data.payload.begin(), data.payload.end(), section.data);
		*/
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

uint16_t Chunk::getBlockID(const LocalBlockPos& pos, bool force) const {
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
	uint16_t id = sections[section_offsets[section]].blocks[offset] + (add << 8);
	if (!force && world_crop.hasBlockMask()) {
		const BlockMask* mask = world_crop.getBlockMask();
		BlockMask::BlockState block_state = mask->getBlockState(id);
		if (block_state == BlockMask::BlockState::COMPLETELY_HIDDEN)
			return 0;
		else if (block_state == BlockMask::BlockState::COMPLETELY_SHOWN)
			return id;
		if (mask->isHidden(id, getBlockData(pos, true)))
			return 0;
	}
	return id;
}

bool Chunk::checkBlockWorldCrop(int x, int z, int y) const {
	// first of all check if we should crop unpopulated chunks
	if (!terrain_populated && world_crop.hasCropUnpopulatedChunks())
		return false;
	// now about the actual world cropping:
	// get the global position of the block, with the original world rotation
	BlockPos global_pos = LocalBlockPos(x, z, y).toGlobalPos(chunkpos_original);
	// check whether the block is contained in the y-bounds.
	if (!world_crop.isBlockContainedY(global_pos))
		return false;
	// only check x/z-bounds if the chunk is not completely contained
	if (!chunk_completely_contained && !world_crop.isBlockContainedXZ(global_pos))
		return false;
	return true;
}

uint8_t Chunk::getData(const LocalBlockPos& pos, int array, bool force) const {
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

	uint8_t data = 0;
	// calculate the offset and get the block data
	int offset = ((pos.y % 16) * 16 + z) * 16 + x;
	// handle bottom/top nibble
	if ((offset % 2) == 0)
		data = sections[section_offsets[section]].getArray(array)[offset / 2] & 0xf;
	else
		data = (sections[section_offsets[section]].getArray(array)[offset / 2] >> 4) & 0x0f;
	if (!force && world_crop.hasBlockMask()) {
		const BlockMask* mask = world_crop.getBlockMask();
		if (mask->isHidden(getBlockID(pos, true), data))
			return array == 2 ? 15 : 0;
	}
	return data;
}

uint16_t Chunk::getBlockExtraData(const LocalBlockPos& pos, uint16_t id) const {
	if (id == 26) {
		return getExtraData(pos, 14); // Default is red
	}

	return 0;
}

uint8_t Chunk::getBlockData(const LocalBlockPos& pos, bool force) const {
	return getData(pos, 0, force);
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

void Chunk::insertExtraData(const LocalBlockPos &pos, uint16_t extra_data) {
	int key = positionToKey(pos.x, pos.z, pos.y);
	std::pair<int,uint16_t> pair (key, extra_data);
	extra_data_map.insert(pair);
}

uint16_t Chunk::getExtraData(const LocalBlockPos &pos, uint16_t default_value) const {
	int x = pos.x;
	int z = pos.z;
	if (rotation)
		rotateBlockPos(x, z, rotation);
	int key = positionToKey(x, z, pos.y);

	auto result = extra_data_map.find(key);
	if (result == extra_data_map.end()) {
		// Not found, possibly from an old world
		// Default value is 14 = red
		return default_value;
	}

	return result->second;
}

}
}
