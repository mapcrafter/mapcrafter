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

#include "heightoverlay.h"

#include "../image.h"
#include "../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

HeightOverlay::HeightOverlay()
	: OverlayRenderMode(OverlayMode::PER_BLOCK) {
}

std::string HeightOverlay::getName() const {
	return "height";
}

RGBAPixel HeightOverlay::getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	// TODO make the gradient configurable
	double h1 = (double) (64 - pos.y) / 64;
	if (pos.y > 64)
		h1 = 0;

	double h2 = 0;
	if (pos.y >= 64 && pos.y < 96)
		h2 = (double) (96 - pos.y) / 32;
	else if (pos.y > 16 && pos.y < 64)
		h2 = (double) (pos.y - 16) / 48;

	double h3 = 0;
	if (pos.y > 64)
		h3 = (double) (pos.y - 64) / 64;

	int r = h1 * 128.0 + 128.0;
	int g = h2 * 255.0;
	int b = h3 * 255.0;
	return rgba(r, g, b, 85);
}

} /* namespace renderer */
} /* namespace mapcrafter */
