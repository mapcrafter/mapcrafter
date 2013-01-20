/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEXTURES_H_
#define TEXTURES_H_

#include "render/image.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>

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

// count of rotation needed for the block images needed
// depending on the north direction
#define NORTH_TOP_LEFT 0
#define NORTH_TOP_RIGHT 1
#define NORTH_BOTTOM_RIGHT 2
#define NORTH_BOTTOM_LEFT 3

#define FACE_NORTH 1
#define FACE_EAST 2
#define FACE_SOUTH 4
#define FACE_WEST 8

#define FACE_TOP 16
#define FACE_BOTTOM 32

// extra data starting at the 5. bit
#define DATA_NORTH 16
#define DATA_SOUTH 32
#define DATA_EAST 64
#define DATA_WEST 128

// the last three bits of 2 bytes
#define EDGE_NORTH 8192
#define EDGE_EAST 16384
#define EDGE_BOTTOM 32768

// some data values and stuff for special blocks
#define DOOR_NORTH 16
#define DOOR_SOUTH 32
#define DOOR_EAST 64
#define DOOR_WEST 128
#define DOOR_TOP 256
#define DOOR_FLIP_X 512

#define CHEST_FRONT 0
#define CHEST_SIDE 1
#define CHEST_TOP 2

#define LARGECHEST_FRONT_LEFT 0
#define LARGECHEST_FRONT_RIGHT 1
#define LARGECHEST_SIDE 2
#define LARGECHEST_TOP_LEFT 3
#define LARGECHEST_TOP_RIGHT 4
#define LARGECHEST_BACK_LEFT 5
#define LARGECHEST_BACK_RIGHT 6

#define LARGECHEST_DATA_LARGE 32
#define LARGECHEST_DATA_LEFT 16

namespace mapcrafter {
namespace render {

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

void blitFace(Image& image, int face, const Image& texture, int xoff = 0, int yoff = 0,
		bool darken = true);
void blitItemStyleBlock(Image& image, const Image& north_south, const Image& east_west);

/**
 * A block with 6 face textures, used to create the block images and also rotate them.
 */
class BlockImage {
private:
	int type;
	Image faces[6];
	int x_offsets[6], y_offsets[6];
	Image empty_image;
public:
	BlockImage(int type = NORMAL);
	~BlockImage();

	BlockImage& setFace(int face, const Image& texture, int xoff = 0, int yoff = 0);
	const Image& getFace(int face) const;
	int getXOffset(int face) const;
	int getYOffset(int face) const;

	BlockImage rotate(int count) const;
	Image buildImage() const;

	static const int NORMAL = 1;
	static const int ITEM_STYLE = 2;
};

/**
 * This class is responsible for reading the Minecraft textures and creating the block
 * images.
 */
class BlockTextures {
private:
	int texture_size;
	int rotation;
	bool render_unknown_blocks;
	bool render_leaves_transparent;

	Image textures[16 * 16];
	Image empty_texture;
	Image fire_texture;
	Image endportal_texture;

	Image chest[3];
	Image largechest[7];
	Image enderchest[3];

	int max_water;
	Image opaque_water[4];
	Image shadow_edge_masks[4];

	// map of block images
	// key is a 32 bit integer, first two bytes id, second two bytes data
	std::unordered_map<uint32_t, Image> block_images;
	// set of id/data block combinations, which contain transparency
	std::unordered_set<uint32_t> block_transparency;
	Image unknown_block;

	void splitTerrain(const Image& terrain);
	const Image& getTexture(int x, int y) const;

	uint16_t filterBlockData(uint16_t id, uint16_t data) const;
	bool checkImageTransparency(const Image& block) const;
	void addBlockShadowEdges(uint16_t id, uint16_t data, const Image& block);
	void setBlockImage(uint16_t id, uint16_t data, const BlockImage& block);
	void setBlockImage(uint16_t id, uint16_t data, const Image& block);

	void testWaterTransparency();

	uint32_t darkenLeft(uint32_t pixel) const;
	uint32_t darkenRight(uint32_t pixel) const;

	BlockImage buildSmallerBlock(const Image& left_texture, const Image& right_texture,
	        const Image& top_texture, int y1, int y2);

