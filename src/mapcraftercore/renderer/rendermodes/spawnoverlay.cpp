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

#include "spawnoverlay.h"

#include "lighting.h"

namespace mapcrafter {
namespace renderer {

SpawnOverlay::SpawnOverlay(bool day)
	: TintingOverlay(OverlayMode::PER_FACE, (day ? "spawnday" : "spawnnight"),
			(day ? "Spawn at day" : "Spawn at night")), day(day) {
}

SpawnOverlay::~SpawnOverlay() {
}

RGBAPixel SpawnOverlay::getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	// TODO more options
	// TODO also mobs can't spawn on specific blocks?
	mc::Block block = getBlock(pos, mc::GET_ID | mc::GET_DATA | mc::GET_LIGHT);
	LightingData light = LightingData::estimate(block, images, world, *current_chunk);
	uint8_t light_level = light.getLightLevel(day);
	if (light_level < 8)
		return rgba(255, 0, 0, 85);
	return rgba(0, 0, 0, 0);
}

}
}

