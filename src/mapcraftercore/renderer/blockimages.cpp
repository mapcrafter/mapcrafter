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
#include "../mc/blockstate.h"

#include <chrono>
#include <map>
#include <vector>

namespace mapcrafter {
namespace util {

template <>
renderer::ColorMapType as<renderer::ColorMapType>(const std::string& str) {
	if (str == "foliage") {
		return renderer::ColorMapType::FOLIAGE;
	} else if (str == "foliage_flipped") {
		return renderer::ColorMapType::FOLIAGE_FLIPPED;
	} else if (str == "grass") {
		return renderer::ColorMapType::GRASS;
	} else if (str == "water") {
		return renderer::ColorMapType::WATER;
	} else {
		throw std::invalid_argument("Must be 'foliage', 'foliage_flipped', 'grass' or 'water'!");
	}
}

template <>
renderer::LightingType as<renderer::LightingType>(const std::string& str) {
	if (str == "none") {
		return renderer::LightingType::NONE;
	} else if (str == "simple") {
		return renderer::LightingType::SIMPLE;
	} else if (str == "smooth") {
		return renderer::LightingType::SMOOTH;
	} else if (str == "smooth_bottom") {
		return renderer::LightingType::SMOOTH_BOTTOM;
	} else {
		throw std::invalid_argument("Must be 'none', 'simple' or 'smooth'!");
	}
}

}
}

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
	return (loadSingle(base_filename + "white.png", 0, texture_size) &&
			loadSingle(base_filename + "orange.png", 1, texture_size) &&
			loadSingle(base_filename + "magenta.png", 2, texture_size) &&
			loadSingle(base_filename + "light_blue.png", 3, texture_size) &&
			loadSingle(base_filename + "yellow.png", 4, texture_size) &&
			loadSingle(base_filename + "lime.png", 5, texture_size) &&
			loadSingle(base_filename + "pink.png", 6, texture_size) &&
			loadSingle(base_filename + "gray.png", 7, texture_size) &&
			loadSingle(base_filename + "silver.png", 8, texture_size) &&
			loadSingle(base_filename + "cyan.png", 9, texture_size) &&
			loadSingle(base_filename + "purple.png", 10, texture_size) &&
			loadSingle(base_filename + "blue.png", 11, texture_size) &&
			loadSingle(base_filename + "brown.png", 12, texture_size) &&
			loadSingle(base_filename + "green.png", 13, texture_size) &&
			loadSingle(base_filename + "red.png", 14, texture_size) &&
			loadSingle(base_filename + "black.png", 15, texture_size));
}

bool ShulkerTextures::loadSingle(const std::string& filename, int color_index, int texture_size) {
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

	// resize the shulker images to texture size
	bottom.resize((*this)[offset + ShulkerTextures::BOTTOM], texture_size, texture_size);
	side.resize((*this)[offset + ShulkerTextures::SIDE], texture_size, texture_size);
	top.resize((*this)[offset + ShulkerTextures::TOP], texture_size, texture_size);

	return true;
}

bool BedTextures::load(const std::string& base_filename, int texture_size) {
	return (loadSingle(base_filename + "white.png", 0, texture_size) &&
			loadSingle(base_filename + "orange.png", 1, texture_size) &&
			loadSingle(base_filename + "magenta.png", 2, texture_size) &&
			loadSingle(base_filename + "light_blue.png", 3, texture_size) &&
			loadSingle(base_filename + "yellow.png", 4, texture_size) &&
			loadSingle(base_filename + "lime.png", 5, texture_size) &&
			loadSingle(base_filename + "pink.png", 6, texture_size) &&
			loadSingle(base_filename + "gray.png", 7, texture_size) &&
			loadSingle(base_filename + "silver.png", 8, texture_size) &&
			loadSingle(base_filename + "cyan.png", 9, texture_size) &&
			loadSingle(base_filename + "purple.png", 10, texture_size) &&
			loadSingle(base_filename + "blue.png", 11, texture_size) &&
			loadSingle(base_filename + "brown.png", 12, texture_size) &&
			loadSingle(base_filename + "green.png", 13, texture_size) &&
			loadSingle(base_filename + "red.png", 14, texture_size) &&
			loadSingle(base_filename + "black.png", 15, texture_size));
}

