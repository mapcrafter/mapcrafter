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


#ifndef RENDERMODES_OVERLAY_H_
#define RENDERMODES_OVERLAY_H_

#include "../rendermode.h"

#include "../image.h"

#include <tuple>

namespace mapcrafter {
namespace renderer {

class RGBAImage;

enum class OverlayMode {
	PER_BLOCK,
	PER_FACE,
};

/**
 * A render mode that renders an overlay on top of the blocks. You just have to implement
 * the function that returns the color for each block. Return something with alpha == 0
 * if there should be no color.
 */
class OverlayRenderMode : public BaseRenderMode {
public:
	OverlayRenderMode(OverlayMode overlay_mode);
	virtual ~OverlayRenderMode();

	virtual void draw(RGBAImage& image, const BlockImage& block_image, const mc::BlockPos& pos, uint16_t id);

protected:
	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, const BlockImage& block_image) { return 0; }

private:
	OverlayMode overlay_mode;
};

}
}

#endif /* RENDERMODES_OVERLAY_H_ */

