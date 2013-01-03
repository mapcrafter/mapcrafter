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

#ifndef POS_H_
#define POS_H_

#include "util.h"

/**
 * Here are some Minecraft position things. In Minecraft, x/z are the horizontal axes
 * and y is the vertical axis.
 *
 * Here are the Minecraft directions (also on the bottom of this file):
 * north = -z
 * south = +z
 * east = +x
 * west = -x
 */

namespace mapcrafter {
namespace mc {

class RegionPos {
public:
	int x, z;

	RegionPos();
	RegionPos(int x, int z);

	bool operator==(const RegionPos& other) const;
	bool operator!=(const RegionPos& other) const;
	bool operator<(const RegionPos& other) const;

	static RegionPos byFilename(const std::string& filename);
};

class BlockPos;

class ChunkPos {
public:
	int x, z;

	ChunkPos();
	ChunkPos(int x, int z);
	ChunkPos(const BlockPos& block);

	int getLocalX() const;
	int getLocalZ() const;

	RegionPos getRegion() const;

	bool operator==(const ChunkPos& other) const;
	bool operator!=(const ChunkPos& other) const;
	bool operator<(const ChunkPos& other) const;

	int getRow() const;
	int getCol() const;
	static ChunkPos byRowCol(int row, int col);
};

class LocalBlockPos;

class BlockPos {
public:
	int x, z, y;

	BlockPos();
	BlockPos(int x, int z, int y);

	int getRow() const;
	int getCol() const;

	BlockPos& operator+=(const BlockPos& p);
	BlockPos& operator-=(const BlockPos& p);
	BlockPos operator+(const BlockPos& p2) const;
	BlockPos operator-(const BlockPos& p2) const;

	bool operator<(const BlockPos& other) const;
};

class LocalBlockPos {
public:
	int x, z, y;

	LocalBlockPos();
	LocalBlockPos(int x, int z, int y);
	LocalBlockPos(const BlockPos& pos);

	int getRow() const;
	int getCol() const;

	BlockPos toGlobalPos(const ChunkPos& chunk) const;

	bool operator<(const LocalBlockPos& other) const;
};

std::ostream& operator<<(std::ostream& stream, const RegionPos& region);
std::ostream& operator<<(std::ostream& stream, const ChunkPos& chunk);
std::ostream& operator<<(std::ostream& stream, const BlockPos& block);
std::ostream& operator<<(std::ostream& stream, const LocalBlockPos& block);

#define DIR_NORTH mc::BlockPos(0, -1, 0)
#define DIR_SOUTH mc::BlockPos(0, 1, 0)
#define DIR_EAST mc::BlockPos(1, 0, 0)
#define DIR_WEST mc::BlockPos(-1, 0, 0)
#define DIR_TOP mc::BlockPos(0, 0, 1)
#define DIR_BOTTOM mc::BlockPos(0, 0, -1)

}
}

#endif
