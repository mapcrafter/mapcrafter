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

#include "blockimages.h"
#include "rendermode.h"
#include "renderview.h"
#include "tileset.h"
#include "../mc/blockstate.h"
#include "../mc/pos.h"
#include "../util.h"

namespace mapcrafter {
namespace renderer {

bool TileImage::operator<(const TileImage& other) const {
	if (pos == other.pos) {
		return z_index < other.z_index;
	}
	return pos < other.pos;
}

TileRenderer::TileRenderer(const RenderView* render_view, mc::BlockStateRegistry& block_registry,
		BlockImages* images, int tile_width, mc::WorldCache* world, RenderMode* render_mode)
	: block_registry(block_registry), images(images), block_images(dynamic_cast<RenderedBlockImages*>(images)),
	  tile_width(tile_width), world(world), current_chunk(nullptr),
	  render_mode(render_mode),
	  render_biomes(true), use_preblit_water(false) {
	assert(block_images);
	render_mode->initialize(render_view, images, world, &current_chunk);

	// TODO can we make this somehow less hardcoded?
	full_water_ids.insert(block_registry.getBlockID(mc::BlockState::parse("minecraft:water", "level=0")));
	full_water_ids.insert(block_registry.getBlockID(mc::BlockState::parse("minecraft:water", "level=8")));
	full_water_like_ids.insert(block_registry.getBlockID(mc::BlockState::parse("minecraft:ice", "")));
	full_water_like_ids.insert(block_registry.getBlockID(mc::BlockState::parse("minecraft:packed_ice", "")));

	for (uint8_t i = 0; i < 8; i++) {
		bool up = i & 0x1;
		bool south = i & 0x2;
		bool west = i & 0x4;

		mc::BlockState block("minecraft:full_water");
		block.setProperty("up", up ? "true" : "false");
		block.setProperty("south", south ? "true" : "false");
		block.setProperty("west", west ? "true" : "false");
		partial_full_water_ids.push_back(block_registry.getBlockID(block));
	}

	/*
	for (uint8_t i = 0; i < 64; i++) {
		bool north = i & 0x1;
		bool south = i & 0x2;
		bool east = i & 0x4;
		bool west = i & 0x8;
		bool up = i & 0x10;
		bool down = i & 0x20;

		mc::BlockState block("minecraft:full_ice");
		block.setProperty("north", north ? "true" : "false");
		block.setProperty("south", south ? "true" : "false");
		block.setProperty("east", east ? "true" : "false");
		block.setProperty("west", west ? "true" : "false");
		block.setProperty("up", up ? "true" : "false");
		block.setProperty("down", down ? "true" : "false");
		partial_ice_ids.push_back(block_registry.getBlockID(block));
	}
	*/

	waterlog_id = block_registry.getBlockID(mc::BlockState("minecraft:waterlog"));
}

TileRenderer::~TileRenderer() {
}

void TileRenderer::setRenderBiomes(bool render_biomes) {
	this->render_biomes = render_biomes;
}

void TileRenderer::setUsePreblitWater(bool use_preblit_water) {
	this->use_preblit_water = use_preblit_water;
}

void TileRenderer::renderTile(const TilePos& tile_pos, RGBAImage& tile) {
	tile.setSize(getTileSize(), getTileSize());

	std::set<TileImage> tile_images;
	renderTopBlocks(tile_pos, tile_images);

	for (auto it = tile_images.begin(); it != tile_images.end(); ++it) {
		tile.alphaBlit(it->image, it->x, it->y);
	}
}

void TileRenderer::renderBlocks(int x, int y, mc::BlockPos top, const mc::BlockPos& dir, std::set<TileImage>& tile_images) {
	for (; top.y >= 0 ; top += dir) {
		// get current chunk position
		mc::ChunkPos current_chunk_pos(top);

		// check if current chunk is not null
		// and if the chunk wasn't replaced in the cache (i.e. position changed)
		if (current_chunk == nullptr || current_chunk->getPos() != current_chunk_pos) {
			//if (!state.world->hasChunkSection(current_chunk, top.current.y))
			//	continue;
			current_chunk = world->getChunk(current_chunk_pos);
		}
		if (current_chunk == nullptr) {
			continue;
		}

		// get local block position
		mc::LocalBlockPos local(top);

		uint16_t id = current_chunk->getBlockID(local);
		const BlockImage* block_image = &block_images->getBlockImage(id);
		if (block_image->is_air) {
			continue;
		}

		auto is_full_water = [this](uint16_t id) -> bool {
			return full_water_ids.count(id)
				|| full_water_like_ids.count(id)
				|| block_images->getBlockImage(id).is_waterloggable;
		};
		auto is_ice = [this](uint16_t id) -> bool {
			return block_images->getBlockImage(id).is_ice;
		};

		if (full_water_ids.count(id)) {
			uint16_t up = getBlock(top + mc::DIR_TOP).id;
			uint16_t south = getBlock(top + mc::DIR_SOUTH).id;
			uint16_t west = getBlock(top + mc::DIR_WEST).id;

			uint8_t index = is_full_water(up)
								| (is_full_water(south) << 1)
								| (is_full_water(west) << 2);
			// skip water blocks that are completely empty
			// (that commented thing hides the water surface)
			if (index == 1+2+4 /*|| index % 2 == 0*/) {
				continue;
			}
			assert(index < 8);
			id = partial_full_water_ids[index];
			block_image = &block_images->getBlockImage(id);
		}

		/*
		if (block_image->is_ice) {
			uint16_t north = getBlock(top + mc::DIR_NORTH).id;
			uint16_t south = getBlock(top + mc::DIR_SOUTH).id;
			uint16_t east = getBlock(top + mc::DIR_EAST).id;
			uint16_t west = getBlock(top + mc::DIR_WEST).id;
			uint16_t up = getBlock(top + mc::DIR_TOP).id;
			uint16_t down = getBlock(top + mc::DIR_BOTTOM).id;

			uint8_t index = is_ice(north)
								| (is_ice(south) << 1)
								| (is_ice(east) << 2)
								| (is_ice(west) << 3)
								| (is_ice(up) << 4)
								| (is_ice(down) << 5);
			assert(index < 64);
			id = partial_ice_ids[index];
			block_image = &block_images->getBlockImage(id);
		}
		*/

		// when we have a block that is waterlogged:
		// remove upper water texture if it's not the block at the water surface
		if (block_image->is_waterloggable && block_image->is_waterlogged) {
			uint16_t up = getBlock(top + mc::DIR_TOP).id;
			if (is_full_water(up)) {
				id = block_image->non_waterlogged_id;
				block_image = &block_images->getBlockImage(id);
			}
		}

		auto addTileImage = [this, x, y, top, &tile_images](uint16_t id, const BlockImage& block_image, int z_index) {
			TileImage tile_image;
			tile_image.x = x;
			tile_image.y = y;
			tile_image.image = block_image.image;
			tile_image.pos = top;
			tile_image.z_index = z_index;

			if (block_image.is_biome) {
				Biome biome = getBiomeOfBlock(top, current_chunk);
				block_images->prepareBiomeBlockImage(top.y, tile_image.image, block_image, biome);
			}

			// let the render mode do their magic with the block image
			//render_mode->draw(node.image, node.pos, id, data);
			render_mode->draw(tile_image.image, block_image, tile_image.pos, id);

			tile_images.insert(tile_image);
		};

		addTileImage(id, *block_image, 0);

		if (block_image->has_water_top) {
			addTileImage(waterlog_id, block_images->getBlockImage(waterlog_id), 1);
		}

		// if this block is not transparent, then break
		if (!block_image->is_transparent) {
			break;
		}
	}
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
	mc::Block block(pos, id, data);
	mc::Block north, south, east, west, top, bottom;

	if (id == 2) { // grass blocks
		// check if snow is on top to use the snowy sides instead of grass
		top = getBlock(pos + mc::DIR_TOP);
		if (top.id == 78 || top.id == 80)
			data |= GRASS_SNOW;

	} else if (block.isFullWater()) { // full water blocks
		west = getBlock(pos + mc::DIR_WEST);
		south = getBlock(pos + mc::DIR_SOUTH);
		top = getBlock(pos + mc::DIR_TOP);

		// check if the neighbors on visible faces (top, west, south) are also full water blocks
		// show water textures on these sides only if there is no water as neighbor too
		// exception for the top-face and when preblit water is used
		data |= FULL_WATER;
		if (use_preblit_water || !top.isFullWater())
			data |= FULL_WATER_TOP;
		if (!west.isFullWater())
			data |= FULL_WATER_WEST;
		if (!south.isFullWater())
			data |= FULL_WATER_SOUTH;

	} else if (block.isStairs()) { // stairs
		// corner stairs... wtf
		// using corner stair detection code of Minecraft Overviewer for now:
		// https://github.com/overviewer/Minecraft-Overviewer/blob/master/overviewer_core/src/iterate.c#L454
		/* 4 ancillary bits will be added to indicate which quarters of the block contain the 
		 * upper step. Regular stairs will have 2 bits set & corner stairs will have 1 or 3.
		 *     Southwest quarter is part of the upper step - 0x40
		 *    / Southeast " - 0x20
		 *    |/ Northeast " - 0x10
		 *    ||/ Northwest " - 0x8
		 *    |||/ flip upside down (Minecraft)
		 *    ||||/ has North/South alignment (Minecraft)
		 *    |||||/ ascends North or West, not South or East (Minecraft)
		 *    ||||||/
		 *  0b0011011 = Stair ascending north, upside up, with both north quarters filled
		 */

		/* keep track of whether neighbors are stairs, and their data */
		unsigned char stairs_base[8];
		unsigned char neigh_base[8];
		unsigned char *stairs = stairs_base;
		unsigned char *neigh = neigh_base;

		/* amount to rotate/roll to get to east, west, south, north */
		size_t rotations[] = {0,2,3,1};

		/* masks for the filled (ridge) stair quarters: */
		/* Example: the ridge for an east-ascending stair are the two east quarters */
		/*                  ascending: east  west south north */
		unsigned char ridge_mask[] = { 0x30, 0x48, 0x60, 0x18 };

		/* masks for the open (trench) stair quarters: */
		unsigned char trench_mask[] = { 0x48, 0x30, 0x18, 0x60 };

		/* boat analogy! up the stairs is toward the bow of the boat */
		/* masks for port and starboard, i.e. left and right sides while ascending: */
		unsigned char port_mask[] = { 0x18, 0x60, 0x30, 0x48 };
		unsigned char starboard_mask[] = { 0x60, 0x18, 0x48, 0x30 };

		/* we may need to lock some quarters into place depending on neighbors */
		unsigned char lock_mask = 0;

		unsigned char repair_rot[] = { 0, 1, 2, 3,  2, 3, 1, 0,  1, 0, 3, 2,  3, 2, 0, 1 };

		/* need to get northdirection of the render */
		/* TODO: get this just once? store in state? */
		// PyObject *texrot;
		int northdir;
		// texrot = PyObject_GetAttrString(state->textures, "rotation");
		// northdir = PyInt_AsLong(texrot);
		northdir = world->getWorld().getRotation();
		uint16_t ancilData = data & 0x7;

		/* fix the rotation value for different northdirections */
		#define FIX_ROT(x) (((x) & ~0x3) | repair_rot[((x) & 0x3) | (northdir << 2)])
		ancilData = FIX_ROT(ancilData);

		/* fill the ancillary bits assuming normal stairs with no corner yet */
		ancilData |= ridge_mask[ancilData & 0x3];

		/* get block & data for neighbors in this order: east, north, west, south */
		/* so we can rotate things easily */
		/*
		stairs[0] = stairs[4] = is_stairs(get_data(state, BLOCKS, x+1, y, z));
		stairs[1] = stairs[5] = is_stairs(get_data(state, BLOCKS, x, y, z-1));
		stairs[2] = stairs[6] = is_stairs(get_data(state, BLOCKS, x-1, y, z));
		stairs[3] = stairs[7] = is_stairs(get_data(state, BLOCKS, x, y, z+1));
		neigh[0] = neigh[4] = FIX_ROT(get_data(state, DATA, x+1, y, z));
		neigh[1] = neigh[5] = FIX_ROT(get_data(state, DATA, x, y, z-1));
		neigh[2] = neigh[6] = FIX_ROT(get_data(state, DATA, x-1, y, z));
		neigh[3] = neigh[7] = FIX_ROT(get_data(state, DATA, x, y, z+1));
		*/
		int x = pos.x, y = pos.y, z = pos.z;
		stairs[0] = stairs[4] = getBlock(mc::BlockPos(x+1, z, y)).isStairs();
		stairs[1] = stairs[5] = getBlock(mc::BlockPos(x, z-1, y)).isStairs();
		stairs[2] = stairs[6] = getBlock(mc::BlockPos(x-1, z, y)).isStairs();
		stairs[3] = stairs[7] = getBlock(mc::BlockPos(x, z+1, y)).isStairs();
		neigh[0] = neigh[4] = FIX_ROT(getBlock(mc::BlockPos(x+1, z, y)).data);
		neigh[1] = neigh[5] = FIX_ROT(getBlock(mc::BlockPos(x, z-1, y)).data);
		neigh[2] = neigh[6] = FIX_ROT(getBlock(mc::BlockPos(x-1, z, y)).data);
		neigh[3] = neigh[7] = FIX_ROT(getBlock(mc::BlockPos(x, z+1, y)).data);

		#undef FIX_ROT

		/* Rotate the neighbors so we only have to worry about one orientation
		 * No matter which way the boat is facing, the the neighbors will be:
		 *   0: bow
		 *   1: port
		 *   2: stern
		 *   3: starboard */
		stairs += rotations[ancilData & 0x3];
		neigh += rotations[ancilData & 0x3];

		/* Matching neighbor stairs to the sides should prevent cornering on that side */
		/* If found, set bits in lock_mask to lock the current quarters as they are */
		if (stairs[1] && (neigh[1] & 0x7) == (ancilData & 0x7)) {
			/* Neighbor on port side is stairs of the same orientation as me */
			/* Do NOT allow changing quarters on the port side */
			lock_mask |= port_mask[ancilData & 0x3];
		}
		if (stairs[3] && (neigh[3] & 0x7) == (ancilData & 0x7)) {
			/* Neighbor on starboard side is stairs of the same orientation as me */
			/* Do NOT allow changing quarters on the starboard side */
			lock_mask |= starboard_mask[ancilData & 0x3];
		}

		/* Make corner stairs -- prefer outside corners like Minecraft */
		if (stairs[0] && (neigh[0] & 0x4) == (ancilData & 0x4)) {
			/* neighbor at bow is stairs with same flip */
			if ((neigh[0] & 0x2) != (ancilData & 0x2)) {
				/* neighbor is perpendicular, cut a trench, but not where locked */
				ancilData &= ~trench_mask[neigh[0] & 0x3] | lock_mask;
			}
		} else if (stairs[2] && (neigh[2] & 0x4) == (ancilData & 0x4)) {
			/* neighbor at stern is stairs with same flip */
			if ((neigh[2] & 0x2) != (ancilData & 0x2)) {
				/* neighbor is perpendicular, add a ridge, but not where locked */
				ancilData |= ridge_mask[neigh[2] & 0x3] & ~lock_mask;
			}
		}

		data = ancilData;
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

	} else if (id == 79 || id == 212) {
		// ice blocks
		west = getBlock(pos + mc::DIR_WEST);
		south = getBlock(pos + mc::DIR_SOUTH);

		// check if west and south neighbors are also ice blocks
		if (west.id == 79 || west.id == 212)
			data |= DATA_WEST;
		if (south.id == 79 || south.id == 212)
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
	} else if (id == 111) {
		// direction of lily pad
		// http://llbit.se/?p=1537
		// TODO this seems to work only partially
		long pr = (pos.x * 3129871) ^ (pos.z * 116129781) ^ (pos.y);
		pr = pr * pr * 42317861 + pr * 11;
		int rotation = (pr >> 16) & 3;
		data = rotation;
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
		if (north.id == 0 || (images->isBlockTransparent(north.id, north.data) && !north.isStairs()))
			data |= EDGE_NORTH;
		if (east.id == 0 || (images->isBlockTransparent(east.id, east.data) && !east.isStairs()))
			data |= EDGE_EAST;
		if (bottom.id == 0 || (images->isBlockTransparent(bottom.id, bottom.data) && !bottom.isStairs()))
			data |= EDGE_BOTTOM;
	}

	return data;
}

}
}
