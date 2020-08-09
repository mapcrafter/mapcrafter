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

#ifndef SIDE_TILERENDERER_H_
#define SIDE_TILERENDERER_H_

#include "../../tilerenderer.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace renderer {

class SideTileRenderer : public TileRenderer {
public:
	SideTileRenderer(const RenderView* render_view, mc::BlockStateRegistry& block_registry,
			BlockImages* images, int tile_width, mc::WorldCache* world, RenderMode* render_mode);
	~SideTileRenderer();

	virtual int getTileSize() const;
	virtual int getTileWidth() const;
	virtual int getTileHeight() const;

protected:
	virtual void renderTopBlocks(const TilePos& tile_pos, std::set<TileImage>& tile_images);
};

}
}

#endif /* SIDE_TILERENDERER_H_ */
