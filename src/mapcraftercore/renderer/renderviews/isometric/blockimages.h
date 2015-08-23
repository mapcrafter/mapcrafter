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

#ifndef ISOMETRIC_BLOCKIMAGES_H_
#define ISOMETRIC_BLOCKIMAGES_H_

#include "../../blockimages.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 * For the rendering we need to transform the Minecraft textures to some kind of block
 * images.
 *
 * Here you can see an example of a block image (with texture size = 6)
 *
 * L = left texture
 * R = right texture
 * T = top texture
 *
 *                                       TT       |
 * LLLLLL     RRRRRR     TTTTTT        TTTTTT     |
 * LLLLLL     RRRRRR     TTTTTT      TTTTTTTTTT   |
 * LLLLLL  +  RRRRRR  +  TTTTTT  =  LTTTTTTTTTTR  |
 * LLLLLL     RRRRRR     TTTTTT     LLLTTTTTTRRR  |
 * LLLLLL     RRRRRR     TTTTTT     LLLLLTTRRRRR  | 2*texture-size
 * LLLLLL     RRRRRR     TTTTTT     LLLLLLRRRRRR  |
 *                                  LLLLLLRRRRRR  |
 * ------                           LLLLLLRRRRRR  |
 * texture size                      LLLLLRRRRR   |
 *                                     LLLRRR     |
 *                                       LR       |
 *
 *                                  ------------
 *                                  2*texture size
 *
 * On a normal map, the left side of a block image is west and the right side is south.
 * The block images have a size of 2*texture size x 2*texture size (this is called block
 * size).
 *
 * To transform the pixels from a texture to the pixels of a block image side, iterators
 * are used. The iterators iterate through the pixels of the source image and
 * calculate the position of the pixel in the block image.
 *
 * The SideFaceIterator class is used to transform the textures to the sides:
 *
 * ABCDEF      A                F
 * ABCDEF      ABC            DEF
 * ABCDEF      ABCDE        BCDEF
 * ABCDEF  =>  ABCDEF  or  ABCDEF
 * ABCDEF      ABCDEF      ABCDEF
 * ABCDEF      ABCDEF      ABCDEF
 *              BCDEF      ABCDE
 *                DEF      ABC
 *                  F      A
 *
 * The TopFaceIterator class is used to transform the textures to the top side:
 *
 * ABCDEF          AB
 * ABCDEF        AABBCD
 * ABCDEF  =>  AABBCCDDEF
 * ABCDEF      ABCCDDEEFF
 * ABCDEF        CDEEFF
 * ABCDEF          EF
 */

namespace mapcrafter {
namespace renderer {

// count of rotations needed for the block images
// depending on the north direction
const int NORTH_TOP_LEFT = 0;
const int NORTH_TOP_RIGHT = 1;
const int NORTH_BOTTOM_RIGHT = 2;
const int NORTH_BOTTOM_LEFT = 3;

enum class SlabType {
	STONE, STONE2, WOOD
};

/**
 * The base for an iterator to transform the pixels of a source image to the pixels of a
 * destination image.
 */
class FaceIterator {
private:
	bool is_end;
protected:
	int size;
public:
	FaceIterator();
	FaceIterator(int size);
	virtual ~FaceIterator();

	void next();
	bool end() const;

	// current position in the source image
	int src_x;
	int src_y;
	// current position in the destination image
	int dest_x;
	int dest_y;
};

/**
 * Transforms a texture to the left or right face of a block image.
 */
class SideFaceIterator: public FaceIterator {
private:
	int side;
	int delta;
public:
	SideFaceIterator();
	SideFaceIterator(int size, int side);

	void next();

	static const int LEFT = 1;
	static const int RIGHT = -1;
};

/**
 * Transforms a texture to the top face of a block image.
 */
class TopFaceIterator: public FaceIterator {
private:
	int next_x;
	int next_y;
public:
	TopFaceIterator();
	TopFaceIterator(int size);

