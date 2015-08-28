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

#include "overlay.h"

#include "../blockimages.h"
#include "../image.h"
#include "../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

OverlayRenderer::OverlayRenderer()
	: high_contrast(true) {
}

OverlayRenderer::~OverlayRenderer() {
}

void OverlayRenderer::setHighContrast(bool high_contrast) {
	this->high_contrast = high_contrast;
}

void OverlayRenderer::tintBlock(RGBAImage& image, RGBAPixel color) const {
	if (high_contrast) {
		// do the high contrast mode magic
		auto overlay = getRecolor(color);
		for (int y = 0; y < image.getWidth(); y++) {
			for (int x = 0; x < image.getHeight(); x++) {
				RGBAPixel& pixel = image.pixel(x, y);
				if (pixel != 0)
					pixel = rgba_add_clamp(pixel, overlay);
			}
		}
	} else {
		// otherwise just simple alphablending
		for (int y = 0; y < image.getWidth(); y++) {
			for (int x = 0; x < image.getHeight(); x++) {
				RGBAPixel& pixel = image.pixel(x, y);
				if (pixel != 0)
					blend(pixel, color);
			}
		}
	}
}

std::tuple<int, int, int> OverlayRenderer::getRecolor(RGBAPixel color) const {
	// get luminance of recolor:
	// "10*r + 3*g + b" should actually be "3*r + 10*g + b"
	// it was a typo, but doesn't look bad either
	int luminance = (10 * rgba_red(color) + 3 * rgba_green(color) + rgba_blue(color)) / 14;

	float alpha_factor = 3; // 3 is similar to alpha=85
	// something like that would be possible too, but overlays won't look exactly like
	// overlays with that alpha value, so don't use it for now
	// alpha_factor = (float) 255.0 / rgba_alpha(color);

	// try to do luminance-neutral additive/subtractive color
	// instead of alpha blending (for better contrast)
	// so first subtract luminance from each component
	int nr = (rgba_red(color) - luminance) / alpha_factor;
	int ng = (rgba_green(color) - luminance) / alpha_factor;
	int nb = (rgba_blue(color) - luminance) / alpha_factor;
	return std::make_tuple(nr, ng, nb);
}

const RenderModeRendererType OverlayRenderer::TYPE = RenderModeRendererType::OVERLAY;

OverlayRenderMode::OverlayRenderMode(OverlayMode overlay_mode)
	: overlay_mode(overlay_mode) {
}

OverlayRenderMode::~OverlayRenderMode() {
}

void OverlayRenderMode::draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	// TODO handle some special cases, for example: colorize blocks under water?
	if (overlay_mode == OverlayMode::PER_BLOCK) {
		// simple mode where we just tint whole blocks
		RGBAPixel color = getBlockColor(pos, id, data);
		if (rgba_alpha(color) == 0)
			return;
		renderer->tintBlock(image, color);
	} else {
		// "advanced" mode where each block/position has a color,
		// and adjacent faces are tinted / or the transparent blocks themselves
		// TODO potential for optimization, maybe cache colors of blocks?
		if (images->isBlockTransparent(id, data)) {
			RGBAPixel color = getBlockColor(pos, id, data);
			if (rgba_alpha(color) == 0)
				return;
			renderer->tintBlock(image, color);
		} else {
			mc::Block top, left, right;
			RGBAPixel color_top, color_left, color_right;
			top = getBlock(pos + mc::DIR_TOP, mc::GET_ID | mc::GET_DATA);
			left = getBlock(pos + mc::DIR_WEST, mc::GET_ID | mc::GET_DATA);
			right = getBlock(pos + mc::DIR_SOUTH, mc::GET_ID | mc::GET_DATA);
			color_top = getBlockColor(pos + mc::DIR_TOP, top.id, top.data);
			color_left = getBlockColor(pos + mc::DIR_WEST, left.id, left.data);
			color_right = getBlockColor(pos + mc::DIR_SOUTH, right.id, right.data);
			
			if (rgba_alpha(color_top) != 0)
				renderer->tintTop(image, color_top, 0);
			if (rgba_alpha(color_left) != 0)
				renderer->tintLeft(image, color_left);
			if (rgba_alpha(color_right) != 0)
				renderer->tintRight(image, color_right);
		}
	}
}

}
}

