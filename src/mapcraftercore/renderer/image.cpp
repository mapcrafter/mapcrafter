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

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#endif

#include "image.h"

#include "image/dithering.h"
#include "image/quantization.h"
#include "image/scaling.h"
#include "../util.h"

#include <jpeglib.h>
#include <algorithm>
#include <iostream>
#include <fstream>

namespace mapcrafter {
namespace renderer {

RGBAPixel rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a << 24) | (b << 16) | (g << 8) | r;
}

uint8_t rgba_red(RGBAPixel value) {
	return value & 0xff;
}

uint8_t rgba_green(RGBAPixel value) {
	return (value & 0xff00) >> 8;
}

uint8_t rgba_blue(RGBAPixel value) {
	return (value & 0xff0000) >> 16;
}

uint8_t rgba_alpha(RGBAPixel value) {
	return (value & 0xff000000) >> 24;
}

uint8_t clamp(int c) {
	if (c < 0)
		return 0;
	if (c > 255)
		return 255;
	return c;
}

RGBAPixel rgba_add_clamp(RGBAPixel value, int r, int g, int b, int a) {
	return rgba(
		clamp(r + rgba_red(value)),
		clamp(g + rgba_green(value)),
		clamp(b + rgba_blue(value)),
		clamp(a + rgba_alpha(value))
	);
}

RGBAPixel rgba_add_clamp(RGBAPixel value, const std::tuple<int, int, int>& values) {
	return rgba_add_clamp(value, std::get<0>(values), std::get<1>(values), std::get<2>(values));
}

RGBAPixel rgba_multiply(RGBAPixel value, double r, double g, double b, double a) {
	uint8_t red = rgba_red(value);
	uint8_t green = rgba_green(value);
	uint8_t blue = rgba_blue(value);
	uint8_t alpha = rgba_alpha(value);
	return rgba(red * r, green * g, blue * b, alpha * a);
}

RGBAPixel rgba_multiply(RGBAPixel value, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	int red = (rgba_red(value) * r) / 255;
	int green = (rgba_green(value) * g) / 255;
	int blue = (rgba_blue(value) * b) / 255;
	int alpha = (rgba_alpha(value) * a) / 255;
	return rgba(red, green, blue, alpha);
}

int rgba_distance2(RGBAPixel value1, RGBAPixel value2) {
	return std::pow(rgba_red(value1) - rgba_red(value2), 2)
		+ std::pow(rgba_green(value1) - rgba_green(value2), 2)
		+ std::pow(rgba_blue(value1) - rgba_blue(value2), 2)
		+ std::pow(rgba_alpha(value1) - rgba_alpha(value2), 2);
}

# ifndef UINT64_C
#  if __WORDSIZE == 64
#   define UINT64_C(c)	c ## UL
#  else
#   define UINT64_C(c)	c ## ULL
#  endif
# endif

/**
 * This code is from pigmap.
 * Thanks to Michael J. Nelson (equalpants) for this fast alpha blending.
 *
 * https://github.com/equalpants/pigmap (rgba.cpp)
 */
