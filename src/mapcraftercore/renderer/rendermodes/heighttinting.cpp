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

#include "heighttinting.h"

#include "../image.h"
#include "../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

HeightTintingRenderer::~HeightTintingRenderer() {
}

void HeightTintingRenderer::draw(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b,
		bool high_contrast) {
	if (high_contrast) {
		// if high contrast mode is enabled, then do some magic here

		// get luminance of recolor
		int luminance = (10*r + 3*g + b) / 14;

		// try to do luminance-neutral additive/subtractive color
		// instead of alpha blending (for better contrast)
		// so first subtract luminance from each component
		int nr = (r - luminance) / 3; // /3 is similar to alpha=85
		int ng = (g - luminance) / 3;
		int nb = (b - luminance) / 3;

		int size = image.getWidth();
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				uint32_t pixel = image.getPixel(x, y);
				if (pixel != 0)
					image.setPixel(x, y, rgba_add_clamp(pixel, nr, ng, nb));
			}
		}
	} else {
		// otherwise just simple alphablending
		uint32_t color = rgba(r, g, b, 128);

		int size = image.getWidth();
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				uint32_t pixel = image.getPixel(x, y);
				if (pixel != 0)
					image.setPixel(x, y, color);
			}
		}
	}
}

HeightTintingRenderMode::HeightTintingRenderMode(bool high_contrast)
	: high_contrast(high_contrast) {
}

HeightTintingRenderMode::~HeightTintingRenderMode() {
}

void HeightTintingRenderMode::draw(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
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

	HeightTintingRenderer* renderer = dynamic_cast<HeightTintingRenderer*>(renderer_ptr);
	assert(renderer != nullptr);
	renderer->draw(image, r, g, b, high_contrast);
}

BaseRenderModeType HeightTintingRenderMode::getType() const {
	return BaseRenderModeType::HEIGHTTINTING;
}

} /* namespace renderer */
} /* namespace mapcrafter */
