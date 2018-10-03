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

#include "overlay.h"

#include "../blockimages.h"
#include "../image.h"
#include "../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

OverlayRenderMode::OverlayRenderMode(OverlayMode overlay_mode)
	: overlay_mode(overlay_mode) {
}

OverlayRenderMode::~OverlayRenderMode() {
}

void OverlayRenderMode::draw(RGBAImage& image, const BlockImage& block_image, const mc::BlockPos& pos, uint16_t id) {
	// TODO handle some special cases, for example: colorize blocks under water?
	if (overlay_mode == OverlayMode::PER_BLOCK) {
		// simple mode where we just tint whole blocks
		RGBAPixel color = getBlockColor(pos, block_image);
		if (rgba_alpha(color) == 0)
			return;
		blockImageTintHighContrast(image, color);
	} else {
		// "advanced" mode where each block/position has a color,
		// and adjacent faces are tinted / or the transparent blocks themselves
		// TODO potential for optimization, maybe cache colors of blocks?
		if (block_image.is_transparent) {
			RGBAPixel color = getBlockColor(pos, block_image);
			if (rgba_alpha(color) == 0)
				return;
			blockImageTintHighContrast(image, color);
		} else {
			mc::Block top, left, right;
			RGBAPixel color_top, color_left, color_right;
			top = getBlock(pos + mc::DIR_TOP, mc::GET_ID | mc::GET_DATA);
			left = getBlock(pos + mc::DIR_WEST, mc::GET_ID | mc::GET_DATA);
			right = getBlock(pos + mc::DIR_SOUTH, mc::GET_ID | mc::GET_DATA);
			color_top = getBlockColor(pos + mc::DIR_TOP, block_images->getBlockImage(top.id));
			color_left = getBlockColor(pos + mc::DIR_WEST, block_images->getBlockImage(left.id));
			color_right = getBlockColor(pos + mc::DIR_SOUTH, block_images->getBlockImage(right.id));
			
			if (rgba_alpha(color_top) != 0)
				blockImageTintHighContrast(image, block_image.uv_image, FACE_UP_INDEX, color_top);
			if (rgba_alpha(color_left) != 0)
				blockImageTintHighContrast(image, block_image.uv_image, FACE_LEFT_INDEX, color_left);
			if (rgba_alpha(color_right) != 0)
				blockImageTintHighContrast(image, block_image.uv_image, FACE_RIGHT_INDEX, color_right);
		}
	}
}

}
}

