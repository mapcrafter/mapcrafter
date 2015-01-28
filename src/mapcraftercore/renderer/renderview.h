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

#ifndef RENDERVIEW_H_
#define RENDERVIEW_H_

#include "blockimages.h"
#include "rendermode.h"
#include "tileset.h"
#include "tilerenderer.h"
#include "../mc/world.h"

namespace mapcrafter {
namespace renderer {

class RenderView {
public:
	virtual ~RenderView();

	virtual BlockImages* createBlockImages() const = 0;
	virtual TileSet* createTileSet(int tile_width) const = 0;
	virtual TileRenderer* createTileRenderer(BlockImages* images, int tile_width,
			mc::WorldCache* world, RenderModes& render_modes) const = 0;

	/**
	 * Configures a block images object by calling some (eventually per render view
	 * specific) methods of the block images and passing information from the world- and
	 * map configuration. It is possible to call these methods manually (for example
	 * needed in the testtextures program where we don't want to initialize a whole
	 * configuration object just for the block images).
	 *
	 * If you overwrite this method, you should also call the parent method since it
	 * sets generic block images options (for example whether to render leaves transparent).
	 *
	 * It is also very important that you pass only BlockImages*'s to this method which
	 * were created by this render view (because the render view might rely on using
	 * the BlockImages* as an IsometricBlockImages* like it was created for example).
	 */
	virtual void configureBlockImages(BlockImages* block_images,
			const config::WorldSection& world_config,
			const config::MapSection& map_config) const;

	/**
	 * Configures a tile renderer object just like the configureBlockImages() method.
	 * Make sure when overwriting this method that you should also call the parent
	 * method since it sets generic tile renderer options.
	 *
	 * It is also very important that you pass only TileRenderer*'s to this method which
	 * were created by this render view.
	 */
	virtual void configureTileRenderer(TileRenderer* tile_renderer,
			const config::WorldSection& world_config,
			const config::MapSection& map_config) const;
};

RenderView* createRenderView(const std::string& render_view);

} /* namespace renderer */
} /* namespace mapcrafter */

#endif /* RENDERVIEW_H_ */
