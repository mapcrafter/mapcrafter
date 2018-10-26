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
	  render_biomes(true), use_preblit_water(false), shadow_edges({0, 0, 0, 0, 0}) {
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

	for (uint16_t i = 0; i < 4; i++) {
		mc::BlockState block("minecraft:lily_pad_rotated");
		block.setProperty("rotation", util::str(i));
		lily_pad_ids.push_back(block_registry.getBlockID(block));
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
	waterlog_block_image = &block_images->getBlockImage(waterlog_id);
}

TileRenderer::~TileRenderer() {
}

void TileRenderer::setRenderBiomes(bool render_biomes) {
	this->render_biomes = render_biomes;
}

void TileRenderer::setUsePreblitWater(bool use_preblit_water) {
	this->use_preblit_water = use_preblit_water;
}

void TileRenderer::setShadowEdges(std::array<uint8_t, 5> shadow_edges) {
	this->shadow_edges = shadow_edges;
}

void TileRenderer::renderTile(const TilePos& tile_pos, RGBAImage& tile) {
	tile.setSize(getTileWidth(), getTileHeight());

	std::set<TileImage> tile_images;
	renderTopBlocks(tile_pos, tile_images);

	for (auto it = tile_images.begin(); it != tile_images.end(); ++it) {
		tile.alphaBlit(it->image, it->x, it->y);
	}
}

int TileRenderer::getTileWidth() const {
	return getTileSize();
}

int TileRenderer::getTileHeight() const {
	return getTileSize();
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
		if (block_image->is_air || render_mode->isHidden(top, *block_image)) {
			continue;
		}

		auto is_full_water = [this](uint16_t id) -> bool {
			return full_water_ids.count(id)
				|| full_water_like_ids.count(id)
				|| block_images->getBlockImage(id).is_waterlogged;
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

		if (block_image->is_lily_pad) {
			// TODO this seems to be different than in Minecraft
			// if anyone wants to fix this, go for it
			long long pr = ((long long) top.x * 3129871LL) ^ ((long long) top.z * 116129781LL) ^ (long long)(top.y);
			pr = pr * pr * 42317861LL + pr * 11LL;
			uint16_t rotation = 3 & (pr >> 16);
			id = lily_pad_ids[rotation];
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
				block_images->prepareBiomeBlockImage(tile_image.image, block_image, getBiomeColor(top, block_image, current_chunk));
			}

			if (block_image.has_water_top) {
				// get waterlog block image and biomize it
				RGBAImage waterlog = waterlog_block_image->image;
				const RGBAImage& waterlog_uv = waterlog_block_image->uv_image;
				block_images->prepareBiomeBlockImage(waterlog, *waterlog_block_image, getBiomeColor(top, *waterlog_block_image, current_chunk));

				// blend waterlog water surface on top of block
				blockImageBlendTop(tile_image.image, block_image.uv_image, waterlog, waterlog_uv);
			}

			if (block_image.shadow_edges > 0) {
				auto shadow_edge = [this, top](const mc::BlockPos& dir) {
					const BlockImage& b = block_images->getBlockImage(getBlock(top + dir).id);
					//return b.is_transparent && !(b.is_full_water || b.is_waterlogged);
					return b.shadow_edges == 0;
				};
				uint8_t north = shadow_edges[0] && shadow_edge(mc::DIR_NORTH);
				uint8_t south = shadow_edges[1] && shadow_edge(mc::DIR_SOUTH);
				uint8_t east = shadow_edges[2] && shadow_edge(mc::DIR_EAST);
				uint8_t west = shadow_edges[3] && shadow_edge(mc::DIR_WEST);
				uint8_t bottom = shadow_edges[4] && shadow_edge(mc::DIR_BOTTOM);

				if (north + south + east + west + bottom != 0) {
					int f = block_image.shadow_edges;
					north *= shadow_edges[0] * f;
					south *= shadow_edges[1] * f;
					east *= shadow_edges[2] * f;
					west *= shadow_edges[3] * f;
					bottom *= shadow_edges[4] * f;
					blockImageShadowEdges(tile_image.image, block_image.uv_image,
							north, south, east, west, bottom);
				}
			}

			// let the render mode do their magic with the block image
			//render_mode->draw(node.image, node.pos, id, data);
			render_mode->draw(tile_image.image, block_image, tile_image.pos, id);

			tile_images.insert(tile_image);
		};

		addTileImage(id, *block_image, 0);

		// if this block is not transparent, then stop looking for more blocks
		if (!block_image->is_transparent) {
			break;
		}
	}
}

mc::Block TileRenderer::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, current_chunk, get);
}

uint32_t TileRenderer::getBiomeColor(const mc::BlockPos& pos, const BlockImage& block, const mc::Chunk* chunk) {
	const int radius = 2;
	float f = ((2*radius+1)*(2*radius+1));
	float r = 0.0, g = 0.0, b = 0.0;

	int n = 0;
	for (int dx = -radius; dx <= radius; dx++) {
		for (int dz = -radius; dz <= radius; dz++) {
			mc::BlockPos other = pos + mc::BlockPos(dx, dz, 0);
			mc::ChunkPos chunk_pos(other);
			
			const BlockImage* other_block;
			uint16_t biome_id;
			mc::LocalBlockPos local(other);
			if (chunk_pos != chunk->getPos()) {
				mc::Chunk* other_chunk = world->getChunk(chunk_pos);
				if (other_chunk == nullptr) {
					f -= 1.0f;
					continue;
				}
				biome_id = other_chunk->getBiomeAt(local);
			} else {
				biome_id = chunk->getBiomeAt(local);
			}
			Biome biome = getBiome(biome_id);
			uint32_t c = biome.getColor(other, block.biome_color, block.biome_colormap);
			r += (float) rgba_red(c);
			g += (float) rgba_green(c);
			b += (float) rgba_blue(c);
		}
	}

	return rgba(r / f, g / f, b / f, 255);
}

}
}
