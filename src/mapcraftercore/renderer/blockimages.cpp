/*
 * Copyright 2012-2016 Moritz Hilscher
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
	front.resize((*this)[ChestTextures::FRONT], texture_size, texture_size);
	side.resize((*this)[ChestTextures::SIDE], texture_size, texture_size);
	top.resize((*this)[ChestTextures::TOP], texture_size, texture_size);

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
	front_left.resize((*this)[DoubleChestTextures::FRONT_LEFT], texture_size, texture_size);
	front_right.resize((*this)[DoubleChestTextures::FRONT_RIGHT], texture_size, texture_size);
	side.resize((*this)[DoubleChestTextures::SIDE], texture_size, texture_size);
	top_left.resize((*this)[DoubleChestTextures::TOP_LEFT], texture_size, texture_size);
	top_right.resize((*this)[DoubleChestTextures::TOP_RIGHT], texture_size, texture_size);
	back_left.resize((*this)[DoubleChestTextures::BACK_LEFT], texture_size, texture_size);
	back_right.resize((*this)[DoubleChestTextures::BACK_RIGHT], texture_size, texture_size);

	return true;
}

bool ShulkerTextures::load(const std::string& base_filename, int texture_size) {
    return (load_single(base_filename + "white.png", 0, texture_size) &&
            load_single(base_filename + "orange.png", 1, texture_size) &&
            load_single(base_filename + "magenta.png", 2, texture_size) &&
            load_single(base_filename + "light_blue.png", 3, texture_size) &&
            load_single(base_filename + "yellow.png", 4, texture_size) &&
            load_single(base_filename + "lime.png", 5, texture_size) &&
            load_single(base_filename + "pink.png", 6, texture_size) &&
            load_single(base_filename + "gray.png", 7, texture_size) &&
            load_single(base_filename + "silver.png", 8, texture_size) &&
            load_single(base_filename + "cyan.png", 9, texture_size) &&
            load_single(base_filename + "purple.png", 10, texture_size) &&
            load_single(base_filename + "blue.png", 11, texture_size) &&
            load_single(base_filename + "brown.png", 12, texture_size) &&
            load_single(base_filename + "green.png", 13, texture_size) &&
            load_single(base_filename + "red.png", 14, texture_size) &&
            load_single(base_filename + "black.png", 15, texture_size));
}

bool ShulkerTextures::load_single(const std::string& filename, int color_index, int texture_size) {
    RGBAImage image;
    if (!image.readPNG(filename)) {
        LOG(ERROR) << "Unable to read '" << filename << "'.";
        return false;
    }

    if (image.getWidth() != image.getHeight()) {
        LOG(ERROR) << "Shulker texture has invalid size (width:height must be 1:1): '"
                   << filename << "'.";
        return false;
    }
    // if the image is 64px wide, the shulker box images are 16x16
    int ratio = image.getHeight() / 64;
    int size = ratio * 16;
    int size_h = ratio * 12; // Sides are slightly smaller

    RGBAImage top = image.clip(size, 0, size, size);
    RGBAImage side = image.clip(0, size, size, size);
    RGBAImage side_bottom = image.clip(0, 2 * size + (size - size_h), size, size);
    RGBAImage bottom = image.clip(2 * size, size + size_h, size, size);

    side.alphaBlit(side_bottom, 0, 0);

    int offset = color_index * ShulkerTextures::DATA_SIZE;

    // resize the chest images to texture size
    bottom.resize((*this)[offset + ShulkerTextures::BOTTOM], texture_size, texture_size);
    side.resize((*this)[offset + ShulkerTextures::SIDE], texture_size, texture_size);
    top.resize((*this)[offset + ShulkerTextures::TOP], texture_size, texture_size);

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
		int texture_size, int texture_blur, double water_opacity) {
	// set texture size and blur
	this->texture_size = texture_size;
	this->texture_blur = texture_blur;
	this->water_opacity = water_opacity;

	// add a slash to the path if it's not already there
	std::string dir = texture_dir;
	if (dir[dir.size() - 1] != '/')
		dir = dir + '/';
	bool ok = true;
	if (!loadChests(dir + "entity/chest/normal.png",
			dir + "entity/chest/normal_double.png",
			dir + "entity/chest/ender.png",
			dir + "entity/chest/trapped.png",
			dir + "entity/chest/trapped_double.png",
            dir + "entity/shulker/shulker_"))
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

const ShulkerTextures& TextureResources::getShulkerBoxTextures() const {
    return shulker_textures;
}

const RGBAImage& TextureResources::getFoliageColors() const {
	return foliage_colors;
}

const RGBAImage& TextureResources::getGrassColors() const {
	return grass_colors;
}

bool TextureResources::loadChests(const std::string& normal_png,
		const std::string& normal_double_png, const std::string& ender_png,
		const std::string& trapped_png, const std::string& trapped_double_png,
        const std::string& shulker_base_png) {
	if (!normal_chest.load(normal_png, texture_size)
			|| !normal_double_chest.load(normal_double_png, texture_size)
			|| !ender_chest.load(ender_png, texture_size)
			|| !trapped_chest.load(trapped_png, texture_size)
			|| !trapped_double_chest.load(trapped_double_png, texture_size)
            || !shulker_textures.load(shulker_base_png, texture_size))
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
	if (!block_textures.load(block_dir, texture_size, texture_blur, water_opacity))
		return false;
	empty_texture.setSize(texture_size, texture_size);

	RGBAImage endportal;
	if(!endportal.readPNG(endportal_png)) {
		LOG(ERROR) << "Unable to read '" << endportal_png << "'.";
		return false;
	}
	endportal.resize(endportal_texture, texture_size, texture_size);

	return true;
}

BlockImages::~BlockImages() {
}

AbstractBlockImages::AbstractBlockImages()
	: texture_size(12), rotation(0), render_unknown_blocks(false),
	  render_leaves_transparent(true), max_water_preblit(9042) /* it's over 9000! */ {
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

void AbstractBlockImages::generateBlocks(const TextureResources& resources) {
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
	max_water_preblit = createOpaqueWater();
}

RGBAImage AbstractBlockImages::exportBlocks() const {
	std::vector<RGBAImage> blocks = getExportBlocks();

	int width = 16;
	int height = std::ceil((double) blocks.size() / width);
	int block_size = getBlockSize();
	RGBAImage image(width * block_size, height * block_size);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int offset = y * width + x;
			if ((size_t) offset >= blocks.size())
				break;
			image.alphaBlit(blocks.at(offset), x * block_size, y * block_size);
		}
	}

	return image;
}

