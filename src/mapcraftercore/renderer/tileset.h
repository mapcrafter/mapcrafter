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

#ifndef TILESET_H_
#define TILESET_H_

#include <iostream>
#include <string>
#include <vector>

namespace mapcrafter {
namespace renderer {

const int TILE_WIDTH = 1;

/**
 * This class represents the position of a tile in the quadtree.
 */
class TilePos {
public:
	TilePos(int x = 0, int y = 0);

	/**
	 * Returns x/y tile coordinate.
	 */
	int getX() const;
	int getY() const;

	// some operations with tile positions
	TilePos& operator+=(const TilePos& p);
	TilePos& operator-=(const TilePos& p);
	TilePos operator+(const TilePos& p2) const;
	TilePos operator-(const TilePos& p2) const;

	bool operator==(const TilePos& other) const;
	bool operator!=(const TilePos& other) const;
	bool operator<(const TilePos& other) const;
private:
	// actual coordinates
	int x, y;
};

/**
 * This class represents the path of a tile in the quadtree.
 * Every part in the path is a 1, 2, 3 or 4.
 * The length of the path is the zoom level of the tile.
 */
class TilePath {
public:
	TilePath();
	TilePath(const std::vector<int>& path);
	~TilePath();

	/**
	 * Returns the zoom level of the path.
	 */
	int getDepth() const;

	/**
	 * Returns the path.
	 */
	const std::vector<int>& getPath() const;

	/**
	 * Returns the path of the parent tile.
	 * For example: The parent path of 1/2/3/4 is 1/2/3.
	 */
	TilePath parent() const;

	/**
	 * Calculates the tile position of the path.
	 */
	TilePos getTilePos() const;

	/**
	 * Calculates the path (with a specified zoom level) of a tile position.
	 * Opposite of getTilePos-method.
	 */
	static TilePath byTilePos(const TilePos& tile, int depth);

	/**
	 * Adds a node to the path.
	 */
	TilePath& operator+=(int node);
	TilePath operator+(int node) const;

	// some more comparison operations
	bool operator==(const TilePath& other) const;
	bool operator<(const TilePath& other) const;

	/**
	 * Returns the string representation of the path, for example "1/2/3/4".
	 */
	std::string toString() const;
private:
	std::vector<int> path;
};

std::ostream& operator<<(std::ostream& stream, const TilePath& path);
std::ostream& operator<<(std::ostream& stream, const TilePos& tile);

class TileSet {
};

}
}

#endif /* TILESET_H_ */
