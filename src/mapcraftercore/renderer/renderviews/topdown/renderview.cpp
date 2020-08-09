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

#include "renderview.h"

#include "tileset.h"
#include "tilerenderer.h"
#include "../../rendermode.h"
#include "../../rendermodes/overlay.h"
#include "../../../mc/blockstate.h"

namespace mapcrafter {
namespace renderer {

BlockImages* TopdownRenderView::createBlockImages(mc::BlockStateRegistry& block_registry) const {
	return new RenderedBlockImages(block_registry);
}

TileSet* TopdownRenderView::createTileSet(int tile_width) const {
	return new TopdownTileSet(tile_width);
}

TileRenderer* TopdownRenderView::createTileRenderer(mc::BlockStateRegistry& block_registry,
		BlockImages* images, int tile_width, mc::WorldCache* world, RenderMode* render_mode) const {
	return new TopdownTileRenderer(this, block_registry, images, tile_width, world, render_mode);
}

void TopdownRenderView::configureBlockImages(BlockImages* images,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) const {
}

void TopdownRenderView::configureTileRenderer(TileRenderer* tile_renderer,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) const {
	assert(tile_renderer != nullptr);
	RenderView::configureTileRenderer(tile_renderer, world_config, map_config);

	tile_renderer->setShadowEdges({2, 1, 2, 2, 0});
}

} /* namespace renderer */
} /* namespace mapcrafter */
