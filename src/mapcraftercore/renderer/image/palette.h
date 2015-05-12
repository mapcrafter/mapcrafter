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

#ifndef IMAGE_PALETTE_H_
#define IMAGE_PALETTE_H_

#include "../image.h"

#include <vector>

namespace mapcrafter {
namespace renderer {

class Palette {
public:
	virtual ~Palette();

	virtual RGBAPixel getNearestColor(const RGBAPixel& color) const = 0;
};

class SimplePalette : public Palette {
public:
	SimplePalette();
	SimplePalette(const std::vector<RGBAPixel>& colors);
	virtual ~SimplePalette();

	virtual RGBAPixel getNearestColor(const RGBAPixel& color) const;

protected:
	std::vector<RGBAPixel> colors;

	int getColorDistance(RGBAPixel color1, RGBAPixel color2) const;
};

}
}

#endif /* IMAGE_PALETTE_H_ */
