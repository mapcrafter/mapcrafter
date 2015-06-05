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

#include "../../rendermodes/overlay.h"

#ifndef TOPDOWN_RENDERMODES_H_
#define TOPDOWN_RENDERMODES_H_

namespace mapcrafter {
namespace renderer {

class TopdownOverlayRenderer : public OverlayRenderer {
public:
	virtual void tintLeft(RGBAImage& image, RGBAPixel color);
	virtual void tintRight(RGBAImage& image, RGBAPixel color);
	virtual void tintTop(RGBAImage& image, RGBAPixel color, int offset);
};

}
}

#endif /* TOPDOWN_RENDERVIEWS_H_ */
