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

#include "tilerenderer.h"

#include "../../rendermode.h"
#include "../../biomes.h"

#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace mapcrafter {
namespace renderer {

TopdownTileRenderer::TopdownTileRenderer(std::shared_ptr<BlockImages> images,
		std::shared_ptr<mc::WorldCache> world)
	: TileRenderer(images, world) {
}

TopdownTileRenderer::~TopdownTileRenderer() {
}

void TopdownTileRenderer::renderChunk(const mc::Chunk& chunk, RGBAImage& tile, int dx, int dy) {
	int texture_size = images->getTextureSize();

	for (int x = 0; x < 16; x++)
		for (int z = 0; z < 16; z++) {
			std::deque<RGBAImage> blocks;

			mc::LocalBlockPos localpos(x, z, 0);
			//int height = chunk.getHeightAt(localpos);
			//localpos.y = height;
			localpos.y = -1;
			if (localpos.y > 256 || localpos.y < 0)
				localpos.y = 255;

			uint16_t id = chunk.getBlockID(localpos);
			while (id == 0 && localpos.y > 0) {
				localpos.y--;
				id = chunk.getBlockID(localpos);
			}
			if (localpos.y < 0)
				continue;

			while (localpos.y > 0) {
				mc::BlockPos globalpos = localpos.toGlobalPos(chunk.getPos());

				id = chunk.getBlockID(localpos);
				if (id == 0) {
					localpos.y--;
					continue;
				}
				uint16_t data = chunk.getBlockData(localpos);
				RGBAImage block = images->getBlock(id, data);
				if (Biome::isBiomeBlock(id, data)) {
					block = images->getBiomeDependBlock(id, data, getBiomeOfBlock(globalpos, &chunk));
				}
				//for (size_t i = 0; i < rendermodes.size(); i++)
				//	rendermodes[i]->draw(block, globalpos, id, data);
				blocks.push_back(block);
				if (!images->isBlockTransparent(id, data)) {
					break;
				}
				localpos.y--;
			}

			while (blocks.size() > 0) {
				RGBAImage block = blocks.back();
				tile.alphablit(block, dx + x*texture_size, dy + z*texture_size);
				blocks.pop_back();
			}
		}
}

void TopdownTileRenderer::renderTile(const TilePos& tile_pos, RGBAImage& tile) {
	int texture_size = images->getTextureSize();
	tile.setSize(getTileSize(), getTileSize());

	// call start method of the rendermodes
	// for (size_t i = 0; i < rendermodes.size(); i++)
	// 	rendermodes[i]->start();

	for (int x = 0; x < TILE_WIDTH; x++)
		for (int z = 0; z < TILE_WIDTH; z++) {
			mc::ChunkPos chunkpos(tile_pos.getX() * TILE_WIDTH + x, tile_pos.getY() * TILE_WIDTH + z);
			current_chunk = world->getChunk(chunkpos);
			if (current_chunk != nullptr)
				renderChunk(*current_chunk, tile, texture_size*16*x, texture_size*16*z);
		}

	// call the end method of the rendermodes
	// for (size_t i = 0; i < rendermodes.size(); i++)
	// 	rendermodes[i]->end();
}

int TopdownTileRenderer::getTileSize() const {
	// TODO tile_width
	return images->getBlockSize() * 16;
}

}
}
