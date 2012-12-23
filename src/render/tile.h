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

#ifndef TILE_H_
#define TILE_H_

#include "mc/world.h"
#include "render/textures.h"

#include <set>
#include <vector>
#include <string>

namespace mapcrafter {
namespace render {

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

	bool operator==(const Path& other) const;
	bool operator<(const Path& other) const;

	std::string toString() const;

	static Path byTilePos(const TilePos& tile, int depth);
};

std::ostream& operator<<(std::ostream& stream, const Path& path);
std::ostream& operator<<(std::ostream& stream, const TilePos& tile);

class TileSet {
private:
	mc::World& world;
	int last_change_time;

	int depth;
	int tile_size;

	std::set<TilePos> render_tiles;
	std::set<TilePos> required_render_tiles;
	std::set<Path> composite_tiles;
	std::set<Path> required_composite_tiles;

	std::set<TilePos> rendered_render_tiles;
	std::set<Path> rendered_composite_tiles;

	void initMapSize();
	void findRequiredRenderTiles();
	int findRequiredCompositeTiles(const Path& path,
	        const std::set<TilePos>& render_tiles, std::set<Path>& tiles);
public:
	TileSet(mc::World& World, int last_change_time);
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
};

}
}

#endif /* TILE_H_ */
