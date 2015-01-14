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

#include "renderviews/isometric/renderview.h"
#include "renderviews/topdown/renderview.h"

namespace mapcrafter {
namespace renderer {

RenderView::~RenderView() {
}

void RenderView::configureTileRenderer(TileRenderer* tile_renderer,
		const config::WorldSection& world_config,
		const config::MapSection& map_config) const {
	tile_renderer->setRenderBiomes(map_config.renderBiomes());
}

RenderView* createRenderView(const std::string& render_view) {
	if (render_view == "isometric")
		return new IsometricRenderView();
	else if (render_view == "topdown")
		return new TopdownRenderView();
	return nullptr;
}

} /* namespace renderer */
} /* namespace mapcrafter */