bool BedTextures::loadSingle(const std::string& filename, int color_index, int texture_size) {
	RGBAImage image;
	if (!image.readPNG(filename)) {
		LOG(ERROR) << "Unable to read '" << filename << "'.";
		return false;
	}

	if (image.getWidth() != image.getHeight()) {
		LOG(ERROR) << "Bed texture has invalid size (width:height must be 1:1): '"
				   << filename << "'.";
		return false;
	}
	// if the image is 64px wide, the bed box images are 16x16
	int ratio = image.getHeight() / 64;
	int size = ratio * 16;

	// Copy parts from the texture file (sizes according to base texture)
	RGBAImage top_head = image.clip(6 * ratio, 6 * ratio, size, size); // 16 x 16
	RGBAImage top_foot = image.clip(6 * ratio, 28 * ratio, size, size); // 16 x 16
	RGBAImage small_side_head_left = image.clip(0, 6 * ratio, 6 * ratio, size); // 6 x 16
	RGBAImage small_side_head_right = image.clip(22 * ratio, 6 * ratio, 6 * ratio, size); // 6 x 16
	RGBAImage small_side_foot_left = image.clip(0, 28 * ratio, 6 * ratio, size); // 6 x 16
	RGBAImage small_side_foot_right = image.clip(22 * ratio, 28 * ratio, 6 * ratio, size); // 6 x 16
	RGBAImage small_side_head_end = image.clip(6 * ratio, 0, size, 6 * ratio); // 16 x 16
	RGBAImage small_side_foot_end = image.clip(22 * ratio, 22 * ratio, size, 6 * ratio); // 16 x 16
	RGBAImage leg = image.clip(50 * ratio, 3 * ratio, 3 * ratio, 3 * ratio); // Only one leg, used on multiple locations, 3 x 3

	// Create blank canvases for the side textures at their proper size
	RGBAImage side_head_left(size, size); side_head_left.clear();
	RGBAImage side_head_right(size, size); side_head_right.clear();
	RGBAImage side_foot_left(size, size); side_foot_left.clear();
	RGBAImage side_foot_right(size, size); side_foot_right.clear();
	RGBAImage side_head_end(size, size); side_head_end.clear();
	RGBAImage side_foot_end(size, size); side_foot_end.clear();

	// Render textures by copying things and adding legs
	side_head_left.simpleBlit(small_side_head_left, 3 * ratio, 0);
	side_head_left.simpleBlit(leg.rotate(ROTATE_90).flip(false, true), 0, 0);
	side_head_left = side_head_left.rotate(ROTATE_270);

	side_head_right.simpleBlit(small_side_head_right, 7 * ratio, 0);
	side_head_right.simpleBlit(leg.rotate(ROTATE_90).flip(false, true), 13 * ratio, 0);
	side_head_right = side_head_right.rotate(ROTATE_90);

	side_foot_left.simpleBlit(small_side_foot_left, 3 * ratio, 0);
	side_foot_left.simpleBlit(leg.rotate(ROTATE_90), 0, 13 * ratio);
	side_foot_left = side_foot_left.rotate(ROTATE_270);

	side_foot_right.simpleBlit(small_side_foot_right, 7 * ratio, 0);
	side_foot_right.simpleBlit(leg.rotate(ROTATE_90), 13 * ratio, 13 * ratio);
	side_foot_right = side_foot_right.rotate(ROTATE_90);

	side_head_end.simpleBlit(small_side_head_end.flip(false, true), 0, 7 * ratio);
	side_head_end.simpleBlit(leg.flip(true, false), 0, 13 * ratio);
	side_head_end.simpleBlit(leg, 13 * ratio, 13 * ratio);

	side_foot_end.simpleBlit(small_side_foot_end.flip(false, true), 0, 7 * ratio);
	side_foot_end.simpleBlit(leg.flip(true, false), 0, 13 * ratio);
	side_foot_end.simpleBlit(leg, 13 * ratio, 13 * ratio);

	int offset = color_index * BedTextures::DATA_SIZE;

	// resize the bed images to texture size
	top_head.resize((*this)[offset + BedTextures::TOP_HEAD], texture_size, texture_size);
	top_foot.resize((*this)[offset + BedTextures::TOP_FOOT], texture_size, texture_size);
	side_head_left.resize((*this)[offset + BedTextures::SIDE_HEAD_LEFT], texture_size, texture_size);
	side_head_right.resize((*this)[offset + BedTextures::SIDE_HEAD_RIGHT], texture_size, texture_size);
	side_foot_left.resize((*this)[offset + BedTextures::SIDE_FOOT_LEFT], texture_size, texture_size);
	side_foot_right.resize((*this)[offset + BedTextures::SIDE_FOOT_RIGHT], texture_size, texture_size);
	side_head_end.resize((*this)[offset + BedTextures::SIDE_HEAD_END], texture_size, texture_size);
	side_foot_end.resize((*this)[offset + BedTextures::SIDE_FOOT_END], texture_size, texture_size);

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
	/*
	if (!loadChests(dir + "entity/chest/normal.png",
			dir + "entity/chest/normal_double.png",
			dir + "entity/chest/ender.png",
			dir + "entity/chest/trapped.png",
			dir + "entity/chest/trapped_double.png",
			dir + "entity/shulker/shulker_",
			dir + "entity/bed/"))
		ok = false;
	*/
	if (!loadColors(dir + "colormap/foliage.png",
			dir + "colormap/grass.png"))
		ok = false;
	/*
	if (!loadBlocks(dir + "blocks", dir + "endportal.png"))
		ok = false;
	*/
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

const BedTextures &TextureResources::getBedTextures() const {
	return bed_textures;
}

const RGBAImage& TextureResources::getFoliageColors() const {
	return foliage_colors;
}

const RGBAImage& TextureResources::getGrassColors() const {
	return grass_colors;
}

const RGBAImage& TextureResources::getColorMap(ColorMapType type) const {
	if (type == ColorMapType::FOLIAGE) {
		return foliage_colors;
	} else if (type == ColorMapType::FOLIAGE_FLIPPED) {
		return foliage_flipped_colors;
	} else if (type == ColorMapType::GRASS) {
		return grass_colors;
	} else if (type == ColorMapType::WATER) {
		return water_colors;
	}
	assert(false);
}

bool TextureResources::loadChests(const std::string& normal_png,
		const std::string& normal_double_png, const std::string& ender_png,
		const std::string& trapped_png, const std::string& trapped_double_png,
		const std::string& shulker_base_png, const std::string& bed_base_png) {
	if (!normal_chest.load(normal_png, texture_size)
			|| !normal_double_chest.load(normal_double_png, texture_size)
			|| !ender_chest.load(ender_png, texture_size)
			|| !trapped_chest.load(trapped_png, texture_size)
			|| !trapped_double_chest.load(trapped_double_png, texture_size)
			|| !shulker_textures.load(shulker_base_png, texture_size)
			|| !bed_textures.load(bed_base_png, texture_size))
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
	foliage_flipped_colors = foliage_colors.rotate(2);
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

	// special case for beds, as they are handled in a different way

	// FIXME
	if (id == 64 || id == 71 || id == 26)
		return true;
	if (block_images.count(id | (data << 16)) == 0)
		return !render_unknown_blocks;
	return block_transparency.count(id | (data << 16)) != 0;
}

bool AbstractBlockImages::hasBlock(uint16_t id, uint16_t data) const {
	return block_images.count(id | (data << 16)) != 0;
}

bool AbstractBlockImages::hasBedBlock(uint16_t data, uint16_t extra_data) const {
	return block_images_bed.count(data | (extra_data << 16)) != 0;
}

const RGBAImage& AbstractBlockImages::getBlock(uint16_t id, uint16_t data, uint16_t extra_data) const {
	data = filterBlockData(id, data);

	if (id == 26) { // Beds
		if (!hasBedBlock(data, extra_data)) {
			return unknown_block;
		}
		return block_images_bed.at(data | (extra_data << 16));
	}

	if (!hasBlock(id, data))
		return unknown_block;
	return block_images.at(id | (data << 16));
}

RGBAImage AbstractBlockImages::getBiomeBlock(uint16_t id, uint16_t data,
		const Biome& biome, uint16_t extra_data) const {
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

void AbstractBlockImages::setBedImage(uint16_t data, uint16_t extra_data,
		const RGBAImage& block) {
	block_images_bed[data | (extra_data << 16)] = block;
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

void blockImageTest(RGBAImage& block, const RGBAImage& uv_mask) {
	assert(block.getWidth() == uv_mask.getWidth());
	assert(block.getHeight() == uv_mask.getHeight());
	
	for (size_t x = 0; x < block.getWidth(); x++) {
		for (size_t y = 0; y < block.getHeight(); y++) {
			uint32_t& pixel = block.pixel(x, y);
			uint32_t uv_pixel = uv_mask.pixel(x, y);
			if (rgba_alpha(uv_pixel) == 0) {
				continue;
			}

			uint8_t side = rgba_blue(uv_pixel);
			if (side == FACE_LEFT_INDEX) {
				pixel = rgba(255, 0, 0);
			}
			if (side == FACE_RIGHT_INDEX) {
				pixel = rgba(0, 255, 0);
			}
			if (side == FACE_UP_INDEX) {
				pixel = rgba(0, 0, 255);;
			}
		}
	}
}

void blockImageMultiply(RGBAImage& block, const RGBAImage& uv_mask,
		float factor_left, float factor_right, float factor_up) {
	assert(block.getWidth() == uv_mask.getWidth());
	assert(block.getHeight() == uv_mask.getHeight());
	
	for (size_t x = 0; x < block.getWidth(); x++) {
		for (size_t y = 0; y < block.getHeight(); y++) {
			uint32_t& pixel = block.pixel(x, y);
			uint32_t uv_pixel = uv_mask.pixel(x, y);
			if (rgba_alpha(uv_pixel) == 0) {
				continue;
			}

			uint8_t side = rgba_blue(uv_pixel);
			if (side == FACE_LEFT_INDEX) {
				pixel = rgba_multiply(pixel, factor_left, factor_left, factor_left);
			}
			if (side == FACE_RIGHT_INDEX) {
				pixel = rgba_multiply(pixel, factor_right, factor_right, factor_right);
			}
			if (side == FACE_UP_INDEX) {
				pixel = rgba_multiply(pixel, factor_up, factor_up, factor_up);
			}
		}
	}
}

void blockImageMultiplyExcept(RGBAImage& block, const RGBAImage& uv_mask,
		uint8_t except_face, float factor) {
	assert(block.getWidth() == uv_mask.getWidth());
	assert(block.getHeight() == uv_mask.getHeight());
	
	for (size_t x = 0; x < block.getWidth(); x++) {
		for (size_t y = 0; y < block.getHeight(); y++) {
			uint32_t& pixel = block.pixel(x, y);
			uint32_t uv_pixel = uv_mask.pixel(x, y);
			if (rgba_alpha(uv_pixel) == 0) {
				continue;
			}

			uint8_t side = rgba_blue(uv_pixel);
			if (side != except_face) {
				pixel = rgba_multiply(pixel, factor, factor, factor);
			}
		}
	}
}

namespace {

/*
inline uint32_t divide255(uint32_t v1, uint32_t v2) {
	uint32_t t = v1 * v2 + 128;
	return ((t >> 8) + t) >> 8;
}
*/

inline uint32_t mix(uint32_t x, uint32_t y, uint32_t a) {
	// >> 8 = / 256, serves as approximation for division by 255
	return ((x * (255-a)) + (y * a)) >> 8;
}

}

void blockImageMultiply(RGBAImage& block, const RGBAImage& uv_mask,
		const CornerValues& factors_left, const CornerValues& factors_right, const CornerValues& factors_up) {
	assert(block.getWidth() == uv_mask.getWidth());
	assert(block.getHeight() == uv_mask.getHeight());

	uint32_t fl[4], fr[4], fu[4];
	for (int i = 0; i < 4; i++) {
		fl[i] = factors_left[i] * 255;
		fr[i] = factors_right[i] * 255;
		fu[i] = factors_up[i] * 255;
	}

	
	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		uint32_t& pixel = block.data[i];
		uint32_t uv_pixel = uv_mask.data[i];
		if (rgba_alpha(uv_pixel) == 0) {
			continue;
		}

		//const CornerValues* vptr = nullptr;
		uint32_t* f = nullptr;
		uint8_t side = rgba_blue(uv_pixel);
		if (side == FACE_LEFT_INDEX) {
			//vptr = &factors_left;
			f = fl;
		} else if (side == FACE_RIGHT_INDEX) {
			//vptr = &factors_right;
			f = fr;
		} else if (side == FACE_UP_INDEX) {
			//vptr = &factors_up;
			f = fu;
		} else {
			continue;
		}

		/*
		const CornerValues& values = *vptr;
		float u = (float) rgba_red(uv_pixel) / 255.0;
		float v = (float) rgba_green(uv_pixel) / 255.0;
		float ab = (1-u) * values[0] + u * values[1];
		float cd = (1-u) * values[2] + u * values[3];
		float x = (1-v) * ab + v * cd;
		*/

		uint32_t u = rgba_red(uv_pixel);
		uint32_t v = rgba_green(uv_pixel);
		
		//uint32_t ab = divide255((255-u), f[0]) + divide255(u, f[1]);
		//uint32_t cd = divide255((255-u), f[2]) + divide255(u, f[3]);
		//uint32_t x = divide255((255-v), ab) + divide255(v,  cd);
		
		// jetzt sogar 34.17
		// und mit noch mehr rgba_multiply sogar 35.28
		uint32_t ab = mix(f[0], f[1], u); // divide255((255-u) * f[0], u * f[1]);
		uint32_t cd = mix(f[2], f[3], u); // divide255((255-u) * f[2], u * f[3]);
		uint32_t x = mix(ab, cd, v); // divide255((255-v) * ab, v * cd);

		// OHNE BASIS
		// 45.68
		//pixel = rgba_multiply(pixel, 0.5);
		
		// 34.44
		//float x = 0.5;
		//pixel = rgba_multiply(pixel, x, x, x);
	
		// FLOAT ALS BASIS
		// 25.68
		//pixel = rgba_multiply(pixel, x, x, x);
		
		// geht. aber vielleicht auch nicht mega viel schneller
		//uint8_t factor = x * 255;
		//pixel = rgba_multiply(pixel, factor, factor, factor);

		// geht, 29.13
		//int factor = x * 255;
		//assert(factor >= 0 && factor <= 255);
		//pixel = rgba_multiply(pixel, factor);
		
		// INTEGER ALS BASIS
		//assert(x >= 0 && x <= 255);
		
		// geht, 28.93
		//double factor = (double) x / 255;
		//pixel = rgba_multiply(pixel, factor, factor, factor);

		// geht, 28.00
		//uint8_t factor = x;
		//pixel = rgba_multiply(pixel, factor, factor, factor);

		// geht, 29.83
		// ohne uv-alpha check sogar 32.15
		// ohne uv-alpha check und uint8_t 32.39
		// ... div255 32.60
		// ... anderes div255 32.88
		// rgba_ inline: 33.64
		pixel = rgba_multiply_scalar(pixel, x);
	}
}

void blockImageMultiply(RGBAImage& block, uint8_t factor) {
	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		block.data[i] = rgba_multiply_scalar(block.data[i], factor);
	}
}

void blockImageTint(RGBAImage& block, const RGBAImage& mask, uint32_t color) {
	assert(block.getWidth() == mask.getWidth());
	assert(block.getHeight() == mask.getHeight());

	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		uint32_t& pixel = block.data[i];
		uint32_t mask_pixel = mask.data[i];
		/*
		if (rgba_alpha(mask_pixel) == 0) {
			continue;
		}
		*/
		if ((mask_pixel & 0xff000000) > 0) {
			pixel = rgba_multiply(mask_pixel, color);
		}
	}
}

