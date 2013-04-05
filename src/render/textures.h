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

namespace mapcrafter {
namespace render {

// count of rotations needed for the block images
// depending on the north direction
const int NORTH_TOP_LEFT = 0;
const int NORTH_TOP_RIGHT = 1;
const int NORTH_BOTTOM_RIGHT = 2;
const int NORTH_BOTTOM_LEFT = 3;

const int FACE_NORTH = 1;
const int FACE_EAST = 2;
const int FACE_SOUTH = 4;
const int FACE_WEST = 8;

const int FACE_TOP = 16;
const int FACE_BOTTOM = 32;

// extra data starting at the 5. bit
const int DATA_NORTH = 16;
const int DATA_EAST = 32;
const int DATA_SOUTH = 64;
const int DATA_WEST = 128;

// the last three bits of 2 bytes
const int EDGE_NORTH = 8192;
const int EDGE_EAST = 16384;
const int EDGE_BOTTOM = 32768;

// some data values and stuff for special blocks
const int DOOR_NORTH = 16;
const int DOOR_SOUTH = 32;
const int DOOR_EAST = 64;
const int DOOR_WEST = 128;
const int DOOR_TOP = 256;
const int DOOR_FLIP_X = 512;

const int CHEST_FRONT = 0;
const int CHEST_SIDE = 1;
const int CHEST_TOP = 2;

const int LARGECHEST_FRONT_LEFT = 0;
const int LARGECHEST_FRONT_RIGHT = 1;
const int LARGECHEST_SIDE = 2;
const int LARGECHEST_TOP_LEFT = 3;
const int LARGECHEST_TOP_RIGHT = 4;
const int LARGECHEST_BACK_LEFT = 5;
const int LARGECHEST_BACK_RIGHT = 6;

const int LARGECHEST_DATA_LARGE = 256;
const int LARGECHEST_DATA_LEFT = 512;

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

/**
 * A single Minecraft block texture image.
 */
class TextureImage : public Image {
private:
	std::string name;
public:
	TextureImage();
	TextureImage(const std::string& name);
	~TextureImage();

	bool load(const std::string& path, int size);

	const std::string& getName() const;
};

/**
 * A Minecraft biome with data to tint the biome-depend blocks.
 */
struct Biome {
	Biome(double temperature, double rainfall)
		: temperature(temperature), rainfall(rainfall), r(255), g(255), b(255) {};
	Biome(double temperature, double rainfall, uint8_t r, uint8_t g, uint8_t b)
		: temperature(temperature), rainfall(rainfall), r(r), g(g), b(b) {};

	Biome& operator+=(const Biome& other);
	Biome& operator/=(int n);
	bool operator==(const Biome& other) const;

	uint32_t getColor(const Image& colors, bool flip_xy = false) const;

	// temperature and rainfall
	// used to calculate the position of the tinting color in the color image
	double temperature;
	double rainfall;

	int r, g, b;
};

// different Minecraft biomes
// from Minecraft Overviewer (from Minecraft MCP source code)
static const Biome BIOMES[] = {
	{0.5, 0.5}, // Ocean
	{0.8, 0.4}, // Plains
	{2.0, 0.0}, // Desert
	{0.2, 0.3}, // Extreme Hills
	{0.7, 0.8}, // Forest

	{0.05, 0.8}, // Taiga
	{0.8, 0.9, 205, 128, 255}, // Swampland
	{0.5, 0.5}, // River
	{2.0, 0.0}, // Hell
	{0.5, 0.5}, // Sky

	{0.0, 0.5}, // Frozen Ocean
	{0.0, 0.5}, // Frozen River
	{0.0, 0.5}, // Ice Plains
	{0.0, 0.5}, // Ice Mountains
	{0.9, 1.0}, // Mushroom Island

	{0.9, 1.0}, // Mushroom Island Shore
	{0.8, 0.4}, // Beach
	{2.0, 0.0}, // Desert Hills
	{0.7, 0.8}, // Forest Hills
	{0.05, 0.8}, // Taiga Hills

	{0.2, 0.3}, // Extreme Hills Edge
	{2.0, 0.45}, // Jungle
	{2.0, 0.25}, // Jungle Mountains
};

static const size_t BIOMES_SIZE = sizeof(BIOMES) / sizeof(Biome);
static const int DEFAULT_BIOME = 21; // Jungle

/**
 * Collection of Minecraft block textures.
 */
class BlockTextures {
public:
	BlockTextures();
	~BlockTextures();

	bool load(const std::string& block_dir, int size);

	// this is generated code by texture_code.py

