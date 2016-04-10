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

#include "heightoverlay.h"

#include "../image.h"
#include "../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

HeightOverlay::HeightOverlay(std::shared_ptr<config::ConfigSection> overlay_config)
	: TintingOverlay(OverlayMode::PER_BLOCK, overlay_config) {
	initializeColors(config->getColorPoints());
}

RGBAPixel HeightOverlay::getBlockColor(const mc::BlockPos& pos,
		const mc::BlockPos& pos_for, uint16_t id, uint16_t data) {
	/*
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
	*/
	return colors[pos.y];
}

void HeightOverlay::initializeColors(std::vector<std::tuple<int, util::Color>> color_points) {
	if (color_points.empty()) {
		color_points.push_back(std::make_tuple(0, util::Color(0, 0, 0, 0)));
		color_points.push_back(std::make_tuple(255, util::Color(0, 0, 0, 0)));
	}

	colors.resize(256);
	auto it = color_points.begin();
	while (it != color_points.end()) {
		int y_a = std::get<0>(*it);
		util::Color color_a = std::get<1>(*it).withAlphaIfAbsent(85);
		colors[y_a] = color_a.getRGBA();
		auto next = ++it;
		if (next == color_points.end())
			continue;
		int y_b = std::get<0>(*next);
		util::Color color_b = std::get<1>(*next).withAlphaIfAbsent(85);
		for (int y = y_a; y <= y_b; y++) {
			double alpha = (double) (y - y_a) / (y_b - y_a);
			colors[y] = color_a.mix(color_b, alpha).getRGBA();
		}
	}
}

} /* namespace renderer */
} /* namespace mapcrafter */
