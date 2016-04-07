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

#ifndef BLOCKHANDLER_H_
#define BLOCKHANDLER_H_

#include "../mc/worldcache.h"

#include <iosfwd>
#include <vector>

namespace mapcrafter {

namespace config {
class MapSection;
}

namespace mc {
class BlockPos;
}

namespace renderer {

class RenderView;
class BlockImages;

enum class BlockHandlerType {
	DEFAULT,
	CAVE
};

std::ostream& operator<<(std::ostream& out, BlockHandlerType type);

class BlockHandler {
public:
	virtual ~BlockHandler() {};

	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk) = 0;

	virtual void setUsePreblitWater(bool use_preblit_water) = 0;

	virtual void handleBlock(const mc::BlockPos& pos, uint16_t& id, uint16_t& data) = 0;
};

class AbstractBlockHandler : public BlockHandler {
public:
	AbstractBlockHandler();

	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk);

	virtual void setUsePreblitWater(bool use_preblit_water);

	virtual void handleBlock(const mc::BlockPos& pos, uint16_t& id, uint16_t& data);

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);

	bool use_preblit_water;
	BlockImages* images;
	mc::WorldCache* world;
	mc::Chunk** current_chunk;
};

class CaveBlockHandler : public AbstractBlockHandler {
public:
	CaveBlockHandler(const std::vector<mc::BlockPos>& hidden_dirs);

	virtual void handleBlock(const mc::BlockPos& pos, uint16_t& id, uint16_t& data);

protected:
	bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	bool isLight(const mc::BlockPos& pos);
	bool isTransparentBlock(const mc::Block& block) const;
	
	// we want to hide some additional cave blocks to be able to look "inside" the caves,
	// so it's possible to specify directions where cave blocks must touch transparent
	// blocks (or air), there must be a transparent block in at least one directions
	// for example, for the isometric render view this would be: south, west and top
	// (because you are looking from the south-west-top at the map and don't want your
	// view into the cave covered by the southern, western, and top walls)
	std::vector<mc::BlockPos> hidden_dirs;
};

BlockHandler* createBlockHandler(const config::MapSection& map_config);

}
}

#endif /* BLOCKHANDLER_h_ */
