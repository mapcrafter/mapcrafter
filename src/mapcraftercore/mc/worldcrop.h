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

#ifndef WORLDCROP_H_
#define WORLDCROP_H_

#include "pos.h"
#include "../util.h"

#include <bitset>
#include <memory>
#include <string>
#include <vector>

namespace mapcrafter {
namespace mc {

/**
 * Data structure to hold information about which blocks should be hidden/shown.
 */
class BlockMask {
public:
	/**
	 * Describes the state of a block with a specific id.
	 */
	enum BlockState {
		// all blocks with this id are hidden, independent of the block data
		COMPLETELY_HIDDEN,
		// all blocks with this id are shown, also independent of the block data
		COMPLETELY_SHOWN,
		// some block with this id are hidden, some are shown, depending of the block data
		PARTIALLY_HIDDEN_SHOWN
	};

	BlockMask();
	~BlockMask();

	/**
	 * Sets all blocks with a specific id to be hidden/shown;
	 */
	void set(uint16_t id, bool shown);

	/**
	 * Sets a block with a specific id/data to be hidden/shown;
	 */
	void set(uint16_t id, uint8_t data, bool shown);

	/**
	 * Sets all blocks with specific id and data values to be hidden/shown.
	 * Using a bitmask all blocks with data values (block data value & bitmask) == specified data
	 * are set to be shown/hidden.
	 */
	void set(uint16_t id, uint8_t data, uint8_t bitmask, bool shown);

	/**
	 * Sets all blocks from id1 to id2 to be hidden/shown.
	 */
	void setRange(uint16_t id1, uint16_t id2, bool shown);

	/**
	 * Sets all blocks to be hidden or shown.
	 */
	void setAll(bool shown);

	/**
	 * Loads the block mask from a string definition. Throws a std::invalid_argument
	 * exception if the supplied block mask definition is invalid.
	 *
	 * The block mask definition is a space separated list of block groups you want to
	 * hide/show. If a '!' precedes a block group, all blocks of this block group are
	 * hidden, otherwise they are shown. Per default, all blocks are shown.
	 *
	 * Possible block groups are:
	 * - All blocks:
	 *     '*'
	 * - A single block (independent of block data):
	 *     '[blockid]'
	 * - A single block with specific block data:
	 *     '[blockid]:[blockdata]'
	 * - A range of blocks:
	 *     '[blockid1]-[blockid2]'
	 * - All block with a specific id and (block data & bitmask) == specified data:
	 *     '[blockid]:[blockdata]b[bitmask]'
	 *
	 * For example:
	 * - Hide all blocks except blocks with id 1,7,8,9 or id 3 / data 2:
	 *     '!* 1 3:2 7-9'
	 * - Show all blocks except jungle wood and jungle leaves:
	 *     '!17:3b3 !18:3b3' (Jungle wood and jungle leaves have id 17 and 18
	 *                        and use data value 3 for first two bits (bitmask 3 = 0b11),
	 *                        other bits are used otherwise -> ignoring all those bits)
	 *
	 * TL;DR (except the space at the end): (!?(\*|\d+(:\d+(b\d+)?)?|\d+-\d+) )+
	 */
	void loadFromStringDefinition(const std::string& definition);

	/**
	 * Returns whether all, none, or some blocks with a specific id are hidden/shown.
	 */
	const BlockMask::BlockState& getBlockState(uint16_t id) const;

	/**
	 * Returns whether a block
	 */
	bool isHidden(uint16_t id, uint8_t data) const;

private:
	std::vector<BlockMask::BlockState> block_states;

	// the actual block mask
	// 65536 entries for the 16 bit long block id
	// * 16 entries for the 4 bit block data
	std::bitset<65536 * 16> block_mask;

	/**
	 * Updates whether all, none, or some blocks with a specific id are shown/hidden.
	 */
	void updateBlockState(uint16_t id);
};

class Area {
public:
	/**
	 * Creates a dummy area with all blocks contained.
	 */
	static Area dummy();

	/**
	 * Creates a rectangular area with the given x- and y-boundings.
	 */
	static Area rectangular(const util::Interval1D<int>& x, const util::Interval1D<int>& z);

	/**
	 * Creates a circular area with the given center and radius.
	 */
	static Area circular(const BlockPos& center, long radius);

	/**
	 * Returns the area with the y-coordinate cropped.
	 */
	Area withYBounding(const util::Interval1D<int>& y) const;

	/**
	 * Returns whether a specific region is contained.
	 */
	bool isRegionContained(const mc::RegionPos& region) const;

	/**
	 * Returns whether a specific chunk is contained.
	 */
	bool isChunkContained(const mc::ChunkPos& chunk) const;

	/**
	 * Returns whether a specific chunk is completely contained. With completely is
	 * meant that all x- and z-coordinates are included, only some y-coordinates may not
	 * be included.
	 */
	bool isChunkCompletelyContained(const mc::ChunkPos& chunk) const;

	/**
	 * Returns whether a block is contained regarding x- and z-coordinates.
	 * Used to check if a chunk is only partly contained.
	 */
	bool isBlockContainedXZ(const mc::BlockPos& block) const;

	/**
	 * Returns whether a block is contained regarding its y-coordinate.
	 */
	bool isBlockContainedY(const mc::BlockPos& block) const;

	/**
	 * Returns whether a block is contained regarding all coordinate components.
	 */
	bool isBlockContained(const mc::BlockPos& block) const;

	// different types of areas
	const static int DUMMY = 0;
	const static int RECTANGULAR = 1;
	const static int CIRCULAR = 2;

private:
	Area(int type);

	void setXBounds(const util::Interval1D<int>& bounds);
	void setZBounds(const util::Interval1D<int>& bounds);
	void setYBounds(const util::Interval1D<int>& bounds);

	// type of world boundaries -- either RECTANGULAR or CIRCULAR
	int type;

	// usable for both rectangular and circular
	util::Interval1D<int> bounds_y;

	// rectangular limits:
	// in block coordinates
	util::Interval1D<int> bounds_x, bounds_z;
	// in chunk coordinates
	util::Interval1D<int> bounds_chunk_x, bounds_chunk_z;
	// in region coordinates
	util::Interval1D<int> bounds_region_x, bounds_region_z;

	// circular limits
	BlockPos center;
	long radius;
};

/**
 * Boundaries to crop a Minecraft World.
 */
class WorldCrop {
public:
	WorldCrop();
	~WorldCrop();

	/**
	 * Returns the used area.
	 */
	const Area& getArea() const;

	/**
	 * Sets the area.
	 */
	void setArea(const Area& area);

	/**
	 * Returns/sets whether unpopulated chunks should be cropped.
	 */
	bool hasCropUnpopulatedChunks() const;
	void setCropUnpopulatedChunks(bool crop);

	/**
	 * Returns whether this world has a block mask.
	 */
	bool hasBlockMask() const;

	/**
	 * Returns the block mask of this world. Returns a nullptr if !hasBlockMask().
	 */
	const BlockMask* getBlockMask() const;

	/**
	 * Loads the block mask from a string definition. See BlockMask::loadFromString.
	 */
	void loadBlockMask(const std::string& definition);

private:
	// area that is only visible
	Area area;

	// whether to hide unpopulated chunks
	bool crop_unpopulated_chunks;

	// block mask
	std::shared_ptr<BlockMask> block_mask;
};

}
}

#endif /* WORLDCROP_H_ */
