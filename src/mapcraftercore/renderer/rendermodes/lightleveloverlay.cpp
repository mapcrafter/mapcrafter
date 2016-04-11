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

#include "lightleveloverlay.h"

#include "lighting.h"
#include "../../util.h"

namespace mapcrafter {
namespace renderer {

LightLevelOverlay::LightLevelOverlay(std::shared_ptr<config::ConfigSection> overlay_config)
	: TintingOverlay(TintingOverlayMode::PER_FACE, overlay_config) {
	for (int i = 0; i <= 7; i++) {
		double t = (double) i / 7.0;
		colors[i] = util::Color(255, 0, 0, 150).mix(util::Color(255, 0, 0, 75), t).getRGBA();
	}

	for (int i = 8; i < 16; i++) {
		double t = (double) (i - 8) / 8.0;
		colors[i] = util::Color(0, 255, 0, 75).mix(util::Color(0, 255, 0, 150), t).getRGBA();
	}
}

RGBAPixel LightLevelOverlay::getBlockColor(const mc::BlockPos& pos,
		const mc::BlockPos& for_pos, uint16_t id, uint16_t data) {
	// TODO more options
	// TODO also mobs can't spawn on specific blocks?
	mc::Block block = getBlock(pos, mc::GET_ID | mc::GET_DATA | mc::GET_LIGHT);
	LightingData light = LightingData::estimate(block, images, world, *current_chunk);
	uint8_t light_level = light.getLightLevel(config->isDay());
	return colors.at(light_level);
}

}
}

