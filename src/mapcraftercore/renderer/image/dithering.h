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

#ifndef IMAGE_DITHERING_H_
#define IMAGE_DITHERING_H_

#include <vector>

namespace mapcrafter {
namespace renderer {

class RGBAImage;
class Palette;

/**
 * Applies a Floyd-Steinberg dithering to an image with a given palette.
 *
 * The dithering is performened in-place, so the dithered colors are saved to the image
 * object. Also the dithered image data (indices of palette colors as pixels) is saved to
 * the supplied vector. You can supply a reference to an empty int vector, it will be
 * resized and all the image pixels are saved as data[y * width + x].
 */ 
void imageDither(RGBAImage& image, Palette& palette, std::vector<int>& data);

}
}

#endif /* IMAGE_DITHERING_H_ */
