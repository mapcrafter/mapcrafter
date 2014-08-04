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

#include "worldcrop.h"

#include "../util.h"

#include <cmath>
#include <sstream>

namespace mapcrafter {
namespace mc {

BlockMask::BlockMask() {
	// set all blocks to be shown by default
	block_states.resize(65536, BlockState::COMPLETELY_SHOWN);
	block_mask.set();
}

BlockMask::~BlockMask() {

}

void BlockMask::set(uint16_t id, bool shown) {
	for (size_t i = 0; i < 16; i++)
		block_mask[16 * id + i] = shown;
	updateBlockState(id);
}

void BlockMask::set(uint16_t id, uint8_t data, bool shown) {
	if (data >= 16)
		return;
	block_mask[16 * id + data] = shown;
	updateBlockState(id);
}

void BlockMask::set(uint16_t id, uint8_t data, uint8_t bitmask, bool shown) {
	// iterate through every possible block data values and check if (i % bitmask) == data
	for (uint8_t i = 0; i < 16; i++)
		if ((i & bitmask) == data)
			block_mask[16 * id + i] = shown;
	updateBlockState(id);
}

void BlockMask::setRange(uint16_t id1, uint16_t id2, bool shown) {
	for (size_t id = id1; id <= id2; id++)
		set(id, shown);
}

void BlockMask::setAll(bool shown) {
	if (shown) {
		block_mask.set();
		std::fill(block_states.begin(), block_states.end(), BlockState::COMPLETELY_SHOWN);
	} else {
		block_mask.reset();
		std::fill(block_states.begin(), block_states.end(), BlockState::COMPLETELY_HIDDEN);
	}
}

void BlockMask::loadFromString(const std::string& str) {
	// TL;DR: Parsing this in C++ is annoying
	std::stringstream ss(util::trim(str));
	std::string group;
	// go through the specified block groups and try to parse them...
	while (ss >> group) {
		// whether this group is to be shown/hidden
		bool shown = group[0] != '!';
		if (!shown)
			group = group.substr(1);
		// just try to convert parts of this block group
		// throw another exception with an error message in case anything is invalid
		try {
			if (group.find('-') != std::string::npos) {
				uint16_t id1 = util::as<uint16_t>(group.substr(0, group.find('-')));
				uint16_t id2 = util::as<uint16_t>(group.substr(group.find('-') + 1));
				setRange(id1, id2, shown);
			} else if (group.find(':') != std::string::npos) {
				std::string id_part = group.substr(0, group.find(':'));
				std::string data_part = group.substr(group.find(':') + 1);
				// bitmask defaults to 15 if not specified
				// means that exactly that id:data is selected
				std::string bitmask_part = "15";

				if (data_part.find('b') != std::string::npos) {
					bitmask_part = data_part.substr(data_part.find('b') + 1);
					data_part = data_part.substr(0, data_part.find('b'));
				}

				uint16_t id = util::as<uint16_t>(id_part);

				uint16_t data;
				data = util::as<uint16_t>(data_part);
				if (data >= 16)
					throw std::invalid_argument("Invalid data value '" + data_part
							+ "', data value is limited to four bits");

				uint16_t bitmask;
				bitmask = util::as<uint16_t>(bitmask_part);
				if (bitmask >= 16)
					throw std::invalid_argument("Invalid bitmask '" + bitmask_part
							+ "', bitmask is limited to four bits");

				set(id, data, bitmask, shown);
			} else {
				if (group == "*")
					setAll(shown);
				else
					set(util::as<uint16_t>(group), shown);
			}
		} catch (std::invalid_argument& exception) {
			throw std::invalid_argument("Invalid block group '" + group
					+ "' (" + exception.what() + ")");
		}
	}
}

const BlockMask::BlockState BlockMask::getBlockState(uint16_t id) const {
	return block_states[id];
}

bool BlockMask::isHidden(uint16_t id, uint8_t data) const {
	if (data >= 16)
		return false;
	return !block_mask[16 * id + data];
}

void BlockMask::updateBlockState(uint16_t id) {
	// copy state of blocks to separate bitset to make checking them all easier
	std::bitset<16> block;
	for (size_t i = 0; i < 16; i++)
		block[i] = block_mask[16 * id + i];

	if (block.all())
		block_states[id] = BlockState::COMPLETELY_SHOWN;
	else if (block.none())
		block_states[id] = BlockState::COMPLETELY_HIDDEN;
	else
		block_states[id] = BlockState::PARTIALLY_HIDDEN_SHOWN;
}

WorldCrop::WorldCrop()
	: type(RECTANGULAR), radius(0), crop_unpopulated_chunks(true) {
}

WorldCrop::~WorldCrop() {
}

void WorldCrop::setMinY(int value) {
	bounds_y.setMin(value);
}

void WorldCrop::setMaxY(int value) {
	bounds_y.setMax(value);
}

void WorldCrop::setMinX(int value) {
	bounds_x.setMin(value);
	bounds_chunk_x.setMin(util::floordiv(value, 16));
	bounds_region_x.setMin(util::floordiv(value, 512));

	type = RECTANGULAR;
}

void WorldCrop::setMaxX(int value) {
	bounds_x.setMax(value);
	bounds_chunk_x.setMax(util::floordiv(value, 16));
	bounds_region_x.setMax(util::floordiv(value, 512));

	type = RECTANGULAR;
}

void WorldCrop::setMinZ(int value) {
	bounds_z.setMin(value);
	bounds_chunk_z.setMin(util::floordiv(value, 16));
	bounds_region_z.setMin(util::floordiv(value, 512));

	type = RECTANGULAR;
}

void WorldCrop::setMaxZ(int value) {
	bounds_z.setMax(value);
	bounds_chunk_z.setMax(util::floordiv(value, 16));
	bounds_region_z.setMax(util::floordiv(value, 512));

	type = RECTANGULAR;
}

void WorldCrop::setCenter(const BlockPos& pos) {
	center = pos;
	type = CIRCULAR;
}

void WorldCrop::setRadius(int radius) {
	this->radius = radius;
	type = CIRCULAR;
}

bool WorldCrop::isRegionContained(const mc::RegionPos& region) const {
	if (type == RECTANGULAR) {
		// rectangular crop:
		// just check if the region is contained in the calculated bounds
		return bounds_region_x.contains(region.x) && bounds_region_z.contains(region.z);
	} else if (type == CIRCULAR) {
		// circular crop:
		// check roughly whether at least one block of the region is included
		// use the midpoint of the region and determine the distance to the center
		BlockPos region_center(region.x * 512 + 256, region.z * 512 + 256, 0);
		int dx = region_center.x - center.x;
		int dz = region_center.z - center.z;
		// and check whether it is at most radius + size of a region blocks away
		return (radius+512)*(radius+512) >= dx*dx + dz*dz;
	}

	return true;
}

bool WorldCrop::isChunkContained(const mc::ChunkPos& chunk) const {
	if (type == RECTANGULAR) {
		// rectangular crop:
		// just check if the chunk is contained in the calculated chunk bounds
		return bounds_chunk_x.contains(chunk.x) && bounds_chunk_z.contains(chunk.z);
	} else if (type == CIRCULAR) {
		// circular crop:
		// do the same thing doing with regions
		BlockPos chunk_center(chunk.x * 16 + 8, chunk.z * 16 + 8, 0);
		int dx = chunk_center.x - center.x;
		int dz = chunk_center.z - center.z;
		return (radius+16)*(radius+16) >= dx*dx + dz*dz;
	}

	return true;
}

bool WorldCrop::isChunkCompletelyContained(const mc::ChunkPos& chunk) const {
	mc::BlockPos corner(chunk.x * 16, chunk.z * 16, 0);
	return isBlockContainedXZ(corner)
			&& isBlockContainedXZ(corner + mc::BlockPos(15, 0, 0))
			&& isBlockContainedXZ(corner + mc::BlockPos(0, 15, 0))
			&& isBlockContainedXZ(corner + mc::BlockPos(15, 15, 0));
}

bool WorldCrop::isBlockContainedXZ(const mc::BlockPos& block) const {
	if (type == RECTANGULAR) {
		// rectangular crop:
		// just check if the chunk is contained in the bounds
		return bounds_x.contains(block.x) && bounds_z.contains(block.z);
	} else if (type == CIRCULAR) {
		// circular crop:
		// also check distance to the center (like with regions and chunks)
		int dx = block.x - center.x;
		int dz = block.z - center.z;
		return radius*radius >= dx*dx + dz*dz;
	}

	return true;
}

bool WorldCrop::isBlockContainedY(const mc::BlockPos& block) const {
	return bounds_y.contains(block.y);
}

bool WorldCrop::hasCropUnpopulatedChunks() const {
	return crop_unpopulated_chunks;
}

void WorldCrop::setCropUnpopulatedChunks(bool crop_unpopulated_chunks) {
	this->crop_unpopulated_chunks = crop_unpopulated_chunks;
}

bool WorldCrop::hasBlockMask() const {
	if (block_mask)
		return true;
	return false;
}

const BlockMask* WorldCrop::getBlockMask() const {
	return block_mask.get();
}

void WorldCrop::loadBlockMask(const std::string& str) {
	block_mask.reset(new BlockMask);
	try {
		block_mask->loadFromString(str);
	} catch (std::invalid_argument& exception) {
		block_mask.reset();
		throw exception;
	}
}

}
}