void blockImageTint(RGBAImage& block, uint32_t color) {
	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		block.data[i] = rgba_multiply(block.data[i], color);
	}
}

void blockImageTintHighContrast(RGBAImage& block, uint32_t color) {
	// get luminance of recolor:
	// "10*r + 3*g + b" should actually be "3*r + 10*g + b"
	// it was a typo, but doesn't look bad either
	int luminance = (10 * rgba_red(color) + 3 * rgba_green(color) + rgba_blue(color)) / 14;

	float alpha_factor = 3; // 3 is similar to alpha=85
	// something like that would be possible too, but overlays won't look exactly like
	// overlays with that alpha value, so don't use it for now
	// alpha_factor = (float) 255.0 / rgba_alpha(color);

	// try to do luminance-neutral additive/subtractive color
	// instead of alpha blending (for better contrast)
	// so first subtract luminance from each component
	int nr = (rgba_red(color) - luminance) / alpha_factor;
	int ng = (rgba_green(color) - luminance) / alpha_factor;
	int nb = (rgba_blue(color) - luminance) / alpha_factor;

	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		RGBAPixel& pixel = block.data[i];
		if ((pixel & 0xff000000) > 0) {
			pixel = rgba_add_clamp(pixel, nr, ng, nb, 0);
		}
	}
}

