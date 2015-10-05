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

// general stuff both render views can use
const int FACE_NORTH = 1;
const int FACE_EAST = 2;
const int FACE_SOUTH = 4;
const int FACE_WEST = 8;
const int FACE_TOP = 16;
const int FACE_BOTTOM = 32;

// extra data starting at the 5. bit
const int DATA_NORTH = 1 << 4;
const int DATA_EAST = 1 << 5;
const int DATA_SOUTH = 1 << 6;
const int DATA_WEST = 1 << 7;
const int DATA_TOP = 1 << 8;

// the last three bits of 2 bytes
const int EDGE_NORTH = 1 << 13;
const int EDGE_EAST = 1 << 14;
const int EDGE_BOTTOM = 1 << 15;

// some data values and stuff for special blocks
const int FULL_WATER = DATA_EAST;
const int FULL_WATER_SOUTH = DATA_SOUTH;
const int FULL_WATER_WEST = DATA_WEST;
const int FULL_WATER_TOP = DATA_TOP;
const int GRASS_SNOW = 16;

const int OPAQUE_WATER = DATA_NORTH;
const int OPAQUE_WATER_SOUTH = DATA_SOUTH;
const int OPAQUE_WATER_WEST = DATA_WEST;

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
	 * You can also specify a texture size, a texture blur radius to apply to
	 * the texture files and a custom water opacity. A texture size of 12px and
	 * a texture blur radius of 0px is used per default. The water opacity must be within
	 * range [0; 1] (Default 1) and is a factor which is applied to the opacity of the
	 * water texture. With 1.0 the alpha channel of the texture is not changed, with 0.0
	 * the texture will be completely transparent.
	 */
	bool loadTextures(const std::string& texture_dir, int texture_size = 12,
			int texture_blur = 0, double water_opacity = 1.0);

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
	double water_opacity;

	// all the loaded texture images
	BlockTextures block_textures;
	RGBAImage empty_texture;
	RGBAImage endportal_texture;

	ChestTextures normal_chest, ender_chest, trapped_chest;
	DoubleChestTextures normal_double_chest, trapped_double_chest;

	RGBAImage foliage_colors, grass_colors;
};

/**
 * This is an interface for a class responsible for generating and managing the block
 * images which are required to render a map.
 */
class BlockImages {
public:
	virtual ~BlockImages();

	/**
	 * Sets the rotation of the world these block images are used for.
	 * Call this method before you call the loadBlocks-method.
	 */
	virtual void setRotation(int rotation) = 0;

	/**
	 * Sets the options how some special blocks should be rendered.
	 * Call this method before you call the loadBlocks-method.
	 */
	virtual void setRenderSpecialBlocks(bool render_unknown_blocks,
			bool render_leaves_transparent) = 0;

	/**
	 * Generates the block images with the supplied textures.
	 */
	virtual void generateBlocks(const TextureResources& resources) = 0;

	/**
	 * Exports the block images by just blitting all the generated block images together
	 * to a big image.
	 */
	virtual RGBAImage exportBlocks() const = 0;

	/**
	 * Returns whether a block is transparent, i.e. the block image contains transparent
	 * pixels.
	 */
	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const = 0;

	/**
	 * Returns whether there is a block image of a specific block.
	 */
	virtual bool hasBlock(uint16_t id, uint16_t) const = 0;

	/**
	 * Returns the block image of a specific block.
	 */
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const = 0;

	/**
	 * Returns the block image of a block whose appearance is depending on the biome.
	 */
	virtual RGBAImage getBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const = 0;

	/**
	 * Returns how many blocks of water are needed in a row until the water becomes (almost)
	 * opaque and a preblit water block can be used instead of wasting performance with
	 * alphablitting. This applies only to the behavior of rendering water in the plain
	 * render mode.
	 *
	 * // TODO maybe move this to AbstractBlockImages and also create an AbstractTileRenderer?
	 */
	virtual int getMaxWaterPreblit() const = 0;

	/**
	 * Returns the used texture size. This should be the texture size of the textures
	 * that were used to generate the block images (resources.getTextureSize() when
	 * generateBlocks-method is called).
	 */
	virtual int getTextureSize() const = 0;

	/**
	 * Returns the size (width/height) of the generated block images.
	 */
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
	 * Implements the method of the interface. Handles the creation of the blocks by
	 * calling the abstract methods (createBlocks(), createBiomeBlocks(), ...).
	 */
	virtual void generateBlocks(const TextureResources& resources);

	/**
	 * Implements the method of the interface. Blits all the block images returned by the
	 * getExportBlocks-method to a big image with 16 block images per row.
	 */
	virtual RGBAImage exportBlocks() const;

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const;
	virtual bool hasBlock(uint16_t id, uint16_t) const;
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const;

	virtual RGBAImage getBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const;

	virtual int getMaxWaterPreblit() const;

	virtual int getTextureSize() const;
	virtual int getBlockSize() const = 0;

protected:
	/**
	 * Filters out unnecessary parts of the block data since of some blocks not everything
	 * of the block data is relevant. This base class already implements some general
	 * block data filtering, that's why you should call this parent method in every child
	 * class.
	 */
	virtual uint16_t filterBlockData(uint16_t id, uint16_t data) const;

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

	virtual int createOpaqueWater() = 0;

	/**
	 * Returns the blocks which should be (in that order) exported by the exportBlocks-method.
	 * You can overwrite this if you want to export other blocks as well / have other
	 * things that need special handling. By default this method just returns all the
	 * blocks in block_images ordered by id and data.
	 */
	virtual std::vector<RGBAImage> getExportBlocks() const;

	// some options that were passed to us
	int texture_size;
	int rotation;
	bool render_unknown_blocks;
	bool render_leaves_transparent;

	TextureResources resources;
	RGBAImage empty_texture;

	// map of block images
	// key is a 32 bit integer, first two bytes id, second two bytes data
	std::unordered_map<uint32_t, RGBAImage> block_images;

	// comparator to sort block images by their id/data extracted  from the uint32_t keys
	// used to sort blocks when exporting them
	struct block_images_comparator {
		bool operator()(uint32_t key1, uint32_t key2) const {
			uint16_t id1 = key1 & 0xffff;
			uint16_t id2 = key2 & 0xffff;
			if (id1 != id2)
				return id1 < id2;
			uint16_t data1 = (key1 & 0xffff0000) >> 16;
			uint16_t data2 = (key2 & 0xffff0000) >> 16;
			return data1 < data2;
		}
	};

	// map of biome block images
	// key is a 64 bit integer, first four bytes id, data, next byte is the biome id
	std::unordered_map<uint64_t, RGBAImage> biome_images;

	// set of blocks (id, data as key again) which contain transparency
	std::unordered_set<uint32_t> block_transparency;
	RGBAImage unknown_block;

	int max_water_preblit;
};

}
}

#endif /* BLOCKIMAGES_H_ */
