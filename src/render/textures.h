/*
 * Copyright 2012 Moritz Hilscher
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

#define FACE_TOP 1
#define FACE_BOTTOM 2

#define FACE_NORTH 16
#define FACE_SOUTH 32
#define FACE_EAST 64
#define FACE_WEST 128

#define EDGE_NORTH 8192
#define EDGE_EAST 16384
#define EDGE_BOTTOM 32768

#define DOOR_NORTH 16
#define DOOR_SOUTH 32
#define DOOR_EAST 64
#define DOOR_WEST 128
#define DOOR_TOP 256
#define DOOR_FLIP_X 512

namespace mapcrafter {
namespace render {

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

	int src_x;
	int src_y;
	int dest_x;
	int dest_y;
};

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

class TopFaceIterator: public FaceIterator {
private:
	int next_x;
	int next_y;
public:
	TopFaceIterator();
	TopFaceIterator(int size);

	void next();
};

class BlockTextures {
private:
	int texture_size;
	Image textures[16 * 16];
	Image fire_texture;
	Image endportal_texture;

	Image empty_texture;
	bool render_unknown_blocks;
	bool render_leaves_transparent;
	Image unknown_block;

	int max_water;
	Image opaque_water[4];
	Image shadow_edge_masks[4];

	std::unordered_map<uint32_t, Image > block_images;
	std::unordered_set<uint32_t> block_transparency;

	void splitTerrain(const Image& terrain);
	const Image& getTexture(int x, int y) const;

	uint16_t filterBlockData(uint16_t id, uint16_t data) const;
	bool checkImageTransparency(const Image& block) const;
	void addBlockShadowEdges(uint16_t id, uint16_t data, const Image& block);
	void setBlockImage(uint16_t id, uint16_t data, const Image& block);

	void testWaterTransparency();

	uint32_t darkenLeft(uint32_t pixel) const;
	uint32_t darkenRight(uint32_t pixel) const;

	void blitFace(Image& image, int face, const Image& texture, int xoff = 0,
	        int yoff = 0, bool darken = true) const;

	Image buildImage(const Image& left_texture, const Image& right_texture,
	        const Image& upper_texture);
	Image buildImage(const Image& left_texture, const Image& right_texture,
	        const Image& upper_texture, int left_xoff, int left_yoff, int right_xoff,
	        int right_yoff, int upper_xoff, int upper_yoff);
	Image buildSmallerImage(const Image& left_texture, const Image& right_texture,
	        const Image& upper_texture, int y1, int y2, bool move_only_top);

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
	        const Image& upper_texture);
	void createBlock(uint16_t id, uint16_t data, const Image& left_texture,
	        const Image& right_texture, const Image& upper_texture);

	void createSmallerBlock(uint16_t id, uint16_t data, const Image& left_texture,
	        const Image& right_texture, const Image& upper_texture, int y1, int y2,
	        bool moveOnlyTop = false);
	void createSmallerBlock(uint16_t id, uint16_t data, const Image& side_face,
	        const Image& upper_texture, int y1, int y2, bool moveOnlyTop = false);
	void createRotatedBlock(uint16_t id, const Image& front_face, const Image& side_face,
	        const Image& upper_texture);
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
	void createDoor(uint16_t id, const Image& bottom, const Image& top); // id 64, 71
	void createRails(); // id 66
	void createButton(uint16_t id, const Image& tex); // id 77, 143
	void createSnow(); // id 78
	void createCactus(); // id 81
	void createPumkin(uint16_t id, const Image& front); // id 86, 91
	void createCake(); // id 92
	void createRedstoneRepeater(uint16_t id, const Image& texture); // id 93, 94
	void createTrapdoor(); // id 96
	void createHugeMushroom(uint16_t id, const Image& cap_texture); // id 99, 100
	void createBarsPane(uint16_t id, const Image& texture); // id 101, 102
	void createVines(); // id 106
	void createCauldron(); // id 118

	void loadBlocks();
public:
	BlockTextures();
	~BlockTextures();

	void setSettings(int texture_size, bool render_unknown_blocks,
	        bool render_leaves_transparent);

	bool loadChests(const std::string& normal, const std::string& large,
	        const std::string& ender);
	bool loadOther(const std::string& fire, const std::string& endportal);
	bool loadBlocks(const std::string& terrain_filename);
	bool saveBlocks(const std::string& filename);

	bool isBlockTransparent(uint16_t id, uint16_t data) const;
	bool hasBlock(uint16_t id, uint16_t ) const;
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
