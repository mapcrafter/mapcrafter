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

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace mapcrafter {
namespace renderer {

TileTopBlockIterator::TileTopBlockIterator(const TilePos& tile, int block_size,
		int tile_width)
		: block_size(block_size), is_end(false) {
	// row/col 0,0 are the top left chunk of the tile 0,0
	// each tile is four rows high, two columns wide
	
	// at first get the chunk, whose row and column is at the top right of the tile
	// top right chunk of a tile is the top left chunk of the tile x+1,y
	mc::ChunkPos topright_chunk = mc::ChunkPos::byRowCol(
			4 * tile_width * tile.getY(),
			2 * tile_width * (tile.getX() + 1));

	// now get the first visible block from this chunk in this tile
	top = mc::LocalBlockPos(8, 6, mc::CHUNK_HEIGHT * 16 - 1).toGlobalPos(topright_chunk);
	// and set this as start
	current = top;

	// calculate bounds of the tile
	min_row = top.getRow() + 1;
	max_row = top.getRow() + (64 * tile_width) + 4;
	max_col = top.getCol() + 2;
	min_col = max_col - (32 * tile_width);

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

bool RenderBlock::operator<(const RenderBlock& other) const {
	return pos < other.pos;
}

IsometricTileRenderer::IsometricTileRenderer(const RenderView* render_view,
		BlockImages* images, int tile_width, mc::WorldCache* world,
		RenderMode* render_mode)
	: TileRenderer(render_view, images, tile_width, world, render_mode) {
}

IsometricTileRenderer::~IsometricTileRenderer() {
}

void IsometricTileRenderer::renderTile(const TilePos& tile_pos, RGBAImage& tile) {
	// some vars, set correct image size
	int block_size = images->getBlockSize();
	tile.setSize(getTileSize(), getTileSize());

	// get the maximum count of water blocks
	// blitted about each over, until they are nearly opaque
	int max_water = images->getMaxWaterPreblit();

	// all visible blocks which are rendered in this tile
	std::set<RenderBlock> blocks;

	// iterate over the highest blocks in the tile
	// we use as tile position tile_pos+tile_offset because the offset means that
	// we treat the tile position as tile_pos, but it's actually tile_pos+tile_offset
	for (TileTopBlockIterator it(tile_pos, block_size, tile_width);
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
			mc::ChunkPos current_chunk_pos(block.current);

			// check if current chunk is not null
			// and if the chunk wasn't replaced in the cache (i.e. position changed)
			if (current_chunk == nullptr || current_chunk->getPos() != current_chunk_pos)
				// get chunk if not
				//if (!state.world->hasChunkSection(current_chunk, block.current.y))
				//	continue;
				current_chunk = world->getChunk(current_chunk_pos);
			if (current_chunk == nullptr) {
				// here is nothing (= air),
				// so reset state if we are in water
				in_water = false;
				continue;
			}

			// get local block position
			mc::LocalBlockPos local(block.current);

			// now get block id and block data
			uint16_t id, data, extra_data;
            current_chunk->getBlockInfo(local, id, data, extra_data);

			// air is completely transparent so continue
			if (id == 0) {
				in_water = false;
				continue;
			}

			// check if the render mode hides this block
			if (render_mode->isHidden(block.current, id, data))
				continue;

			bool is_water = (id == 8 || id == 9) && data == 0;
			if (is_water && !use_preblit_water) {
				// water render behavior n1:
				// render only the top sides of the water blocks
				// and darken the ground with the lighting data
				// used for lighting rendermode

				// if we are already in water, skip checking this water block
				if (is_water && in_water)
					continue;
				in_water = is_water;

			} else if (use_preblit_water) {
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
								south = getBlock(top.pos + mc::DIR_SOUTH);
								west = getBlock(top.pos + mc::DIR_WEST);

								id = 8;
								data = OPAQUE_WATER;
								bool neighbor_south = !south.isFullWater();
								if (neighbor_south)
								//	data |= DATA_SOUTH;
									data |= OPAQUE_WATER_SOUTH;
								bool neighbor_west = !west.isFullWater();
								if (neighbor_west)
								//	data |= DATA_WEST;
									data |= OPAQUE_WATER_WEST;

								// get image and replace the old render block with this
								//top.image = images->getOpaqueWater(neighbor_south,
								//		neighbor_west);
								top.image = images->getBlock(id, data, extra_data);

								// don't forget the render mode
								render_mode->draw(top.image, top.pos, id, data);

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
			bool transparent = images->isBlockTransparent(id, data);

			// check for biome data
			if (Biome::isBiomeBlock(id, data))
				image = images->getBiomeBlock(id, data, getBiomeOfBlock(block.current, current_chunk), extra_data);
			else
				image = images->getBlock(id, data, extra_data);

			RenderBlock node;
			node.x = it.draw_x;
			node.y = it.draw_y;
			node.pos = block.current;
			node.image = image;
			node.id = id;
			node.data = data;

			// let the render mode do their magic with the block image
			render_mode->draw(node.image, node.pos, id, data);

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
		tile.alphaBlit(it->image, it->x, it->y);
	}
}

int IsometricTileRenderer::getTileSize() const {
	return images->getBlockSize() * 16 * tile_width;
}

}
}