void blend(RGBAPixel& dest, const RGBAPixel& source) {
	// if source is transparent, there's nothing to do
	if (source <= 0xffffff)
		return;
	// if source is opaque, or if destination is transparent, just copy it over
	else if (source >= 0xff000000 || dest <= 0xffffff)
		dest = source;
	// if source is translucent and dest is opaque, the color channels need to be blended,
	//  but the new pixel will be opaque
	else if (dest >= 0xff000000) {
		// get sa and sainv in the range 1-256; this way, the possible results of blending 8-bit color channels sc and dc
		//  (using sc*sa + dc*sainv) span the range 0x0000-0xffff, so we can just truncate and shift
		int64_t sa = rgba_alpha(source) + 1;
		int64_t sainv = 257 - sa;
		// compute the new RGB channels
		int64_t d = dest, s = source;
		d = ((d << 16) & UINT64_C(0xff00000000)) | ((d << 8) & 0xff0000) | (d & 0xff);
		s = ((s << 16) & UINT64_C(0xff00000000)) | ((s << 8) & 0xff0000) | (s & 0xff);
		int64_t newrgb = s * sa + d * sainv;
		// destination alpha remains 100%; combine everything and write it out
		dest = 0xff000000 | ((newrgb >> 24) & 0xff0000) | ((newrgb >> 16) & 0xff00)
		        | ((newrgb >> 8) & 0xff);

		// both source and dest are translucent; we need the whole deal
	} else {
		// get sa and sainv in the range 1-256; this way, the possible results of blending 8-bit color channels sc and dc
		//  (using sc*sa + dc*sainv) span the range 0x0000-0xffff, so we can just truncate and shift
		int64_t sa = rgba_alpha(source) + 1;
		int64_t sainv = 257 - sa;
		// compute the new RGB channels
		int64_t d = dest, s = source;
		d = ((d << 16) & UINT64_C(0xff00000000)) | ((d << 8) & 0xff0000) | (d & 0xff);
		s = ((s << 16) & UINT64_C(0xff00000000)) | ((s << 8) & 0xff0000) | (s & 0xff);
		int64_t newrgb = s * sa + d * sainv;
		// compute the new alpha channel
		int64_t dainv = 256 - rgba_alpha(dest);
		int64_t newa = sainv * dainv; // result is from 1-0x10000
		newa = (newa - 1) >> 8; // result is from 0-0xff
		newa = 255 - newa; // final result; if either input was 255, so is this, so opacity is preserved
		// combine everything and write it out
		dest = (newa << 24) | ((newrgb >> 24) & 0xff0000) | ((newrgb >> 16) & 0xff00)
		        | ((newrgb >> 8) & 0xff);
	}
}

/**
 * http://www.piko3d.com/tutorials/libpng-tutorial-loading-png-files-from-streams
 */
void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length) {
	//Here we get our IO pointer back from the read struct.
	//This is the parameter we passed to the png_set_read_fn() function.
	//Our std::istream pointer.
	png_voidp a = png_get_io_ptr(pngPtr);
	//Cast the pointer to std::istream* and read 'length' bytes into 'data'
	((std::istream*) a)->read((char*) data, length);
}

void pngWriteData(png_structp pngPtr, png_bytep data, png_size_t length) {
	png_voidp a = png_get_io_ptr(pngPtr);
	((std::ostream*) a)->write((char*) data, length);
}

RGBAImage::RGBAImage(int width, int height)
	: Image<RGBAPixel>(width, height) {
}

RGBAImage::~RGBAImage() {
}

void RGBAImage::simpleBlit(const RGBAImage& image, int x, int y) {
	if (x >= width || y >= height)
		return;

	int sx = std::max(0, -x);
	int sy;
	for (; sx < image.width && sx+x < width; sx++) {
		sy = std::max(0, -y);
		for (; sy < image.height && sy+y < height; sy++) {
				data[(sy+y) * width + (sx+x)] = image.data[sy * image.width + sx];
		}
	}
}

void RGBAImage::simpleAlphaBlit(const RGBAImage& image, int x, int y) {
	if (x >= width || y >= height)
		return;

	/*
	int dx = MAX(x, 0);
	int sx = MAX(0, -x);
	for (; sx < image.width && dx < width; sx++, dx++) {
		int dy = MAX(y, 0);
		int sy = MAX(0, -y);
		for (; sy < image.height && dy < height; sy++, dy++) {
			//blend(data[dy*width+dx], image.data[sy*image.width+sx]);

			if (ALPHA(image.data[sy*image.width+sx]) != 0) {
				data[dy * width + dx] = image.data[sy * image.width + sx];
			}
		}
	}
	*/

	int sx = std::max(0, -x);
	int sy;
	for (; sx < image.width && sx+x < width; sx++) {
		sy = std::max(0, -y);
		for (; sy < image.height && sy+y < height; sy++) {
			if (rgba_alpha(image.data[sy*image.width+sx]) != 0) {
				data[(sy+y) * width + (sx+x)] = image.data[sy * image.width + sx];
			}
		}
	}
}

void RGBAImage::alphaBlit(const RGBAImage& image, int x, int y) {
	if (x >= width || y >= height)
		return;

	/*
	int dx = MAX(x, 0);
	int sx = MAX(0, -x);
	for (; sx < image.width && dx < width; sx++, dx++) {
		int dy = MAX(y, 0);
		int sy = MAX(0, -y);
		for (; sy < image.height && dy < height; sy++, dy++) {
			blend(data[dy * width + dx], image.data[sy * image.width + sx]);
		}
	}
	*/

	int sx = std::max(0, -x);
	int sy;
	for (; sx < image.width && sx+x < width; sx++) {
		sy = std::max(0, -y);
		for (; sy < image.height && sy+y < height; sy++) {
			blend(data[(sy+y) * width + (sx+x)], image.data[sy * image.width + sx]);
		}
	}
}