void blockImageTintHighContrast(RGBAImage& block, const RGBAImage& mask, int face, uint32_t color) {
	assert(block.getWidth() == mask.getWidth());
	assert(block.getHeight() == mask.getHeight());

	// same as above
	int luminance = (10 * rgba_red(color) + 3 * rgba_green(color) + rgba_blue(color)) / 14;
	float alpha_factor = 3;
	int nr = (rgba_red(color) - luminance) / alpha_factor;
	int ng = (rgba_green(color) - luminance) / alpha_factor;
	int nb = (rgba_blue(color) - luminance) / alpha_factor;

	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		RGBAPixel& pixel = block.data[i];
		RGBAPixel mask_pixel = mask.data[i];
		if (rgba_blue(mask_pixel) == face) {
			pixel = rgba_add_clamp(pixel, nr, ng, nb, 0);
		}
	}
}

void blockImageShadowEdges(RGBAImage& block, const RGBAImage& uv_mask,
		uint8_t north, uint8_t south, uint8_t east, uint8_t west, uint8_t bottom) {
	assert(block.getWidth() == uv_mask.getWidth());
	assert(block.getHeight() == uv_mask.getHeight());

	size_t n = block.getWidth() * block.getHeight();
	for (size_t i = 0; i < n; i++) {
		RGBAPixel& pixel = block.data[i];
		const RGBAPixel& uv_pixel = uv_mask.data[i];

		// TODO
		// not really optimized yet, and quite dirty code
		float u = (float) rgba_red(uv_pixel) / 255;
		float v = (float) rgba_green(uv_pixel) / 255;
		uint8_t face = rgba_blue(uv_pixel);

		uint8_t alpha = 0;
		#define setalpha(x) (alpha = std::max(alpha, (uint8_t) (x)))
		auto genalpha = [&alpha, &face](int mask_face, int edge, float uv) {
			// explanation of edge influence:
			// edge=0: no edge
			// edge=1: edge with threshold 2px
			// edge=2: edge with threshold 3px
			// edge=3: edge with threshold 3px, a bit darker (for stronger visual on leaves etc.)
			float t = (float) (1 + std::min(2, edge)) / 16.0;
			float strong = 64;
			float weak = 32;
			if (edge > 2) {
				strong = 128;
				weak = 64;
			}
			if (edge && face == mask_face && uv < t) {
				if (uv < t / 2.0) {
					setalpha(strong);
				} else {
					float a = (uv-t/2.0) / (t/2.0);
					setalpha((float) (1-a) * weak + a*16.0);
				}
			}
		};

		genalpha(FACE_UP_INDEX, north, v);
		genalpha(FACE_UP_INDEX, south, 1.0 - v);
		genalpha(FACE_UP_INDEX, east, 1.0 - u);
		genalpha(FACE_UP_INDEX, west, u);

		genalpha(FACE_LEFT_INDEX, bottom, 1.0 - v);
		genalpha(FACE_RIGHT_INDEX, bottom, 1.0 - v);

		#undef setalpha

		pixel = rgba_multiply_scalar(pixel, 255 - alpha);
		/*
		if (alpha != 0) {
			pixel = rgba_multiply(pixel, rgba(255 - alpha, 0, 0));
		}
		*/
	}
}

