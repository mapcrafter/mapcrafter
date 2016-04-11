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

#ifndef TILERENDERER_H_
#define TILERENDERER_H_

#include "biomes.h"
#include "../mc/worldcache.h" // mc::DIR_*

#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {

// some forward declarations
namespace mc {
class BlockPos;
class Chunk;
}

namespace renderer {

class BlockHandler;
class BlockImages;
class Overlay;
class TilePos;
class RenderMode;
class RenderView;
class RGBAImage;

class TileRenderer {
public:
	TileRenderer(const RenderView* render_view, BlockHandler* block_handler,
			BlockImages* images, int tile_width,
			mc::WorldCache* world, RenderMode* render_mode,
			std::shared_ptr<Overlay> hardcode_overlay,
			std::vector<std::shared_ptr<Overlay>> overlays);
	virtual ~TileRenderer();

	void setRenderBiomes(bool render_biomes);
	void setUsePreblitWater(bool use_preblit_water);

	virtual void renderTile(const TilePos& tile_pos, RGBAImage& tile, std::vector<RGBAImage>& overlay_tiles) = 0;

	virtual int getTileSize() const = 0;

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);
	Biome getBiomeOfBlock(const mc::BlockPos& pos, const mc::Chunk* chunk);

	void drawHardcodeOverlay(RGBAImage& block, const mc::BlockPos& pos, uint16_t id, uint16_t data) const;

	BlockHandler* block_handler;
	BlockImages* images;
	int tile_width;
	mc::WorldCache* world;
	mc::Chunk* current_chunk;
	RenderMode* render_mode;
	std::shared_ptr<Overlay> hardcode_overlay;
	std::vector<std::shared_ptr<Overlay>> overlays;

	bool render_biomes;
	bool use_preblit_water;
};

}
}

#endif /* TILERENDERER_H_ */
