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

#ifndef SIDE_RENDERVIEW_H_
#define SIDE_RENDERVIEW_H_

#include "../../renderview.h"

namespace mapcrafter {
namespace renderer {

class SideRenderView : public RenderView {
public:
	virtual BlockImages* createBlockImages(mc::BlockStateRegistry& block_registry) const;
	virtual TileSet* createTileSet(int tile_width) const;
	virtual TileRenderer* createTileRenderer(mc::BlockStateRegistry& block_registry,
			BlockImages* images, int tile_width, mc::WorldCache* world, RenderMode* render_mode) const;

	virtual RenderModeRenderer* createRenderModeRenderer(
			const RenderModeRendererType& renderer) const;

	virtual void configureBlockImages(BlockImages* images,
			const config::WorldSection& world_config,
			const config::MapSection& map_config) const;

	virtual void configureTileRenderer(TileRenderer* tile_renderer,
			const config::WorldSection& world_config,
			const config::MapSection& map_config) const;
};

} /* namespace renderer */
} /* namespace mapcrafter */

#endif /* SIDE_RENDERVIEW_H_ */
