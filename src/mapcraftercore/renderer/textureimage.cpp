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

bool TextureImage::load(const std::string& path, int size) {
	// at first try to load the texture file
	if (!original.readPNG(path + "/" + name + ".png")) {
		// make sure the texture image does not have zero dimension
		// even if the texture does not exist / is broken
		this->setSize(size, size);
		original = original_resized = *this;
		return false;
	}

	// check if this is an animated texture, calculate how many frames it has
	// also make sure there are exactly n frames, so height mod width = 0
	if ((original.getHeight() % original.getWidth()) != 0) {
		LOG(WARNING) << "Texture '" << name << "' has odd size: " << original.getWidth()
				<< "x" << original.getHeight();
	}
	int frames = original.getHeight() / original.getWidth();

	// now resize the texture image
	// resize some textures with the nearest neighbor interpolation:
	// - transparent leaves
	// - redstone
	// because smooth interpolation causes here half-transparent pixel which are not
	// good for performance and makes redstone looking not very good and identifiable,
	// instead of that the nearest neighbor interpolation preserves the pixelated
	// style of the textures and prevents fuzziness when resizing
	if ((util::startswith(name, "leaves") && !util::endswith(name, "opaque"))
		|| util::startswith(name, "redstone_dust"))
		original.resizeSimple(size, size * frames, original_resized);
	else
		original.resizeAuto(size, size * frames, original_resized);

	// assign actual texture to parent RGBAImage object
	// uses first frame if this is an animated texture
	this->setSize(size, size);
	this->simpleblit(getFrame(0), 0, 0);
	return true;
}

const std::string& TextureImage::getName() const {
	return name;
}

const RGBAImage& TextureImage::getOriginal() const {
	return original;
}

bool TextureImage::isAnimated() const {
	return original.getWidth() < original.getHeight();
}

RGBAImage TextureImage::getFrame(int frame) const {
	int width = original_resized.getWidth();
	// check if texture has this frame, return empty texture if not
	if ((frame+1) * width > original_resized.getHeight()) {
		LOG(WARNING) << "Texture '" << name << "' does not have frame " << frame << ".";
		return RGBAImage(width, width);
	}
	return original_resized.clip(0, width * frame, width, width);
}

} /* namespace render */
} /* namespace mapcrafter */