	TextureImage ACTIVATOR_RAIL, ACTIVATOR_RAIL_POWERED, ANVIL_BASE, ANVIL_TOP,
	        ANVIL_TOP_DAMAGED_1, ANVIL_TOP_DAMAGED_2, BEACON, BED_FEET_END, BED_FEET_SIDE,
	        BED_FEET_TOP, BED_HEAD_END, BED_HEAD_SIDE, BED_HEAD_TOP, BEDROCK,
	        BLOCK_DIAMOND, BLOCK_EMERALD, BLOCK_GOLD, BLOCK_IRON, BLOCK_LAPIS,
	        BLOCK_REDSTONE, BOOKSHELF, BREWING_STAND, BREWING_STAND_BASE, BRICK,
	        CACTUS_BOTTOM, CACTUS_SIDE, CACTUS_TOP, CAKE_BOTTOM, CAKE_INNER, CAKE_SIDE,
	        CAKE_TOP, CARROTS_0, CARROTS_1, CARROTS_2, CARROTS_3, CAULDRON_BOTTOM,
	        CAULDRON_INNER, CAULDRON_SIDE, CAULDRON_TOP, CLAY, CLOTH_0, CLOTH_1, CLOTH_10,
	        CLOTH_11, CLOTH_12, CLOTH_13, CLOTH_14, CLOTH_15, CLOTH_2, CLOTH_3, CLOTH_4,
	        CLOTH_5, CLOTH_6, CLOTH_7, CLOTH_8, CLOTH_9, COCOA_0, COCOA_1, COCOA_2,
	        COMMAND_BLOCK, COMPARATOR, COMPARATOR_LIT, CROPS_0, CROPS_1, CROPS_2, CROPS_3,
	        CROPS_4, CROPS_5, CROPS_6, CROPS_7, DAYLIGHT_DETECTOR_SIDE,
	        DAYLIGHT_DETECTOR_TOP, DEADBUSH, DESTROY_0, DESTROY_1, DESTROY_2, DESTROY_3,
	        DESTROY_4, DESTROY_5, DESTROY_6, DESTROY_7, DESTROY_8, DESTROY_9,
	        DETECTOR_RAIL, DETECTOR_RAIL_ON, DIRT, DISPENSER_FRONT,
	        DISPENSER_FRONT_VERTICAL, DOOR_IRON_LOWER, DOOR_IRON_UPPER, DOOR_WOOD_LOWER,
	        DOOR_WOOD_UPPER, DRAGON_EGG, DROPPER_FRONT, DROPPER_FRONT_VERTICAL,
	        ENCHANTMENT_BOTTOM, ENCHANTMENT_SIDE, ENCHANTMENT_TOP, ENDFRAME_EYE,
	        ENDFRAME_SIDE, ENDFRAME_TOP, FARMLAND_DRY, FARMLAND_WET, FENCE_IRON, FERN,
	        FIRE_0, FIRE_1, FLOWER, FLOWER_POT, FURNACE_FRONT, FURNACE_FRONT_LIT,
	        FURNACE_SIDE, FURNACE_TOP, GLASS, GOLDEN_RAIL, GOLDEN_RAIL_POWERED,
	        GRASS_SIDE, GRASS_SIDE_OVERLAY, GRASS_TOP, GRAVEL, HELLROCK, HELLSAND, HOPPER,
	        HOPPER_INSIDE, HOPPER_TOP, ICE, ITEMFRAME_BACK, JUKEBOX_TOP, LADDER, LAVA,
	        LAVA_FLOW, LEAVES, LEAVES_JUNGLE, LEAVES_JUNGLE_OPAQUE, LEAVES_OPAQUE,
	        LEAVES_SPRUCE, LEAVES_SPRUCE_OPAQUE, LEVER, LIGHTGEM, MELON_SIDE, MELON_TOP,
	        MOB_SPAWNER, MUSHROOM_BROWN, MUSHROOM_INSIDE, MUSHROOM_RED,
	        MUSHROOM_SKIN_BROWN, MUSHROOM_SKIN_RED, MUSHROOM_SKIN_STEM, MUSIC_BLOCK,
	        MYCEL_SIDE, MYCEL_TOP, NETHER_BRICK, NETHER_STALK_0, NETHER_STALK_1,
	        NETHER_STALK_2, NETHERQUARTZ, OBSIDIAN, ORE_COAL, ORE_DIAMOND, ORE_EMERALD,
	        ORE_GOLD, ORE_IRON, ORE_LAPIS, ORE_REDSTONE, PISTON_BOTTOM, PISTON_INNER_TOP,
	        PISTON_SIDE, PISTON_TOP, PISTON_TOP_STICKY, PORTAL, POTATOES_0, POTATOES_1,
	        POTATOES_2, POTATOES_3, PUMPKIN_FACE, PUMPKIN_JACK, PUMPKIN_SIDE, PUMPKIN_TOP,
	        QUARTZBLOCK_BOTTOM, QUARTZBLOCK_CHISELED, QUARTZBLOCK_CHISELED_TOP,
	        QUARTZBLOCK_LINES, QUARTZBLOCK_LINES_TOP, QUARTZBLOCK_SIDE, QUARTZBLOCK_TOP,
	        RAIL, RAIL_TURN, REDSTONE_DUST_CROSS, REDSTONE_DUST_CROSS_OVERLAY,
	        REDSTONE_DUST_LINE, REDSTONE_DUST_LINE_OVERLAY, REDSTONE_LIGHT,
	        REDSTONE_LIGHT_LIT, REDTORCH, REDTORCH_LIT, REEDS, REPEATER, REPEATER_LIT,
	        ROSE, SAND, SANDSTONE_BOTTOM, SANDSTONE_CARVED, SANDSTONE_SIDE,
	        SANDSTONE_SMOOTH, SANDSTONE_TOP, SAPLING, SAPLING_BIRCH, SAPLING_JUNGLE,
	        SAPLING_SPRUCE, SNOW, SNOW_SIDE, SPONGE, STEM_BENT, STEM_STRAIGHT, STONE,
	        STONE_MOSS, STONEBRICK, STONEBRICKSMOOTH, STONEBRICKSMOOTH_CARVED,
	        STONEBRICKSMOOTH_CRACKED, STONEBRICKSMOOTH_MOSSY, STONESLAB_SIDE,
	        STONESLAB_TOP, TALLGRASS, THINGLASS_TOP, TNT_BOTTOM, TNT_SIDE, TNT_TOP, TORCH,
	        TRAPDOOR, TREE_BIRCH, TREE_JUNGLE, TREE_SIDE, TREE_SPRUCE, TREE_TOP,
	        TRIP_WIRE, TRIP_WIRE_SOURCE, VINE, WATER, WATER_FLOW, WATERLILY, WEB,
	        WHITE_STONE, WOOD, WOOD_BIRCH, WOOD_JUNGLE, WOOD_SPRUCE, WORKBENCH_FRONT,
	        WORKBENCH_SIDE, WORKBENCH_TOP;
	std::vector<TextureImage*> textures;
};

void blitFace(Image& image, int face, const Image& texture, int xoff = 0, int yoff = 0,
		bool darken = true);
void blitItemStyleBlock(Image& image, const Image& north_south, const Image& east_west);

void rotateImages(Image& north, Image& south, Image& east, Image& west, int rotation);

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
class BlockImages {
private:
	int texture_size;
	int rotation;
	bool render_unknown_blocks;
	bool render_leaves_transparent;

