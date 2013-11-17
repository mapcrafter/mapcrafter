/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <string>
#include <vector>
#include <png.h>
#include <stdint.h>

#define ROTATE_90 1
#define ROTATE_180 2
#define ROTATE_270 3

#define ALPHA(value) (((value) & 0xff000000) >> 24)
#define BLUE(value) (((value) & 0xff0000) >> 16)
#define GREEN(value) (((value) & 0xff00) >> 8)
#define RED(value) ((value) & 0xff)

# ifndef UINT64_C
#  if __WORDSIZE == 64
#   define UINT64_C(c)	c ## UL
#  else
#   define UINT64_C(c)	c ## ULL
#  endif
# endif

namespace mapcrafter {
namespace render {

uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
uint32_t rgba_multiply(uint32_t value, double r, double g, double b, double a = 1);
uint32_t rgba_multiply(uint32_t value, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

void blend(uint32_t& dest, const uint32_t& source);

void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length);
void pngWriteData(png_structp pngPtr, png_bytep data, png_size_t length);

class Image {
private:
	int width;
	int height;

	std::vector<uint32_t> data;

public:
	Image();
	Image(int width, int height);
	~Image();

	void setSize(int w, int h);

	int getWidth() const;
	int getHeight() const;

	uint32_t getPixel(int x, int y) const;
	void setPixel(int x, int y, uint32_t pixel);

	const uint32_t& pixel(int x, int y) const;
	uint32_t& pixel(int x, int y);

	void simpleblit(const Image& image, int x, int y);
	void alphablit(const Image& image, int x, int y);
	void blendPixel(uint32_t color, int x, int y);
	void fill(uint32_t color, int x1, int y1, int w, int h);
	void clear();

	Image clip(int x, int y, int width, int height) const;
	Image colorize(double r, double g, double b, double a = 1) const;
	Image colorize(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) const;
	Image rotate(int rotation) const;
	Image flip(bool flip_x, bool flip_y) const;
	Image move(int x_off, int y_off) const;

	void resizeInterpolated(int new_width, int new_height, Image& dest) const;
	void resizeSimple(int new_width, int new_height, Image& dest) const;
	// automatically chooses an image resize interpolation
	void resizeAuto(int new_width, int new_height, Image& dest) const;
	void resizeHalf(Image& dest) const;

	bool readPNG(const std::string& filename);
	bool writePNG(const std::string& filename) const;
};

}
}

#endif /* IMAGE_H_ */
