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

#include "../../rendermodes/base.h"
#include "../../biomes.h"

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace mapcrafter {
namespace renderer {

TileTopBlockIterator::TileTopBlockIterator(const TilePos& tile, int block_size,
        int tile_size)
		: block_size(block_size), tile_size(tile_size), is_end(false) {
	// at first get the chunk, whose row and column is at the top right of the tile
	mc::ChunkPos topright_chunk = mc::ChunkPos::byRowCol(4 * TILE_WIDTH * tile.getY(),
			2 * TILE_WIDTH * tile.getX() + 2);

	// now get the first visible block from this chunk in this tile
	top = mc::LocalBlockPos(8, 6, mc::CHUNK_HEIGHT * 16 - 1).toGlobalPos(topright_chunk);
	// and set this as start
	current = top;

	// calculate bounds of the tile
	min_row = top.getRow() + 1;
	max_row = top.getRow() + (64 * TILE_WIDTH) + 4;
	max_col = top.getCol() + 2;
	min_col = max_col - (32 * TILE_WIDTH);

	// calculate position of the first block, relative row/col in this tile are needed
	int row = current.getRow() - min_row;
	int col = current.getCol() - min_col;
	// every column is a 1/2 block and every row is a 1/4 block
	draw_x = col * block_size / 2;
	// -1/2 blocksize, because we would see the top side of the blocks in the tile if not
	draw_y = row * block_size / 4 - block_size / 2; // -16
}

TileTopBlockIterator::~TileTopBlockIterator() {

}

void TileTopBlockIterator::next() {
	if (is_end)
		return;

	// go one block to bottom right (z+1)
	current += mc::BlockPos(0, 1, 0);

	// check if row/col is too big
	if (current.getCol() > max_col || current.getRow() > max_row) {
		// move the top one block to the left
		top -= mc::BlockPos(1, 1, 0);
		// and set the current block to the top block
		current = top;

		// check if the current top block is out of the tile
		if (current.getCol() < min_col - 1) {
			// then move it by a few blocks to bottom right
			current += mc::BlockPos(0, min_col - current.getCol() - 1, 0);
		}
	}

	// now calculate the block position like in the constructor
	int row = current.getRow();
	int col = current.getCol();
	draw_x = (col - min_col) * block_size / 2;
	draw_y = (row - min_row) * block_size / 4 - block_size / 2; // -16

	// and set end if reached
	if (row == max_row && col == min_col)
		is_end = true;
	else if (row == max_row && col == min_col + 1)
		is_end = true;
}

bool TileTopBlockIterator::end() const {
	return is_end;
}

BlockRowIterator::BlockRowIterator(const mc::BlockPos& block) {
	current = block;
}

BlockRowIterator::~BlockRowIterator() {

}

void BlockRowIterator::next() {
	//current += mc::BlockPos(1, -1, -1);
	current.x++;
	current.z--;
	current.y--;
}

bool BlockRowIterator::end() const {
	return current.y < 0;
}

mc::Block RenderState::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, chunk, get);
}

bool RenderBlock::operator<(const RenderBlock& other) const {
	return pos < other.pos;
}

IsometricTileRenderer::IsometricTileRenderer()
	: TileRenderer(), water_preblit(false) {
}

IsometricTileRenderer::~IsometricTileRenderer() {
}

void IsometricTileRenderer::setStuff(std::shared_ptr<mc::WorldCache> world,
		std::shared_ptr<BlockImages> images,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) {
	TileRenderer::setStuff(world, images, world_config, map_config);

	this->water_preblit = map_config.getRendermode() != "daylight" && map_config.getRendermode() != "nightlight";

	rendermodes.clear();
	createRendermode(world_config, map_config, state, rendermodes);
}

