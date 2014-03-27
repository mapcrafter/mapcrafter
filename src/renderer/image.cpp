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

#include "image.h"

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

void RGBAImage::simpleblit(const RGBAImage& image, int x, int y) {
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

void RGBAImage::alphablit(const RGBAImage& image, int x, int y) {
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

void RGBAImage::resizeInterpolated(int new_width, int new_height, RGBAImage& dest) const {
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
			RGBAPixel a = getPixel(sx, sy);
			RGBAPixel b = getPixel(sx + 1, sy);
			RGBAPixel c = getPixel(sx, sy + 1);
			RGBAPixel d = getPixel(sx + 1, sy + 1);

			uint8_t red = interpolate(rgba_red(a), rgba_red(b), rgba_red(c), rgba_red(d),
					x_diff, y_diff);
			uint8_t green = interpolate(rgba_green(a), rgba_green(b), rgba_green(c), rgba_green(d),
					x_diff, y_diff);
			uint8_t blue = interpolate(rgba_blue(a), rgba_blue(b), rgba_blue(c), rgba_blue(d),
					x_diff, y_diff);
			uint8_t alpha = interpolate(rgba_alpha(a), rgba_alpha(b), rgba_alpha(c), rgba_alpha(d),
					x_diff, y_diff);

			dest.setPixel(x, y, rgba(red, green, blue, alpha));
		}
	}
}

void RGBAImage::resizeSimple(int new_width, int new_height, RGBAImage& dest) const {
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

void RGBAImage::resizeAuto(int new_width, int new_height, RGBAImage& dest) const {
	// for increasing an image resolution the nearest neighbor interpolation is the best
	// for Minecraft textures because it preserves the pixelated style of the textures
	// and prevents the textures becoming blurry
	if (width < new_width)
		resizeSimple(new_width, new_height, dest);
	else
		resizeInterpolated(new_width, new_height, dest);
}

void RGBAImage::resizeHalf(RGBAImage& dest) const {
	dest.setSize(width / 2, height / 2);

	for (int x = 0; x < width - 1; x += 2) {
		for (int y = 0; y < height - 1; y += 2) {
			RGBAPixel p1 = (data[y * width + x] >> 2) & 0x3f3f3f3f;
			RGBAPixel p2 = (data[y * width + x + 1] >> 2) & 0x3f3f3f3f;
			RGBAPixel p3 = (data[(y + 1) * width + x] >> 2) & 0x3f3f3f3f;
			RGBAPixel p4 = (data[(y + 1) * width + x + 1] >> 2) & 0x3f3f3f3f;
			dest.data[(y / 2) * dest.width + (x / 2)] = p1 + p2 + p3 + p4;
		}
	}
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

	if (mapcrafter::util::isBigEndian()) {
		png_set_bgr(png);
		png_set_swap_alpha(png);
	}
	png_read_image(png, rows);
	png_read_end(png, NULL);
	png_destroy_read_struct(&png, &info, NULL);
	delete[] rows;

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

	png_bytep* rows = new png_bytep[height];
	const uint32_t* p = &data[0];
	for (int32_t i = 0; i < height; i++, p += width)
		rows[i] = (png_bytep) p;

	png_set_rows(png, info, rows);

	if (mapcrafter::util::isBigEndian())
		png_write_png(png, info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ALPHA, NULL);
	else
		png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

	file.close();
	delete[] rows;
	png_destroy_write_struct(&png, &info);
	return true;
}

bool RGBAImage::readJPEG(const std::string& filename) {
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

	int y = 0;
	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		for (int x = 0; x < width; x++) {
			RGBAPixel color = pixel(x, y);
			// jpeg does not support transparency, add background color
			// if this pixel has transparency
			if (rgba_alpha(color) != 255)
				blend(color, background);

			line_buffer[3 * x] = rgba_red(color);
			line_buffer[3 * x + 1] = rgba_green(color);
			line_buffer[3 * x + 2] = rgba_blue(color);
		}
		(void) jpeg_write_scanlines(&cinfo, &scanlineData, 1);
		y++;
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
