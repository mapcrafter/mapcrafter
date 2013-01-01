/*
 * Copyright 2012 Moritz Hilscher
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

#include "render/tile.h"

#include "mc/pos.h"
#include "render/render.h"
#include "util.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <stdint.h>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace render {

TilePos::TilePos()
		: x(0), y(0) {
}

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

bool TilePos::operator<(const TilePos& other) const {
	if (x == other.x)
		return y < other.y;
	return x < other.x;
}

Path::Path() {
}

Path::Path(const std::vector<int>& path)
		: path(path) {
}

Path::~Path() {
}

const std::vector<int>& Path::getPath() const {
	return path;
}

int Path::getDepth() const {
	return path.size();
}

/**
 * This method calculates the
 */
TilePos Path::getTilePos() const {
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
		// increase y by the radius if this tile is bottom (3 or 4)
		if (tile == 3 || tile == 4)
			y += radius;
		// divide size by two, because the next zoom level has only the half radius
		radius /= 2;
	}
	return TilePos(x, y);
}

std::string Path::toString() const {
	std::stringstream ss;
	for (size_t i = 0; i < path.size(); i++) {
		ss << path[i];
		if (i != path.size() - 1)
			ss << "/";
	}
	return ss.str();
}

Path& Path::operator+=(int node) {
	path.push_back(node);
	return *this;
}

Path Path::operator+(int node) const {
	Path copy(path);
	copy.path.push_back(node);
	return copy;
}

bool Path::operator==(const Path& other) const {
	return path == other.path;
}

bool Path::operator<(const Path& other) const {
	if (getDepth() == other.getDepth()
	        && std::equal(path.begin(), path.end(), other.path.begin()))
		return false;
	int maxsize = MAX(getDepth(), other.getDepth());
	for (int i = 0; i < maxsize; i++) {
		if (i == getDepth())
			return true;
		if (i == other.getDepth())
			return false;
		if (path[i] != other.path[i])
			return path[i] < other.path[i];
	}
	return false;
}

/**
 * This method calculates the path by a specific tile position on a specific level.
 * This is the opposite of Path::getTilePos().
 */
