#include "dither.h"

#include "palette.h"
#include "../image.h"

namespace mapcrafter {
namespace renderer {

void imageDither(RGBAImage& image, const Palette& palette, bool dither) {
	for (int y = 0; y < image.getHeight(); y++) {
		for (int x = 0; x < image.getWidth(); x++) {
			RGBAPixel old_color = image.pixel(x, y);
			RGBAPixel new_color = palette.getNearestColor(old_color);
			image.pixel(x, y) = new_color;

			if (dither) {
				int error_r = rgba_red(old_color) - rgba_red(new_color);
				int error_g = rgba_green(old_color) - rgba_green(new_color);
				int error_b = rgba_blue(old_color) - rgba_blue(new_color);

				image.setPixel(x+1, y, rgba_add_clamp(image.getPixel(x+1, y), error_r * 7/16, error_g * 7/16, error_b * 7/16));
				image.setPixel(x-1, y+1, rgba_add_clamp(image.getPixel(x-1, y+1), error_r * 3/16, error_g * 3/16, error_b * 3/16));
				image.setPixel(x, y+1, rgba_add_clamp(image.getPixel(x, y+1), error_r * 5/16, error_g * 5/16, error_b * 5/16));
				image.setPixel(x+1, y+1, rgba_add_clamp(image.getPixel(x+1, y+1), error_r / 16, error_g / 16, error_b / 16));
			}
		}
	}
}

}
}
