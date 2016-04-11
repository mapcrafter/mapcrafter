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
	initializeColors(config->getColors(), config->getDefaultOpacity());
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

void HeightOverlay::initializeColors(const std::vector<config::HeightColor>& segments, int default_opacity) {
	auto base_colors = segments;
	// make sure that we have at least one segment
	if (base_colors.empty()) {
		base_colors.push_back({0, util::Color(0, 0, 0, 0)});
		base_colors.push_back({255, util::Color(0, 0, 0, 0)});
	}

	// go through the segments and generate colors for each y of the segments
	colors.resize(256);
	auto it = base_colors.begin();
	while (it != base_colors.end()) {
		// set first color of segment
		int y_a = it->y;
		util::Color color_a = it->color.withAlphaIfAbsent(default_opacity);
		colors[y_a] = color_a.getRGBA();
		auto next = ++it;
		// stop here if the color is the last color
		if (next == base_colors.end())
			break;

		// otherwise generate all colors of segment
		int y_b = it->y;
		util::Color color_b = next->color.withAlphaIfAbsent(default_opacity);
		for (int y = y_a; y <= y_b; y++) {
			double t = (double) (y - y_a) / (y_b - y_a);
			colors[y] = color_a.mix(color_b, t).getRGBA();
		}
	}
}

} /* namespace renderer */
} /* namespace mapcrafter */
