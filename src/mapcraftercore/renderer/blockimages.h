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

#ifndef BLOCKIMAGES_H_
#define BLOCKIMAGES_H_

#include "blocktextures.h"
#include "image.h"
#include "../mc/pos.h"

#include <boost/filesystem.hpp>
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace fs = boost::filesystem;

namespace mapcrafter {

namespace mc {
class BlockState;
class BlockStateRegistry;
}

namespace renderer {

class Biome;

enum class ColorMapType {
	GRASS,
	FOLIAGE,
	FOLIAGE_FLIPPED,
	WATER
};

struct ColorMap {
	ColorMap()
		: colors({0, 0, 0}) {}

	bool parse(const std::string& str);
	uint32_t getColor(float x, float y) const;

	// colors of colormap triangle points
	std::array<uint32_t, 3> colors;
};

/**
 * This is an interface for a class responsible for generating and managing the block
 * images which are required to render a map.
 */
class BlockImages {
public:
	virtual ~BlockImages();

	virtual int getBlockSize() const = 0;
};

typedef std::array<float, 4> CornerValues;

// TODO rename these maybe
static const uint8_t FACE_LEFT_INDEX = ((float) 255.0 / 6.0) * 1;
static const uint8_t FACE_RIGHT_INDEX = ((float) 255.0 / 6.0) * 4;
static const uint8_t FACE_UP_INDEX = ((float) 255.0 / 6.0) * 2;

void blockImageTest(RGBAImage& block, const RGBAImage& uv_mask);
void blockImageMultiply(RGBAImage& block, const RGBAImage& uv_mask,
		float factor_left, float factor_right, float factor_up);
void blockImageMultiplyExcept(RGBAImage& block, const RGBAImage& uv_mask,
		uint8_t except_face, float factor);
void blockImageMultiply(RGBAImage& block, const RGBAImage& uv_mask,
		const CornerValues& factors_left, const CornerValues& factors_right, const CornerValues& factors_up);
void blockImageMultiply(RGBAImage& block, uint8_t factor);
void blockImageTint(RGBAImage& block, const RGBAImage& mask,
		uint32_t color);
// TODO maybe this should be named something with multiply too
void blockImageTint(RGBAImage& block, uint32_t color);
void blockImageTintHighContrast(RGBAImage& block, uint32_t color);
void blockImageTintHighContrast(RGBAImage& block, const RGBAImage& mask, int face, uint32_t color);
void blockImageBlendTop(RGBAImage& block, const RGBAImage& uv_mask,
		const RGBAImage& top, const RGBAImage& top_uv_mask);
void blockImageShadowEdges(RGBAImage& block, const RGBAImage& uv_mask,
		uint8_t north, uint8_t south, uint8_t east, uint8_t west, uint8_t bottom);
bool blockImageIsTransparent(RGBAImage& block, const RGBAImage& uv_mask);
std::array<bool, 3> blockImageGetSideMask(const RGBAImage& uv);

enum class LightingType {
	NONE,
	SIMPLE,
	SMOOTH,
	SMOOTH_TOP_REMAINING_SIMPLE,
	SMOOTH_BOTTOM,
};

struct BlockImage {
	// TODO
	// this needs some order and refactoring
	BlockImage()
		: lighting_specified(false) {}

	RGBAImage image, uv_image;
	std::array<bool, 3> side_mask;
	bool is_transparent, is_air, is_full_water, is_ice;
	
	bool is_biome;
	bool is_masked_biome;
	ColorMapType biome_color;
	ColorMap biome_colormap;
	RGBAImage biome_mask;

	bool is_waterloggable;
	bool is_waterlogged;
	bool has_water_top;
	uint16_t non_waterlogged_id;

	bool is_lily_pad;

	bool lighting_specified;
	LightingType lighting_type;
	bool has_faulty_lighting;

	int shadow_edges;
};

class RenderedBlockImages : public BlockImages {
public:
	// OLD METHODS
	virtual void setRotation(int rotation) {}
	virtual void setRenderSpecialBlocks(bool render_unknown_blocks,
			bool render_leaves_transparent) {}
	//virtual RGBAImage exportBlocks() const {}
	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const { return false; };
	virtual bool hasBlock(uint16_t id, uint16_t) const { return true; };
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data, uint16_t extra_data = 0) const { return unknown_block.image; };
	virtual RGBAImage getBiomeBlock(uint16_t id, uint16_t data, const Biome& biome, uint16_t extra_data = 0) const { return unknown_block.image; };
	virtual int getMaxWaterPreblit() const { return 0; };
	//virtual int getBlockSize() const {};

	RenderedBlockImages(mc::BlockStateRegistry& block_registry);
	~RenderedBlockImages();

	void setBlockSideDarkening(float darken_left, float darken_right);

	bool loadBlockImages(fs::path block_dir, std::string view, int rotation, int texture_size);
	virtual RGBAImage exportBlocks() const;

	const BlockImage& getBlockImage(uint16_t id) const;
	void prepareBiomeBlockImage(RGBAImage& image, const BlockImage& block, uint32_t color);

	virtual int getTextureSize() const;
	virtual int getBlockSize() const;
	virtual int getBlockWidth() const;
	virtual int getBlockHeight() const;

private:
	void prepareBlockImages();
	void runBenchmark();

	mc::BlockStateRegistry& block_registry;

	float darken_left, darken_right;

	int texture_size;
	int block_width, block_height;
	// Mapcrafter-local block ID -> BlockImage (image, uv_image, is_transparent, ...)
	//std::unordered_map<uint16_t, BlockImage> block_images;
	std::vector<BlockImage*> block_images;
	BlockImage unknown_block;
};

}
}

#endif /* BLOCKIMAGES_H_ */
