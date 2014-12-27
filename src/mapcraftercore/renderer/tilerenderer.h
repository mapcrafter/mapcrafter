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

#ifndef TILERENDERER_H_
#define TILERENDERER_H_

#include "image.h"
#include "blockimages.h"
#include "rendermode.h"
#include "tileset.h"
#include "../config/sections/map.h"
#include "../config/sections/world.h"
#include "../mc/pos.h"
#include "../mc/worldcache.h"
#include "../util.h"

#include <memory>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace renderer {

class TileRenderer {
public:
	TileRenderer(std::shared_ptr<BlockImages> images, int tile_width,
			std::shared_ptr<mc::WorldCache> world, RenderModes& render_modes);
	virtual ~TileRenderer();

	void setRenderBiomes(bool render_biomes);

	virtual void renderTile(const TilePos& tile_pos, RGBAImage& tile) = 0;

	virtual int getTileSize() const = 0;

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);
	Biome getBiomeOfBlock(const mc::BlockPos& pos, const mc::Chunk* chunk);
	uint16_t checkNeighbors(const mc::BlockPos& pos, uint16_t id, uint16_t data);

	std::shared_ptr<BlockImages> images;
	int tile_width;
	std::shared_ptr<mc::WorldCache> world;
	mc::Chunk* current_chunk;
	RenderModes render_modes;

	bool render_biomes;
};

}
}

#endif /* TILERENDERER_H_ */
