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

#include "cave.h"

#include "../blockimages.h"
#include "../image.h"
#include "../../mc/chunk.h"

namespace mapcrafter {
namespace renderer {

CaveRenderMode::CaveRenderMode(const std::vector<mc::BlockPos>& hidden_dirs)
	: hidden_dirs(hidden_dirs) {
}

CaveRenderMode::~CaveRenderMode() {
}

bool CaveRenderMode::isLight(const mc::BlockPos& pos) {
	return getBlock(pos, mc::GET_SKY_LIGHT).sky_light > 0;
}

bool CaveRenderMode::isTransparentBlock(const mc::Block& block) const {
	return block.id == 0 || images->isBlockTransparent(block.id, block.data);
}

bool CaveRenderMode::isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	mc::BlockPos directions[6] = {
			mc::DIR_NORTH, mc::DIR_SOUTH, mc::DIR_EAST, mc::DIR_WEST,
			mc::DIR_TOP, mc::DIR_BOTTOM
	};
	// check if this block touches sky light
	for (int i = 0; i < 6; i++) {
		if (isLight(pos + directions[i]))
			return true;
	}

	// water and blocks under water are a special case
	// because water is transparent, the renderer thinks this is a visible part of a cave
	// we need to check if there is sunlight on the surface of the water
	// if yes => no cave, hide block
	// if no  => lake in a cave, show it
	mc::Block top = getBlock(pos + mc::DIR_TOP,
			mc::GET_ID | mc::GET_DATA | mc::GET_SKY_LIGHT);
	if (id == 8 || id == 9 || top.id == 8 || top.id == 9) {
		mc::BlockPos p = pos + mc::DIR_TOP;
		mc::Block block(top.id, top.data, 0, 0, top.sky_light);

		while (block.id == 8 || block.id == 9) {
			block = getBlock(p, mc::GET_ID | mc::GET_DATA | mc::GET_SKY_LIGHT);
			p.y++;
		}

		if (block.sky_light > 0)
			return true;
	}

	// so we show all block which aren't touched by sunlight...
	// and also only the ones that have a transparent block (or air)
	// on at least one of specific sides
	for (auto it = hidden_dirs.begin(); it != hidden_dirs.end(); ++it)
		if (isTransparentBlock(getBlock(pos + *it)))
			return false;
	return true;
}

} /* namespace render */
} /* namespace mapcrafter */
