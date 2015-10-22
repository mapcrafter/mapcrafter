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

IsometricLightingRenderer::~IsometricLightingRenderer() {
}

void IsometricLightingRenderer::lightLeft(RGBAImage& image, const CornerColors& colors,
		int y_start, int y_end) const {
	int size = image.getWidth() / 2;
	RGBAImage shade(size, size);
	createShade(shade, colors);

	for (SideFaceIterator it(size, SideFaceIterator::LEFT); !it.end(); it.next()) {
		if (it.src_y < y_start || it.src_y > y_end)
			continue;
		uint32_t& pixel = image.pixel(it.dest_x, it.dest_y + size/2);
		uint8_t d = rgba_alpha(shade.pixel(it.src_x, it.src_y));
		pixel = rgba(0, 0, 0, 255 - d);
		/*
		if (pixel != 0) {
			uint8_t d = rgba_alpha(shade.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
		*/
	}
}

void IsometricLightingRenderer::lightLeft(RGBAImage& image, const CornerColors& colors) const {
	lightLeft(image, colors, 0, image.getHeight() / 2);
}

void IsometricLightingRenderer::lightRight(RGBAImage& image, const CornerColors& colors,
		int y_start, int y_end) const {
	int size = image.getWidth() / 2;
	RGBAImage shade(size, size);
	createShade(shade, colors);

	for (SideFaceIterator it(size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
		if (it.src_y < y_start || it.src_y > y_end)
			continue;
		uint32_t& pixel = image.pixel(it.dest_x + size, it.dest_y + size/2);
		uint8_t d = rgba_alpha(shade.pixel(it.src_x, it.src_y));
		pixel = rgba(0, 0, 0, 255 - d);
		/*
		if (pixel != 0) {
			uint8_t d = rgba_alpha(shade.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
		*/
	}
}

void IsometricLightingRenderer::lightRight(RGBAImage& image, const CornerColors& colors) const {
	lightRight(image, colors, 0, image.getHeight() / 2);
}

void IsometricLightingRenderer::lightTop(RGBAImage& image, const CornerColors& colors,
		int yoff) const {
		int size = image.getWidth() / 2;
	RGBAImage shade(size, size);
	// we need to rotate the corners a bit to make them suitable for the TopFaceIterator
	CornerColors rotated = {{colors[1], colors[3], colors[0], colors[2]}};
	createShade(shade, rotated);

	for (TopFaceIterator it(size); !it.end(); it.next()) {
		uint32_t& pixel = image.pixel(it.dest_x, it.dest_y + yoff);
		uint8_t d = rgba_alpha(shade.pixel(it.src_x, it.src_y));
		pixel = rgba(0, 0, 0, 255 - d);
		/*
		if (pixel != 0) {
			uint8_t d = rgba_alpha(shade.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
		*/
	}
}

void IsometricOverlayRenderer::tintLeft(RGBAImage& image, RGBAPixel color) const {
	int texture_size = image.getWidth() / 2;
	
	auto overlay = getRecolor(color);
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x, it.dest_y + texture_size/2);
		pixel = color;
		/*
		if (high_contrast)
			pixel = rgba_add_clamp(pixel, overlay);
		else
			blend(pixel, color);
		*/
	}
}

void IsometricOverlayRenderer::tintRight(RGBAImage& image, RGBAPixel color) const {
	int texture_size = image.getWidth() / 2;
	
	auto overlay = getRecolor(color);
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x + texture_size, it.dest_y + texture_size/2);
		pixel = color;
		/*
		if (high_contrast)
			pixel = rgba_add_clamp(pixel, overlay);
		else
			blend(pixel, color);
		*/
	}
}

void IsometricOverlayRenderer::tintTop(RGBAImage& image, RGBAPixel color, int offset) const {
	int texture_size = image.getWidth() / 2;

	auto overlay = getRecolor(color);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		RGBAPixel& pixel = image.pixel(it.dest_x, it.dest_y);
		pixel = color;
		/*
		if (high_contrast)
			pixel = rgba_add_clamp(pixel, overlay);
		else
			blend(pixel, color);
		*/
	}
}

}
}

