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

#include "blockimages.h"

#include "biomes.h"
#include "../util.h"

#include <map>
#include <vector>

namespace mapcrafter {
namespace renderer {

BlockImageTextureResources::BlockImageTextureResources()
	: texture_size(12), texture_blur(0) {
}

BlockImageTextureResources::~BlockImageTextureResources() {
}

void BlockImageTextureResources::setTextureSize(int texture_size, int texture_blur) {
	this->texture_size = texture_size;
	this->texture_blur = texture_blur;
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
	front.alphaBlit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front.alphaBlit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 6 * ratio, 3 * ratio);
	RGBAImage side = image.clip(0, 29 * ratio, size, size);
	side.alphaBlit(image.clip(0, size, size, 4 * ratio), 0, 0);
	RGBAImage top = image.clip(size, 0, size, size);

	// resize the chest images to texture size
	front.resizeAuto(textures[BlockImageTextureResources::CHEST_FRONT], texture_size, texture_size);
	side.resizeAuto(textures[BlockImageTextureResources::CHEST_SIDE], texture_size, texture_size);
	top.resizeAuto(textures[BlockImageTextureResources::CHEST_TOP], texture_size, texture_size);

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
	front_left.alphaBlit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front_left.alphaBlit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 13 * ratio,
	        3 * ratio);
	RGBAImage front_right = image.clip(2 * size + 2 * ratio, 29 * ratio, size, size);
	front_right.alphaBlit(image.clip(2 * size + 2 * ratio, size, size, 4 * ratio), 0, 0);
	front_right.alphaBlit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), -ratio,
	        3 * ratio);

	RGBAImage side = image.clip(0, 29 * ratio, size, size);
	side.alphaBlit(image.clip(0, size, size, 4 * ratio), 0, 0);

	RGBAImage top_left = image.clip(size, 0, size, size);
	RGBAImage top_right = image.clip(2 * size + 2 * ratio, 0, size, size);

	RGBAImage back_left = image.clip(4 * size + 2, 29 * ratio, size, size);
	back_left.alphaBlit(image.clip(4 * size + 2, size, size, 4 * ratio), 0, 0);
	RGBAImage back_right = image.clip(5 * size + 4, 29 * ratio, size, size);
	back_right.alphaBlit(image.clip(5 * size + 4, size, size, 4 * ratio), 0, 0);

	// resize the chest images to texture size
	front_left.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_FRONT_LEFT],
			texture_size, texture_size);
	front_right.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_FRONT_RIGHT],
			texture_size, texture_size);
	side.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_SIDE],
			texture_size, texture_size);
	top_left.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_TOP_LEFT],
			texture_size, texture_size);
	top_right.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_TOP_RIGHT],
			texture_size, texture_size);
	back_left.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_BACK_LEFT],
			texture_size, texture_size);
	back_right.resizeAuto(textures[BlockImageTextureResources::LARGECHEST_BACK_RIGHT],
			texture_size, texture_size);

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
	endportal_img.resizeAuto(endportal_texture, texture_size, texture_size);
	return true;
}

