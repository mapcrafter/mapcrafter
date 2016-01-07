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

#ifndef RENDERMODES_CAVE_H_
#define RENDERMODES_CAVE_H_

#include "../rendermode.h"
#include "../../mc/pos.h"

#include <vector>

namespace mapcrafter {
namespace renderer {

class CaveRenderMode : public BaseRenderMode<> {
public:
	CaveRenderMode(const std::vector<mc::BlockPos>& hidden_dirs);
	virtual ~CaveRenderMode();

	virtual bool isHidden(const mc::BlockPos& pos,
			uint16_t id, uint16_t data);

protected:
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

} /* namespace render */
} /* namespace mapcrafter */
#endif /* RENDERMODES_CAVE_H_ */