bool blockImageIsTransparent(RGBAImage& block, const RGBAImage& uv_mask) {
	assert(block.getWidth() == uv_mask.getWidth());
	assert(block.getHeight() == uv_mask.getHeight());
	
	for (size_t x = 0; x < block.getWidth(); x++) {
		for (size_t y = 0; y < block.getHeight(); y++) {
			uint32_t& pixel = block.pixel(x, y);
			uint32_t uv_pixel = uv_mask.pixel(x, y);
			if (rgba_alpha(uv_pixel) == 0) {
				continue;
			}

			if (rgba_alpha(pixel) != rgba_alpha(uv_pixel)) {
				return true;
			}
		}
	}

	return false;
}

std::array<bool, 3> blockImageGetSideMask(const RGBAImage& uv) {
	std::array<bool, 3> side_mask = {false, false, false};
	uint8_t mask_indices[3] = {FACE_LEFT_INDEX, FACE_RIGHT_INDEX, FACE_UP_INDEX};
	for (size_t x = 0; x < uv.getWidth(); x++) {
		for (size_t y = 0; y < uv.getHeight(); y++) {
			uint32_t pixel = uv.pixel(x, y);
			if (rgba_alpha(pixel) == 0) {
				continue;
			}

			uint8_t face = rgba_blue(pixel);
			for (uint8_t i = 0; i < 3; i++) {
				if (face == mask_indices[i]) {
					side_mask[i] = true;
				}
			}
		}
	}
	return side_mask;
}

