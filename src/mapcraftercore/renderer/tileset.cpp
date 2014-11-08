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

#include "tileset.h"

namespace mapcrafter {
namespace renderer {

TilePos::TilePos(int x, int y)
	: x(x), y(y) {
}

int TilePos::getX() const {
	return x;
}

int TilePos::getY() const {
	return y;
}

TilePos& TilePos::operator+=(const TilePos& p) {
	x += p.x;
	y += p.y;
	return *this;
}

TilePos& TilePos::operator-=(const TilePos& p) {
	x -= p.x;
	y -= p.y;
	return *this;
}

TilePos TilePos::operator+(const TilePos& p2) const {
	TilePos p = *this;
	return p += p2;
}

TilePos TilePos::operator-(const TilePos& p2) const {
	TilePos p = *this;
	return p -= p2;
}

bool TilePos::operator==(const TilePos& other) const {
	return x == other.x && y == other.y;
}

bool TilePos::operator!=(const TilePos& other) const {
	return !(*this == other);
}

bool TilePos::operator<(const TilePos& other) const {
	if (x == other.x)
		return y < other.y;
	return x < other.x;
}

TilePath::TilePath() {
}

TilePath::TilePath(const std::vector<int>& path)
	: path(path) {
}

TilePath::~TilePath() {
}

int TilePath::getDepth() const {
	return path.size();
}

const std::vector<int>& TilePath::getPath() const {
	return path;
}

TilePath TilePath::parent() const {
	TilePath copy(path);
	copy.path.pop_back();
	return copy;
}

TilePos TilePath::getTilePos() const {
	// calculate the radius of all tiles on the top zoom level (2^zoomlevel / 2)
	int radius = pow(2, path.size()) / 2;
	// the startpoint is top left
	int x = -radius;
	int y = -radius;
	for (size_t i = 0; i < path.size(); i++) {
		// now for every zoom level:
		// get the current tile
		int tile = path[i];
		// increase x by the radius if this tile is on the right side (2 or 4)
		if (tile == 2 || tile == 4)
			x += radius;
		// increase y by the radius if this tile is on the bottom side (3 or 4)
		if (tile == 3 || tile == 4)
			y += radius;
		// divide size by two, because the next zoom level has only the half radius
		radius /= 2;
	}
	return TilePos(x, y);
}

TilePath TilePath::byTilePos(const TilePos& tile, int depth) {
	TilePath path;

	// at first calculate the radius in tiles of this zoom level
	int radius = pow(2, depth) / 2;
	// check if the tile is in this bounds
	if (tile.getX() > radius  || tile.getY() > radius
			|| tile.getX() < -radius || tile.getY() < -radius)
		throw std::runtime_error("Invalid tile position " + util::str(tile.getX())
			+ ":" + util::str(tile.getY()) + " on depth " + util::str(depth));
	// the tactic is here to calculate the bounds where the tile is inside
	int bounds_left = -radius;
	int bounds_right = radius;
	int bounds_top = radius;
	int bounds_bottom = -radius;

	for (int level = 1; level <= depth; level++) {
		// for each zoom level (but we already have the 1st zoom level):
		// calculate the midpoint in the bounds
		int middle_x = (bounds_right + bounds_left) / 2;
		int middle_y = (bounds_top + bounds_bottom) / 2;
		// with this midpoint we can decide what the next part in the path is
		// when tile x < midpoint x:
		//   and tile y < midpoint y -> top left tile
		//   and tile y >= midpoint y -> bottom left tile
		// when tile y >= midpoint y:
		//   and tile y < midpoint y -> top right tile
		//   and tile y >= midpoint y -> bottom right tile

		// if we have a midpoint, we update the bounds with the midpoint
		// the (smaller) bounds are now the part of the path, we found out
		if (tile.getX() < middle_x) {
			if (tile.getY() < middle_y) {
				path += 1;
				bounds_right = middle_x;
				bounds_top = middle_y;
			} else {
				path += 3;
				bounds_right = middle_x;
				bounds_bottom = middle_y;
			}
		} else {
			if (tile.getY() < middle_y) {
				path += 2;
				bounds_left = middle_x;
				bounds_top = middle_y;
			} else {
				path += 4;
				bounds_left = middle_x;
				bounds_bottom = middle_y;
			}
		}
	}

	return path;
}

TilePath& TilePath::operator+=(int node) {
	path.push_back(node);
	return *this;
}

TilePath TilePath::operator+(int node) const {
	TilePath copy(path);
	copy.path.push_back(node);
	return copy;
}

bool TilePath::operator==(const TilePath& other) const {
	return path == other.path;
}

bool TilePath::operator<(const TilePath& other) const {
	return path < other.path;
}

std::ostream& operator<<(std::ostream& stream, const TilePos& tile) {
	stream << tile.getX() << ":" << tile.getY();
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const TilePath& path) {
	stream << path.toString();
	return stream;
}

std::string TilePath::toString() const {
	std::stringstream ss;
	for (size_t i = 0; i < path.size(); i++) {
		ss << path[i];
		if (i != path.size() - 1)
			ss << "/";
	}
	return ss.str();
}

}
}
