/*
 * Copyright 2012-2015 Moritz Hilscher
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

#include "blockimages.h"
#include "image.h"
#include "rendermode.h"
#include "tileset.h"
#include "../mc/pos.h"
#include "../util.h"

namespace mapcrafter {
namespace renderer {

TileRenderer::TileRenderer(BlockImages* images, int tile_width,
		mc::WorldCache* world, RenderMode* render_mode)
	: images(images), tile_width(tile_width), world(world), current_chunk(nullptr),
	  render_mode(render_mode),
	  render_biomes(true) {
	render_mode->initialize(images, world, &current_chunk);
}

TileRenderer::~TileRenderer() {
}

void TileRenderer::setRenderBiomes(bool render_biomes) {
	this->render_biomes = render_biomes;
}

mc::Block TileRenderer::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, current_chunk, get);
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
				mc::Chunk* other_chunk = world->getChunk(chunk_pos);
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
		top = getBlock(pos + mc::DIR_TOP);
		if (top.id == 78 || top.id == 80)
			data |= GRASS_SNOW;

	} else if ((id == 8 || id == 9) && data == 0) { // full water blocks
		west = getBlock(pos + mc::DIR_WEST);
		south = getBlock(pos + mc::DIR_SOUTH);
		top = getBlock(pos + mc::DIR_TOP);

		// TODO
		// check if the neighbors on visible faces (top, west, south)
		// are also full water blocks
		if (/*!water_preblit &&*/ top.isFullWater())
			data |= DATA_TOP;

		if (west.isFullWater())
			data |= DATA_WEST;

		if (south.isFullWater())
			data |= DATA_SOUTH;
	} else if (id == 54 || id == 130 || id == 146) { // chests
		// at first get all neighbor blocks
		north = getBlock(pos + mc::DIR_NORTH);
		south = getBlock(pos + mc::DIR_SOUTH);
		east = getBlock(pos + mc::DIR_EAST);
		west = getBlock(pos + mc::DIR_WEST);

		// determine the direction of the chest
		if (data == 2)
			data = DATA_NORTH;
		else if (data == 3)
			data = DATA_SOUTH;
		else if (data == 4)
			data = DATA_WEST;
		else
			data = DATA_EAST;

		// normal chests and trapped chests can appear as double chests
		if (id == 54 || id == 146) {
			if (north.id == id)
				data |= DATA_NORTH << 4;
			if (south.id == id)
				data |= DATA_SOUTH << 4;
			if (east.id == id)
				data |= DATA_EAST << 4;
			if (west.id == id)
				data |= DATA_WEST << 4;
		}
	} else if (id == 55 || id == 132) { // redstone wire, tripwire
		// check if the redstone wire is connected to other redstone wires
		if (getBlock(pos + mc::DIR_NORTH).id == id
				|| getBlock(pos + mc::DIR_NORTH + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_NORTH;
		else if (getBlock(pos + mc::DIR_TOP + mc::DIR_NORTH).id == id)
			data |= REDSTONE_NORTH | REDSTONE_TOPNORTH;

		if (getBlock(pos + mc::DIR_SOUTH).id == id
				|| getBlock(pos + mc::DIR_SOUTH + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_SOUTH;
		else if (getBlock(pos + mc::DIR_TOP + mc::DIR_SOUTH).id == id)
			data |= REDSTONE_SOUTH | REDSTONE_TOPSOUTH;

		if (getBlock(pos + mc::DIR_EAST).id == id
				|| getBlock(pos + mc::DIR_EAST + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_EAST;
		else if (getBlock(pos + mc::DIR_TOP + mc::DIR_EAST).id == id)
			data |= REDSTONE_EAST | REDSTONE_TOPEAST;

		if (getBlock(pos + mc::DIR_WEST).id == id
				|| getBlock(pos + mc::DIR_WEST + mc::DIR_BOTTOM).id == id)
			data |= REDSTONE_WEST;
		else if (getBlock(pos + mc::DIR_TOP + mc::DIR_WEST).id == id)
			data |= REDSTONE_WEST | REDSTONE_TOPWEST;

		if (id == 132) {
			if (getBlock(pos + mc::DIR_NORTH).id == 131
					|| getBlock(pos + mc::DIR_NORTH + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_NORTH;
			else if (getBlock(pos + mc::DIR_TOP + mc::DIR_NORTH).id == 131)
				data |= REDSTONE_NORTH | REDSTONE_TOPNORTH;

			if (getBlock(pos + mc::DIR_SOUTH).id == 131
					|| getBlock(pos + mc::DIR_SOUTH + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_SOUTH;
			else if (getBlock(pos + mc::DIR_TOP + mc::DIR_SOUTH).id == 131)
				data |= REDSTONE_SOUTH | REDSTONE_TOPSOUTH;

			if (getBlock(pos + mc::DIR_EAST).id == 131
					|| getBlock(pos + mc::DIR_EAST + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_EAST;
			else if (getBlock(pos + mc::DIR_TOP + mc::DIR_EAST).id == 131)
				data |= REDSTONE_EAST | REDSTONE_TOPEAST;

			if (getBlock(pos + mc::DIR_WEST).id == 131
					|| getBlock(pos + mc::DIR_WEST + mc::DIR_BOTTOM).id == 131)
				data |= REDSTONE_WEST;
			else if (getBlock(pos + mc::DIR_TOP + mc::DIR_WEST).id == 131)
				data |= REDSTONE_WEST | REDSTONE_TOPWEST;
		}
	} else if (id == 64 || id == 71 || (id >= 193 && id <= 197)) {
		// doors
		uint16_t top = data & 8 ? DOOR_TOP : 0;
		uint16_t top_data, bottom_data;
		// at first get the data of both parts of the door, top and bottom
		if (top) {
			top_data = data;
			bottom_data = getBlock(pos + mc::DIR_BOTTOM).data;

			data |= DOOR_TOP;
		} else {
			top_data = getBlock(pos + mc::DIR_TOP).data;
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
		west = getBlock(pos + mc::DIR_WEST);
		south = getBlock(pos + mc::DIR_SOUTH);

		// check if west and south neighbors are also ice blocks
		if (west.id == 79)
			data |= DATA_WEST;
		if (south.id == 79)
			data |= DATA_SOUTH;

	} else if (id == 85 || id == 101 || id == 102 || id == 139 || id == 113 || id == 160
			|| (id >= 188 && id <= 192)) {
		// fence, iron bars, glass panes, cobblestone walls, nether fence,
		// stained glass pane, special wood type fences
		north = getBlock(pos + mc::DIR_NORTH);
		south = getBlock(pos + mc::DIR_SOUTH);
		east = getBlock(pos + mc::DIR_EAST);
		west = getBlock(pos + mc::DIR_WEST);

		// check for same neighbors
		if (north.id != 0 && (north.id == id
				|| !images->isBlockTransparent(north.id, north.data)))
			data |= DATA_NORTH;
		if (south.id != 0 && (south.id == id
				|| !images->isBlockTransparent(south.id, south.data)))
			data |= DATA_SOUTH;
		if (east.id != 0 && (east.id == id
				|| !images->isBlockTransparent(east.id, east.data)))
			data |= DATA_EAST;
		if (west.id != 0 && (west.id == id
				|| !images->isBlockTransparent(west.id, west.data)))
			data |= DATA_WEST;

		// check fences and cobblestone walls, they can also connect with fence gates
		if (id == 85 || id == 139 || (id >= 188 && id <= 192)) {
			if (north.id == 107)
				data |= DATA_NORTH;
			if (south.id == 107)
				data |= DATA_SOUTH;
			if (east.id == 107)
				data |= DATA_EAST;
			if (west.id == 107)
				data |= DATA_WEST;
		}
	} else if (id == 175) {
		// large plants
		if (data >= 8) {
			// if this is the top part of a plant,
			// get the flower type from the block below
			// and add the special 'flower-top-part' bit
			return getBlock(pos + mc::DIR_BOTTOM).data | LARGEPLANT_TOP;
		}
	}


	if (!images->isBlockTransparent(id, data)) {
		// add shadow edges on opaque blockes
		north = getBlock(pos + mc::DIR_NORTH);
		east = getBlock(pos + mc::DIR_EAST);
		bottom = getBlock(pos + mc::DIR_BOTTOM);

		// check if neighbors are opaque
		if (north.id == 0 || images->isBlockTransparent(north.id, north.data))
			data |= EDGE_NORTH;
		if (east.id == 0 || images->isBlockTransparent(east.id, east.data))
			data |= EDGE_EAST;
		if (bottom.id == 0 || images->isBlockTransparent(bottom.id, bottom.data))
			data |= EDGE_BOTTOM;
	}

	return data;
}

}
}
