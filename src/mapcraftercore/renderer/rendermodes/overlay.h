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


#ifndef RENDERMODES_OVERLAY_H_
#define RENDERMODES_OVERLAY_H_

#include "../rendermode.h"

#include "../image.h"

namespace mapcrafter {
namespace renderer {

class RGBAImage;

class OverlayRenderer : public RenderModeRenderer {
public:
	OverlayRenderer();
	virtual ~OverlayRenderer();

	void setHighContrast(bool high_contrast);

	virtual void tintBlock(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b);

	virtual void tintLeft(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b) = 0;
	virtual void tintRight(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b) = 0;
	virtual void tintTop(RGBAImage& image, uint8_t r, uint8_t g, uint8_t b, int offset) = 0;

	static const RenderModeRendererType TYPE;

protected:
	bool high_contrast;
};

/**
 * A render mode that renders an overlay on top of the blocks. You just have to implement
 * the function that returns the color for each block. Return something with alpha == 0
 * if there should be no color (alpha values are ignored, except the alpha == 0 thing).
 */
class OverlayRenderMode : public BaseRenderMode<OverlayRenderer> {
public:
	virtual ~OverlayRenderMode();
	
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;
};

}
}

#endif /* RENDERMODES_OVERLAY_H_ */