RenderedBlockImages::RenderedBlockImages(mc::BlockStateRegistry& block_registry)
	: block_registry(block_registry), darken_left(1.0), darken_right(1.0) {
}

RenderedBlockImages::~RenderedBlockImages() {
	for (auto it = block_images.begin(); it != block_images.end(); ++it) {
		if (*it != nullptr) {
			delete *it;
		}
	}
}

void RenderedBlockImages::setBlockSideDarkening(float darken_left, float darken_right) {
	this->darken_left = darken_left;
	this->darken_right = darken_right;
}

bool RenderedBlockImages::loadBlockImages(fs::path path, std::string view, int rotation, int texture_size) {
	LOG(INFO) << "I will load block images from " << path << " now";

	if (!fs::is_directory(path)) {
		LOG(ERROR) << "Unable to load block images: " << path << " is not a directory!";
		return false;
	}

	std::string name = view + "_" + util::str(rotation) + "_" + util::str(texture_size);
	fs::path info_file = path / (name + ".txt");
	fs::path block_file = path / (name + ".png");

	if (!fs::is_regular_file(info_file)) {
		LOG(ERROR) << "Unable to load block images: Block info file " << info_file
			<< " does not exist!";
		return false;
	}
	if (!fs::is_regular_file(block_file)) {
		LOG(ERROR) << "Unable to load block images: Block image file " << block_file
			<< " does not exist!";
		return false;
	}

	RGBAImage blocks;
	if (!blocks.readPNG(block_file.string())) {
		LOG(ERROR) << "Unable to load block images: Block image file " << block_file
			<< " not readable!";
		return false;
	}

	std::ifstream in(info_file.string());
	std::string first_line;
	
	std::getline(in, first_line);
	std::vector<std::string> parts = util::split(first_line, ' ');
	bool ok = true;
	int columns = 0;
	try {
		if (parts.size() == 3) {
			block_width = util::as<int>(parts[0]);
			block_height = util::as<int>(parts[1]);
			columns = util::as<int>(parts[2]);
		}
	} catch (std::invalid_argument& e) {
		ok = false;
	}
	if (!ok || parts.size() != 3) {
		LOG(ERROR) << "Invalid first line in block info file " << info_file << "!";
		LOG(ERROR) << "Line 1: '" << first_line << "'";
		return false;
	}

	int lineno = 2;
	for (std::string line; std::getline(in, line); lineno++) {
		line = util::trim(line);
		if (line.size() == 0) {
			continue;
		}

		std::vector<std::string> parts = util::split(line, ' ');
		if (parts.size() != 3) {
			LOG(ERROR) << "Invalid line in block info file '" << info_file << "'!";
			LOG(ERROR) << "Line " << lineno << ": '" << line << "'";
			return false;
		}
		
		std::string block_name = parts[0];
		std::string variant = parts[1];
		std::map<std::string, std::string> block_info = util::parseProperties(parts[2]);

		int image_index = util::as<int>(block_info["color"]);
		int image_uv_index = util::as<int>(block_info["uv"]);

		int x, y;
		x = (image_index % columns) * block_width;
		y = (image_index / columns) * block_height;
		RGBAImage image = blocks.clip(x, y, block_width, block_height);

		x = (image_uv_index % columns) * block_width;
		y = (image_uv_index / columns) * block_height;
		RGBAImage image_uv = blocks.clip(x, y, block_width, block_height);

		if (image.getWidth() != image_uv.getWidth() || image.getHeight() != image_uv.getHeight()) {
			LOG(ERROR) << "Size mismatch of block " << block_name;
			return false;
		}

		mc::BlockState block_state = mc::BlockState::parse(block_name, variant);
		uint16_t id = block_registry.getBlockID(block_state);
		BlockImage* b = new BlockImage();
		BlockImage& block = *b;
		block.image = image;
		block.uv_image = image_uv;
		block.is_air = block_info.count("is_air");

		block.is_full_water = false;
		if (block_name == "minecraft:water") {
			block.is_full_water = block_state.getProperty("level") == "0"
				|| block_state.getProperty("level") == "8";
		}
		if (block_name == "minecraft:full_water") {
			block.is_full_water = true;
		}

		block.is_ice = block_name == "minecraft:ice"
			|| block_name == "minecraft:blue_ice"
			|| block_name == "minecraft:packed_ice";

		block.is_biome = block_info.count("biome_type");
		if (block.is_biome) {
			block.is_masked_biome = block_info["biome_type"] == "masked";
			block.biome_color = util::as<ColorMapType>(block_info["biome_colors"]);
		}
		block.is_waterloggable = block_info.count("is_waterloggable");
		if (block.is_waterloggable) {
			block.is_waterlogged = block_state.getProperty("waterlogged", "true") == "true"
				|| block_state.getProperty("was_waterlogged") == "true";
			block.has_water_top = block_state.getProperty("waterlogged", "true") == "true"
				&& block_state.getProperty("was_waterlogged") != "true";

			mc::BlockState non_waterlogged = block_state;
			non_waterlogged.setProperty("waterlogged", "false");
			// required for tile renderer / render modes to know if a block is actually waterlogged
			non_waterlogged.setProperty("was_waterlogged", "true");
			block.non_waterlogged_id = block_registry.getBlockID(non_waterlogged);
		} else {
			block.is_waterlogged = false;
			block.has_water_top = false;
		}
		if (block_info.count("lighting_type")) {
			block.lighting_specified = true;
			block.lighting_type = util::as<LightingType>(block_info["lighting_type"]);
		}
		block.has_faulty_lighting = block_info.count("faulty_lighting");

		block.shadow_edges = -1;
		if (block_info.count("shadow_edges")) {
			block.shadow_edges = util::as<int>(block_info["shadow_edges"]);
		}

		if (block_images.size() < id + 1) {
			block_images.resize(id + 1, nullptr);
		}
		block_images[id] = b;

		auto properties = block_state.getProperties();
		for (auto it = properties.begin(); it != properties.end(); ++it) {
			block_registry.addKnownProperty(block_state.getName(), it->first);
		}

		//std::cout << block_name << " " << variant << std::endl;
	}
	in.close();

	prepareBlockImages();
	//runBenchmark();
	
	return true;
}