void IsometricTileRenderer::renderTile(const TilePos& tile_pos, RGBAImage& tile) {
	// some vars, set correct image size
	int block_size = state.images->getBlockSize();
	tile.setSize(getTileSize(), getTileSize());

	// get the maximum count of water blocks
	// blitted about each over, until they are nearly opaque
	int max_water = state.images->getMaxWaterNeededOpaque();

	// all visible blocks which are rendered in this tile
	std::set<RenderBlock> blocks;

	// call start method of the rendermodes
	for (size_t i = 0; i < rendermodes.size(); i++)
		rendermodes[i]->start();

	// iterate over the highest blocks in the tile
	// we use as tile position tile_pos+tile_offset because the offset means that
	// we treat the tile position as tile_pos, but it's actually tile_pos+tile_offset
	for (TileTopBlockIterator it(tile_pos, block_size, getTileSize());
			!it.end(); it.next()) {
		// water render behavior n1:
		// are we already in a row of water?
		bool in_water = false;

		// water render behavior n2:
		// water counter, how many water blocks are at the moment in this row?
		int water = 0;

		// the render block objects in our current block row
		std::set<RenderBlock> row_nodes;
		// then iterate over the blocks, which are on the tile at the same position,
		// beginning from the highest block
		for (BlockRowIterator block(it.current); !block.end(); block.next()) {
			// get current chunk position
			mc::ChunkPos current_chunk(block.current);

			// check if current chunk is not null
			// and if the chunk wasn't replaced in the cache (i.e. position changed)
			if (state.chunk == nullptr || state.chunk->getPos() != current_chunk)
				// get chunk if not
				//if (!state.world->hasChunkSection(current_chunk, block.current.y))
				//	continue;
				state.chunk = state.world->getChunk(current_chunk);
			if (state.chunk == nullptr) {
				// here is nothing (= air),
				// so reset state if we are in water
				in_water = false;
				continue;
			}

			// get local block position
			mc::LocalBlockPos local(block.current);

			// now get block id
			uint16_t id = state.chunk->getBlockID(local);
			// air is completely transparent so continue
			if (id == 0) {
				in_water = false;
				continue;
			}

			// now get the block data
			uint16_t data = state.chunk->getBlockData(local);

			// check if a rendermode hides this block
			bool visible = true;
			for (size_t i = 0; i < rendermodes.size(); i++) {
				if (rendermodes[i]->isHidden(block.current, id, data)) {
					visible = false;
					break;
				}
			}
			if (!visible)
				continue;

			bool is_water = (id == 8 || id == 9) && data == 0;
			if (is_water && !water_preblit) {
				// water render behavior n1:
				// render only the top sides of the water blocks
				// and darken the ground with the lighting data
				// used for lighting rendermode

				// if we are already in water, skip checking this water block
				if (is_water && in_water)
					continue;
				in_water = is_water;

			} else if (water_preblit) {
				// water render behavior n2:
				// render the top side of every water block
				// have also preblit water blocks to skip redundant alphablitting

				// no lighting is needed because the 'opaque-water-effect'
				// is created by blitting the top sides of the water blocks
				// one above the other

				if (!is_water) {
					// if not water, reset the counter
					water = 0;
				} else {
					water++;

					// when we have enough water in a row
					// we can stop searching more blocks
					// and replace the already added render blocks with a preblit water block
					if (water > max_water) {
						std::set<RenderBlock>::const_iterator it = row_nodes.begin();
						// iterate through the render blocks in this row
						while (it != row_nodes.end()) {
							std::set<RenderBlock>::const_iterator current = it++;
							// check if we have reached the top most water block
							if (it == row_nodes.end() || (it->id != 8 && it->id != 9)) {
								RenderBlock top = *current;
								row_nodes.erase(current);

								// check for neighbors
								mc::Block south, west;
								south = state.getBlock(top.pos + mc::DIR_SOUTH);
								west = state.getBlock(top.pos + mc::DIR_WEST);

								bool neighbor_south = (south.id == 8 || south.id == 9);
								if (neighbor_south)
									data |= DATA_SOUTH;
								bool neighbor_west = (west.id == 8 || west.id == 9);
								if (neighbor_west)
									data |= DATA_WEST;

								// get image and replace the old render block with this
								top.image = state.images->getOpaqueWater(neighbor_south,
										neighbor_west);

								// don't forget the rendermodes
								for (size_t i = 0; i < rendermodes.size(); i++)
									rendermodes[i]->draw(top.image, top.pos, id, data);

								row_nodes.insert(top);
								break;

							} else {
								// water render block
								row_nodes.erase(current);
							}
						}

						break;
					}
				}
			}

			// check for special data (neighbor related)
			// get block image, check for transparency, create render block...
			data = checkNeighbors(block.current, id, data);
			//if (is_water && (data & DATA_WEST) && (data & DATA_SOUTH))
			//	continue;
			RGBAImage image;
			bool transparent = state.images->isBlockTransparent(id, data);

			// check for biome data
			if (Biome::isBiomeBlock(id, data))
				image = state.images->getBiomeDependBlock(id, data, getBiomeOfBlock(block.current, state.chunk));
			else
				image = state.images->getBlock(id, data);

			RenderBlock node;
			node.x = it.draw_x;
			node.y = it.draw_y;
			node.pos = block.current;
			node.image = image;
			node.id = id;
			node.data = data;

			// let the rendermodes do their magic with the block image
			for (size_t i = 0; i < rendermodes.size(); i++)
				rendermodes[i]->draw(node.image, node.pos, id, data);

			// insert into current row
			row_nodes.insert(node);

			// if this block is not transparent, then break
			if (!transparent)
				break;
		}

		// iterate through the created render blocks
		for (std::set<RenderBlock>::const_iterator it = row_nodes.begin();
		        it != row_nodes.end(); ++it) {
			std::set<RenderBlock>::const_iterator next = it;
			next++;
			// insert render block to
			if (next == row_nodes.end()) {
				blocks.insert(*it);
			} else {
				// skip unnecessary leaves
				if (it->id == 18 && next->id == 18 && (next->data & 3) == (it->data & 3))
					continue;
				blocks.insert(*it);
			}
		}
	}

	// now blit all blocks
	for (std::set<RenderBlock>::const_iterator it = blocks.begin(); it != blocks.end();
			++it) {
		tile.alphablit(it->image, it->x, it->y);
	}

	// call the end method of the rendermodes
	for (size_t i = 0; i < rendermodes.size(); i++)
		rendermodes[i]->end();
}

int IsometricTileRenderer::getTileSize() const {
	// TODO tile_width
	return state.images->getBlockSize() * 16;
}

}
}