void RGBAImage::blendPixel(RGBAPixel color, int x, int y) {
	if (x >= 0 && y >= 0 && x < width && y < height)
		blend(data[y * width + x], color);
}

void RGBAImage::fill(RGBAPixel color, int x, int y, int w, int h) {
	if (x >= width || y >= height)
		return;

	int dx = std::max(x, 0);
	int sx = std::max(0, -x);
	for (; sx < w && dx < width; sx++, dx++) {
		int dy = std::max(y, 0);
		int sy = std::max(0, -y);
		for (; sy < h && dy < height; sy++, dy++) {
			data[dy * width + dx] = color;
		}
	}
}

void RGBAImage::clear() {
	std::fill(data.begin(), data.end(), 0);
}

RGBAImage RGBAImage::clip(int x, int y, int width, int height) const {
	RGBAImage image(width, height);
	for (int xx = 0; xx < width && xx + x < this->width; xx++) {
		for (int yy = 0; yy < height && yy + y < this->height; yy++) {
			image.setPixel(xx, yy, getPixel(x + xx, y + yy));
		}
	}
	return image;
}

RGBAImage RGBAImage::colorize(double r, double g, double b, double a) const {
	RGBAImage img(width, height);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img.setPixel(x, y, rgba_multiply(getPixel(x, y), r, g, b, a));
		}
	}
	return img;
}

RGBAImage RGBAImage::colorize(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const {
	RGBAImage img(width, height);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img.setPixel(x, y, rgba_multiply(getPixel(x, y), r, g, b, a));
		}
	}
	return img;
}

RGBAImage RGBAImage::rotate(int rotation) const {
	// TODO rotate by rotation % 4?
	if (rotation == 0)
		return *this;
	int newWidth = rotation == ROTATE_90 || rotation == ROTATE_270 ? height : width;
	int newHeight = rotation == ROTATE_90 || rotation == ROTATE_270 ? width : height;
	RGBAImage copy(newWidth, newHeight);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			RGBAPixel pixel = 0;
			if (rotation == ROTATE_90)
				pixel = getPixel(y, width - x - 1);
			else if (rotation == ROTATE_180)
				pixel = getPixel(width - x - 1, height - y - 1);
			else if (rotation == ROTATE_270)
				pixel = getPixel(height - y - 1, x);
			copy.setPixel(x, y, pixel);
		}
	}
	return copy;
}

RGBAImage RGBAImage::flip(bool flipX, bool flipY) const {
	RGBAImage copy(width, height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int xx = flipX ? width - x - 1 : x;
			int yy = flipY ? height - y - 1 : y;
			copy.setPixel(x, y, getPixel(xx, yy));
		}
	}
	return copy;
}

RGBAImage RGBAImage::move(int xOffset, int yOffset) const {
	RGBAImage img(width, height);
	for (int y = 0; y < height && y + yOffset < height; y++) {
		for (int x = 0; x < width && x + xOffset < width; x++) {
			img.setPixel(x + xOffset, y + yOffset, getPixel(x, y));
		}
	}
	return img;
}

void RGBAImage::resize(RGBAImage& dest, int width, int height, InterpolationType interpolation) const {
	if (width == getWidth() && height == getHeight()) {
		dest = *this;
		return;
	}
	if (interpolation == InterpolationType::AUTO) {
		interpolation = InterpolationType::BILINEAR;
		if (width > getWidth() || height > getWidth())
			interpolation = InterpolationType::NEAREST;
		if (width == getWidth() / 2 && height == getHeight() / 2)
			interpolation = InterpolationType::HALF;
	}

	if (interpolation == InterpolationType::NEAREST) {
		imageResizeSimple(*this, dest, width, height);
	} else if (interpolation == InterpolationType::BILINEAR) {
		imageResizeBilinear(*this, dest, width, height);
	} else if (interpolation == InterpolationType::HALF) {
		imageResizeHalf(*this, dest);
	} else {
		// should not happen
		assert(false);
	}
}

