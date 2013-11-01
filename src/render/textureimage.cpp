/*
 * textureimage.cpp
 *
 *  Created on: 12.10.2013
 *      Author: moritz
 */

#include "textureimage.h"

namespace mapcrafter {
namespace render {

TextureImage::TextureImage() {
}

TextureImage::TextureImage(const std::string& name)
		: name(name) {
}

TextureImage::~TextureImage() {
}

/**
 * Loads a single block texture image.
 */
bool TextureImage::load(const std::string& path, int size) {
	Image tmp;
	if (!tmp.readPNG(path + "/" + name + ".png"))
		return false;

	original = tmp;

	// check if this is an animated texture
	// -> use only the first frame
	if (tmp.getWidth() < tmp.getHeight())
		tmp = tmp.clip(0, 0, tmp.getWidth(), tmp.getWidth());

	// we resize the transparent version of the leaves without interpolation,
	// because this can cause half-transparent pixels, which aren't good for performance
	// redstone also without interpolation
	if (name == "leaves"
			|| name == "leaves_jungle"
			|| name == "leaves_spruce"
			|| name == "redstoneDust_cross"
			|| name == "redstoneDust_line")
		tmp.resizeSimple(size, size, *this);
	else
		tmp.resizeInterpolated(size, size, *this);
	return true;
}

const std::string& TextureImage::getName() const {
	return name;
}

} /* namespace render */
} /* namespace mapcrafter */
