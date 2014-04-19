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

#include "textureimage.h"

#include "../util.h"

namespace mapcrafter {
namespace renderer {

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
	if (!tmp.readPNG(path + "/" + name + ".png")) {
		// make sure the texture image does not have zero dimension
		// even if the texture does not exist
		this->setSize(size, size);
		original = *this;
		return false;
	}

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
