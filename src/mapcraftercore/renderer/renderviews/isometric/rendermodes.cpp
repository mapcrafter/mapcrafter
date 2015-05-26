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

#include "rendermodes.h"

#include "blockimages.h"
#include "../../image.h"

namespace mapcrafter {
namespace renderer {

void IsometricOverlayRenderer::tintLeft(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b) {
    // TODO implement
}

void IsometricOverlayRenderer::tintRight(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b) {
    // TODO implement
}

void IsometricOverlayRenderer::tintTop(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b,
		int offset) {
	int luminance = (10*r + 3*g + b) / 14;
	int nr = (r - luminance) / 3; // /3 is similar to alpha=85
	int ng = (g - luminance) / 3;
	int nb = (b - luminance) / 3;

	int texture_size = image.getWidth() / 2;
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x, it.dest_y);
		pixel = rgba_add_clamp(pixel, nr, ng, nb);
	}
}

}
}

