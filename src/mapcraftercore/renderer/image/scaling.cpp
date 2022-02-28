#include "scaling.h"

#include "../image.h"

namespace mapcrafter {
namespace renderer {

void imageResizeSimple(const RGBAImage& image, RGBAImage& dest, int width, int height) {
	dest.setSize(width, height);

	double x_ratio = (double) width / image.getWidth();
	double y_ratio = (double) height / image.getHeight();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			dest.setPixel(x, y, image.getPixel(x / x_ratio, y / y_ratio));
		}
	}
}

namespace {

uint8_t interpolate(uint8_t a, uint8_t b, uint8_t c, uint8_t d, double w, double h) {
	double aa = (double) a / 255.0;
	double bb = (double) b / 255.0;
	double cc = (double) c / 255.0;
	double dd = (double) d / 255.0;
	double result = aa * (1 - w) * (1 - h) + bb * w * (1 - h) + cc * h * (1 - w) + dd * (w * h);
	return result * 255;
}

}

void imageResizeBilinear(const RGBAImage& image, RGBAImage& dest, int width, int height) {
	dest.setSize(width, height);

	double x_ratio = (double) image.getWidth() / width;
	double y_ratio = (double) image.getWidth() / height;
	if(image.getWidth() < width)
		x_ratio = (double) (image.getWidth() - 1) / width;
	if(image.getHeight() < height)
		y_ratio = (double) (image.getWidth() - 1) / height;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int sx = x_ratio * x;
			int sy = y_ratio * y;
			double x_diff = (x_ratio * x) - sx;
			double y_diff = (y_ratio * y) - sy;
			RGBAPixel a = image.getPixel(sx, sy);
			RGBAPixel b = image.getPixel(sx + 1, sy);
			RGBAPixel c = image.getPixel(sx, sy + 1);
			RGBAPixel d = image.getPixel(sx + 1, sy + 1);

			uint8_t red = interpolate(rgba_red(a), rgba_red(b), rgba_red(c), rgba_red(d), x_diff, y_diff);
			uint8_t green = interpolate(rgba_green(a), rgba_green(b), rgba_green(c), rgba_green(d), x_diff, y_diff);
			uint8_t blue = interpolate(rgba_blue(a), rgba_blue(b), rgba_blue(c), rgba_blue(d), x_diff, y_diff);
			uint8_t alpha = interpolate(rgba_alpha(a), rgba_alpha(b), rgba_alpha(c), rgba_alpha(d), x_diff, y_diff);

			// make sure that that no transparency (aka alpha=254) sneaks into images
			// caused by weird interpolation bugses, otherwise shit hits the fan
			// when all blocks contain just a bit of transparency
			
			// do this by just clamping alpha >= threshold to 255, 245 or 255 won't be
			// a big visual difference
			if (alpha >= 245)
				alpha = 255;
		
			dest.setPixel(x, y, rgba(red, green, blue, alpha));
		}
	}
}

void imageResizeHalf(const RGBAImage& image, RGBAImage& dest) {
	int width = image.getWidth();
	int height = image.getHeight();
	dest.setSize(width / 2, height / 2);

	for (int x = 0; x < width - 1; x += 2) {
		for (int y = 0; y < height - 1; y += 2) {
			RGBAPixel p1 = image.pixel(x, y);
			RGBAPixel p2 = image.pixel(x + 1, y);
			RGBAPixel p3 = image.pixel(x, y + 1);
			RGBAPixel p4 = image.pixel(x + 1, y + 1);
			RGBAPixel highBits = ((p1 >> 2) & 0x3f3f3f3f) + ((p2 >> 2) & 0x3f3f3f3f) + ((p3 >> 2) & 0x3f3f3f3f) + ((p4 >> 2) & 0x3f3f3f3f);
			RGBAPixel lowBits = (((p1 & 0x03030303) + (p2 & 0x03030303) + (p3 & 0x03030303) + (p4 & 0x03030303)) >> 2) & 0x03030303;
			dest.pixel(x >> 1, y >> 1) = highBits + lowBits;
		}
	}
}

}
}