	Image buildStairsSouth(const Image& texture);
	Image buildStairsNorth(const Image& texture);
	Image buildStairsWest(const Image& texture);
	Image buildStairsEast(const Image& texture);
	Image buildUpsideDownStairsNorth(const Image& texture);
	Image buildUpsideDownStairsSouth(const Image& texture);
	Image buildUpsideDownStairsEast(const Image& texture);
	Image buildUpsideDownStairsWest(const Image& texture);

	void buildCustomTextures();

	void createBlock(uint16_t id, uint16_t data, const Image& texture);
	void createBlock(uint16_t id, uint16_t data, const Image& side_texture,
	        const Image& top_texture);
	void createBlock(uint16_t id, uint16_t data, const Image& left_texture,
	        const Image& right_texture, const Image& top_texture);

	void createSmallerBlock(uint16_t id, uint16_t data, const Image& left_texture,
	        const Image& right_texture, const Image& top_texture, int y1, int y2);
	void createSmallerBlock(uint16_t id, uint16_t data, const Image& side_face,
	        const Image& top_texture, int y1, int y2);
	void createRotatedBlock(uint16_t id, uint16_t extra_data, const Image& front_texture,
	        const Image& side_texture, const Image& top_texture);
	void createRotatedBlock(uint16_t id, uint16_t extra_data, const Image& front_texture,
	        const Image& back_texture, const Image& side_texture,
	        const Image& top_texture);
	void createItemStyleBlock(uint16_t id, uint16_t data, const Image& texture);
	void createItemStyleBlock(uint16_t id, uint16_t data, const Image& north_south,
	        const Image& east_west);
	void createSingleFaceBlock(uint16_t id, uint16_t data, int face,
	        const Image& texture);

	void createGrassBlock(); // id 2
	void createWater(); // id 8, 9
	void createLava(); // id 10, 11
	void createWood(uint16_t data, const Image& side_texture); // id 17
	void createLeaves(); // id 18
	void createBed(); // id 26
	void createStraightRails(uint16_t id, uint16_t extra_data, const Image& texture); // id 27, 28, 66
	void createPiston(uint16_t id, bool sticky); // id 29, 33
	void createSlabs(uint16_t id, bool stone_slabs, bool double_slabs); // id 43, 44, 125, 126
	void createTorch(uint16_t, const Image& texture); // id 50, 75, 76
	void createStairs(uint16_t id, const Image& texture); // id 53, 67, 108, 109, 114, 128, 134, 135, 136
	void createChest(uint16_t id, Image* textures); // id 54, 95, 130
	void createDoubleChest(uint16_t id, Image* textures); // id 54
	void createDoor(uint16_t id, const Image& bottom, const Image& top); // id 64, 71
	void createRails(); // id 66
	void createButton(uint16_t id, const Image& tex); // id 77, 143
	void createSnow(); // id 78
	void createCactus(); // id 81
	void createFence(uint16_t id, const Image& texture); // id 85, 113
	void createPumkin(uint16_t id, const Image& front); // id 86, 91
	void createCake(); // id 92
	void createRedstoneRepeater(uint16_t id, const Image& texture); // id 93, 94
	void createTrapdoor(); // id 96
	void createHugeMushroom(uint16_t id, const Image& cap_texture); // id 99, 100
	void createBarsPane(uint16_t id, const Image& texture); // id 101, 102
	void createStem(uint16_t id); // id 104, 105
	void createVines(); // id 106
	void createFenceGate(); // id 107
	void createCauldron(); // id 118
	void createBeacon(); // id 138

	void loadBlocks();
public:
	BlockTextures();
	~BlockTextures();

	void setSettings(int texture_size, int rotation, bool render_unknown_blocks,
	        bool render_leaves_transparent);

	bool loadChests(const std::string& normal, const std::string& large,
	        const std::string& ender);
	bool loadOther(const std::string& fire, const std::string& endportal);
	bool loadBlocks(const std::string& terrain_filename);
	bool saveBlocks(const std::string& filename);

	bool isBlockTransparent(uint16_t id, uint16_t data) const;
	bool hasBlock(uint16_t id, uint16_t) const;
	const Image& getBlock(uint16_t id, uint16_t data) const;

	int getMaxWaterNeededOpaque() const;
	const Image& getOpaqueWater(bool south, bool west) const;

	int getBlockImageSize() const;
	int getTextureSize() const;
	int getTileSize() const;
};

}
}

#endif /* TEXTURES_H_ */
