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

#ifndef BLOCK_H_
#define BLOCK_H_

#include "pos.h"

#include <cstdint>

namespace mapcrafter {
namespace mc {

/**
 * A block with id/data/biome/lighting data.
 */
struct Block {
	Block();
	Block(const mc::BlockPos& pos, uint16_t id, uint16_t data);

	// which block does this data belong to (set by getBlock-method)
	mc::BlockPos pos;
	uint16_t id, data;
	uint8_t biome;
	uint8_t block_light, sky_light;
	// which of the fields above are set? (set by getBlock-method)
	int fields_set;

	bool isFullWater() const;
	bool isStairs() const;
};

const int GET_ID = 1;
const int GET_DATA = 2;
const int GET_BIOME = 4;
const int GET_BLOCK_LIGHT = 8;
const int GET_SKY_LIGHT = 16;
const int GET_LIGHT = GET_BLOCK_LIGHT | GET_SKY_LIGHT;

}
}

#endif /* BLOCK_H_ */
