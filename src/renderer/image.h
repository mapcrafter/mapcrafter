/*
 * Copyright 2012-2014 Moritz Hilscher
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

#ifndef IMAGE_H_
#define IMAGE_H_

#include <png.h>
#include <cstdint>
#include <string>
#include <vector>

# ifndef UINT64_C
#  if __WORDSIZE == 64
#   define UINT64_C(c)	c ## UL
#  else
#   define UINT64_C(c)	c ## ULL
#  endif
# endif

namespace mapcrafter {
namespace renderer {

uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
uint8_t rgba_red(uint32_t value);
uint8_t rgba_green(uint32_t value);
uint8_t rgba_blue(uint32_t value);
uint8_t rgba_alpha(uint32_t value);

uint32_t rgba_multiply(uint32_t value, double r, double g, double b, double a = 1);
uint32_t rgba_multiply(uint32_t value, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

void blend(uint32_t& dest, const uint32_t& source);

void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length);
void pngWriteData(png_structp pngPtr, png_bytep data, png_size_t length);

const int ROTATE_90 = 1;
const int ROTATE_180 = 2;
const int ROTATE_270 = 3;

class RGBAImage {
private:
	int width;
	int height;

	std::vector<uint32_t> data;

public:
	RGBAImage();
	RGBAImage(int width, int height);
	~RGBAImage();

	void setSize(int w, int h);

	int getWidth() const;
	int getHeight() const;

	uint32_t getPixel(int x, int y) const;
	void setPixel(int x, int y, uint32_t pixel);

	const uint32_t& pixel(int x, int y) const;
	uint32_t& pixel(int x, int y);

	void simpleblit(const RGBAImage& image, int x, int y);
	void alphablit(const RGBAImage& image, int x, int y);
	void blendPixel(uint32_t color, int x, int y);
	void fill(uint32_t color, int x1, int y1, int w, int h);
	void clear();

	RGBAImage clip(int x, int y, int width, int height) const;
	RGBAImage colorize(double r, double g, double b, double a = 1) const;
	RGBAImage colorize(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) const;
	RGBAImage rotate(int rotation) const;
	RGBAImage flip(bool flip_x, bool flip_y) const;
	RGBAImage move(int x_off, int y_off) const;

	void resizeInterpolated(int new_width, int new_height, RGBAImage& dest) const;
	void resizeSimple(int new_width, int new_height, RGBAImage& dest) const;
	// automatically chooses an image resize interpolation
	void resizeAuto(int new_width, int new_height, RGBAImage& dest) const;
	void resizeHalf(RGBAImage& dest) const;

	bool readPNG(const std::string& filename);
	bool writePNG(const std::string& filename) const;
};

}
}

#endif /* IMAGE_H_ */
