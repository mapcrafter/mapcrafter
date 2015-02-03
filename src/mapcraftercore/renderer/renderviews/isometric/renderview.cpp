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

#include "renderview.h"

#include "blockimages.h"
#include "tileset.h"
#include "tilerenderer.h"
#include "../../../config/configsections/map.h"
#include "../../../config/configsections/world.h"
#include "../../../util.h"

#include <cassert>

namespace mapcrafter {
namespace renderer {

BlockImages* IsometricRenderView::createBlockImages() const {
	return new IsometricBlockImages();
}

TileSet* IsometricRenderView::createTileSet(int tile_width) const {
	return new IsometricTileSet(tile_width);
}

TileRenderer* IsometricRenderView::createTileRenderer(BlockImages* images,
		int tile_width, mc::WorldCache* world, RenderMode* render_mode) const {
	return new IsometricTileRenderer(images, tile_width, world, render_mode);
}

void IsometricRenderView::configureBlockImages(BlockImages* block_images,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) const {
	assert(block_images != nullptr);
	RenderView::configureBlockImages(block_images, world_config, map_config);

	IsometricBlockImages* images = dynamic_cast<IsometricBlockImages*>(block_images);
	assert(images != nullptr);

	std::string render_mode = map_config.getRenderMode();
	if (render_mode == "daylight" || render_mode == "nightlight")
		images->setBlockSideDarkening(0.95, 0.8);
	else
		images->setBlockSideDarkening(0.75, 0.6);
}

void IsometricRenderView::configureTileRenderer(TileRenderer* tile_renderer,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) const {
	assert(tile_renderer != nullptr);
	RenderView::configureTileRenderer(tile_renderer, world_config, map_config);

	IsometricTileRenderer* renderer = dynamic_cast<IsometricTileRenderer*>(tile_renderer);
	assert(renderer != nullptr);

	// TODO
	renderer->setUsePreblitWater(false);
}

} /* namespace renderer */
} /* namespace mapcrafter */