bool AbstractBlockImages::isBlockTransparent(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);

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

const RGBAImage& AbstractBlockImages::getBlock(uint16_t id, uint16_t data, mc::BlockPos position) const {
	// TODO: (Bjarno) for given ids, return a different image, depending on the position
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

int AbstractBlockImages::getMaxWaterPreblit() const {
	return max_water_preblit;
}

int AbstractBlockImages::getTextureSize() const {
	return texture_size;
}

uint16_t AbstractBlockImages::filterBlockData(uint16_t id, uint16_t data) const {
	if (id == 54 || id == 130 || id == 146) { // chests
		// at first get the direction of the chest and rotate if needed
		uint16_t dir_rotate = (data >> 4) & 0xf;
		uint16_t dir = util::rotateShiftLeft(dir_rotate, rotation, 4) << 4;
		// then get the neighbor chests
		uint16_t neighbors = (data >> 4) & 0xf0;

		// if no neighbors, this is a small chest
		// the data contains only the direction
		if (neighbors == 0 || id == 130)
			return dir;

		// this is a double chest
		// the data contains the direction and a bit, which shows that this is a large chest
		// check also if this is the left part of the large chest
		uint16_t new_data = dir | LARGECHEST_DATA_LARGE;
		if ((dir == DATA_NORTH && neighbors == DATA_WEST)
				|| (dir == DATA_SOUTH && neighbors == DATA_EAST)
				|| (dir == DATA_EAST && neighbors == DATA_NORTH)
				|| (dir == DATA_WEST && neighbors == DATA_SOUTH))
			new_data |= LARGECHEST_DATA_LEFT;
		return new_data;
	} else if (id == 55) { // redstone wire, tripwire
		// check if powered
		if ((data & util::binary<1111>::value) != 0)
			return (data & ~(util::binary<1111>::value)) | REDSTONE_POWERED;
		return data & ~(util::binary<1111>::value);
	}
	return data;
}

void AbstractBlockImages::setBlockImage(uint16_t id, uint16_t data,
		const RGBAImage& block) {
	block_images[id | (data << 16)] = block;

	// check if block contains transparency
	if (isImageTransparent(block))
		block_transparency.insert(id | (data << 16));
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

std::vector<RGBAImage> AbstractBlockImages::getExportBlocks() const {
	std::map<uint32_t, RGBAImage, block_images_comparator> blocks_sorted;
	for (auto it = block_images.begin(); it != block_images.end(); ++it)
			blocks_sorted[it->first] = it->second;

	std::vector<RGBAImage> blocks;
	for (auto it = blocks_sorted.begin(); it != blocks_sorted.end(); ++it)
		blocks.push_back(it->second);
	return blocks;
}

}
}
