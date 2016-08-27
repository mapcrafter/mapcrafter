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

#include "areaoverlay.h"

#include "../image.h"
#include "../../mc/pos.h"
#include "../../mc/worldcrop.h"

namespace mapcrafter {
namespace renderer {

AreaOverlay::AreaOverlay(std::shared_ptr<config::ConfigSection> overlay_config)
	: TintingOverlay(TintingOverlayMode::PER_BLOCK, overlay_config),
	  color(config->getColor().withAlphaIfAbsent(85).getRGBA()) {
}

RGBAPixel AreaOverlay::getBlockColor(const mc::BlockPos& pos,
		const mc::BlockPos& pos_for, uint16_t id, uint16_t data) {
	if (config->getArea().isBlockContained(pos))
		return color;
	return rgba(0, 0, 0, 0);
}

} /* namespace renderer */
} /* namespace mapcrafter */