RGBAImage RenderedBlockImages::exportBlocks() const {
	/*
	std::vector<RGBAImage> blocks;

	for (auto it = block_images.begin(); it != block_images.end(); ++it) {
		blocks.push_back(it->second.image);
	}

	if (blocks.size() == 0) {
		return RGBAImage(0, 0);
	}

	int width = 16;
	int height = std::ceil((double) blocks.size() / width);
	int block_size = blocks[0].getWidth();
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
	*/
	return RGBAImage(1, 1);
}

const BlockImage& RenderedBlockImages::getBlockImage(uint16_t id) const {
	if (block_images.size() < id + 1) {
		const mc::BlockState& block_state = block_registry.getBlockState(id);
		
		if (!block_state.hasProperty("waterlogged")) {
			mc::BlockState test = mc::BlockState::parse(block_state.getName(), block_state.getVariantDescription());
			test.setProperty("waterlogged", "false");
			return getBlockImage(block_registry.getBlockID(test));
		}
		LOG(INFO) << "Unknown block " << block_state.getName() << " " << block_state.getVariantDescription();

		return unknown_block;
	}
	return *block_images[id];
}

void RenderedBlockImages::prepareBiomeBlockImage(const mc::BlockPos& pos, RGBAImage& image, const BlockImage& block, const Biome& biome) {
	uint32_t color = biome.getColor(pos, block.biome_color, resources.getColorMap(block.biome_color), false);

	if (block.is_masked_biome) {
		blockImageTint(image, block.biome_mask, color);
	} else {
		blockImageTint(image, color);
	}
}

