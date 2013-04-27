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

#ifndef TILE_H_
#define TILE_H_

#include "mc/world.h"

#include "render/textures.h"

#include <set>
#include <vector>

/**
 * The renderer renders the world to tiles, which are arranged in a quadtree. Every node
 * is a tile with 2x2 (maximal 4) children. These children have numbers, depending on
 * their position:
 *   1: children is on the top left,
 *   2: top right,
 *   3: bottom left,
 *   4: bottom right
 *
 * The leaf nodes in the tree don't have children. These tiles are rendered directly
 * from the world data and are called render tiles or top level tiles. The other tiles
 * are composite tiles and are composed from their children tiles.
 *
 * The render tiles have a position. All tiles have a path from the root node to the tile.
 * The length of this path is the zoom level. The root node has the zoom level 0 and the
 * render tiles the maximum zoom level.
 *
 * The TileSet class manages the tiles for the rendering process. The render tiles are
 * stored as tile positions, all other composite tiles as tile paths. The tile set scans
 * the world and all chunks, calculates the maximum needed zoom level and finds out, which
 * tiles exists and which tiles need to get rendered (useful for incremental rendering,
 * when only a few chunks where changed).
 *
 * The tile renderer can then render the render tiles and compose the composite tiles
 * recursively from the render tiles.
 *
 * All tile images are stored on disk like the tree structure. The tile with the zoom
 * level 0 is "base.png". All children tiles are stored in directories 1/ 2/ 3/ 4/ and
 * images 1.png 2.png 3.png 4.png.
 */

namespace mapcrafter {
namespace render {

/**
 * This class represents the position of a render tile.
 */
class TilePos {
private:
	int x, y;
public:
	TilePos();
	TilePos(int x, int y);

	int getX() const;
	int getY() const;

	TilePos& operator+=(const TilePos& p);
	TilePos& operator-=(const TilePos& p);
	TilePos operator+(const TilePos& p2) const;
	TilePos operator-(const TilePos& p2) const;

	bool operator==(const TilePos& other) const;
	bool operator<(const TilePos& other) const;
};

/**
 * This class represents the path to a tile in the quadtree. Every part in the path
 * is a 1, 2, 3 or 4.
 * The length of the path is the zoom level of the tile.
 */
class Path {
private:
	std::vector<int> path;
public:
	Path();
	Path(const std::vector<int>& path);
	~Path();

	const std::vector<int>& getPath() const;
	int getDepth() const;
	TilePos getTilePos() const;

	Path& operator+=(int node);
	Path operator+(int node) const;

	Path parent() const;

	bool operator==(const Path& other) const;
	bool operator<(const Path& other) const;

	std::string toString() const;

	static Path byTilePos(const TilePos& tile, int depth);
};

std::ostream& operator<<(std::ostream& stream, const Path& path);
std::ostream& operator<<(std::ostream& stream, const TilePos& tile);

/**
 * This class manages all tiles required to render a world.
 */
class TileSet {
private:
	mc::World& world;
	// time of the last check
	int last_check_time;

	// depth of the tile set = maximum zoom level
	int depth;

	// all available top level tiles
	std::set<TilePos> render_tiles;
	// the top level tiles which actually need to get rendered
	std::set<TilePos> required_render_tiles;
	// same here for composite tiles
	std::set<Path> composite_tiles;
	std::set<Path> required_composite_tiles;

	void initMapSize();
	void findRequiredRenderTiles();
	void findRequiredCompositeTiles(const std::set<TilePos>& render_tiles,
			std::set<Path>& tiles);
public:
	TileSet(mc::World& World, int last_check_time);
	virtual ~TileSet();

	int getRequiredRenderTilesCount() const;
	int getRequiredCompositeTilesCount() const;
	int getMaxZoom() const;

	bool hasTile(const Path& path) const;
	bool isTileRequired(const Path& path) const;

	const std::set<TilePos>& getAvailableRenderTiles() const;
	const std::set<Path>& getAvailableCompositeTiles() const;
	const std::set<TilePos>& getRequiredRenderTiles() const;
	const std::set<Path>& getRequiredCompositeTiles() const;

	int findRenderTasks(int worker_count, std::vector<std::map<Path, int> >& workers) const;
};

}
}

#endif /* TILE_H_ */
