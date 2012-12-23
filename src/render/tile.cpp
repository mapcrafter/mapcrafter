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

TilePos Path::getTilePos() const {
	int size = pow(2, path.size()) / 2;
	int height = size;
	int x = -size;
	int y = -size;
	for (size_t i = 0; i < path.size(); i++) {
		int tile = path[i];
		if (tile == 2 || tile == 4)
			x += size;
		if (tile == 3 || tile == 4)
			y += size;
		size /= 2;
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

Path Path::byTilePos(const TilePos& tile, int depth) {
	Path path;

	int radius = pow(2, depth) / 2;
	if (tile.getX() > radius || tile.getY() > radius || tile.getX() < -radius
	        || tile.getY() < -radius)
		throw std::runtime_error(
		        "Invalid tile position " + str(tile.getX()) + ":" + str(tile.getY())
		                + " on depth " + str(depth));
	int bounds_left = -radius;
	int bounds_right = radius;
	int bounds_top = radius;
	int bounds_bottom = -radius;

	for (int level = 1; level <= depth; level++) {
		int middle_x = (bounds_right + bounds_left) / 2;
		int middle_y = (bounds_top + bounds_bottom) / 2;
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

TileSet::TileSet(mc::World& world, int last_change_time)
		: world(world), last_change_time(last_change_time), depth(0) {
	findRequiredRenderTiles();
	findRequiredCompositeTiles(Path(), render_tiles, composite_tiles);
	findRequiredCompositeTiles(Path(), required_render_tiles, required_composite_tiles);
}

TileSet::~TileSet() {
}

void getChunkTiles(const mc::ChunkPos& chunk, std::set<TilePos>& tiles) {
	int row = chunk.getRow();
	int col = chunk.getCol();
	int base_x = col / 2;
	int base_y = row / 4;
	for (int tile_y = base_y - 1; tile_y <= base_y + 8; tile_y++) {
		tiles.insert(TilePos(base_x, tile_y));
		if (col % 2 == 0)
			tiles.insert(TilePos(base_x - 1, tile_y));
	}
}

void TileSet::findRequiredRenderTiles() {
	int tiles_x_min = 0, tiles_x_max = 0, tiles_y_min = 0, tiles_y_max = 0;

	auto regions = world.getAvailableRegions();
	for (auto region_it = regions.begin(); region_it != regions.end(); ++region_it) {
		mc::RegionFile region;
		if (!world.getRegion(*region_it, region) || !region.loadHeaders())
			continue;
		const std::set<mc::ChunkPos>& region_chunks = region.getContainingChunks();
		for (auto chunk_it = region_chunks.begin(); chunk_it != region_chunks.end();
		        ++chunk_it) {
			bool required = region.getChunkTimestamp(*chunk_it) > last_change_time;

			std::set<TilePos> tiles;
			getChunkTiles(*chunk_it, tiles);
			for (std::set<TilePos>::const_iterator tile_it = tiles.begin();
			        tile_it != tiles.end(); ++tile_it) {

				tiles_x_min = MIN(tiles_x_min, tile_it->getX());
				tiles_x_max = MAX(tiles_x_max, tile_it->getX());
				tiles_y_min = MIN(tiles_y_min, tile_it->getY());
				tiles_y_max = MAX(tiles_y_max, tile_it->getY());

				render_tiles.insert(*tile_it);
				if (required)
					required_render_tiles.insert(*tile_it);
			}
		}
	}

	for (depth = 0; depth < 32; depth++) {
		int radius = pow(2, depth) / 2;
		if (tiles_x_min > -radius && tiles_x_max < radius && tiles_y_min > -radius
		        && tiles_y_max < radius)
			break;
	}
}

int TileSet::findRequiredCompositeTiles(const Path &path,
        const std::set<TilePos>& render_tiles, std::set<Path>& tiles) {
	int count = 0;
	if (path.getDepth() == depth - 1) {
		if (render_tiles.count((path + 1).getTilePos()) != 0)
			count++;
		if (render_tiles.count((path + 2).getTilePos()) != 0)
			count++;
		if (render_tiles.count((path + 3).getTilePos()) != 0)
			count++;
		if (render_tiles.count((path + 4).getTilePos()) != 0)
			count++;
	} else {
		if (findRequiredCompositeTiles(path + 1, render_tiles, tiles) > 0)
			count++;
		if (findRequiredCompositeTiles(path + 2, render_tiles, tiles) > 0)
			count++;
		if (findRequiredCompositeTiles(path + 3, render_tiles, tiles) > 0)
			count++;
		if (findRequiredCompositeTiles(path + 4, render_tiles, tiles) > 0)
			count++;
	}
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