	void next();
};

void blitFace(RGBAImage& image, int face, const RGBAImage& texture,
		int xoff = 0, int yoff = 0,
		bool darken = true, double dleft = 0.6, double dright = 0.75);
void blitItemStyleBlock(RGBAImage& image, const RGBAImage& north_south, const RGBAImage& east_west);

void rotateImages(RGBAImage& north, RGBAImage& south, RGBAImage& east, RGBAImage& west, int rotation);

/**
 * A block with 6 face textures, used to create the block images and also rotate them.
 */
class BlockImage {
private:
	int type;
	RGBAImage faces[6];
	int x_offsets[6], y_offsets[6];
	RGBAImage empty_image;
public:
	BlockImage(int type = NORMAL);
	~BlockImage();

	BlockImage& setFace(int face, const RGBAImage& texture, int xoff = 0, int yoff = 0);
	const RGBAImage& getFace(int face) const;
	int getXOffset(int face) const;
	int getYOffset(int face) const;

	BlockImage rotate(int count) const;
	RGBAImage buildImage(double dleft, double dright) const;

	static const int NORMAL = 1;
	static const int ITEM_STYLE = 2;
};

/**
 * This class is responsible for reading the Minecraft textures and creating the block
 * images.
 */
class IsometricBlockImages : public AbstractBlockImages {
public:
	IsometricBlockImages();
	~IsometricBlockImages();

	void setBlockSideDarkening(double left, double right);

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const;

	/**
	 * We need to overwrite this because there is a special case for the snowy grass block.
	 */
	virtual RGBAImage getBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const;

	virtual int getBlockSize() const;

protected:
	// factor to darken the side faces
	// defaults to 0.75 and 0.6
	double dleft, dright;

	RGBAImage shadow_edge_masks[4];

	virtual uint16_t filterBlockData(uint16_t id, uint16_t data) const;
	virtual bool isImageTransparent(const RGBAImage& block) const;
	void addBlockShadowEdges(uint16_t id, uint16_t data, const RGBAImage& block);

	void setBlockImage(uint16_t id, uint16_t data, const BlockImage& block);
	virtual void setBlockImage(uint16_t id, uint16_t data, const RGBAImage& block);

	uint32_t darkenLeft(uint32_t pixel) const;
	uint32_t darkenRight(uint32_t pixel) const;

	RGBAImage buildImage(const BlockImage& image) const;

	BlockImage buildSmallerBlock(const RGBAImage& left_texture, const RGBAImage& right_texture,
	        const RGBAImage& top_texture, int y1, int y2) const;

