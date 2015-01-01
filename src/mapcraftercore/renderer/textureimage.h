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

#ifndef TEXTUREIMAGE_H_
#define TEXTUREIMAGE_H_

#include "image.h"

namespace mapcrafter {
namespace renderer {

/**
 * A single Minecraft block texture image.
 */
class TextureImage : public RGBAImage {
public:
	TextureImage();
	TextureImage(const std::string& name);
	~TextureImage();

	/**
	 * Tries to load the texture from the given path.
	 */
	bool load(const std::string& path, int size);

	/**
	 * Returns the name of the texture.
	 */
	const std::string& getName() const;

	/**
	 * Returns the original texture image.
	 */
	const RGBAImage& getOriginal() const;

	/**
	 * Returns whether the texture is animated.
	 */
	bool isAnimated() const;

	/**
	 * If the texture is animated, returns a (resized to texture size) frame of it.
	 */
	RGBAImage getFrame(int frame) const;

private:
	std::string name;

	// the original texture image
	// also original resized texture image (used for animated textures)
	RGBAImage original, original_resized;
};

} /* namespace render */
} /* namespace mapcrafter */

#endif /* TEXTUREIMAGE_H_ */
