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

#include "../../overlays/lighting.h"
#include "../../overlay.h"

#ifndef TOPDOWN_RENDERMODES_H_
#define TOPDOWN_RENDERMODES_H_

namespace mapcrafter {
namespace renderer {

class TopdownLightingRenderer : public LightingRenderer {
public:
	virtual ~TopdownLightingRenderer();

	virtual void lightLeft(RGBAImage& image, const CornerColors& colors,
			int y_start, int y_end) const;
	virtual void lightLeft(RGBAImage& image, const CornerColors& colors) const;

	virtual void lightRight(RGBAImage& image, const CornerColors& colors,
			int y_start, int y_end) const;
	virtual void lightRight(RGBAImage& image, const CornerColors& colors) const;

	virtual void lightTop(RGBAImage& image, const CornerColors& colors, int yoff) const;
};

class TopdownOverlayRenderer : public TintingOverlayRenderer {
public:
	virtual void tintLeft(RGBAImage& image, RGBAPixel color) const;
	virtual void tintRight(RGBAImage& image, RGBAPixel color) const;
	virtual void tintTop(RGBAImage& image, RGBAPixel color, int offset) const;
};

}
}

#endif /* TOPDOWN_RENDERMODES_H_ */
