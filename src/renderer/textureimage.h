/*
 * Copyright 2012-2014 Moritz Hilscher
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

#ifndef TEXTUREIMAGE_H_
#define TEXTUREIMAGE_H_

#include "image.h"

namespace mapcrafter {
namespace renderer {

/**
 * A single Minecraft block texture image.
 */
class TextureImage : public Image {
private:
	std::string name;
public:
	// the original (not resized) texture image
	Image original;

	TextureImage();
	TextureImage(const std::string& name);
	~TextureImage();

	bool load(const std::string& path, int size);

	const std::string& getName() const;
};

} /* namespace render */
} /* namespace mapcrafter */

#endif /* TEXTUREIMAGE_H_ */
