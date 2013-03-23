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

#include "render/render.h"

#include "render/tile.h"
#include "util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

namespace mapcrafter {
namespace render {

TileTopBlockIterator::TileTopBlockIterator(const TilePos& tile, int block_size,
        int tile_size)
		: block_size(block_size), tile_size(tile_size), is_end(false) {
	// at first get the chunk, whose row and column is at the top right of the tile
	mc::ChunkPos topright_chunk = mc::ChunkPos::byRowCol(4 * tile.getY(),
	        2 * tile.getX() + 2);

	// now get the first visible block from this chunk in this tile
	top = mc::LocalBlockPos(8, 6, 255).toGlobalPos(topright_chunk);
	// and set this as start
	current = top;

	// calculate bounds of the tile
	min_row = top.getRow() + 1;
	max_row = top.getRow() + 64 + 4;
	max_col = top.getCol() + 2;
	min_col = max_col - 32;

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

TileRenderer::TileRenderer(mc::WorldCache& world, const BlockTextures& textures)
		: world(world), textures(textures) {
}

TileRenderer::~TileRenderer() {
}

/**
 * Looks for a block and puts the id and data in the variables.
 */
inline void getNeighbor(const mc::BlockPos& pos, uint16_t& id, uint16_t& data,
        mc::WorldCache& world, const mc::Chunk* current_chunk) {
	const mc::Chunk* chunk;
	// check if we already have the right chunk
	mc::ChunkPos pos_chunk(pos);
	if (current_chunk != NULL && pos_chunk == current_chunk->getPos())
		chunk = current_chunk;
	// if not get the chunk from cache
	else
		chunk = world.getChunk(pos_chunk);
	// chunk may be NULL
	if (chunk == NULL) {
		id = 0;
		data = 0;
	// otherwise get id and data
	} else {
		mc::LocalBlockPos local(pos);
		id = chunk->getBlockID(local);
		data = chunk->getBlockData(local);
	}
}

/**
 * This function returns the real face direction for a closed door.
 */
uint16_t getDoorDirectionClosed(uint16_t direction, bool flip) {
	if (!flip) {
		switch (direction) {
		case DOOR_NORTH:
			return DOOR_EAST;
		case DOOR_SOUTH:
			return DOOR_WEST;
		case DOOR_EAST:
			return DOOR_SOUTH;
		case DOOR_WEST:
			return DOOR_NORTH;
		default:
			return 0;
		}
	} else {
		switch (direction) {
		case DOOR_NORTH:
			return DOOR_WEST;
		case DOOR_SOUTH:
			return DOOR_EAST;
		case DOOR_EAST:
			return DOOR_NORTH;
		case DOOR_WEST:
			return DOOR_SOUTH;
		default:
			return 0;
		}
	}
}

/**
 * Checks for a specific block the neighbors and sets extra block data if necessary.
 */
uint16_t TileRenderer::checkNeighbors(const mc::BlockPos& pos, uint16_t id, uint16_t data,
        const mc::Chunk* chunk) const {

	uint16_t id_north, id_south, id_east, id_west, id_bottom;
	uint16_t data_north, data_south, data_east, data_west, data_bottom;

	if ((id == 8 || id == 9) && data == 0) { // full water blocks
		getNeighbor(pos + DIR_WEST, id_west, data_west, world, chunk);
		getNeighbor(pos + DIR_SOUTH, id_south, data_south, world, chunk);

		// check if west and south neighbors are also full water blocks
		if ((id_west == 8 || id_west == 9) && data_west == 0) {
			data |= DATA_WEST;
		} if ((id_south == 8 || id_south == 9) && data_south == 0) {
			data |= DATA_SOUTH;
		}
	} else if (id == 54 || id == 95 || id == 130) { // chests
		// at first get all neighbor blocks
		getNeighbor(pos + DIR_NORTH, id_north, data_north, world, chunk);
		getNeighbor(pos + DIR_SOUTH, id_south, data_south, world, chunk);
		getNeighbor(pos + DIR_EAST, id_east, data_east, world, chunk);
		getNeighbor(pos + DIR_WEST, id_west, data_west, world, chunk);

		// we put here in the data the direction of the chest
		// and if there are neighbor chests

		if (data == 2)
			data = DATA_NORTH;
		else if (data == 3)
			data = DATA_SOUTH;
		else if (data == 4)
			data = DATA_WEST;
		else
			data = DATA_EAST;

		if (id_north == 54)
			data |= DATA_NORTH << 4;
		if (id_south == 54)
			data |= DATA_SOUTH << 4;
		if (id_east == 54)
			data |= DATA_EAST << 4;
		if (id_west == 54)
			data |= DATA_WEST << 4;
	} else if(id == 64 || id == 71) {
		/* doors */
		uint16_t top = data & 8 ? DOOR_TOP : 0;
		uint16_t tmp, top_data, bottom_data;
		// at first get the data of both parts of the door, top and bottom
		if(top) {
			top_data = data;
			getNeighbor(pos + DIR_BOTTOM, tmp, bottom_data, world,chunk);

			data |= DOOR_TOP;
		} else {
			getNeighbor(pos + DIR_TOP, tmp, top_data, world, chunk);
			bottom_data = data;
		}

		// then find out if this door is the left door of a double door
		bool door_flip = top_data & 1;
		if(door_flip)
			data |= DOOR_FLIP_X;
		// find out if the door is openend
		bool opened = !(bottom_data & 4);

		// get the direction of the door
		uint16_t direction = bottom_data & 3;
		if(direction == 0) {
			direction = DOOR_WEST;
		} else if(direction == 1) {
			direction = DOOR_NORTH;
		} else if(direction == 2) {
			direction = DOOR_EAST;
		} else if(direction == 3) {
			direction = DOOR_SOUTH;
		}

		// if the door is closed, the direction need to get changed
		if(!opened) {
			data |= getDoorDirectionClosed(direction, door_flip);
		} else {
			data |= direction;
		}

	} else if(id == 85 || id == 101 || id == 102 || id == 113) {
		// fence, iron bars, glas panes, nether fence
		getNeighbor(pos + DIR_NORTH, id_north, data_north, world, chunk);
		getNeighbor(pos + DIR_SOUTH, id_south, data_south, world, chunk);
		getNeighbor(pos + DIR_EAST, id_east, data_east, world, chunk);
		getNeighbor(pos + DIR_WEST, id_west, data_west, world, chunk);

		// check for same neighbors
		if(id_north != 0 && (id_north == id || !textures.isBlockTransparent(id_north,data_north)))
			data |= DATA_NORTH;
		if(id_south != 0 && (id_south == id || !textures.isBlockTransparent(id_south, data_south)))
			data |= DATA_SOUTH;
		if(id_east != 0 && (id_east == id || !textures.isBlockTransparent(id_east, data_east)))
			data |= DATA_EAST;
		if(id_west != 0 && (id_west == id || !textures.isBlockTransparent(id_west, data_west)))
			data |= DATA_WEST;

		// check fences, they can also connect with fence gates
		if(id == 85 && id_north == 107)
			data |= DATA_NORTH;
		if(id == 85 && id_south == 107)
			data |= DATA_SOUTH;
		if(id == 85 && id_east == 107)
			data |= DATA_EAST;
		if(id == 85 && id_west == 107)
			data |= DATA_WEST;
	}

	if (!textures.isBlockTransparent(id, data)) {
		/* add shadow edges on opaque blockes */
		getNeighbor(pos + DIR_NORTH, id_north, data_north, world, chunk);
		getNeighbor(pos + DIR_EAST, id_east, data_east, world, chunk);
		getNeighbor(pos + DIR_BOTTOM, id_bottom, data_bottom, world, chunk);

		// check if neighbors are opaque
		if(id_north == 0 || textures.isBlockTransparent(id_north, data_north))
			data |= EDGE_NORTH;
		if(id_east == 0 || textures.isBlockTransparent(id_east, data_east))
			data |= EDGE_EAST;
		if(id_bottom == 0 || textures.isBlockTransparent(id_bottom, data_bottom))
			data |= EDGE_BOTTOM;
	}

	return data;
}

void TileRenderer::renderTile(const TilePos& pos, Image& tile) const {
	// some vars, set correct image size
	int block_size = textures.getBlockImageSize();
	int tile_size = textures.getTileSize();
	tile.setSize(tile_size, tile_size);

	// get the maximum count of water blocks,
	// blitted about each over, until they are nearly opaque
	int max_water = textures.getMaxWaterNeededOpaque();

	// all visible blocks, which are rendered in this tile
	std::set<RenderBlock> blocks;

	// our current chunk, have it also here to access it faster
	mc::Chunk* chunk = NULL;

	// iterate over the highest blocks in the tile
	for (TileTopBlockIterator it(pos, block_size, tile_size); !it.end(); it.next()) {
		// water counter
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
			if (chunk == NULL || chunk->getPos() != current_chunk)
				// get chunk if not
				chunk = world.getChunk(current_chunk);
			if (chunk == NULL)
				continue;

			// get local block position
			mc::LocalBlockPos local(block.current);

			// now get block id
			uint16_t id = chunk->getBlockID(local);
			// air is completely transparent so continue
			if (id == 0)
				continue;

			// now get the block data
			uint16_t data = chunk->getBlockData(local);

			// check for water
			if ((id == 8 || id == 9) && data == 0) {
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
							uint16_t south, west, data_south, data_west;
							getNeighbor(top.pos + DIR_SOUTH, south, data_south, world,chunk);
							getNeighbor(top.pos + DIR_WEST, west, data_west, world, chunk);

							bool neighbor_south = (south == 8 || south == 9);
							bool neighbor_west = (west == 8 || west == 9);

							// get image and replace the old render block with this
							top.image = textures.getOpaqueWater(neighbor_south,
							        neighbor_west);
							row_nodes.insert(top);
							break;

						} else {
							// water render block
							row_nodes.erase(current);
						}
					}

					break;
				}
			} else
				// if not water, reset the counter
				water = 0;

			// check for special data (neighbor related)
			// get block image, check for transparency, create render block...
			data = checkNeighbors(block.current, id, data, chunk);
			Image image = textures.getBlock(id, data);
			bool transparent = textures.isBlockTransparent(id, data);

			RenderBlock node;
			node.x = it.draw_x;
			node.y = it.draw_y;
			node.pos = block.current;
			node.image = image;
			node.id = id;
			node.data = data;

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
}

}
}