bool BlockImageTextureResources::loadBlocks(const std::string& block_dir) {
	if (!textures.load(block_dir, texture_size, texture_blur))
		return false;
	empty_texture.setSize(texture_size, texture_size);
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

int BlockImageTextureResources::getTextureSize() const {
	return texture_size;
}

int BlockImageTextureResources::getTextureBlur() const {
	return texture_blur;
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

AbstractBlockImages::AbstractBlockImages()
	: texture_size(12), rotation(0), render_unknown_blocks(false),
	  render_leaves_transparent(true) {
}

AbstractBlockImages::~AbstractBlockImages() {
}

void AbstractBlockImages::setRotation(int rotation) {
	this->rotation = rotation;
}

void AbstractBlockImages::setRenderSpecialBlocks(bool render_unknown_blocks,
		bool render_leaves_transparent) {
	this->render_unknown_blocks = render_unknown_blocks;
	this->render_leaves_transparent = render_leaves_transparent;
}

void AbstractBlockImages::loadBlocks(const BlockImageTextureResources& resources) {
	this->resources = resources;
	this->texture_size = resources.getTextureSize();

	empty_texture.setSize(texture_size, texture_size);
	unknown_block = createUnknownBlock();
	createBlocks();
	createBiomeBlocks();
}

namespace {

/**
 * Comparator to sort the block int's with id and data.
 */
struct block_comparator {
	bool operator()(uint32_t b1, uint32_t b2) const {
		uint16_t id1 = b1 & 0xffff;
		uint16_t id2 = b2 & 0xffff;
		if (id1 != id2)
			return id1 < id2;
		uint16_t data1 = (b1 & 0xffff0000) >> 16;
		uint16_t data2 = (b2 & 0xffff0000) >> 16;
		return data1 < data2;
	}
};

}

bool AbstractBlockImages::saveBlocks(const std::string& filename) {
	std::map<uint32_t, RGBAImage, block_comparator> blocks_sorted;
	for (auto it = block_images.begin(); it != block_images.end(); ++it) {
		uint16_t data = (it->first & 0xffff0000) >> 16;
		if ((data & (EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM)) == 0)
			blocks_sorted[it->first] = it->second;
	}

	std::vector<RGBAImage> blocks;
	for (auto it = blocks_sorted.begin(); it != blocks_sorted.end(); ++it)
		blocks.push_back(it->second);

	/*
	blocks.push_back(opaque_water[0]);
	blocks.push_back(opaque_water[1]);
	blocks.push_back(opaque_water[2]);
	blocks.push_back(opaque_water[3]);
	*/

	/*
	for (std::unordered_map<uint64_t, RGBAImage>::const_iterator it = biome_images.begin();
			it != biome_images.end(); ++it)
		blocks.push_back(it->second);
	*/

	int blocksize = getBlockSize();
	int width = 16;
	int height = std::ceil(blocks.size() / (double) width);
	RGBAImage img(width * blocksize, height * blocksize);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int offset = y * width + x;
			if ((size_t) offset >= blocks.size())
				break;
			img.alphaBlit(blocks.at(offset), x * blocksize, y * blocksize);
		}
	}
	std::cout << block_images.size() << " blocks" << std::endl;
	std::cout << "all: " << blocks.size() << std::endl;

	return img.writePNG(filename);
}

bool AbstractBlockImages::isBlockTransparent(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	// remove edge data
	data &= ~(EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM);

	// special case for doors because they are only used with special data
	// and not with the original minecraft data
	// without this the lighting code for example would need to filter the door data
	// FIXME
	if (id == 64 || id == 71)
		return true;
	if (block_images.count(id | (data << 16)) == 0)
		return !render_unknown_blocks;
	return block_transparency.count(id | (data << 16)) != 0;
}

bool AbstractBlockImages::hasBlock(uint16_t id, uint16_t data) const {
	return block_images.count(id | (data << 16)) != 0;
}

const RGBAImage& AbstractBlockImages::getBlock(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	if (!hasBlock(id, data))
		return unknown_block;
	return block_images.at(id | (data << 16));
}

RGBAImage AbstractBlockImages::getBiomeBlock(uint16_t id, uint16_t data,
		const Biome& biome) const {
	data = filterBlockData(id, data);
	if (!hasBlock(id, data))
		return unknown_block;

	// check if this biome block is precalculated
	if (biome == getBiome(biome.getID())) {
		int64_t key = id | (((int64_t) data) << 16) | (((int64_t) biome.getID()) << 32);
		if (!biome_images.count(key))
			return unknown_block;
		return biome_images.at(key);
	}

	// create the block if not
	return createBiomeBlock(id, data, biome);
}

int AbstractBlockImages::getTextureSize() const {
	return texture_size;
}

void AbstractBlockImages::setBlockImage(uint16_t id, uint16_t data,
		const RGBAImage& block) {
	block_images[id | (data << 16)] = block;

	// check if block contains transparency
	if (checkImageTransparency(block))
		block_transparency.insert(id | (data << 16));
	// TODO in IsometricBlockImages
	// if block is not transparent, add shadow edges
	//else
	//	addBlockShadowEdges(id, data, block);
}

void AbstractBlockImages::createBiomeBlocks() {
	for (std::unordered_map<uint32_t, RGBAImage>::iterator it = block_images.begin();
			it != block_images.end(); ++it) {
		uint16_t id = it->first & 0xffff;
		uint16_t data = (it->first & 0xffff0000) >> 16;

		// check if this is a biome block
		if (!Biome::isBiomeBlock(id, data))
			continue;

		for (size_t i = 0; i < BIOMES_SIZE; i++) {
			Biome biome = BIOMES[i];
			uint64_t b = biome.getID();
			biome_images[id | ((uint64_t) data << 16) | (b << 32)] =
					createBiomeBlock(id, data, biome);
		}
	}
}

}
}