RGBAImage RGBAImage::resize(int width, int height, InterpolationType interpolation) const {
	if (width == getWidth() && height == getHeight())
		return *this;
	RGBAImage temp;
	resize(temp, width, height, interpolation);
	return temp;
}


RGBAImage& RGBAImage::shearX(double factor) {
	for (int y = 0; y < height; y++) {
		int shear = -(y - height/2) * factor;
		int start = shear > 0 ? width-1 : 0;
		int dir = shear > 0 ? -1 : 1;
		for (int x = start; x >= 0 && x < width; x += dir)
			setPixel(x, y, getPixel(x - shear, y));
	}
	return *this;
}

RGBAImage& RGBAImage::shearY(double factor) {
	for (int x = 0; x < width; x++) {
		int shear = -(x - width/2) * factor;
		int start = shear > 0 ? height-1 : 0;
		int dir = shear > 0 ? -1 : 1;
		for (int y = start; y >= 0 && y < height; y += dir)
			setPixel(x, y, getPixel(x, y - shear));
	}
	return *this;
}

RGBAImage& RGBAImage::rotateByShear(double degrees) {
	// some tricks to minimize the broken parts of the image caused by the shearing
	while (degrees < 0)
		degrees += 360;
	while (degrees > 360)
		degrees -= 360;
	if (degrees > 90) {
		int n = degrees / 90;
		// unfortunately this is not in-place // TODO ?
		*this = rotate(n);
		degrees -= n*90;
	}

	double radians = degrees / 180.0 * M_PI;
	shearX(-std::tan(radians/2));
	shearY(std::sin(radians));
	shearX(-std::tan(radians/2));
	return *this;
}

RGBAPixel blurKernel(const RGBAImage& image, int x, int y, int radius) {
	int r = 0, g = 0, b = 0, a = 0;
	int count = 0;
	for (int dx = -radius; dx <= radius; dx++)
		for (int dy = -radius; dy <= radius; dy++) {
			int x2 = x + dx;
			int y2 = y + dy;
			if (x2 < 0 || y2 < 0 || x2 >= image.getWidth() || y2 >= image.getHeight())
				continue;
			RGBAPixel pixel = image.getPixel(x2, y2);
			r += (int) rgba_red(pixel) * (int) rgba_alpha(pixel);
			g += (int) rgba_green(pixel) * (int) rgba_alpha(pixel);
			b += (int) rgba_blue(pixel) * (int) rgba_alpha(pixel);
			a += rgba_alpha(pixel);
			count++;
		}
	return a ? rgba(r / a, g / a, b / a, a / count) : 0;
}

void RGBAImage::blur(RGBAImage& dest, int radius) const {
	dest.setSize(width, height);

	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			dest.pixel(x, y) = blurKernel(*this, x, y, radius);
}

bool RGBAImage::readPNG(const std::string& filename) {
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file) {
		return false;
	}

	uint8_t png_signature[8];
	file.read((char*) &png_signature, 8);
	if (png_sig_cmp(png_signature, 0, 8) != 0)
		return false;

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		return false;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_read_struct(&png, NULL, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL);
		return false;
	}

	png_set_read_fn(png, (png_voidp) &file, pngReadData);
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);
	int color = png_get_color_type(png, info);
	int bit_depth = png_get_bit_depth(png, info);

	// strip down images of 16 bits per channel to 8 bits per channel
	if (bit_depth == 16)
		png_set_strip_16(png);

	// convert gray images to rgb(a)
	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);
	// make sure they are also using 8 bit per channel
	if (color == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	// convert indexed images to rgb(a)
	if (color == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// add alpha channel if not existing
	if ((color & PNG_COLOR_MASK_ALPHA) == 0)
		png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);

	setSize(png_get_image_width(png, info), png_get_image_height(png, info));

	png_set_interlace_handling(png);
	png_read_update_info(png, info);

	png_bytep* rows = (png_bytep*) png_malloc(png, height * sizeof(png_bytep));
	uint32_t* p = &data[0];
	for (int32_t i = 0; i < height; i++, p += width)
		rows[i] = (png_bytep) p;

	if (mapcrafter::util::isBigEndian()) {
		png_set_bgr(png);
		png_set_swap_alpha(png);
	}
	png_read_image(png, rows);
	png_read_end(png, NULL);
	
	png_free(png, rows);
	png_destroy_read_struct(&png, &info, NULL);

	return true;
}

