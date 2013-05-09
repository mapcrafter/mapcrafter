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

#include "render/image.h"

#include "util.h"

#include <iostream>
#include <fstream>
#include <algorithm>

namespace mapcrafter {
namespace render {

uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a << 24) | (b << 16) | (g << 8) | r;
}

uint32_t rgba_multiply(uint32_t value, double r, double g, double b, double a) {
	uint8_t red = RED(value);
	uint8_t green = GREEN(value);
	uint8_t blue = BLUE(value);
	uint8_t alpha = ALPHA(value);
	return rgba(red * r, green * g, blue * b, alpha * a);
}

uint32_t rgba_multiply(uint32_t value, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	int red = (RED(value) * r) / 255;
	int green = (GREEN(value) * g) / 255;
	int blue = (BLUE(value) * b) / 255;
	int alpha = (ALPHA(value) * a) / 255;
	return rgba(red, green, blue, alpha);
}

/**
 * This code is from pigmap.
 * Thanks to Michael J. Nelson (equalpants) for this fast alpha blending.
 *
 * https://github.com/equalpants/pigmap (rgba.cpp)
 */
void blend(uint32_t& dest, const uint32_t& source) {
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
		int64_t sa = ALPHA(source) + 1;
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
		int64_t sa = ALPHA(source) + 1;
		int64_t sainv = 257 - sa;
		// compute the new RGB channels
		int64_t d = dest, s = source;
		d = ((d << 16) & UINT64_C(0xff00000000)) | ((d << 8) & 0xff0000) | (d & 0xff);
		s = ((s << 16) & UINT64_C(0xff00000000)) | ((s << 8) & 0xff0000) | (s & 0xff);
		int64_t newrgb = s * sa + d * sainv;
		// compute the new alpha channel
		int64_t dainv = 256 - ALPHA(dest);
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

Image::Image()
		: width(0), height(0), data() {
	//setSize(width, height);
}

Image::Image(int width, int height)
		: width(width), height(height) {
	setSize(width, height);
	clear();
}

Image::~Image() {
}

void Image::setSize(int w, int h) {
	width = w;
	height = h;
	data.resize(width * height);
}

int Image::getWidth() const {
	return width;
}

int Image::getHeight() const {
	return height;
}

uint32_t Image::getPixel(int x, int y) const {
	if (x >= width || x < 0 || y >= height || y < 0)
		return 0;
	return data[y * width + x];
}

void Image::setPixel(int x, int y, uint32_t pixel) {
	if (x >= width || x < 0 || y >= height || y < 0)
		return;
	data[y * width + x] = pixel;
}

// fast pixel access - but no validation
const uint32_t& Image::pixel(int x, int y) const {
	return data[y * width + x];
}

uint32_t& Image::pixel(int x, int y) {
	return data[y * width + x];
}

void Image::simpleblit(const Image& image, int x, int y) {
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

	int sx = MAX(0, -x);
	int sy;
	for (; sx < image.width && sx+x < width; sx++) {
		sy = MAX(0, -y);
		for (; sy < image.height && sy+y < height; sy++) {
			if (ALPHA(image.data[sy*image.width+sx]) != 0) {
				data[(sy+y) * width + (sx+x)] = image.data[sy * image.width + sx];
			}
		}
	}
}

void Image::alphablit(const Image& image, int x, int y) {
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

	int sx = MAX(0, -x);
	int sy;
	for (; sx < image.width && sx+x < width; sx++) {
		sy = MAX(0, -y);
		for (; sy < image.height && sy+y < height; sy++) {
			blend(data[(sy+y) * width + (sx+x)], image.data[sy * image.width + sx]);
		}
	}
}

void Image::blendPixel(uint32_t color, int x, int y) {
	if (x >= 0 && y >= 0 && x < width && y < height)
		blend(data[y * width + x], color);
}

void Image::fill(uint32_t color, int x, int y, int w, int h) {
	if (x >= width || y >= height)
		return;

	int dx = MAX(x, 0);
	int sx = MAX(0, -x);
	for (; sx < w && dx < width; sx++, dx++) {
		int dy = MAX(y, 0);
		int sy = MAX(0, -y);
		for (; sy < h && dy < height; sy++, dy++) {
			data[dy * width + dx] = color;
		}
	}
}

void Image::clear() {
	std::fill(data.begin(), data.end(), 0);
}

Image Image::clip(int x, int y, int width, int height) const {
	Image image(width, height);
	for (int xx = 0; xx < width && xx + x < this->width; xx++) {
		for (int yy = 0; yy < height && yy + y < this->height; yy++) {
			image.setPixel(xx, yy, getPixel(x + xx, y + yy));
		}
	}
	return image;
}

Image Image::colorize(double r, double g, double b, double a) const {
	Image img(width, height);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img.setPixel(x, y, rgba_multiply(getPixel(x, y), r, g, b, a));
		}
	}
	return img;
}

Image Image::rotate(int rotation) const {
	int newWidth = rotation == ROTATE_90 || rotation == ROTATE_270 ? height : width;
	int newHeight = rotation == ROTATE_90 || rotation == ROTATE_270 ? width : height;
	Image copy(newWidth, newHeight);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			uint32_t pixel = 0;
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

Image Image::flip(bool flipX, bool flipY) const {
	Image copy(width, height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int xx = flipX ? width - x - 1 : x;
			int yy = flipY ? height - y - 1 : y;
			copy.setPixel(x, y, getPixel(xx, yy));
		}
	}
	return copy;
}

