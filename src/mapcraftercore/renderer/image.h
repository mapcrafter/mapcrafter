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

#ifndef IMAGE_H_
#define IMAGE_H_

#include <png.h>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>

namespace mapcrafter {
namespace renderer {

typedef uint32_t RGBAPixel;

RGBAPixel rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
uint8_t rgba_red(RGBAPixel value);
uint8_t rgba_green(RGBAPixel value);
uint8_t rgba_blue(RGBAPixel value);
uint8_t rgba_alpha(RGBAPixel value);

RGBAPixel rgba_add_clamp(RGBAPixel value, int r, int g, int b);
RGBAPixel rgba_multiply(RGBAPixel value, double r, double g, double b, double a = 1);
RGBAPixel rgba_multiply(RGBAPixel value, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

void blend(RGBAPixel& dest, const RGBAPixel& source);

void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length);
void pngWriteData(png_structp pngPtr, png_bytep data, png_size_t length);

template <typename Pixel>
class Image {
public:
	Image(int width = 0, int height = 0);
	~Image();

	int getWidth() const;
	int getHeight() const;

	Pixel getPixel(int x, int y) const;
	void setPixel(int x, int y, Pixel pixel);

	const Pixel& pixel(int x, int y) const;
	Pixel& pixel(int x, int y);

	void setSize(int width, int height);

protected:
	int width;
	int height;

	std::vector<Pixel> data;
};

const int ROTATE_90 = 1;
const int ROTATE_180 = 2;
const int ROTATE_270 = 3;

class Palette {
public:
	virtual ~Palette() {}

	virtual RGBAPixel getNearestColor(const RGBAPixel& color) const = 0;
};

class TestPalette : public Palette {
public:
	TestPalette() {};
	TestPalette(const std::vector<RGBAPixel>& colors) : colors(colors) {};
	virtual ~TestPalette() {}

	virtual RGBAPixel getNearestColor(const RGBAPixel& color) const {
		RGBAPixel best_color;
		int min_distance = -1;
		for (auto color_it = colors.begin(); color_it != colors.end(); ++color_it) {
			int distance = getColorDistance(color, *color_it);
			if (min_distance == -1 || distance < min_distance) {
				best_color = *color_it;
				min_distance = distance;
			}
		}
		return best_color;
	}

protected:
	std::vector<RGBAPixel> colors;

	int getColorDistance(RGBAPixel color1, RGBAPixel color2) const {
		return pow(rgba_red(color1) - rgba_red(color2), 2) + pow(rgba_green(color1) - rgba_green(color2), 2) + pow(rgba_blue(color1) - rgba_blue(color2), 2);
	}
};

// TODO better documentation...
class RGBAImage : public Image<RGBAPixel> {
public:
	RGBAImage(int width = 0, int height = 0);
	~RGBAImage();

	/**
	 * Blits one image to another one. Just copies the pixels over without any processing.
	 */
	void simpleBlit(const RGBAImage& image, int x, int y);

	/**
	 * Blits one image to another one. Just copies the pixels over, but skips completely
	 * transparent pixels (alpha(pixel) == 0).
	 */
	void simpleAlphaBlit(const RGBAImage& image, int x, int y);

	/**
	 * Blits one image to another one. Also Alphablends transparent pixels of the source
	 * image with the pixels of the destination image.
	 */
	void alphaBlit(const RGBAImage& image, int x, int y);
	void blendPixel(RGBAPixel color, int x, int y);

	void fill(RGBAPixel color, int x1, int y1, int w, int h);
	void clear();

	RGBAImage clip(int x, int y, int width, int height) const;
	RGBAImage colorize(double r, double g, double b, double a = 1) const;
	RGBAImage colorize(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) const;
	RGBAImage rotate(int rotation) const;
	RGBAImage flip(bool flip_x, bool flip_y) const;
	RGBAImage move(int x_off, int y_off) const;

	void resizeInterpolated(RGBAImage& dest, int new_width, int new_height) const;
	void resizeSimple(RGBAImage& dest, int new_width, int new_height) const;
	// automatically chooses an image resize interpolation
	void resizeAuto(RGBAImage& dest, int new_width, int new_height) const;
	void resizeHalf(RGBAImage& dest) const;

	/**
	 * Applies a simple blur filter to the image. Uses the specified radius for the
	 * (quadratic) blur effect.
	 */
	void blur(RGBAImage& dest, int radius) const;

	void quantize(const Palette& palette, bool dither) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				RGBAPixel old_color = pixel(x, y);
				RGBAPixel new_color = palette.getNearestColor(old_color);
				pixel(x, y) = new_color;

				if (dither) {
					int error_r = rgba_red(old_color) - rgba_red(new_color);
					int error_g = rgba_green(old_color) - rgba_green(new_color);
					int error_b = rgba_blue(old_color) - rgba_blue(new_color);

					setPixel(x+1, y, rgba_add_clamp(getPixel(x+1, y), error_r * 7/16, error_g * 7/16, error_b * 7/16));
					setPixel(x-1, y+1, rgba_add_clamp(getPixel(x-1, y+1), error_r * 3/16, error_g * 3/16, error_b * 3/16));
					setPixel(x, y+1, rgba_add_clamp(getPixel(x, y+1), error_r * 5/16, error_g * 5/16, error_b * 5/16));
					setPixel(x+1, y+1, rgba_add_clamp(getPixel(x+1, y+1), error_r / 16, error_g / 16, error_b / 16));
				}
			}
		}
	}

	bool readPNG(const std::string& filename);
	bool writePNG(const std::string& filename) const;

	bool readJPEG(const std::string& filename);
	bool writeJPEG(const std::string& filename, int quality,
			RGBAPixel background = rgba(255, 255, 255, 255)) const;
};

template <typename Pixel>
Image<Pixel>::Image(int width, int height)
	:width(width), height(height) {
	data.resize(width * height);
}

template <typename Pixel>
Image<Pixel>::~Image() {
}

template <typename Pixel>
int Image<Pixel>::getWidth() const {
	return width;
}

template <typename Pixel>
int Image<Pixel>::getHeight() const {
	return height;
}

template <typename Pixel>
Pixel Image<Pixel>::getPixel(int x, int y) const {
	if (x >= width || x < 0 || y >= height || y < 0)
		return 0;
	return data[y * width + x];
}

template <typename Pixel>
void Image<Pixel>::setPixel(int x, int y, Pixel pixel) {
	if (x >= width || x < 0 || y >= height || y < 0)
		return;
	data[y * width + x] = pixel;
}

template <typename Pixel>
const Pixel& Image<Pixel>::pixel(int x, int y) const {
	return data[y * width + x];
}

template <typename Pixel>
Pixel& Image<Pixel>::pixel(int x, int y) {
	return data[y * width + x];
}

template <typename Pixel>
void Image<Pixel>::setSize(int width, int height) {
	this->width = width;
	this->height = height;
	data.resize(width * height);
}

}
}

#endif /* IMAGE_H_ */
