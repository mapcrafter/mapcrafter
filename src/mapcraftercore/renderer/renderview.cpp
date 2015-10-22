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
#include "rendermodes/overlay.h"
#include "renderviews/isometric/renderview.h"
#include "renderviews/topdown/renderview.h"
#include "../config/configsections/map.h"
#include "../config/configsections/world.h"
#include "../util.h"

#include <cassert>

namespace mapcrafter {
namespace renderer {

RenderView::~RenderView() {
}

void RenderView::configureBlockImages(BlockImages* block_images,
	const config::WorldSection& world_config,
	const config::MapSection& map_config) const {
	assert(block_images != nullptr);
	block_images->setRenderSpecialBlocks(map_config.renderUnknownBlocks(),
			map_config.renderLeavesTransparent());
}

void RenderView::configureTileRenderer(TileRenderer* tile_renderer,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) const {
	assert(tile_renderer != nullptr);
	tile_renderer->setRenderBiomes(map_config.renderBiomes());
	tile_renderer->setUsePreblitWater(map_config.getRenderMode() == RenderModeType::PLAIN);
	tile_renderer->setUsePreblitWater(false);
}

std::ostream& operator<<(std::ostream& out, RenderViewType render_view) {
	switch (render_view) {
	case RenderViewType::ISOMETRIC: return out << "isometric";
	case RenderViewType::TOPDOWN: return out << "topdown";
	default: return out << "unknown";
	}
}

RenderView* createRenderView(RenderViewType render_view) {
	switch (render_view) {
	case RenderViewType::ISOMETRIC: return new IsometricRenderView();
	case RenderViewType::TOPDOWN: return new TopdownRenderView();
	// thou shalt not return nullptr!
	default: assert(false); return nullptr;
	}
}

} /* namespace renderer */
} /* namespace mapcrafter */
