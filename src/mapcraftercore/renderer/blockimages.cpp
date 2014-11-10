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

#include "blockimages.h"

#include "../util.h"

namespace mapcrafter {
namespace renderer {

BlockImageTextureResources::BlockImageTextureResources()
	: texture_size(12) {
}

BlockImageTextureResources::~BlockImageTextureResources() {
}

/*
void BlockImageTextureResources::setSettings(int texture_size, int rotation, bool render_unknown_blocks,
        bool render_leaves_transparent, const std::string& rendermode) {
	this->texture_size = texture_size;
	this->rotation = rotation;
	this->render_unknown_blocks = render_unknown_blocks;
	this->render_leaves_transparent = render_leaves_transparent;

	if (rendermode == "daylight" || rendermode == "nightlight") {
		dleft = 0.95;
		dright = 0.8;
	}
}
*/

void BlockImageTextureResources::setTextureSize(int texture_size) {
	this->texture_size = texture_size;
}

namespace {

/**
 * This function converts the chest image to usable chest textures and stores them
 * in the textures array.
 */
bool loadChestTextures(const std::string& filename, RGBAImage* textures, int texture_size) {
	RGBAImage image;
	if (!image.readPNG(filename)) {
		LOG(ERROR) << "Unable to read '" << filename << "'.";
		return false;
	}

	if (image.getWidth() != image.getHeight()) {
		LOG(ERROR) << "Chest texture has invalid size (width:height must be 1:1): '" << filename << "'.";
		return false;
	}
	// if the image is 64px wide, the chest images are 14x14
	int ratio = image.getHeight() / 64;
	int size = ratio * 14;

	RGBAImage front = image.clip(size, 29 * ratio, size, size);
	front.alphablit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 6 * ratio, 3 * ratio);
	RGBAImage side = image.clip(0, 29 * ratio, size, size);
	side.alphablit(image.clip(0, size, size, 4 * ratio), 0, 0);
	RGBAImage top = image.clip(size, 0, size, size);

	// resize the chest images to texture size
	front.resizeAuto(texture_size, texture_size, textures[BlockImageTextureResources::CHEST_FRONT]);
	side.resizeAuto(texture_size, texture_size, textures[BlockImageTextureResources::CHEST_SIDE]);
	top.resizeAuto(texture_size, texture_size, textures[BlockImageTextureResources::CHEST_TOP]);

	return true;
}

/**
 * This function converts the large chest image to usable chest textures and stores them
 * in the textures array.
 */
bool loadDoubleChestTextures(const std::string& filename, RGBAImage* textures, int texture_size) {
	RGBAImage image;
	if (!image.readPNG(filename)) {
		LOG(ERROR) << "Unable to read '" << filename << "'.";
		return false;
	}

	if (image.getWidth() != image.getHeight() * 2) {
		LOG(ERROR) << "Chest texture has invalid size (width:height must be 1:2): '" << filename << "'.";
		return false;
	}
	int ratio = image.getHeight() / 64;
	int size = ratio * 14;

	// note here that a whole chest is 30*ratio pixels wide, but our
	// chest textures are only 14x14 * ratio pixels, so we need to omit two rows in the middle
	// => the second image starts not at x*size, it starts at x*size+2*ratio
	RGBAImage front_left = image.clip(size, 29 * ratio, size, size);
	front_left.alphablit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front_left.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 13 * ratio,
	        3 * ratio);
	RGBAImage front_right = image.clip(2 * size + 2 * ratio, 29 * ratio, size, size);
	front_right.alphablit(image.clip(2 * size + 2 * ratio, size, size, 4 * ratio), 0, 0);
	front_right.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), -ratio,
	        3 * ratio);

	RGBAImage side = image.clip(0, 29 * ratio, size, size);
	side.alphablit(image.clip(0, size, size, 4 * ratio), 0, 0);

	RGBAImage top_left = image.clip(size, 0, size, size);
	RGBAImage top_right = image.clip(2 * size + 2 * ratio, 0, size, size);

	RGBAImage back_left = image.clip(4 * size + 2, 29 * ratio, size, size);
	back_left.alphablit(image.clip(4 * size + 2, size, size, 4 * ratio), 0, 0);
	RGBAImage back_right = image.clip(5 * size + 4, 29 * ratio, size, size);
	back_right.alphablit(image.clip(5 * size + 4, size, size, 4 * ratio), 0, 0);

	// resize the chest images to texture size
	front_left.resizeAuto(texture_size, texture_size,
	        textures[BlockImageTextureResources::LARGECHEST_FRONT_LEFT]);
	front_right.resizeAuto(texture_size, texture_size,
	        textures[BlockImageTextureResources::LARGECHEST_FRONT_RIGHT]);
	side.resizeAuto(texture_size, texture_size, textures[BlockImageTextureResources::LARGECHEST_SIDE]);
	top_left.resizeAuto(texture_size, texture_size,
	        textures[BlockImageTextureResources::LARGECHEST_TOP_LEFT]);
	top_right.resizeAuto(texture_size, texture_size,
	        textures[BlockImageTextureResources::LARGECHEST_TOP_RIGHT]);
	back_left.resizeAuto(texture_size, texture_size,
	        textures[BlockImageTextureResources::LARGECHEST_BACK_LEFT]);
	back_right.resizeAuto(texture_size, texture_size,
	        textures[BlockImageTextureResources::LARGECHEST_BACK_RIGHT]);

	return true;
}

}

