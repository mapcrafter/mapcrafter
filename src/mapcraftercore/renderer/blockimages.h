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

#ifndef BLOCKIMAGES_H_
#define BLOCKIMAGES_H_

#include "blocktextures.h"
#include "image.h"

#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace mapcrafter {
namespace renderer {

class Biome;

// extra data starting at the 5. bit
const int DATA_NORTH = 16;
const int DATA_EAST = 32;
const int DATA_SOUTH = 64;
const int DATA_WEST = 128;
const int DATA_TOP = 256;

// the last three bits of 2 bytes
const int EDGE_NORTH = 8192;
const int EDGE_EAST = 16384;
const int EDGE_BOTTOM = 32768;

// some data values and stuff for special blocks
const int GRASS_SNOW = 16;

const int DOOR_NORTH = 16;
const int DOOR_SOUTH = 32;
const int DOOR_EAST = 64;
const int DOOR_WEST = 128;
const int DOOR_TOP = 256;
const int DOOR_FLIP_X = 512;

const int LARGECHEST_DATA_LARGE = 256;
const int LARGECHEST_DATA_LEFT = 512;

const int REDSTONE_NORTH = 16;
const int REDSTONE_EAST = 32;
const int REDSTONE_SOUTH = 64;
const int REDSTONE_WEST = 128;
const int REDSTONE_TOPNORTH = 256;
const int REDSTONE_TOPEAST = 512;
const int REDSTONE_TOPSOUTH = 1024;
const int REDSTONE_TOPWEST = 2048;
const int REDSTONE_POWERED = 4096;

const int LARGEPLANT_TOP = 16;

/**
 * The textures of a single chest, just an array with three images.
 */
class ChestTextures : public std::array<RGBAImage, 3> {
public:
	/**
	 * Loads the textures from a chest texture file, you have to specify a texture size
	 * to use.
	 */
	bool load(const std::string& filename, int texture_size);

	static const int FRONT = 0;
	static const int SIDE = 1;
	static const int TOP = 2;
};

/**
 * The textures of a double chest, just an array with seven images.
 */
class DoubleChestTextures : public std::array<RGBAImage, 7> {
public:
	/**
	 * Loads the textures from a double chest texture file, you have to specify a texture
	 * size to use.
	 */
	bool load(const std::string& filename, int texture_size);

	static const int FRONT_LEFT = 0;
	static const int FRONT_RIGHT = 1;
	static const int SIDE = 2;
	static const int TOP_LEFT = 3;
	static const int TOP_RIGHT = 4;
	static const int BACK_LEFT = 5;
	static const int BACK_RIGHT = 6;
};

/**
 * This class is responsible for loading the required texture files from a texture dir.
 */
class TextureResources {
public:
	TextureResources();
	~TextureResources();

	/**
	 * Returns the used texture size (defaults to 12).
	 */
	int getTextureSize() const;

	/**
	 * Returns the used texture blur radius (defaults to 0).
	 */
	int getTextureBlur() const;

	/**
	 * Loads the texture files from a texture directory and returns if it was successful.
	 * Error/warning messages will be logged with the logging facility if there is
	 * something wrong with the textures.
	 *
	 * You can also specify a texture size and a texture blur radius to apply to the
	 * texture files. A texture size of 12px and a texture blur radius of 0px is used
	 * per default.
	 */
	bool loadTextures(const std::string& texture_dir, int texture_size = 12,
			int texture_blur = 0);

	/**
	 * Returns the loaded block texture files.
	 */
	const BlockTextures& getBlockTextures() const;

	/**
	 * Returns the loaded endportal texture file.
	 */
	const RGBAImage& getEndportalTexture() const;

	/**
	 * Returns the loaded texture files of the normal chest.
	 */
	const ChestTextures& getNormalChest() const;

	/**
	 * Returns the loaded texture files of the normal double chest.
	 */
	const DoubleChestTextures& getNormalDoubleChest() const;

	/**
	 * Returns the loaded texture files of the ender chest.
	 */
	const ChestTextures& getEnderChest() const;

	/**
	 * Returns the loaded texture files of the trapped chest.
	 */
	const ChestTextures& getTrappedChest() const;

	/**
	 * Returns the loaded textures files of the trapped double chest.
	 */
	const DoubleChestTextures& getTrappedDoubleChest() const;

	/**
	 * Returns the foliage color biomes texture.
	 */
	const RGBAImage& getFoliageColors() const;

	/**
	 * Returns the grass color biomes texture.
	 */
	const RGBAImage& getGrassColors() const;

private:
	/**
	 * Loads the chest textures from the supplied files.
	 */
	bool loadChests(const std::string& normal_png, const std::string& normal_double_png,
			const std::string& ender_png, const std::string& trapped_png,
			const std::string& trapped_double_png);

	/**
	 * Loads the biome color textures from the supplied files.
	 */
	bool loadColors(const std::string& foliage_png, const std::string& grass_png);

	/**
	 * Loads the block textures and the endportal texture from the supplied directory/file.
	 */
	bool loadBlocks(const std::string& block_dir, const std::string& endportal_png);