	RGBAImage buildStairsSouth(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildStairsNorth(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildStairsWest(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildStairsEast(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildUpsideDownStairsNorth(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildUpsideDownStairsSouth(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildUpsideDownStairsEast(const RGBAImage& texture,
			const RGBAImage& texture_top);
	RGBAImage buildUpsideDownStairsWest(const RGBAImage& texture,
			const RGBAImage& texture_top);

	void buildCustomTextures();

	void createBlock(uint16_t id, uint16_t data, const RGBAImage& texture);
	void createBlock(uint16_t id, uint16_t data, const RGBAImage& side_texture,
	        const RGBAImage& top_texture);
	void createBlock(uint16_t id, uint16_t data, const RGBAImage& left_texture,
	        const RGBAImage& right_texture, const RGBAImage& top_texture);

	void createSmallerBlock(uint16_t id, uint16_t data, const RGBAImage& left_texture,
	        const RGBAImage& right_texture, const RGBAImage& top_texture, int y1, int y2);
	void createSmallerBlock(uint16_t id, uint16_t data, const RGBAImage& side_face,
	        const RGBAImage& top_texture, int y1, int y2);
	void createSmallerBlock(uint16_t id, uint16_t data, const RGBAImage& texture, int y1, int y2);
	void createRotatedBlock(uint16_t id, uint16_t extra_data, const RGBAImage& front_texture,
	        const RGBAImage& side_texture, const RGBAImage& top_texture);
	void createRotatedBlock(uint16_t id, uint16_t extra_data, const RGBAImage& front_texture,
	        const RGBAImage& back_texture, const RGBAImage& side_texture,
	        const RGBAImage& top_texture);
	void createItemStyleBlock(uint16_t id, uint16_t data, const RGBAImage& texture);
	void createItemStyleBlock(uint16_t id, uint16_t data, const RGBAImage& north_south,
	        const RGBAImage& east_west);
	void createSingleFaceBlock(uint16_t id, uint16_t data, int face,
	        const RGBAImage& texture);

	void createGrassBlock(); // id 2
	void createWater(); // id 8, 9
	void createLava(); // id 10, 11
	void createWood(uint16_t id, uint16_t data, const RGBAImage& side_texture, const RGBAImage& top_texture); // id 17
	void createLeaves(); // id 18
	void createGlass(uint16_t id, uint16_t data, const RGBAImage& texture); // id 20, 95
	void createDispenserDropper(uint16_t id, const RGBAImage& front); // id 23, 158
	void createBed(); // id 26
	void createStraightRails(uint16_t id, uint16_t extra_data, const RGBAImage& texture); // id 27, 28, 66
	void createPiston(uint16_t id, bool sticky); // id 29, 33
	void createSlabs(uint16_t id, SlabType type, bool double_slabs); // id 43, 44, 125, 126
	void createTorch(uint16_t, const RGBAImage& texture); // id 50, 75, 76
	void createStairs(uint16_t id, const RGBAImage& texture,
			const RGBAImage& texture_top); // id 53, 67, 108, 109, 114, 128, 134, 135, 136, 180
	void createStairs(uint16_t id, const RGBAImage& texture);
	void createChest(uint16_t id, const ChestTextures& textures); // id 54, 95, 130
	void createDoubleChest(uint16_t id, const DoubleChestTextures& textures); // id 54
	void createRedstoneWire(uint16_t id, uint16_t extra_data,
			uint8_t r, uint8_t g, uint8_t b); // id 55
	void createDoor(uint16_t id, const RGBAImage& bottom, const RGBAImage& top); // id 64, 71
	void createRails(); // id 66
	void createButton(uint16_t id, const RGBAImage& tex); // id 77, 143
	void createSnow(); // id 78
	void createIce(uint8_t id); // id 79
	void createCactus(); // id 81
	void createFence(uint16_t id, uint16_t extra_data, const RGBAImage& texture); // id 85, 113, 188-192
	void createPumkin(uint16_t id, const RGBAImage& front); // id 86, 91
	void createCake(); // id 92
	void createRedstoneRepeater(uint16_t id, const RGBAImage& texture); // id 93, 94
	void createTrapdoor(uint16_t id, const RGBAImage& texture); // id 96, 167
	void createHugeMushroom(uint16_t id, const RGBAImage& cap); // id 99, 100
	void createBarsPane(uint16_t id, uint16_t extra_data, const RGBAImage& texture); // id 101, 102
	void createStem(uint16_t id); // id 104, 105
	void createVines(); // id 106
	void createFenceGate(uint8_t id, RGBAImage texture); // id 107, 183-187
	void createBrewingStand(); // id 117
	void createCauldron(); // id 118
	void createDragonEgg(); // id 122
	RGBAImage buildCocoa(int stage);
	void createCocoas(); // id 127
	void createTripwireHook(); // id 131
	void createBeacon(); // id 138
	void createFlowerPot(); // id 140
	void createLargePlant(uint16_t data, const RGBAImage& texture, const RGBAImage& top_texture); // id 175

	virtual RGBAImage createUnknownBlock() const;
	virtual RGBAImage createBiomeBlock(uint16_t id, uint16_t data, const Biome& biome) const;
	virtual void createBlocks();
	virtual int createOpaqueWater();

	/**
	 * We overwrite this because we don't want to export all the block variations of every
	 * block caused by preblitting those dark edges.
	 */
	virtual std::vector<RGBAImage> getExportBlocks() const;
};

}
}

#endif /* ISOMETRIC_BLOCKIMAGES_H_ */