	BlockTextures textures;
	Image empty_texture;
	Image fire_texture;
	Image endportal_texture;

	Image chest[3];
	Image largechest[7];
	Image enderchest[3];

	Image foliagecolors, grasscolors;

	int max_water;
	Image opaque_water[4];
	Image shadow_edge_masks[4];

	// map of block images
	// key is a 32 bit integer, first two bytes id, second two bytes data
	std::unordered_map<uint32_t, Image> block_images;

	// map of biome block images, first four bytes id+data, next byte is the biome id
	std::unordered_map<uint64_t, Image> biome_images;

	// set of id/data block combinations, which contain transparency
	std::unordered_set<uint32_t> block_transparency;
	Image unknown_block;

	uint16_t filterBlockData(uint16_t id, uint16_t data) const;
	bool checkImageTransparency(const Image& block) const;
	void addBlockShadowEdges(uint16_t id, uint16_t data, const Image& block);

	void setBlockImage(uint16_t id, uint16_t data, const BlockImage& block);
	void setBlockImage(uint16_t id, uint16_t data, const Image& block);

	Image createBiomeBlock(uint16_t id, uint16_t data, const Biome& biome_data) const;
	void createBiomeBlocks();

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
	void createDispenserDropper(uint16_t id, const Image& front); // id 23, 158
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
	void createHugeMushroom(uint16_t id, const Image& cap); // id 99, 100
	void createBarsPane(uint16_t id, const Image& texture); // id 101, 102
	void createStem(uint16_t id); // id 104, 105
	void createVines(); // id 106
	void createFenceGate(); // id 107
	void createCauldron(); // id 118
	void createBeacon(); // id 138

	void loadBlocks();
public:
	BlockImages();
	~BlockImages();

	void setSettings(int texture_size, int rotation, bool render_unknown_blocks,
	        bool render_leaves_transparent);

	bool loadChests(const std::string& normal, const std::string& large,
	        const std::string& ender);
	bool loadColors(const std::string& foliagecolor, const std::string& grasscolor);
	bool loadOther(const std::string& fire, const std::string& endportal);
	bool loadBlocks(const std::string& block_dir);
	bool saveBlocks(const std::string& filename);

	bool isBlockTransparent(uint16_t id, uint16_t data) const;
	bool hasBlock(uint16_t id, uint16_t) const;
	const Image& getBlock(uint16_t id, uint16_t data) const;
	Image getBiomeDependBlock(uint16_t id, uint16_t data, uint8_t biome,
	        const Biome& biome_data) const;

	int getMaxWaterNeededOpaque() const;
	const Image& getOpaqueWater(bool south, bool west) const;

	int getBlockImageSize() const;
	int getTextureSize() const;
	int getTileSize() const;
};

}
}

#endif /* TEXTURES_H_ */