bool RGBAImage::writePNG(const std::string& filename) const {
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file) {
		return false;
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png == NULL)
		return false;

	png_infop info = png_create_info_struct(png);
	if (info == NULL) {
		png_destroy_write_struct(&png, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		return false;
	}

	png_set_write_fn(png, (png_voidp) &file, pngWriteData, NULL);
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
	        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_bytep* rows = (png_bytep*) png_malloc(png, height * sizeof(png_bytep));
	const uint32_t* p = &data[0];
	for (int32_t i = 0; i < height; i++, p += width)
		rows[i] = (png_bytep) p;

	png_set_rows(png, info, rows);

	if (mapcrafter::util::isBigEndian())
		png_write_png(png, info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ALPHA, NULL);
	else
		png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

	file.close();
	png_free(png, rows);
	png_destroy_write_struct(&png, &info);
	return true;
}

namespace {

void setRowPixel(png_byte* line, int bit_depth, int x, uint8_t index) {
	if (bit_depth == 8) {
		line[x] = index;
	} else if (bit_depth == 4) {
		index &= 0xf;
		if ((x % 2) == 0)
			line[x/2] = (line[x/2] & 0x0f) | (index << 4);
		else
			line[x/2] = (line[x/2] & 0xf0) | index;
	} else if (bit_depth == 2) {
		index &= 0x3;
		int mod = 3 - (x % 4);
		line[x/4] = (line[x/4] & ~(0x3 << mod*2)) | (index << mod*2);
	} else if (bit_depth == 1) {
		if (index)
			line[x/8] |= (1 << (7 - (x % 8)));
		else
			line[x/8] &= ~(1 << (7 - (x % 8)));
	}
}

}

