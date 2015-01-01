/*
 * Copyright 2012-2015 Moritz Hilscher
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

#include "../mc/pos.h"
#include "../util.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>

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

TileSet::TileSet()
	: min_depth(0), depth(0) {
}

TileSet::TileSet(const mc::World& world)
	: min_depth(0), depth(0) {
	scan(world);
}

TileSet::~TileSet() {
}

/**
 * Calculates the tiles a row and column covers.
 */
void addRowColTiles(int row, int col, std::set<TilePos>& tiles) {
	// the tiles have are 2 * TILE_WIDTH columns wide
	// and 4 * TILE_WIDTH row tall
	// calculate the approximate position of the tile
	int x = col / (2 * TILE_WIDTH);
	int y = row / (4 * TILE_WIDTH);

	// add this tile
	tiles.insert(TilePos(x, y));

	// check if this row/col is on the border of two tiles
	bool edge_col = col % (2 * TILE_WIDTH) == 0;
	bool edge_row = row % (4 * TILE_WIDTH) == 0;
	// if yes, we have to add the neighbor tiles
	if (edge_col)
		tiles.insert(TilePos(x-1, y));
	if (edge_row)
		tiles.insert(TilePos(x, y-1));
	if (edge_col && edge_row)
		tiles.insert(TilePos(x-1, y-1));
}

/**
 * This function calculates the tiles a chunk covers.
 */
void getChunkTiles(const mc::ChunkPos& chunk, std::set<TilePos>& tiles) {
	// at first get row and column of the top of the chunk
	int row = chunk.getRow();
	int col = chunk.getCol();

	// TODO fix this for different TILE_WIDTH

	// then we go through all sections of the chunk plus one on the bottom side
	// and add the tiles the individual sections cover,

	// plus one on the bottom side because with chunk section is here
	// only the top of a chunk section meant
	for (int i = 0; i <= mc::CHUNK_HEIGHT; i++)
		addRowColTiles(row + 2*i, col, tiles);
}

void TileSet::findRenderTiles(const mc::World& world, bool auto_center,
		TilePos& tile_offset) {
	// clear maybe already calculated tiles
	render_tiles.clear();
	required_render_tiles.clear();

	// the min/max x/y coordinates of the tiles in the world
	int tiles_x_min = std::numeric_limits<int>::max(),
	    tiles_x_max = std::numeric_limits<int>::min(),
	    tiles_y_min = std::numeric_limits<int>::max(),
	    tiles_y_max = std::numeric_limits<int>::min();

	// go through all chunks in the world
	auto regions = world.getAvailableRegions();
	for (auto region_it = regions.begin(); region_it != regions.end(); ++region_it) {
		mc::RegionFile region;
		if (!world.getRegion(*region_it, region) || !region.readOnlyHeaders())
			continue;
		const std::set<mc::ChunkPos>& region_chunks = region.getContainingChunks();
		for (auto chunk_it = region_chunks.begin(); chunk_it != region_chunks.end();
		        ++chunk_it) {
			int timestamp = region.getChunkTimestamp(*chunk_it);

			// now get all tiles of the chunk
			std::set<TilePos> tiles;
			getChunkTiles(*chunk_it, tiles);
			for (std::set<TilePos>::const_iterator tile_it = tiles.begin();
			        tile_it != tiles.end(); ++tile_it) {

				// and update the bounds
				tiles_x_min = std::min(tiles_x_min, tile_it->getX());
				tiles_x_max = std::max(tiles_x_max, tile_it->getX());
				tiles_y_min = std::min(tiles_y_min, tile_it->getY());
				tiles_y_max = std::max(tiles_y_max, tile_it->getY());

				// update tile timestamp
				if (!render_tiles.count(*tile_it))
					tile_timestamps[*tile_it] = timestamp;
				else
					tile_timestamps[*tile_it] = std::max(tile_timestamps[*tile_it], timestamp);

				// insert the tile to the set of available render tiles
				// and also make it required by default
				render_tiles.insert(*tile_it);
				required_render_tiles.insert(*tile_it);
			}
		}
	}

	// center tiles
	if (auto_center || tile_offset != TilePos(0, 0)) {
		// find a tile center if we should do it automatically
		if (auto_center)
			tile_offset = TilePos((tiles_x_min + tiles_x_max) / 2, (tiles_y_min + tiles_y_max) / 2);

		// update all tile positions
		std::set<TilePos> render_tiles_tmp, required_render_tiles_tmp;
		std::map<TilePos, int> tile_timestamps_tmp;
		for (auto it = render_tiles.begin(); it != render_tiles.end(); ++it)
			render_tiles_tmp.insert(*it - tile_offset);
		for (auto it = required_render_tiles.begin(); it != required_render_tiles.end(); ++it)
			required_render_tiles_tmp.insert(*it - tile_offset);
		for (auto it = tile_timestamps.begin(); it != tile_timestamps.end(); ++it)
			tile_timestamps_tmp[it->first - tile_offset] = it->second;

		render_tiles = render_tiles_tmp;
		required_render_tiles = required_render_tiles_tmp;
		tile_timestamps = tile_timestamps_tmp;
		this->tile_offset = tile_offset;
	}

	// now get the necessary depth of the tile quadtree
	for (min_depth = 0; min_depth < 32; min_depth++) {
		// for each level calculate the radius and check if the tiles fit in this bounds
		// also don't forget the tile offset
		int radius = pow(2, min_depth) / 2;
		if (tiles_x_min - tile_offset.getX() > -radius
				&& tiles_x_max - tile_offset.getX() < radius
				&& tiles_y_min - tile_offset.getY() > -radius
				&& tiles_y_max - tile_offset.getY() < radius)
			break;
	}
}

