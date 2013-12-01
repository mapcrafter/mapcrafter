/*
 * textureimage.cpp
 *
 *  Created on: 12.10.2013
 *      Author: moritz
 */

#include "textureimage.h"

#include "../util.h"

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

	// resize some textures with the nearest neighbor interpolation:
	// - transparent leaves
	// - redstone
	// because smooth interpolation causes here half-transparent pixel which are not
	// good for performance and makes redstone looking not very good and identifiable,
	// instead of that the nearest neighbor interpolation preserves the pixelated
	// style of the textures and prevents fuzziness when resizing
	if ((util::startswith(name, "leaves") && !util::endswith(name, "opaque"))
		|| util::startswith(name, "redstone_dust"))
		tmp.resizeSimple(size, size, *this);
	else
		tmp.resizeAuto(size, size, *this);
	return true;
}

const std::string& TextureImage::getName() const {
	return name;
}

} /* namespace render */
} /* namespace mapcrafter */