bool RGBAImage::writeIndexedPNG(const std::string& filename, int palette_bits, bool dithered) const {
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file) {
		return false;
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png == NULL)
		return false;

	png_infop info = png_create_info_struct(png);
	if (info == NULL) {
		png_destroy_write_struct(&png, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		return false;
	}

	int palette_size = 1 << palette_bits;
	png_set_write_fn(png, (png_voidp) &file, pngWriteData, NULL);
	png_set_IHDR(png, info, width, height, palette_bits, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	//std::cout << "Doing quantization." << std::endl;
	Octree* octree;
	std::vector<RGBAPixel> colors;
	octreeColorQuantize(*this, palette_size, colors, &octree);
	palette_size = colors.size();
	//std::cout << "Finished quantization. " << palette_size << " colors." << std::endl;

	png_color* palette = (png_color*) png_malloc(png, palette_size * sizeof(png_color));
	if (palette == NULL) {
		png_destroy_write_struct(&png, &info);
		return false;
	}
	
	png_byte* palette_alpha = (png_byte*) png_malloc(png, palette_size * sizeof(png_byte));
	if (palette == NULL) {
		png_free(png, palette);
		png_destroy_write_struct(&png, &info);
		return false;
	}

	for (int i = 0; i < palette_size; i++) {
		palette[i].red = rgba_red(colors[i]);
		palette[i].green = rgba_green(colors[i]);
		palette[i].blue = rgba_blue(colors[i]);
		palette_alpha[i] = rgba_alpha(colors[i]);
	}

	png_set_PLTE(png, info, palette, palette_size);
	png_set_tRNS(png, info, palette_alpha, palette_size, NULL);

	OctreePalette p(colors);
	//OctreePalette2 p(colors);
	
	std::vector<int> data_dithered;
	if (dithered) {
		RGBAImage copy = *this;
		imageDither(copy, p, data_dithered);
	}

	png_bytep* rows = (png_bytep*) png_malloc(png, height * sizeof(png_bytep));
	for (int y = 0; y < height; y++) {
		rows[y] = (png_byte*) png_malloc(png, width * sizeof(png_byte));
		for (int x = 0; x < width; x++)
			rows[y][x] = 0;
		for (int x = 0; x < width; x++) {
			if (dithered) {
				setRowPixel(rows[y], palette_bits, x, data_dithered[y * width + x]);
			} else {
				setRowPixel(rows[y], palette_bits, x, p.getNearestColor(pixel(x, y)));
			}
		}
	}

	png_set_rows(png, info, rows);

	//if (mapcrafter::util::isBigEndian())
	//	png_write_png(png, info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ALPHA, NULL);
	//else
		png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

	file.close();
	for (int y = 0; y < height; y++)
		png_free(png, rows[y]);
	png_free(png, rows);
	png_free(png, palette);
	png_free(png, palette_alpha);
	delete octree;
	png_destroy_write_struct(&png, &info);
	return true;
}

/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

bool RGBAImage::readJPEG(const std::string& filename) {
	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct my_error_mgr jerr;
	/* More stuff */
	FILE * infile;		/* source file */
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	/* In this example we want to open the input file before doing anything else,
	 * so that the setjmp() error recovery below can assume the file is open.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to read binary files.
	 */

	if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
		//fprintf(stderr, "can't open %s\n", filename.c_str());
		//return 0;
		return false;
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 0;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	 *	 (a) suspension is not possible with the stdio data source, and
	 *	 (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.txt for more info.
	 */

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	 * jpeg_read_header(), so we do nothing here.
	 */

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* We may need to do some setup of our own at this point before reading
	 * the data.	After jpeg_start_decompress() we have the correct scaled
	 * output image dimensions available, as well as the output colormap
	 * if we asked for color quantization.
	 * In this example, we need to make an output work buffer of the right size.
	 */
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*					 jpeg_read_scanlines(...); */

	setSize(cinfo.output_width, cinfo.output_height);

	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */
		for(int x = 0; x < width; x++) {
			uint8_t red = buffer[0][3 * x];
			uint8_t green = buffer[0][3 * x + 1];
			uint8_t blue = buffer[0][3 * x + 2];
			// output_scanline is increased by the jpeg_read_scanlines method
			// before we use it for the image, that's why the -1
			pixel(x, cinfo.output_scanline - 1) = rgba(red, green, blue, 255);
		}
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	 * Here we postpone it until after no more JPEG errors are possible,
	 * so as to simplify the setjmp error logic above.	(Actually, I don't
	 * think that jpeg_destroy can do an error exit, but why assume anything...)
	 */
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */

	/* And we're done! */

	return true;
}

bool RGBAImage::writeJPEG(const std::string& filename, int quality,
		RGBAPixel background) const {

	/* This struct contains the JPEG compression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 * It is possible to have several such structures, representing multiple
	 * compression/decompression processes, in existence at once.	We refer
	 * to any one struct (and its associated working data) as a "JPEG object".
	 */
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.	It is declared separately
	 * because applications often want to supply a specialized error handler
	 * (see the second half of this file for an example).	But here we just
	 * take the easy way out and use the standard error handler, which will
	 * print a message on stderr and call exit() if compression fails.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	 * step fails.	(Unlikely, but it could happen if you are out of memory.)
	 * This routine fills in the contents of struct jerr, and returns jerr's
	 * address which we place into the link field in cinfo.
	 */
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	 * stdio stream.	You can also write your own code to do something else.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to write binary files.
	 */
	if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
		//fprintf(stderr, "can't open %s\n", filename.c_str());
		//exit(1);
		return false;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	 * Four fields of the cinfo struct must be filled in:
	 */
	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	 * (You must set at least cinfo.in_color_space before calling this,
	 * since the defaults depend on the source color space.)
	 */
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	 * Here we just illustrate the use of quality (quantization table) scaling:
	 */
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	 * Pass TRUE unless you are very sure of what you're doing.
	 */
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*					 jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 * To keep things simple, we pass one scanline per call; you can pass
	 * more if you wish, though.
	 */
	std::vector<JSAMPLE> line_buffer(width * 3, 0);
	JSAMPLE* scanlineData = &line_buffer[0];

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		for (int x = 0; x < width; x++) {
			RGBAPixel color = pixel(x, cinfo.next_scanline);
			// jpeg does not support transparency
			// add background color if this pixel has transparency
			// but ignore a bit transparency
			if (rgba_alpha(color) < 250) {
				color = background;
				blend(color, pixel(x, cinfo.next_scanline));
			}

			line_buffer[3 * x] = rgba_red(color);
			line_buffer[3 * x + 1] = rgba_green(color);
			line_buffer[3 * x + 2] = rgba_blue(color);
		}
		(void) jpeg_write_scanlines(&cinfo, &scanlineData, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */
	return true;
}

}
}
