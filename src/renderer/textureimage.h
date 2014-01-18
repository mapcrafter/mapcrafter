/*
 * textureimage.h
 *
 *  Created on: 12.10.2013
 *      Author: moritz
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
