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
	  spawn(config->getColor().getRGBA()), no_spawn(rgba(0, 0, 0, 0)) {
}

/**
 * There are some blocks where no mobs can spawn.
 */
bool isNegativeSpawnException(const mc::Block& block) {
	return block.id == 8 || block.id == 9 // water
		|| block.id == 10 || block.id == 11 // lava
		;
}

/**
 * There are some exceptions where mobs can spawn.
 */
bool isPositiveSpawnException(const mc::Block& block) {
	// mobs can spawn on upside-down slabs (no need to mention the double slabs here)
	if (block.id == 44 || block.id == 126 || block.id == 181 || block.id == 182) {
		return block.data & 0x8;
	}
	// mobs can spawn on upside-down stairs
	return block.isStairs() && block.data & 0x4;
}

RGBAPixel SpawnOverlay::getBlockColor(const mc::BlockPos& pos,
		const mc::BlockPos& for_pos, uint16_t id, uint16_t data) {
	// TODO is that interpretation of where mobs can spawn correct?
	
	// don't colorize sides of blocks or transparent blocks (where pos == for_pos)
	// but colorize the top snow layers
	if (!(id == 78 && pos == for_pos) && pos.y - for_pos.y == 0)
		return no_spawn;

	mc::Block block = getBlock(pos, mc::GET_ID | mc::GET_DATA | mc::GET_LIGHT);
	LightingData light = LightingData::estimate(block, images, world, *current_chunk);
	uint8_t light_level = light.getLightLevel(config->isDay());
	if (light_level < 8) {
		// a mob can spawn on the block below if it is not transparent
		// and the light level of this block is smaller than 8
		mc::Block below = getBlock(pos + mc::DIR_BOTTOM, mc::GET_ID | mc::GET_DATA);
		// there are always some exceptions...
		if (!isNegativeSpawnException(below)
			&& (isPositiveSpawnException(below)
				|| (below.id != 0 && !images->isBlockTransparent(below.id, below.data))))
			return spawn;
	}
	return no_spawn;
}

}
}