void RenderedBlockImages::prepareBiomeBlockImage(RGBAImage& image, const BlockImage& block, uint32_t color) {

	if (block.is_masked_biome) {
		blockImageTint(image, block.biome_mask, color);
	} else {
		blockImageTint(image, color);
	}
}

int RenderedBlockImages::getTextureSize() const {
	return texture_size;
}

int RenderedBlockImages::getBlockSize() const {
	return block_width;
}

int RenderedBlockImages::getBlockWidth() const {
	return block_width;
}

int RenderedBlockImages::getBlockHeight() const {
	return block_height;
}

void RenderedBlockImages::prepareBlockImages() {
	uint16_t solid_id = block_registry.getBlockID(mc::BlockState("minecraft:unknown_block"));
	assert(block_images.size() > solid_id && block_images[solid_id] != nullptr);
	const BlockImage& solid = *block_images[solid_id];

	for (uint16_t id = 0; id < block_images.size(); ++id) {
		if (block_images[id] == nullptr) {
			continue;
		}
		BlockImage& block = *block_images[id];
		const mc::BlockState& block_state = block_registry.getBlockState(id);

		// blockImageTest(image, image_uv);

		// CornerValues values = {0.0, 1.0, 1.0, 0.0};
		// blockImageMultiply(image, image_uv, values, values, values);

		std::string name = block_state.getName();
		if (!util::endswith(name, "_biome_mask")) {
			blockImageMultiply(block.image, block.uv_image, darken_left, darken_right, 1.0);
		}

		block.side_mask = blockImageGetSideMask(block.uv_image);

		if (blockImageIsTransparent(block.image, solid.uv_image)) {
			//LOG(INFO) << block_state.getName() << " " << block_state.getVariantDescription() << " is transparent!";
			block.is_transparent = true;
		} else {
			// to visualize transparent blocks
			//blockImageTest(image, image_uv);
			//LOG(INFO) << block_state.getName() << " " << block_state.getVariantDescription() << " is not transparent!";
			block.is_transparent = false;
		}

		if (block.is_biome && block.is_masked_biome) {
			std::string mask_name = name + "_biome_mask";
			uint16_t mask_id = block_registry.getBlockID(mc::BlockState::parse(mask_name, block_state.getVariantDescription()));
			assert(block_images.size() > mask_id && block_images[mask_id] != nullptr);
			block.biome_mask = block_images[mask_id]->image;
		}

		if (!block.lighting_specified) {
			if (!block.is_transparent) {
				block.lighting_type = LightingType::SMOOTH;
			} else {
				if (block.is_full_water || block.is_ice) {
					block.lighting_type = LightingType::SMOOTH;
				} else if (block.is_waterlogged && block.has_water_top) {
					block.lighting_type = LightingType::SMOOTH_TOP_REMAINING_SIMPLE;
				} else {
					block.lighting_type = LightingType::SIMPLE;
				}
			}
		}

		if (block.shadow_edges == -1) {
			block.shadow_edges = !block.is_transparent;
		}
	}

	unknown_block = solid;
}

void RenderedBlockImages::runBenchmark() {
	LOG(INFO) << "Running benchmark";

	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;

	uint16_t id = block_registry.getBlockID(mc::BlockState::parse("minecraft:full_water", "up=false,south=false,west=false"));
	assert(block_images.size() > id && block_images[id] != nullptr);
	const BlockImage& solid = *block_images[id];

	uint32_t color = rgba(0x30, 0x59, 0xad, 0xff);

	CornerValues left = {1.0, 0.8, 0.5, 1.0};
	CornerValues right = {1.0, 0.6, 0.3, 0.8};
	CornerValues up = {0.5, 1.0, 0.6, 0.8};

	std::chrono::time_point<clock_> begin = clock_::now();

	for (size_t i = 0; i < 1000000; i++) {
		RGBAImage image = solid.image;
		
		// 5.841s
		//blockImageTint(image, image, 0x30, 0x59, 0xad, 0xff);
		
		// 3.441s
		// 3.072s mit nicem rgba_multiply
		// 2.876s mit alpha check als bitmaske und vergleich > 0
		//blockImageTint(image, image, color);

		// 1.597s (wenn alpha check weg!)
		// 1.590s (sonst auch!)
		// 1.105s mit nicem rgba_multiply
		//blockImageTint(image, color);
		
		// 9.534s mit rgb_multiply_scalar
		// 6.345s mit rgb_multiply_scalar inline
		// 6.377s mit rgba_multiply_scalar ohne f+1
		// 6.126s doch wenn der alpha check drin ist
		blockImageMultiply(image, solid.uv_image, left, right, up);
	}
	
	double elapsed = std::chrono::duration_cast<second_>(clock_::now() - begin).count();
	LOG(INFO) << "took " << elapsed << "s";
	exit(0);
}

}
}
