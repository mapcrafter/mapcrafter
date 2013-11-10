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

#include "render.h"

#include "biomes.h"
#include "rendermodes/base.h"

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

mc::Block RenderState::getBlock(const mc::BlockPos& pos, int get) {
	return world.getBlock(pos, chunk, get);
}

bool RenderBlock::operator<(const RenderBlock& other) const {
	return pos < other.pos;
}

TileRenderer::TileRenderer()
		: state(), render_biomes(false), water_preblit(true) {
}

TileRenderer::TileRenderer(const mc::WorldCache& world, const BlockImages& images,
        const config::RenderWorldConfig& config)
		: state(world, images), render_biomes(config.render_biomes),
		  water_preblit(config.rendermode != "daylight"
				  && config.rendermode != "nightlight") {
	createRendermode(config.rendermode, config, state, rendermodes);
}

TileRenderer::~TileRenderer() {
}

Biome TileRenderer::getBiome(const mc::BlockPos& pos, const mc::Chunk* chunk) {
	uint8_t biome_id = chunk->getBiomeAt(mc::LocalBlockPos(pos));
	Biome biome = BIOMES[DEFAULT_BIOME];
	if (render_biomes && biome_id < BIOMES_SIZE)
		biome = BIOMES[biome_id];
	else
		return biome;
	int count = 1;

	// get average biome data to make smooth edges between
	// different biomes
	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++) {
			if (dx == 0 && dz == 0)
				continue;

			mc::BlockPos other = pos + mc::BlockPos(dx, dz, 0);
			mc::ChunkPos chunk_pos(other);
			uint8_t other_id = chunk->getBiomeAt(mc::LocalBlockPos(other));
			if (chunk_pos != chunk->getPos()) {
				mc::Chunk* other_chunk = state.world.getChunk(chunk_pos);
				if (other_chunk == nullptr)
					continue;
				other_id = other_chunk->getBiomeAt(mc::LocalBlockPos(other));
			}

			if (other_id < BIOMES_SIZE) {
				biome += BIOMES[other_id];
				count++;
			}
		}

	biome /= count;
	return biome;
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
uint16_t TileRenderer::checkNeighbors(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	mc::Block north, south, east, west, top, bottom;

	if (id == 2) { // grass blocks
		// check if snow is on top to use the snowy sides instead of grass
		top = state.getBlock(pos + mc::DIR_TOP);
		if (top.id == 78 || top.id == 80)
			data |= GRASS_SNOW;

	} else if ((id == 8 || id == 9) && data == 0) { // full water blocks
		west = state.getBlock(pos + mc::DIR_WEST);
		south = state.getBlock(pos + mc::DIR_SOUTH);
		top = state.getBlock(pos + mc::DIR_TOP);

		// check if the neighbors on visible faces (top, west, south)
		// are also full water blocks
		if (!water_preblit && top.isFullWater())
			data |= DATA_TOP;

		if (west.isFullWater())
			data |= DATA_WEST;

		if (south.isFullWater())
			data |= DATA_SOUTH;
	} else if (id == 54 || id == 95 || id == 130 || id == 146) { // chests
		// at first get all neighbor blocks
		north = state.getBlock(pos + mc::DIR_NORTH);
		south = state.getBlock(pos + mc::DIR_SOUTH);
		east = state.getBlock(pos + mc::DIR_EAST);
		west = state.getBlock(pos + mc::DIR_WEST);

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

		if (id == 54) {
			if (north.id == 54)
				data |= DATA_NORTH << 4;
			if (south.id == 54)
				data |= DATA_SOUTH << 4;
			if (east.id == 54)
				data |= DATA_EAST << 4;
			if (west.id == 54)
				data |= DATA_WEST << 4;
		}
	} else if (id == 55) { // redstone wire
		// check if the redstone wire is connected to other redstone wires
		if (state.getBlock(pos + mc::DIR_NORTH).id == id
				|| state.getBlock(pos + mc::DIR_NORTH + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_NORTH;
		else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_NORTH).id == id)
			data |= REDSTONE_NORTH | REDSTONE_TOPNORTH;

		if (state.getBlock(pos + mc::DIR_SOUTH).id == id
				|| state.getBlock(pos + mc::DIR_SOUTH + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_SOUTH;
		else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_SOUTH).id == id)
			data |= REDSTONE_SOUTH | REDSTONE_TOPSOUTH;

		if (state.getBlock(pos + mc::DIR_EAST).id == id
				|| state.getBlock(pos + mc::DIR_EAST + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_EAST;
		else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_EAST).id == id)
			data |= REDSTONE_EAST | REDSTONE_TOPEAST;

		if (state.getBlock(pos + mc::DIR_WEST).id == id
				|| state.getBlock(pos + mc::DIR_WEST + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_WEST;
		else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_WEST).id == id)
			data |= REDSTONE_WEST | REDSTONE_TOPWEST;
	} else if (id == 64 || id == 71) {
		// doors
		uint16_t top = data & 8 ? DOOR_TOP : 0;
		uint16_t top_data, bottom_data;
		// at first get the data of both parts of the door, top and bottom
		if (top) {
			top_data = data;
			bottom_data = state.getBlock(pos + mc::DIR_BOTTOM).data;

			data |= DOOR_TOP;
		} else {
			top_data = state.getBlock(pos + mc::DIR_TOP).data;
			bottom_data = data;
		}

		// then find out if this door is the left door of a double door
		bool door_flip = top_data & 1;
		if (door_flip)
			data |= DOOR_FLIP_X;
		// find out if the door is openend
		bool opened = !(bottom_data & 4);

		// get the direction of the door
		uint16_t direction = bottom_data & 3;
		if (direction == 0) {
			direction = DOOR_WEST;
		} else if(direction == 1) {
			direction = DOOR_NORTH;
		} else if(direction == 2) {
			direction = DOOR_EAST;
		} else if(direction == 3) {
			direction = DOOR_SOUTH;
		}

		// if the door is closed, the direction need to get changed
		if (!opened) {
			data |= getDoorDirectionClosed(direction, door_flip);
		} else {
			data |= direction;
		}

	} else if (id == 79) {
		// ice blocks
		west = state.getBlock(pos + mc::DIR_WEST);
		south = state.getBlock(pos + mc::DIR_SOUTH);

		// check if west and south neighbors are also ice blocks
		if (west.id == 79)
			data |= DATA_WEST;
		if (south.id == 79)
			data |= DATA_SOUTH;

	} else if (id == 85 || id == 101 || id == 102 || id == 113 || id == 160) {
		// fence, iron bars, glass panes, nether fence, stained glass pane
		north = state.getBlock(pos + mc::DIR_NORTH);
		south = state.getBlock(pos + mc::DIR_SOUTH);
		east = state.getBlock(pos + mc::DIR_EAST);
		west = state.getBlock(pos + mc::DIR_WEST);

		// check for same neighbors
		if (north.id != 0 && (north.id == id
				|| !state.images.isBlockTransparent(north.id, north.data)))
			data |= DATA_NORTH;
		if (south.id != 0 && (south.id == id
				|| !state.images.isBlockTransparent(south.id, south.data)))
			data |= DATA_SOUTH;
		if (east.id != 0 && (east.id == id
				|| !state.images.isBlockTransparent(east.id, east.data)))
			data |= DATA_EAST;
		if (west.id != 0 && (west.id == id
				|| !state.images.isBlockTransparent(west.id, west.data)))
			data |= DATA_WEST;

		// check fences, they can also connect with fence gates
		if (id == 85 && north.id == 107)
			data |= DATA_NORTH;
		if (id == 85 && south.id == 107)
			data |= DATA_SOUTH;
		if (id == 85 && east.id == 107)
			data |= DATA_EAST;
		if (id == 85 && west.id == 107)
			data |= DATA_WEST;
	} else if (id == 175) {
		// large plants
		if (data >= 8) {
			// if this is the top part of a plant,
			// get the flower type from the block below
			// and add the special 'flower-top-part' bit
			return state.getBlock(pos + mc::DIR_BOTTOM).data | LARGEPLANT_TOP;
		}
	}


	if (!state.images.isBlockTransparent(id, data)) {
		// add shadow edges on opaque blockes
		north = state.getBlock(pos + mc::DIR_NORTH);
		east = state.getBlock(pos + mc::DIR_EAST);
		bottom = state.getBlock(pos + mc::DIR_BOTTOM);

		// check if neighbors are opaque
		if (north.id == 0 || state.images.isBlockTransparent(north.id, north.data))
			data |= EDGE_NORTH;
		if (east.id == 0 || state.images.isBlockTransparent(east.id, east.data))
			data |= EDGE_EAST;
		if (bottom.id == 0 || state.images.isBlockTransparent(bottom.id, bottom.data))
			data |= EDGE_BOTTOM;
	}

	return data;
}

void TileRenderer::renderTile(const TilePos& pos, Image& tile) {
	// some vars, set correct image size
	int block_size = state.images.getBlockImageSize();
	int tile_size = state.images.getTileSize();
	tile.setSize(tile_size, tile_size);

	// get the maximum count of water blocks,
	// blitted about each over, until they are nearly opaque
	int max_water = state.images.getMaxWaterNeededOpaque();

	// all visible blocks, which are rendered in this tile
	std::set<RenderBlock> blocks;

	// call start method of the rendermodes
	for (size_t i = 0; i < rendermodes.size(); i++)
		rendermodes[i]->start();

	// iterate over the highest blocks in the tile
	for (TileTopBlockIterator it(pos, block_size, tile_size); !it.end(); it.next()) {
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
				state.chunk = state.world.getChunk(current_chunk);
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
								top.image = state.images.getOpaqueWater(neighbor_south,
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
			Image image;
			bool transparent = state.images.isBlockTransparent(id, data);

			// check for biome data
			if (Biome::isBiomeBlock(id, data))
				image = state.images.getBiomeDependBlock(id, data, getBiome(block.current, state.chunk));
			else
				image = state.images.getBlock(id, data);

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

}
}
