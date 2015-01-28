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

#include "biomes.h"
#include "blocktextures.h"
#include "image.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace mapcrafter {
namespace renderer {

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

class BlockImageTextureResources {
public:
	BlockImageTextureResources();
	~BlockImageTextureResources();

	void setTextureSize(int texture_size, int blur);

	bool loadChests(const std::string& normal, const std::string& normal_double,
			const std::string& ender,
			const std::string& trapped, const std::string& trapped_double);
	bool loadColors(const std::string& foliagecolor, const std::string& grasscolor);
	bool loadOther(const std::string& endportal);
	bool loadBlocks(const std::string& block_dir);
	bool loadAll(const std::string& textures_dir);
	bool saveBlocks(const std::string& filename);

	const BlockTextures& getBlockTextures() const;
	const RGBAImage& getEndportalTexture() const;

	const RGBAImage* getNormalChest() const;
	const RGBAImage* getNormalDoubleChest() const;
	const RGBAImage* getEnderChest() const;
	const RGBAImage* getTrappedChest() const;
	const RGBAImage* getTrappedDoubleChest() const;

	const RGBAImage& getFoliageColors() const;
	const RGBAImage& getGrassColors() const;

	static const int CHEST_FRONT = 0;
	static const int CHEST_SIDE = 1;
	static const int CHEST_TOP = 2;

	static const int LARGECHEST_FRONT_LEFT = 0;
	static const int LARGECHEST_FRONT_RIGHT = 1;
	static const int LARGECHEST_SIDE = 2;
	static const int LARGECHEST_TOP_LEFT = 3;
	static const int LARGECHEST_TOP_RIGHT = 4;
	static const int LARGECHEST_BACK_LEFT = 5;
	static const int LARGECHEST_BACK_RIGHT = 6;

private:
	int texture_size;
	int blur;

	BlockTextures textures;
	RGBAImage empty_texture;
	RGBAImage endportal_texture;

	RGBAImage chest_normal[3], chest_normal_double[7];
	RGBAImage chest_ender[3];
	RGBAImage chest_trapped[3], chest_trapped_double[7];

	RGBAImage foliagecolors, grasscolors;
};

class BlockImages {
public:
	virtual ~BlockImages();

	// TODO replace this later by passing a texture resources object directly
	virtual void setTextureSize(int texture_size, int blur) = 0;
	virtual void setRotation(int rotation) = 0;
	virtual void setRenderSpecialBlocks(bool render_unknown_blocks,
			bool render_leaves_transparent) = 0;

	virtual bool loadAll(const std::string& textures_dir) = 0;
	virtual bool saveBlocks(const std::string& filename) = 0;

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const = 0;
	virtual bool hasBlock(uint16_t id, uint16_t) const = 0;
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const = 0;
	virtual RGBAImage getBiomeDependBlock(uint16_t id, uint16_t data, const Biome& biome) const = 0;

	virtual int getMaxWaterNeededOpaque() const = 0;
	virtual const RGBAImage& getOpaqueWater(bool south, bool west) const = 0;

	virtual int getTextureSize() const = 0;
	virtual int getBlockSize() const = 0;
};

class AbstractBlockImages : public BlockImages {
public:
	AbstractBlockImages();
	virtual ~AbstractBlockImages();

	virtual void setTextureSize(int texture_size, int blur);
	virtual void setRotation(int rotation);
	virtual void setRenderSpecialBlocks(bool render_unknown_blocks,
			bool render_leaves_transparent);

	virtual bool loadAll(const std::string& textures_dir);
	virtual bool saveBlocks(const std::string& filename);

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const;
	virtual bool hasBlock(uint16_t id, uint16_t) const;
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const;
	virtual RGBAImage getBiomeDependBlock(uint16_t id, uint16_t data, const Biome& biome) const;

	virtual int getMaxWaterNeededOpaque() const = 0;
	virtual const RGBAImage& getOpaqueWater(bool south, bool west) const = 0;

	virtual int getTextureSize() const;
	virtual int getBlockSize() const = 0;

protected:
	virtual uint16_t filterBlockData(uint16_t id, uint16_t data) const = 0;
	virtual bool checkImageTransparency(const RGBAImage& block) const = 0;

	// you can overwrite this if you need special handling of generated block images
	virtual void setBlockImage(uint16_t id, uint16_t data, const RGBAImage& block);

	virtual RGBAImage createUnknownBlock() const = 0;
	virtual RGBAImage createBiomeBlock(uint16_t id, uint16_t data, const Biome& biome_data) const = 0;

	virtual void createBlocks() = 0;
	virtual void createBiomeBlocks() = 0;

	int texture_size;
	int blur;
	int rotation;
	bool render_unknown_blocks;
	bool render_leaves_transparent;

	BlockImageTextureResources resources;
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
