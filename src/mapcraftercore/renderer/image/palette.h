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

	virtual const std::vector<RGBAPixel>& getColors() const = 0;
	virtual int getNearestColor(const RGBAPixel& color) = 0;
};

/**
 * Trivial color palette implementation.
 *
 * Very bad O(n^2) color lookup performance! Just used in the test cases.
 */
class SimplePalette : public Palette {
public:
	SimplePalette();
	SimplePalette(const std::vector<RGBAPixel>& colors);
	virtual ~SimplePalette();

	virtual const std::vector<RGBAPixel>& getColors() const;
	virtual int getNearestColor(const RGBAPixel& color);

protected:
	std::vector<RGBAPixel> colors;
};

}
}

#endif /* IMAGE_PALETTE_H_ */