void TileSet::findRequiredCompositeTiles(const std::set<TilePos>& render_tiles,
		std::set<TilePath>& tiles) {

	// iterate through the render tiles on the max zoom level
	// add their parent composite tiles
	for (std::set<TilePos>::iterator it = render_tiles.begin(); it != render_tiles.end(); ++it) {
		TilePath path = TilePath::byTilePos(*it, depth);
		tiles.insert(path.parent());
	}

	// now iterate through the composite tiles from bottom to top
	// and also add their parent composite tiles
	for (int d = depth - 1; d > 0; d--) {
		std::set<TilePath> tmp;
		for (std::set<TilePath>::iterator it = tiles.begin(); it != tiles.end(); ++it) {
			if (it->getDepth() == d)
				tmp.insert(it->parent());
		}
		for (std::set<TilePath>::iterator it = tmp.begin(); it != tmp.end(); ++it)
			tiles.insert(*it);
	}
}

void TileSet::updateContainingRenderTiles() {
	containing_render_tiles.clear();
	// initialize every composite tile with 0
	for (auto it = composite_tiles.begin(); it != composite_tiles.end(); ++it)
		containing_render_tiles[*it] = 0;
	// go through all required render tiles
	// set the containing render tiles for every parent composite tile +1
	// to have the number of required render tiles in every composite tile
	for (auto it = required_render_tiles.begin(); it != required_render_tiles.end(); ++it) {
		TilePath tile = TilePath::byTilePos(*it, depth);
		while (tile.getDepth() != 0) {
			tile = tile.parent();
			containing_render_tiles[tile]++;
		}
	}
}

void TileSet::scan(const mc::World& world) {
	TilePos tile_offset(0, 0);
	scan(world, false, tile_offset);
	setDepth(min_depth);
}

void TileSet::scan(const mc::World& world, bool auto_center, TilePos& tile_offset) {
	findRenderTiles(world, auto_center, tile_offset);
	setDepth(min_depth);
}

void TileSet::scanRequiredByTimestamp(int last_change) {
	required_render_tiles.clear();

	for (std::map<TilePos, int>::iterator it = tile_timestamps.begin();
			it != tile_timestamps.end(); ++it) {
		if (it->second >= last_change)
			required_render_tiles.insert(it->first);
	}

	required_composite_tiles.clear();
	findRequiredCompositeTiles(required_render_tiles, required_composite_tiles);

	updateContainingRenderTiles();
}

void TileSet::scanRequiredByFiletimes(const fs::path& output_dir,
		std::string image_format) {
	required_render_tiles.clear();

	for (std::map<TilePos, int>::iterator it = tile_timestamps.begin();
			it != tile_timestamps.end(); ++it) {
		TilePath path = TilePath::byTilePos(it->first, depth);
		fs::path file = output_dir / (path.toString() + "." + image_format);
		if (!fs::exists(file) || fs::last_write_time(file) <= it->second)
			required_render_tiles.insert(it->first);
	}

	required_composite_tiles.clear();
	findRequiredCompositeTiles(required_render_tiles, required_composite_tiles);

	updateContainingRenderTiles();
}

int TileSet::getMinDepth() const {
	return min_depth;
}

int TileSet::getDepth() const {
	return depth;
}

void TileSet::setDepth(int depth) {
	// only calculate the composite tiles new when the depth has changed
	if (this->depth == depth || depth < min_depth)
		return;

	this->depth = depth;

	// clear already calculated composite tiles and recalculate them
	composite_tiles.clear();
	required_composite_tiles.clear();

	findRequiredCompositeTiles(render_tiles, composite_tiles);
	findRequiredCompositeTiles(required_render_tiles, required_composite_tiles);

	updateContainingRenderTiles();
}

const TilePos& TileSet::getTileOffset() const {
	return tile_offset;
}

bool TileSet::hasTile(const TilePath& path) const {
	if (path.getDepth() == depth)
		return render_tiles.count(path.getTilePos()) != 0;
	return composite_tiles.count(path) != 0;
}

bool TileSet::isTileRequired(const TilePath& path) const {
	if(path.getDepth() == depth)
		return required_render_tiles.count(path.getTilePos()) != 0;
	return required_composite_tiles.count(path) != 0;
}

int TileSet::getRequiredRenderTilesCount() const {
	return required_render_tiles.size();
}

const std::set<TilePos>& TileSet::getRequiredRenderTiles() const {
	return required_render_tiles;
}

int TileSet::getRequiredCompositeTilesCount() const {
	return required_composite_tiles.size();
}

const std::set<TilePath>& TileSet::getRequiredCompositeTiles() const {
	return required_composite_tiles;
}

int TileSet::getContainingRenderTiles(const TilePath& tile) const {
	return containing_render_tiles.at(tile);
}

}
}
