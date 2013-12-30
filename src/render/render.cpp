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
#include <deque>

namespace mapcrafter {
namespace render {

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

TileRenderer::TileRenderer()
		: state(), render_biomes(false), water_preblit(true) {
}

TileRenderer::TileRenderer(std::shared_ptr<mc::WorldCache> world,
		std::shared_ptr<BlockImages> images, const config::MapSection& map)
		: state(world, images), render_biomes(map.renderBiomes()),
		  water_preblit(map.getRendermode() != "daylight"
				  && map.getRendermode() != "nightlight") {
	createRendermode(map.getRendermode(), map, state, rendermodes);
}

TileRenderer::~TileRenderer() {
}

Biome TileRenderer::getBiomeOfBlock(const mc::BlockPos& pos, const mc::Chunk* chunk) {
	// return default biome if we don't want to render different biomes
	if (!render_biomes)
		return getBiome(DEFAULT_BIOME);
	uint8_t biome_id = chunk->getBiomeAt(mc::LocalBlockPos(pos));
	Biome biome = getBiome(biome_id);
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
				mc::Chunk* other_chunk = state.world->getChunk(chunk_pos);
				if (other_chunk == nullptr)
					continue;
				other_id = other_chunk->getBiomeAt(mc::LocalBlockPos(other));
			}

			biome += getBiome(other_id);
			count++;
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
	} else if (id == 54 || id == 130 || id == 146) { // chests
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
	} else if (id == 55 || id == 132) { // redstone wire, tripwire
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

		if (id == 132) {
			if (state.getBlock(pos + mc::DIR_NORTH).id == 131
					|| state.getBlock(pos + mc::DIR_NORTH + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_NORTH;
			else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_NORTH).id == 131)
				data |= REDSTONE_NORTH | REDSTONE_TOPNORTH;

			if (state.getBlock(pos + mc::DIR_SOUTH).id == 131
					|| state.getBlock(pos + mc::DIR_SOUTH + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_SOUTH;
			else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_SOUTH).id == 131)
				data |= REDSTONE_SOUTH | REDSTONE_TOPSOUTH;

			if (state.getBlock(pos + mc::DIR_EAST).id == 131
					|| state.getBlock(pos + mc::DIR_EAST + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_EAST;
			else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_EAST).id == 131)
				data |= REDSTONE_EAST | REDSTONE_TOPEAST;

			if (state.getBlock(pos + mc::DIR_WEST).id == 131
					|| state.getBlock(pos + mc::DIR_WEST + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_WEST;
			else if (state.getBlock(pos + mc::DIR_TOP + mc::DIR_WEST).id == 131)
				data |= REDSTONE_WEST | REDSTONE_TOPWEST;
		}
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

	} else if (id == 85 || id == 101 || id == 102 || id == 139 || id == 113 || id == 160) {
		// fence, iron bars, glass panes, cobblestone walls, nether fence, stained glass pane
		north = state.getBlock(pos + mc::DIR_NORTH);
		south = state.getBlock(pos + mc::DIR_SOUTH);
		east = state.getBlock(pos + mc::DIR_EAST);
		west = state.getBlock(pos + mc::DIR_WEST);

		// check for same neighbors
		if (north.id != 0 && (north.id == id
				|| !state.images->isBlockTransparent(north.id, north.data)))
			data |= DATA_NORTH;
		if (south.id != 0 && (south.id == id
				|| !state.images->isBlockTransparent(south.id, south.data)))
			data |= DATA_SOUTH;
		if (east.id != 0 && (east.id == id
				|| !state.images->isBlockTransparent(east.id, east.data)))
			data |= DATA_EAST;
		if (west.id != 0 && (west.id == id
				|| !state.images->isBlockTransparent(west.id, west.data)))
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


	if (!state.images->isBlockTransparent(id, data)) {
		// add shadow edges on opaque blockes
		north = state.getBlock(pos + mc::DIR_NORTH);
		east = state.getBlock(pos + mc::DIR_EAST);
		bottom = state.getBlock(pos + mc::DIR_BOTTOM);

		// check if neighbors are opaque
		if (north.id == 0 || state.images->isBlockTransparent(north.id, north.data))
			data |= EDGE_NORTH;
		if (east.id == 0 || state.images->isBlockTransparent(east.id, east.data))
			data |= EDGE_EAST;
		if (bottom.id == 0 || state.images->isBlockTransparent(bottom.id, bottom.data))
			data |= EDGE_BOTTOM;
	}

	return data;
}

void TileRenderer::renderChunk(const mc::Chunk& chunk, Image& tile, int dx, int dy) {
	int texture_size = state.images->getTextureSize();

	for (int x = 0; x < 16; x++)
		for (int z = 0; z < 16; z++) {
			std::deque<Image> blocks;

			mc::LocalBlockPos localpos(x, z, 0);
			int height = chunk.getHeightAt(localpos);
			localpos.y = height;
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
				Image block = state.images->getBlock(id, data);
				if (Biome::isBiomeBlock(id, data)) {
					block = state.images->getBiomeDependBlock(id, data, getBiomeOfBlock(globalpos, &chunk));
				}
				for (size_t i = 0; i < rendermodes.size(); i++)
					rendermodes[i]->draw(block, globalpos, id, data);
				blocks.push_back(block);
				if (!state.images->isBlockTransparent(id, data)) {
					break;
				}
				localpos.y--;
			}

			while (blocks.size() > 0) {
				Image block = blocks.front();
				tile.alphablit(block, dx + x*texture_size, dy + z*texture_size);
				blocks.pop_back();
			}
		}
}

void TileRenderer::renderTile(const TilePos& tile_pos, const TilePos& tile_offset,
		Image& tile) {
	int texture_size = state.images->getTextureSize();
	int tile_size = texture_size * 16 * TILE_WIDTH;
	tile.setSize(tile_size, tile_size);

	// call start method of the rendermodes
	for (size_t i = 0; i < rendermodes.size(); i++)
		rendermodes[i]->start();

	for (int x = 0; x < TILE_WIDTH; x++)
		for (int z = 0; z < TILE_WIDTH; z++) {
			mc::ChunkPos chunkpos(tile_pos.getX() * TILE_WIDTH + x, tile_pos.getY() * TILE_WIDTH + z);
			mc::Chunk* chunk = state.world->getChunk(chunkpos);
			if (chunk != nullptr)
				renderChunk(*chunk, tile, texture_size*16*x, texture_size*16*z);
		}

	// call the end method of the rendermodes
	for (size_t i = 0; i < rendermodes.size(); i++)
		rendermodes[i]->end();
}

}
}
