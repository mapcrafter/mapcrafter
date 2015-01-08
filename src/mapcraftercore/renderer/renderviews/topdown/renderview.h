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

#ifndef TOPDOWN_RENDERVIEW_H_
#define TOPDOWN_RENDERVIEW_H_

#include "../../renderview.h"

namespace mapcrafter {
namespace renderer {

class TopdownRenderView : public RenderView {
public:
	virtual BlockImages* createBlockImages() const;
	virtual TileSet* createTileSet(int tile_width) const;
	virtual TileRenderer* createTileRenderer(BlockImages* images, int tile_width,
			mc::WorldCache* world, RenderModes& render_modes) const;
};

} /* namespace renderer */
} /* namespace mapcrafter */

#endif /* TOPDOWN_RENDERVIEW_H_ */
