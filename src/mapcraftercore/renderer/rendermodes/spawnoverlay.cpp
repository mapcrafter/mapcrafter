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

#include "spawnoverlay.h"

namespace mapcrafter {
namespace renderer {

SpawnOverlay::SpawnOverlay()
	: OverlayRenderMode(OverlayMode::PER_FACE) {
}

SpawnOverlay::~SpawnOverlay() {
}

RGBAPixel SpawnOverlay::getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	// just shows where mobs can spawn during the day (light level < 8)
	// TODO more options
	// TODO estimate lighting of special blocks (slabs...), like lighting render mode
	// TODO also mobs can't spawn on specific blocks?
	mc::Block block = getBlock(pos, mc::GET_LIGHT);
	uint8_t day = std::max(block.sky_light, block.block_light);
	// uint8_t night = std::max(block.sky_light - 11, block.block_light + 0);
	if (day < 8)
		return rgba(255, 0, 0, 255);
	return rgba(0, 0, 0, 0);
}

}
}

