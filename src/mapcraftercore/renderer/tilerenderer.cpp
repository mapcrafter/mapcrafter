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

#include "blockhandler.h"
#include "blockimages.h"
#include "image.h"
#include "rendermode.h"
#include "renderview.h"
#include "tileset.h"
#include "rendermodes/overlay.h"
#include "../mc/pos.h"
#include "../util.h"

namespace mapcrafter {
namespace renderer {

TileRenderer::TileRenderer(const RenderView* render_view,
		BlockHandler* block_handler, BlockImages* images,
		int tile_width, mc::WorldCache* world, RenderMode* render_mode,
		std::shared_ptr<OverlayRenderMode> hardcode_overlay,
		std::vector<std::shared_ptr<OverlayRenderMode>> overlays)
	: block_handler(block_handler), images(images), tile_width(tile_width), world(world),
	  current_chunk(nullptr), render_mode(render_mode), hardcode_overlay(hardcode_overlay),
	  overlays(overlays), render_biomes(true), use_preblit_water(false) {
	block_handler->initialize(render_view, images, world, &current_chunk);
	render_mode->initialize(render_view, images, world, &current_chunk);
	if (hardcode_overlay)
		hardcode_overlay->initialize(render_view, images, world, &current_chunk);
	for (size_t i = 0; i < overlays.size(); i++)
		overlays[i]->initialize(render_view, images, world, &current_chunk);
}

TileRenderer::~TileRenderer() {
}

void TileRenderer::setRenderBiomes(bool render_biomes) {
	this->render_biomes = render_biomes;
}

void TileRenderer::setUsePreblitWater(bool use_preblit_water) {
	this->use_preblit_water = use_preblit_water;
	block_handler->setUsePreblitWater(use_preblit_water);
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

void TileRenderer::drawHardcodeOverlay(RGBAImage& block, const mc::BlockPos& pos, uint16_t id, uint16_t data) const {
	if (hardcode_overlay) {
		RGBAImage overlay = block.emptyCopy();
		hardcode_overlay->drawOverlay(block, overlay, pos, id, data);
		overlay.applyMask(block);
		block.alphaBlit(overlay, 0, 0);
	}
}

}
}
