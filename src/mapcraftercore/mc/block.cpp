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

#include "block.h"

namespace mapcrafter {
namespace mc {

Block::Block()
//	: Block(mc::BlockPos(0, 0, 0), 0, 0) { /* gcc 4.4 being stupid :/ */
	: pos(mc::BlockPos(0, 0, 0)), id(0), data(0), biome(0),
	  block_light(0), sky_light(15), fields_set(0) {
}

Block::Block(const mc::BlockPos& pos, uint16_t id, uint16_t data)
	: pos(pos), id(id), data(data), biome(0),
	  block_light(0), sky_light(15), fields_set(GET_ID | GET_DATA) {
}

bool Block::isFullWater() const {
	return (id == 8 || id == 9) && (data & 0x7) == 0;
}

bool Block::isStairs() const {
	return id == 53 || id == 67 || id == 108 || id == 109 || id == 114 || id == 128 || id == 134 || id == 135 || id == 136 || id == 156 || id == 163 || id == 164 || id == 180 || id == 203;
}

}
}