Path Path::byTilePos(const TilePos& tile, int depth) {
	Path path;

	// at first calculate the radius in tiles of this zoom level
	int radius = pow(2, depth) / 2;
	// check if the tile is in this bounds
	if (tile.getX() > radius || tile.getY() > radius || tile.getX() < -radius
	        || tile.getY() < -radius)
		throw std::runtime_error(
		        "Invalid tile position " + str(tile.getX()) + ":" + str(tile.getY())
		                + " on depth " + str(depth));
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

std::ostream& operator<<(std::ostream& stream, const TilePos& tile) {
	stream << tile.getX() << ":" << tile.getY();
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const Path& path) {
	stream << path.toString();
	return stream;
}

TileSet::TileSet(mc::World& world, int last_check_time)
		: world(world), last_check_time(last_check_time), depth(0) {
	// find the available/required tiles
	findRequiredRenderTiles();
	findRequiredCompositeTiles(Path(), render_tiles, composite_tiles);
	findRequiredCompositeTiles(Path(), required_render_tiles, required_composite_tiles);
}

TileSet::~TileSet() {
}

/**
 * This function calculates the tiles, a chunk covers.
 */
void getChunkTiles(const mc::ChunkPos& chunk, std::set<TilePos>& tiles) {
	// at first get row and column of the top of the chunk
	int row = chunk.getRow();
	int col = chunk.getCol();
	// then get from this the tile position (every tile is 2 columns wide and 4 rows tall)
	int base_x = col / 2;
	int base_y = row / 4;
	// now add all tiles,
	// a chunk is 9 tiles tall
	// and one or two tiles wide (two tiles if column is even)
	for (int tile_y = base_y - 1; tile_y <= base_y + 8; tile_y++) {
		tiles.insert(TilePos(base_x, tile_y));
		if (col % 2 == 0)
			tiles.insert(TilePos(base_x - 1, tile_y));
	}
}

/**
 * This method finds out, which top level tiles a world has and which of them need to
 * get rendered.
 */
void TileSet::findRequiredRenderTiles() {
	// the min/max x/y coordinates of the tiles in the world
	int tiles_x_min = 0, tiles_x_max = 0, tiles_y_min = 0, tiles_y_max = 0;

	// go through all chunks in the world
	auto regions = world.getAvailableRegions();
	for (auto region_it = regions.begin(); region_it != regions.end(); ++region_it) {
		mc::RegionFile region;
		if (!world.getRegion(*region_it, region) || !region.loadHeaders())
			continue;
		const std::set<mc::ChunkPos>& region_chunks = region.getContainingChunks();
		for (auto chunk_it = region_chunks.begin(); chunk_it != region_chunks.end();
		        ++chunk_it) {
			// check if the chunk was changed since the last check
			bool changed = region.getChunkTimestamp(*chunk_it) > last_check_time;

			// now get all tiles of the chunk
			std::set<TilePos> tiles;
			getChunkTiles(*chunk_it, tiles);
			for (std::set<TilePos>::const_iterator tile_it = tiles.begin();
			        tile_it != tiles.end(); ++tile_it) {

				// and update the bounds
				tiles_x_min = MIN(tiles_x_min, tile_it->getX());
				tiles_x_max = MAX(tiles_x_max, tile_it->getX());
				tiles_y_min = MIN(tiles_y_min, tile_it->getY());
				tiles_y_max = MAX(tiles_y_max, tile_it->getY());

				// insert the tile in the set of available render tiles
				render_tiles.insert(*tile_it);
				if (changed)
					// if changed also in the list of required render tiles
					required_render_tiles.insert(*tile_it);
			}
		}
	}

	// now get the necessary depth of the tile quadtree
	for (depth = 0; depth < 32; depth++) {
		// for each level calculate the radius and check if the tiles fit in this bounds
		int radius = pow(2, depth) / 2;
		if (tiles_x_min > -radius && tiles_x_max < radius && tiles_y_min > -radius
		        && tiles_y_max < radius)
			break;
	}
}

/**
 * This recursive used method finds out, if a composite tile is needed, depending on a
 * list of available/required render tiles and puts them into a set. So we can find out
 * which composite tiles are available and which composite tiles need to get rendered.
 */
int TileSet::findRequiredCompositeTiles(const Path &path,
        const std::set<TilePos>& render_tiles, std::set<Path>& tiles) {
	int count = 0;
	// check if this composite tile is on the zoom level above the render tiles
	if (path.getDepth() == depth - 1) {
		// then check if the associated render tiles exist
		if (render_tiles.count((path + 1).getTilePos()) != 0)
			count++;
		if (render_tiles.count((path + 2).getTilePos()) != 0)
			count++;
		if (render_tiles.count((path + 3).getTilePos()) != 0)
			count++;
		if (render_tiles.count((path + 4).getTilePos()) != 0)
			count++;
	} else {
		// else check the other composite tiles
		if (findRequiredCompositeTiles(path + 1, render_tiles, tiles) > 0)
			count++;
		if (findRequiredCompositeTiles(path + 2, render_tiles, tiles) > 0)
			count++;
		if (findRequiredCompositeTiles(path + 3, render_tiles, tiles) > 0)
			count++;
		if (findRequiredCompositeTiles(path + 4, render_tiles, tiles) > 0)
			count++;
	}
	// if other tiles are found, insert this tile in the set
	if (count > 0)
		tiles.insert(path);
	return count;
}

bool TileSet::hasTile(const Path& path) const {
	if (path.getDepth() == depth)
		return render_tiles.count(path.getTilePos()) != 0;
	return composite_tiles.count(path) != 0;
}

bool TileSet::isTileRequired(const Path& path) const {
	return required_composite_tiles.count(path) != 0;
}

int TileSet::getRequiredRenderTilesCount() const {
	return required_render_tiles.size();
}

int TileSet::getRequiredCompositeTilesCount() const {
	return required_composite_tiles.size();
}

int TileSet::getMaxZoom() const {
	return depth;
}

const std::set<TilePos>& TileSet::getAvailableRenderTiles() const {
	return render_tiles;
}

const std::set<Path>& TileSet::getAvailableCompositeTiles() const {
	return composite_tiles;
}

const std::set<TilePos>& TileSet::getRequiredRenderTiles() const {
	return required_render_tiles;
}

const std::set<Path>& TileSet::getRequiredCompositeTiles() const {
	return required_composite_tiles;
}

}
}
