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

bool ChestTextures::load(const std::string& filename, int texture_size) {
	RGBAImage image;
	if (!image.readPNG(filename)) {
		LOG(ERROR) << "Unable to read '" << filename << "'.";
		return false;
	}

	if (image.getWidth() != image.getHeight()) {
		LOG(ERROR) << "Chest texture has invalid size (width:height must be 1:1): '"
				<< filename << "'.";
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
	front.resizeAuto((*this)[ChestTextures::FRONT], texture_size, texture_size);
	side.resizeAuto((*this)[ChestTextures::SIDE], texture_size, texture_size);
	top.resizeAuto((*this)[ChestTextures::TOP], texture_size, texture_size);

	return true;
}

bool DoubleChestTextures::load(const std::string& filename, int texture_size) {
	RGBAImage image;
	if (!image.readPNG(filename)) {
		LOG(ERROR) << "Unable to read '" << filename << "'.";
		return false;
	}

	if (image.getWidth() != image.getHeight() * 2) {
		LOG(ERROR) << "Chest texture has invalid size (width:height must be 1:2): '"
				<< filename << "'.";
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
	front_left.resizeAuto((*this)[DoubleChestTextures::FRONT_LEFT], texture_size, texture_size);
	front_right.resizeAuto((*this)[DoubleChestTextures::FRONT_RIGHT], texture_size, texture_size);
	side.resizeAuto((*this)[DoubleChestTextures::SIDE], texture_size, texture_size);
	top_left.resizeAuto((*this)[DoubleChestTextures::TOP_LEFT], texture_size, texture_size);
	top_right.resizeAuto((*this)[DoubleChestTextures::TOP_RIGHT], texture_size, texture_size);
	back_left.resizeAuto((*this)[DoubleChestTextures::BACK_LEFT], texture_size, texture_size);
	back_right.resizeAuto((*this)[DoubleChestTextures::BACK_RIGHT], texture_size, texture_size);

	return true;
}

TextureResources::TextureResources()
	: texture_size(12), texture_blur(0) {
}

TextureResources::~TextureResources() {
}

int TextureResources::getTextureSize() const {
	return texture_size;
}

int TextureResources::getTextureBlur() const {
	return texture_blur;
}

bool TextureResources::loadTextures(const std::string& texture_dir,
		int texture_size, int texture_blur) {
	// set texture size and blur
	this->texture_size = texture_size;
	this->texture_blur = texture_blur;

	// add a slash to the path if it's not already there
	std::string dir = texture_dir;
	if (dir[dir.size() - 1] != '/')
		dir = dir + '/';
	bool ok = true;
	if (!loadChests(dir + "entity/chest/normal.png",
			dir + "entity/chest/normal_double.png",
			dir + "entity/chest/ender.png",
			dir + "entity/chest/trapped.png",
			dir + "entity/chest/trapped_double.png"))
		ok = false;
	if (!loadColors(dir + "colormap/foliage.png",
			dir + "colormap/grass.png"))
		ok = false;
	if (!loadBlocks(dir + "blocks", dir + "endportal.png"))
		ok = false;
	if (!ok) {
		LOG(ERROR) << "Invalid texture directory '" << dir << "'. See previous log messages.";
		return false;
	}
	return true;
}

const BlockTextures& TextureResources::getBlockTextures() const {
	return block_textures;
}

const RGBAImage& TextureResources::getEndportalTexture() const {
	return endportal_texture;
}

const ChestTextures& TextureResources::getNormalChest() const {
	return normal_chest;
}

const DoubleChestTextures& TextureResources::getNormalDoubleChest() const {
	return normal_double_chest;
}

const ChestTextures& TextureResources::getEnderChest() const {
	return ender_chest;
}

const ChestTextures& TextureResources::getTrappedChest() const {
	return trapped_chest;
}

const DoubleChestTextures& TextureResources::getTrappedDoubleChest() const {
	return trapped_double_chest;
}

const RGBAImage& TextureResources::getFoliageColors() const {
	return foliage_colors;
}

const RGBAImage& TextureResources::getGrassColors() const {
	return grass_colors;
}

bool TextureResources::loadChests(const std::string& normal_png,
		const std::string& normal_double_png, const std::string& ender_png,
		const std::string& trapped_png, const std::string& trapped_double_png) {
	if (!normal_chest.load(normal_png, texture_size)
			|| !normal_double_chest.load(normal_double_png, texture_size)
			|| !ender_chest.load(ender_png, texture_size)
			|| !trapped_chest.load(trapped_png, texture_size)
			|| !trapped_double_chest.load(trapped_double_png, texture_size))
		return false;
	return true;
}

bool TextureResources::loadColors(const std::string& foliage_png,
		const std::string& grass_png) {
	bool ok = true;
	if (!foliage_colors.readPNG(foliage_png)) {
		LOG(ERROR) << "Unable to read '" << foliage_png << "'.";
		ok = false;
	}
	if (!grass_colors.readPNG(grass_png)) {
		LOG(ERROR) << "Unable to read '" << grass_png << "'.";
		ok = false;
	}
	return ok;
}

bool TextureResources::loadBlocks(const std::string& block_dir,
		const std::string& endportal_png) {
	if (!block_textures.load(block_dir, texture_size, texture_blur))
		return false;
	empty_texture.setSize(texture_size, texture_size);

	RGBAImage endportal;
	if(!endportal.readPNG(endportal_png)) {
		LOG(ERROR) << "Unable to read '" << endportal_png << "'.";
		return false;
	}
	endportal.resizeAuto(endportal_texture, texture_size, texture_size);

	return true;
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

void AbstractBlockImages::loadBlocks(const TextureResources& resources) {
	this->resources = resources;
	this->texture_size = resources.getTextureSize();

	empty_texture.setSize(texture_size, texture_size);
	unknown_block.setSize(getBlockSize(), getBlockSize());
	// set unknown block to something recognizable if rendering of unknown blocks is enabled,
	// otherwise just keep it a transparent image
	if (render_unknown_blocks)
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
	if (isImageTransparent(block))
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