bool BlockImageTextureResources::loadChests(const std::string& normal, const std::string& normal_double,
		const std::string& ender,
		const std::string& trapped, const std::string& trapped_double) {
	if (!loadChestTextures(normal, chest_normal, texture_size)
			|| !loadDoubleChestTextures(normal_double, chest_normal_double, texture_size)
			|| !loadChestTextures(ender, chest_ender, texture_size)
			|| !loadChestTextures(trapped, chest_trapped, texture_size)
			|| !loadDoubleChestTextures(trapped_double, chest_trapped_double, texture_size))
		return false;
	return true;
}

bool BlockImageTextureResources::loadColors(const std::string& foliagecolor,
		const std::string& grasscolor) {
	bool ok = true;
	if (!foliagecolors.readPNG(foliagecolor)) {
		LOG(ERROR) << "Unable to read '" << foliagecolor << "'.";
		ok = false;
	}
	if (!grasscolors.readPNG(grasscolor)) {
		LOG(ERROR) << "Unable to read '" << grasscolor << "'.";
		ok = false;
	}
	return ok;
}

bool BlockImageTextureResources::loadOther(const std::string& endportal) {
	RGBAImage endportal_img;
	if(!endportal_img.readPNG(endportal)) {
		LOG(ERROR) << "Unable to read '" << endportal << "'.";
		return false;
	}
	endportal_img.resizeAuto(texture_size, texture_size, endportal_texture);
	return true;
}

bool BlockImageTextureResources::loadBlocks(const std::string& block_dir) {
	if (!textures.load(block_dir, texture_size))
		return false;

	empty_texture.setSize(texture_size, texture_size);

	/*
	unknown_block.setSize(texture_size, texture_size);
	if (render_unknown_blocks)
		unknown_block.fill(rgba(255, 0, 0, 255), 0, 0, texture_size, texture_size);

	loadBlocks();
	testWaterTransparency();
	createBiomeBlocks();
	*/
	return true;
}

bool BlockImageTextureResources::loadAll(const std::string& textures_dir) {
	bool ok = true;
	if (!loadChests(textures_dir + "/entity/chest/normal.png",
			textures_dir + "/entity/chest/normal_double.png",
			textures_dir + "/entity/chest/ender.png",
			textures_dir + "/entity/chest/trapped.png",
			textures_dir + "/entity/chest/trapped_double.png"))
		ok = false;
	if (!loadColors(textures_dir + "/colormap/foliage.png",
			textures_dir + "/colormap/grass.png"))
		ok = false;
	if (!loadOther(textures_dir + "/endportal.png"))
		ok = false;
	if (!loadBlocks(textures_dir + "/blocks"))
		ok = false;
	if (!ok) {
		LOG(ERROR) << "Invalid texture directory '" << textures_dir << "'. See previous log messages.";
		return false;
	}
	return true;
}

const BlockTextures& BlockImageTextureResources::getBlockTextures() const {
	return textures;
}

const RGBAImage& BlockImageTextureResources::getEndportalTexture() const {
	return endportal_texture;
}

const RGBAImage* BlockImageTextureResources::getNormalChest() const {
	return chest_normal;
}

const RGBAImage* BlockImageTextureResources::getNormalDoubleChest() const {
	return chest_normal_double;
}

const RGBAImage* BlockImageTextureResources::getEnderChest() const {
	return chest_ender;
}

const RGBAImage* BlockImageTextureResources::getTrappedChest() const {
	return chest_trapped;
}

const RGBAImage* BlockImageTextureResources::getTrappedDoubleChest() const {
	return chest_trapped_double;
}

const RGBAImage& BlockImageTextureResources::getFoliageColors() const {
	return foliagecolors;
}

const RGBAImage& BlockImageTextureResources::getGrassColors() const {
	return grasscolors;
}

BlockImages::~BlockImages() {
}

}
}
