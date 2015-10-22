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

namespace mapcrafter {
namespace renderer {

TopdownLightingRenderer::~TopdownLightingRenderer() {
}

void TopdownLightingRenderer::lightLeft(RGBAImage& image, const CornerColors& colors,
		int y_start, int y_end) const {
	// not available in topdown render view
}

void TopdownLightingRenderer::lightLeft(RGBAImage& image, const CornerColors& colors) const {
	// not available in topdown render view
}

void TopdownLightingRenderer::lightRight(RGBAImage& image, const CornerColors& colors,
		int y_start, int y_end) const {
	// not available in topdown render view
}

void TopdownLightingRenderer::lightRight(RGBAImage& image, const CornerColors& colors) const {
	// not available in topdown render view
}

void TopdownLightingRenderer::lightTop(RGBAImage& image, const CornerColors& colors,
		int yoff) const {
	assert(image.getWidth() == image.getHeight());
	int size = image.getWidth();
	RGBAImage shade(size, size);
	createShade(shade, colors);
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			uint32_t& pixel = image.pixel(x, y);
			uint8_t d = rgba_alpha(shade.pixel(x, y));
			pixel = rgba(0, 0, 0, 255 - d);
			/*
			if (pixel != 0) {
				uint8_t d = rgba_alpha(shade.pixel(x, y));
				pixel = rgba_multiply(pixel, d, d, d);
			}
			*/
		}
	}
}

void TopdownOverlayRenderer::tintLeft(RGBAImage& image, RGBAPixel color) const {
	// not available in topdown render view
}

void TopdownOverlayRenderer::tintRight(RGBAImage& image, RGBAPixel color) const {
	// not available in topdown render view
}

void TopdownOverlayRenderer::tintTop(RGBAImage& image, RGBAPixel color, int offset) const {
	// topdown = just tint the whole image
	tintBlock(image, color);
}

}
}