	// used texture size, blur
	int texture_size, texture_blur;

	// all the loaded texture images
	BlockTextures block_textures;
	RGBAImage empty_texture;
	RGBAImage endportal_texture;

	ChestTextures normal_chest, ender_chest, trapped_chest;
	DoubleChestTextures normal_double_chest, trapped_double_chest;

	RGBAImage foliage_colors, grass_colors;
};

/**
 * Responsible for generating and managing the block images which are required to render
 * a map.
 */
class BlockImages {
public:
	virtual ~BlockImages();

	virtual void setRotation(int rotation) = 0;
	virtual void setRenderSpecialBlocks(bool render_unknown_blocks,
			bool render_leaves_transparent) = 0;

	virtual void loadBlocks(const TextureResources& resources) = 0;
	virtual bool saveBlocks(const std::string& filename) = 0;

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const = 0;
	virtual bool hasBlock(uint16_t id, uint16_t) const = 0;
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const = 0;

	/**
	 * Returns the block image of a block whose appearance is depending on the biome.
	 */
	virtual RGBAImage getBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const = 0;

	virtual int getMaxWaterNeededOpaque() const = 0;
	virtual const RGBAImage& getOpaqueWater(bool south, bool west) const = 0;

	virtual int getTextureSize() const = 0;
	virtual int getBlockSize() const = 0;
};

/**
 * Implements most of the methods of the BlockImages class which are related to managing
 * the generated block images. You just have to implement some methods to generate all
 * the required block images.
 */
class AbstractBlockImages : public BlockImages {
public:
	AbstractBlockImages();
	virtual ~AbstractBlockImages();

	virtual void setRotation(int rotation);
	virtual void setRenderSpecialBlocks(bool render_unknown_blocks,
			bool render_leaves_transparent);

	/**
	 * Creates the block images with the supplied textures. Handles the creation of the
	 * blocks by calling the abstract methods (createBlocks(), createBiomeBlocks(), ...).
	 */
	virtual void loadBlocks(const TextureResources& resources);

	/**
	 * Saves an image with all created blocks.
	 */
	virtual bool saveBlocks(const std::string& filename);

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const;
	virtual bool hasBlock(uint16_t id, uint16_t) const;
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const;

	virtual RGBAImage getBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const;

	virtual int getMaxWaterNeededOpaque() const = 0;
	virtual const RGBAImage& getOpaqueWater(bool south, bool west) const = 0;

	virtual int getTextureSize() const;
	virtual int getBlockSize() const = 0;

protected:
	/**
	 * Filters out unnecessary parts of the block data since of some blocks not everything
	 * of the block data is relevant.
	 */
	virtual uint16_t filterBlockData(uint16_t id, uint16_t data) const = 0;

	/**
	 * Checks whether a block image contains transparent pixels. This is method is called
	 * for every block that is stored with the setBlockImage-method.
	 */
	virtual bool isImageTransparent(const RGBAImage& block) const = 0;

	/**
	 * This method stores a generated block image in the map of generated block images.
	 * It also checks with the isBlockTransparent-method whether a block image is
	 * transparent. You can overwrite this method if you need special handling for
	 * generated blocks.
	 */
	virtual void setBlockImage(uint16_t id, uint16_t data, const RGBAImage& block);

	/**
	 * This method should create a block image of an unknown block (i.e. just a simple
	 * #ff0000 block). You can access is later as the unknown_block variable. The method
	 * will be called depending on whether rendering of unknown block is enabled or not.
	 * If rendering of unknown blocks is disabled, the unknown_block is just a transparent
	 * block image.
	 */
	virtual RGBAImage createUnknownBlock() const = 0;

	/**
	 * Implement this and create the biome-specific version of the specified block.
	 * This method is called by the createBiomeBlocks() method and the result is stored
	 * by it, so you don't need to cache any biome block images, just create them in here.
	 */
	virtual RGBAImage createBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const = 0;

	/**
	 * You have to create all your block images in this method and store them with the
	 * setBlockImage-method.
	 */
	virtual void createBlocks() = 0;

	/**
	 * Creates the biome block images by iterating the generated blocks (the method is
	 * called after createBlocks()), checking with the Biome::isBiomeBlock(id, data)
	 * function if this is a biome block and then calling the createBiomeBlock-method
	 * for every biome. The biome blocks are stored in the biome_images map then.
	 *
	 * Overwrite this if you need special handling for biome blocks.
	 */
	virtual void createBiomeBlocks();

	int texture_size;
	int rotation;
	bool render_unknown_blocks;
	bool render_leaves_transparent;

	TextureResources resources;
	RGBAImage empty_texture;

	// map of block images
	// key is a 32 bit integer, first two bytes id, second two bytes data
	std::unordered_map<uint32_t, RGBAImage> block_images;

	// map of biome block images, first four bytes id+data, next byte is the biome id
	std::unordered_map<uint64_t, RGBAImage> biome_images;

	// set of id/data block combinations, which contain transparency
	std::unordered_set<uint32_t> block_transparency;
	RGBAImage unknown_block;
};

}
}

#endif /* BLOCKIMAGES_H_ */
