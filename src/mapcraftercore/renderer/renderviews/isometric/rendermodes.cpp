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

void IsometricOverlayRenderer::tintLeft(RGBAImage& image, RGBAPixel color) {
	int texture_size = image.getWidth() / 2;
	
	auto overlay = getLuminanceNeutralOverlay(color);
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x, it.dest_y + texture_size/2);
		if (high_contrast)
			pixel = rgba_add_clamp(pixel, overlay);
		else
			blend(pixel, color);
	}
}

void IsometricOverlayRenderer::tintRight(RGBAImage& image, RGBAPixel color) {
	int texture_size = image.getWidth() / 2;
	
	auto overlay = getLuminanceNeutralOverlay(color);
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x + texture_size, it.dest_y + texture_size/2);
		if (high_contrast)
			pixel = rgba_add_clamp(pixel, overlay);
		else
			blend(pixel, color);
	}
}

void IsometricOverlayRenderer::tintTop(RGBAImage& image, RGBAPixel color, int offset) {
	int texture_size = image.getWidth() / 2;

	auto overlay = getLuminanceNeutralOverlay(color);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x, it.dest_y);
		if (high_contrast)
			pixel = rgba_add_clamp(pixel, overlay);
		else
			blend(pixel, color);
	}
}

}
}

