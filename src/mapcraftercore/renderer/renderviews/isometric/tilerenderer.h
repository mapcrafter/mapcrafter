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

#ifndef ISOMETRIC_TILERENDERER_H_
#define ISOMETRIC_TILERENDERER_H_

#include "../../tilerenderer.h"
#include "blockimages.h"
#include "../../image.h"
#include "tileset.h"
#include "../../../config/sections/map.h"
#include "../../../config/sections/world.h"
#include "../../../mc/pos.h"
#include "../../../mc/worldcache.h"
#include "../../../util.h"

#include <memory>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace renderer {

/**
 * Iterates over the top blocks of a tile.
 */
class TileTopBlockIterator {
private:
	int block_size, tile_size;

	bool is_end;
	int min_row, max_row;
	int min_col, max_col;
	mc::BlockPos top;
public:
	TileTopBlockIterator(const TilePos& tile, int block_size, int tile_size);
	~TileTopBlockIterator();

	void next();
	bool end() const;

	mc::BlockPos current;
	int draw_x, draw_y;
};

/**
 * Iterates over the blocks, which are on a tile on the same position,
 * this means every block is (x+1, z-1 and y-1) of the last block
 */
class BlockRowIterator {
public:
	BlockRowIterator(const mc::BlockPos& block);
	~BlockRowIterator();

	void next();
	bool end() const;

	mc::BlockPos current;
};

/**
 * A block, which should get drawed on a tile.
 */
struct RenderBlock {

	// drawing position in pixels on the tile
	int x, y;
	bool transparent;
	RGBAImage image;
	mc::BlockPos pos;
	uint8_t id, data;

	bool operator<(const RenderBlock& other) const;
};

/**
 * Renders tiles from world data.
 */
class IsometricTileRenderer : public TileRenderer {
public:
	IsometricTileRenderer(std::shared_ptr<BlockImages> images,
			std::shared_ptr<mc::WorldCache> world, RenderModes& render_modes);
	virtual ~IsometricTileRenderer();

	void setUsePreblitWater(bool use_preblit_water);

	virtual void renderTile(const TilePos& tile_pos, RGBAImage& tile);

	virtual int getTileSize() const;

protected:
	bool use_preblit_water;
};

}
}

#endif /* ISOMETRIC_TILERENDERER_H_ */
