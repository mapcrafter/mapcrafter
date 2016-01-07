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

#ifndef TILE_H_
#define TILE_H_

#include <map>
#include <set>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

/**
 * The renderer renders the world to tiles which are arranged in a quadtree. Every node
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
 * the world and all chunks, calculates the maximum needed zoom level and finds out which
 * tiles exist and which tiles need to get rendered (useful for incremental rendering
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

namespace mc {
class ChunkPos;
class World;
}

namespace renderer {

/**
 * This class represents the position of a tile in the quadtree.
 */
class TilePos {
public:
	TilePos(int x = 0, int y = 0);

	/**
	 * Returns x tile coordinate.
	 */
	int getX() const;

	/**
	 * Returns the y tile coordinate.
	 */
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

std::ostream& operator<<(std::ostream& stream, const TilePos& tile);

/**
 * This class represents the path to a tile in the quadtree.
 * Every part in the path is a 1, 2, 3 or 4.
 * The length of the path is the zoom level of the tile.
 */
class TilePath {
public:
	TilePath();
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

	/**
	 * Constructs a path (with a specified zoom level) to a tile position.
	 * Opposite of getTilePos-method.
	 */
	static TilePath byTilePos(const TilePos& tile, int depth);

private:
	std::vector<int> path;
};

std::ostream& operator<<(std::ostream& stream, const TilePath& path);

/**
 * This class manages all tiles required to render a world.
 */
class TileSet {
public:
	TileSet(int tile_width);
	virtual ~TileSet();

	virtual void mapChunkToTiles(const mc::ChunkPos& chunk, std::set<TilePos>& tiles) = 0;

	/**
	 * Scans the tiles of a world.
	 * If you use the constructor with a world object as parameter, this method is
	 * automatically called.
	 *
	 * The auto_center parameter describes whether it should automatically center the
	 * found tiles. If set to false (default), it will use tile_offset as center. The
	 * default value for tile_offset is (0, 0) when using scan without the
	 * auto_center and tile_offset parameters.
	 */
	void scan(const mc::World& world);
	void scan(const mc::World& world, bool auto_center, TilePos& tile_offset);

	/**
	 * Resets which tiles are required / not required. All tiles will be required.
	 */
	void resetRequired();

	/**
	 * Scans which tiles are required by testing which tiles were probably changed since
	 * the timestamp last_change.
	 */
	void scanRequiredByTimestamp(int last_change);

	/**
	 * Scans which tiles are required by using the modification times of the already
	 * rendered image files.
	 */
	void scanRequiredByFiletimes(const fs::path& output_dir,
			std::string image_format = "png");

	/**
	 * Returns the width of the tiles in chunks.
	 */
	int getTileWidth() const;

	/**
	 * Returns the minimum maximum zoom level required to render all render tiles.
	 */
	int getMinDepth() const;

	/**
	 * Returns the actual used maximum zoom level of the tile set.
	 */
	int getDepth() const;

	/**
	 * Sets the maximum zoom level to use. The supplied zoom level must be at least
	 * the minimum maximum zoom level.
	 */
	void setDepth(int depth);

	/**
	 * Returns the tile position offset.
	 */
	const TilePos& getTileOffset() const;

	/**
	 * Returns if a specific tile is contained in the tile set.
	 */
	bool hasTile(const TilePath& path) const;

	/**
	 * Returns if a specific tile is required, e.g. needs to get rendered.
	 */
	bool isTileRequired(const TilePath& path) const;

	/**
	 * Returns the count of required render tiles.
	 */
	int getRequiredRenderTilesCount() const;

	/**
	 * Returns the required render tiles.
	 */
	const std::set<TilePos>& getRequiredRenderTiles() const;

	/**
	 * Returns the count of required composite tiles.
	 */
	int getRequiredCompositeTilesCount() const;

	/**
	 * Returns the required composite tiles.
	 */
	const std::set<TilePath>& getRequiredCompositeTiles() const;

	/**
	 * Returns the count of required render tiles a specific composite tiles contains.
	 */
	int getContainingRenderTiles(const TilePath& tile) const;

private:
	// width of the tiles in chunks
	int tile_width;

	// the minimum maximum zoom level which would be required to render all tiles
	int min_depth;
	// actual maximum zoom level used by the tile set
	int depth;

	// offset of render tiles, used to center the map
	// tile_offset means, that all render tiles have the position pos,
	// but are actually rendered as pos+tile_offset
	TilePos tile_offset;

	// all available render tiles
	// (= tiles with the highest zoom level, tree leaves in the quadtree)
	std::set<TilePos> render_tiles;
	// the render tiles which actually need to get rendered
	std::set<TilePos> required_render_tiles;
	// timestamps of render tiles required to re-render a tile
	// (= highest timestamp of all chunks in a tile)
	std::map<TilePos, int> tile_timestamps;

	// same here for composite tiles
	std::set<TilePath> composite_tiles;
	std::set<TilePath> required_composite_tiles;

	// count of required render tiles contained in a composite tile
	std::map<TilePath, int> containing_render_tiles;

	/**
	 * This method finds out which render level tiles a world has and which maximum
	 * zoom level would be required to render them.
	 *
	 * The auto_center parameter describes whether it should automatically center the
	 * found tiles. If set to false (default), it will use tile_offset as center.
	 */
	void findRenderTiles(const mc::World& world, bool auto_center, TilePos& tile_offset);

	/**
	 * This method finds out which composite tiles are needed, depending on a
	 * list of available/required render tiles, and puts them into a set.
	 * So we can find out which composite tiles are available and which composite tiles
	 * need to get rendered.
	 */
	void findRequiredCompositeTiles(const std::set<TilePos>& render_tiles,
			std::set<TilePath>& tiles);

	/**
	 * Updates the containing_render_tiles map.
	 */
	void updateContainingRenderTiles();
};

}
}

#endif /* TILE_H_ */
