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

SpawnOverlay::SpawnOverlay(std::shared_ptr<config::ConfigSection> overlay_config)
	: TintingOverlay(OverlayMode::PER_FACE, overlay_config),
	  spawn(rgba(255, 0, 0, 85)), no_spawn(rgba(0, 0, 0, 0)) {
}

RGBAPixel SpawnOverlay::getBlockColor(const mc::BlockPos& pos,
		const mc::BlockPos& for_pos, uint16_t id, uint16_t data) {
	// TODO is that interpretation of where mobs can spawn correct?
	
	// don't colorize sides of blocks or transparent blocks (where pos == for_pos)
	if (pos.y - for_pos.y == 0)
		return no_spawn;
	mc::Block block = getBlock(pos, mc::GET_ID | mc::GET_DATA | mc::GET_LIGHT);
	LightingData light = LightingData::estimate(block, images, world, *current_chunk);
	uint8_t light_level = light.getLightLevel(config->isDay());
	if (light_level < 8) {
		// a mob can spawn on the block below if it is not transparent
		// and the light level of this block is smaller than 8
		mc::Block below = getBlock(pos + mc::DIR_BOTTOM, mc::GET_ID | mc::GET_DATA);
		if (below.id != 0 && !images->isBlockTransparent(below.id, below.data))
			return spawn;
	}
	return no_spawn;
}

}
}

