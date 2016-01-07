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

#include "palette.h"

namespace mapcrafter {
namespace renderer {

Palette::~Palette() {
}

SimplePalette::SimplePalette() {
}

SimplePalette::SimplePalette(const std::vector<RGBAPixel>& colors)
	: colors(colors) {
}

SimplePalette::~SimplePalette() {
}

const std::vector<RGBAPixel>& SimplePalette::getColors() const {
	return colors;
}

int SimplePalette::getNearestColor(const RGBAPixel& color) {
	int best_color = 0;
	int min_distance = -1;
	for (size_t i = 0; i < colors.size(); i++) {
		int distance = rgba_distance2(color, colors[i]);
		if (min_distance == -1 || distance < min_distance) {
			best_color = i;
			min_distance = distance;
		}
	}
	return best_color;
}

}
}
