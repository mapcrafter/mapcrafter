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

#include "tilerenderer.h"

#include "../../biomes.h"
#include "../../blockimages.h"
#include "../../image.h"
#include "../../rendermode.h"
#include "../../tileset.h"
#include "../../../mc/pos.h"
#include "../../../mc/worldcache.h"
#include "../../../util.h"

#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace mapcrafter {
namespace renderer {

SideTileRenderer::SideTileRenderer(const RenderView* render_view,
		mc::BlockStateRegistry& block_registry,
		BlockImages* images, int tile_width, mc::WorldCache* world,
		RenderMode* render_mode)
	: TileRenderer(render_view, block_registry, images, tile_width, world, render_mode) {
}

SideTileRenderer::~SideTileRenderer() {
}

int SideTileRenderer::getTileSize() const {
	assert(false);
	return 0;
	//return images->getBlockSize() * 16 * tile_width;
}

int SideTileRenderer::getTileWidth() const {
	return block_images->getBlockWidth() * 16 * tile_width;
}

int SideTileRenderer::getTileHeight() const {
	return block_images->getBlockHeight() * 8 * tile_width;
}

void SideTileRenderer::renderTopBlocks(const TilePos& tile_pos, std::set<TileImage>& tile_images) {
	int block_width = block_images->getBlockWidth();
	int block_height = block_images->getBlockHeight();
	for (int cx = 0; cx < tile_width; cx++) {
		for (int cz = 0; cz < tile_width; cz++) {
			mc::ChunkPos chunkpos(tile_pos.getX() * tile_width + cx, tile_pos.getY() * tile_width + cz);
			mc::BlockPos blockpos(chunkpos.x * 16, chunkpos.z * 16, mc::CHUNK_HEIGHT*16 - 1);

			int dx = block_width * cx * 16;
			int dz = block_height * cz * 8;
			for (int z = 0; z < 16 + 1; z++) {
				for (int x = 0; x < 16; x++) {
					int px = dx + x * block_width;
					int py = dz + z * block_height/2 - block_height/2;
					renderBlocks(px, py, blockpos + mc::BlockPos(x, z, 0), mc::BlockPos(0, -1, -1), tile_images);
				}
			}
		}
	}
}

}
}
