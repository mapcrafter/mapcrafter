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

#ifndef RENDERMODE_H_
#define RENDERMODE_H_

#include "blockimages.h"
#include "image.h"
#include "../mc/chunk.h"
#include "../mc/pos.h"
#include "../mc/worldcache.h"
#include "../config/mapcrafterconfig.h"

#include <memory>
#include <string>
#include <vector>

namespace mapcrafter {
namespace renderer {

/**
 * A simple interface to implement different rendermodes.
 */
class RenderMode {
public:
	RenderMode();
	virtual ~RenderMode();

	void initialize(BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk);

	// is called when the tile renderer starts rendering a tile
	virtual void start();
	// is called when the tile renderer finished rendering a tile
	virtual void end();

	// is called to allow the rendermode to hide specific blocks
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	// is called to allow the rendermode to change a block image
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);

	BlockImages* images;
	mc::WorldCache* world;
	mc::Chunk** current_chunk;
};

typedef std::vector<std::shared_ptr<RenderMode>> RenderModes;

bool createRenderModes(const config::WorldSection& world_config,
		const config::MapSection& map_config, RenderModes& render_modes);

} /* namespace render */
} /* namespace mapcrafter */

#include "rendermodes/cave.h"
#include "rendermodes/lighting.h"

#endif /* RENDERMODE_H_ */
