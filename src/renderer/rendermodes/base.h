/*
 * Copyright 2012-2014 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RENDERMODES_BASE_H_
#define RENDERMODES_BASE_H_

#include "../image.h"
#include "../tilerenderer.h"
#include "../../mc/pos.h"
#include "../../mc/worldcache.h"
#include "../../config/mapcrafterconfig.h"

#include <string>
#include <vector>

namespace mapcrafter {
namespace renderer {

/**
 * A simple interface to implement different rendermodes.
 */
class Rendermode {
protected:
	RenderState state;
public:
	Rendermode(const RenderState& state);
	virtual ~Rendermode();

	// is called when the tile renderer starts rendering a tile
	virtual void start();
	// is called when the tile renderer finished rendering a tile
	virtual void end();

	// is called to allow the rendermode to hide specific blocks
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	// is called to allow the rendermode to change a block image
	virtual void draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);
};

bool createRendermode(const config::WorldSection& world_config,
		const config::MapSection& map_config,
		const RenderState& state, std::vector<std::shared_ptr<Rendermode>>& modes);

} /* namespace render */
} /* namespace mapcrafter */

#include "cave.h"
#include "lighting.h"

#endif /* RENDERMODES_BASE_H_ */