Image Image::move(int xOffset, int yOffset) const {
	Image img(width, height);
	for (int y = 0; y < height && y + yOffset < height; y++) {
		for (int x = 0; x < width && x + xOffset < width; x++) {
			img.setPixel(x + xOffset, y + yOffset, getPixel(x, y));
		}
	}
	return img;
}

inline uint8_t interpolate(uint8_t a, uint8_t b, uint8_t c, uint8_t d, double w,
        double h) {
	double aa = (double) a / 255.0;
	double bb = (double) b / 255.0;
	double cc = (double) c / 255.0;
	double dd = (double) d / 255.0;
	double result = aa * (1 - w) * (1 - h) + bb * w * (1 - h) + cc * h * (1 - w)
	        + dd * (w * h);
	return result * 255.0;
}

void Image::resizeInterpolated(int new_width, int new_height, Image& dest) const {
	if (new_width == width && new_height == height) {
		dest = *this;
		return;
	}
	dest.setSize(new_width, new_height);

	double x_ratio = (double) width / new_width;
	double y_ratio = (double) height / new_height;
	if(width < new_width)
		x_ratio = (double) (width - 1) / new_width;
	if(height < new_height)
		y_ratio = (double) (height - 1) / new_height;

	for (int x = 0; x < new_width; x++) {
		for (int y = 0; y < new_height; y++) {
			int sx = x_ratio * x;
			int sy = y_ratio * y;
			double x_diff = (x_ratio * x) - sx;
			double y_diff = (y_ratio * y) - sy;
			uint32_t a = getPixel(sx, sy);
			uint32_t b = getPixel(sx + 1, sy);
			uint32_t c = getPixel(sx, sy + 1);
			uint32_t d = getPixel(sx + 1, sy + 1);

			uint8_t red = interpolate(RED(a), RED(b), RED(c), RED(d), x_diff, y_diff);
			uint8_t green = interpolate(GREEN(a), GREEN(b), GREEN(c), GREEN(d), x_diff,
			        y_diff);
			uint8_t blue = interpolate(BLUE(a), BLUE(b), BLUE(c), BLUE(d), x_diff,
			        y_diff);
			uint8_t alpha = interpolate(ALPHA(a), ALPHA(b), ALPHA(c), ALPHA(d), x_diff,
			        y_diff);

			dest.setPixel(x, y, rgba(red, green, blue, alpha));
		}
	}
}

void Image::resizeSimple(int new_width, int new_height, Image& dest) const {
	if (new_width == width && new_height == height) {
		dest = *this;
		return;
	}
	dest.setSize(new_width, new_height);

	for (int x = 0; x < new_width; x++) {
		for (int y = 0; y < new_height; y++) {
			dest.setPixel(x, y,
			        getPixel(x / ((double) new_width / width),
			                y / ((double) new_height / height)));
		}
	}
}

void Image::resizeHalf(Image& dest) const {
	dest.setSize(width / 2, height / 2);

	for (int x = 0; x < width - 1; x += 2) {
		for (int y = 0; y < height - 1; y += 2) {
			uint32_t p1 = (data[y * width + x] >> 2) & 0x3f3f3f3f;
			uint32_t p2 = (data[y * width + x + 1] >> 2) & 0x3f3f3f3f;
			uint32_t p3 = (data[(y + 1) * width + x] >> 2) & 0x3f3f3f3f;
			uint32_t p4 = (data[(y + 1) * width + x + 1] >> 2) & 0x3f3f3f3f;
			dest.data[(y / 2) * dest.width + (x / 2)] = p1 + p2 + p3 + p4;
		}
	}
}

bool Image::readPNG(const std::string& filename) {
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
	if (png_get_bit_depth(png, info) != 8 || (color & PNG_COLOR_TYPE_RGB) == 0) {
		return false;
	}

	setSize(png_get_image_width(png, info), png_get_image_height(png, info));

	png_set_interlace_handling(png);
	// add alpha channel, if needed
	if ((color & PNG_COLOR_MASK_ALPHA) == 0) {
		png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);
	}
	png_read_update_info(png, info);

	png_bytep* rows = new png_bytep[height];
	uint32_t* p = &data[0];
	for (int32_t i = 0; i < height; i++, p += width)
		rows[i] = (png_bytep) p;

	if (mapcrafter::isBigEndian()) {
		png_set_bgr(png);
		png_set_swap_alpha(png);
	}
	png_read_image(png, rows);
	png_read_end(png, NULL);
	png_destroy_read_struct(&png, &info, NULL);
	delete[] rows;

	return true;
}

bool Image::writePNG(const std::string& filename) const {
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

	png_bytep* rows = new png_bytep[height];
	const uint32_t* p = &data[0];
	for (int32_t i = 0; i < height; i++, p += width)
		rows[i] = (png_bytep) p;

	png_set_rows(png, info, rows);

	if (mapcrafter::isBigEndian())
		png_write_png(png, info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ALPHA, NULL);
	else
		png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

	file.close();
	delete[] rows;
	png_destroy_write_struct(&png, &info);
	return true;
}

}
}
