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

#include <string>
#include <tuple>
#include <vector>

namespace mapcrafter {

namespace config {
class MapSection;
class WorldSection;
}

namespace renderer {

class RGBAImage;

class OverlayRenderer : public RenderModeRenderer {
public:
	OverlayRenderer();
	virtual ~OverlayRenderer();

	// TODO
	// DEPRECATED
	void setHighContrast(bool high_contrast);

	virtual void tintBlock(RGBAImage& image, RGBAPixel color) const;

	virtual void tintLeft(RGBAImage& image, RGBAPixel color) const = 0;
	virtual void tintRight(RGBAImage& image, RGBAPixel color) const = 0;
	virtual void tintTop(RGBAImage& image, RGBAPixel color, int offset) const = 0;

	static const RenderModeRendererType TYPE;

protected:
	std::tuple<int, int, int> getRecolor(RGBAPixel color) const;

	bool high_contrast;
};

enum class OverlayMode {
	PER_BLOCK,
	PER_FACE,
};

/**
 * A render mode that renders an overlay on top of the blocks. You just have to implement
 * the function that returns the color for each block. Return something with alpha == 0
 * if there should be no color.
 */
class OverlayRenderMode : public BaseRenderMode<OverlayRenderer> {
public:
	OverlayRenderMode(OverlayMode overlay_mode);
	virtual ~OverlayRenderMode();

	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual void drawOverlay(RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual std::string getName() const = 0;

protected:
	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

private:
	OverlayMode overlay_mode;
};

std::vector<std::shared_ptr<OverlayRenderMode>> createOverlays(
		const config::WorldSection& world_config, const config::MapSection& map_config,
		int rotation);

}
}

#endif /* RENDERMODES_OVERLAY_H_ */

