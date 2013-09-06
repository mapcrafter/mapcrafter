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

#include "image.h"

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

const int REDSTONE_NORTH = 16;
const int REDSTONE_EAST = 32;
const int REDSTONE_SOUTH = 64;
const int REDSTONE_WEST = 128;
const int REDSTONE_TOPNORTH = 256;
const int REDSTONE_TOPEAST = 512;
const int REDSTONE_TOPSOUTH = 1024;
const int REDSTONE_TOPWEST = 2048;
const int REDSTONE_POWERED = 4096;

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
	// the original (not resized) texture image
	Image original;

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
	Biome(uint8_t id, double temperature, double rainfall, uint8_t r, uint8_t g, uint8_t b)
		: id(id), temperature(temperature), rainfall(rainfall), r(r), g(g), b(b) {};
	Biome(uint8_t id, double temperature, double rainfall)
		: id(id), temperature(temperature), rainfall(rainfall), r(255), g(255), b(255) {};

	Biome& operator+=(const Biome& other);
	Biome& operator/=(int n);
	bool operator==(const Biome& other) const;

	uint32_t getColor(const Image& colors, bool flip_xy = false) const;

	uint8_t id;

	// temperature and rainfall
	// used to calculate the position of the tinting color in the color image
	double temperature;
	double rainfall;

	int r, g, b;
};

// different Minecraft biomes
// from Minecraft Overviewer (from Minecraft MCP source code)
static const Biome BIOMES[] = {
	{0, 0.5, 0.5}, // Ocean
	{1, 0.8, 0.4}, // Plains
	{2, 2.0, 0.0}, // Desert
	{3, 0.2, 0.3}, // Extreme Hills
	{4, 0.7, 0.8}, // Forest

	{5, 0.05, 0.8}, // Taiga
	{6, 0.8, 0.9, 205, 128, 255}, // Swampland
	{7, 0.5, 0.5}, // River
	{8, 2.0, 0.0}, // Hell
	{9, 0.5, 0.5}, // Sky

	{10, 0.0, 0.5}, // Frozen Ocean
	{11, 0.0, 0.5}, // Frozen River
	{12, 0.0, 0.5}, // Ice Plains
	{13, 0.0, 0.5}, // Ice Mountains
	{14, 0.9, 1.0}, // Mushroom Island

	{15, 0.9, 1.0}, // Mushroom Island Shore
	{16, 0.8, 0.4}, // Beach
	{17, 2.0, 0.0}, // Desert Hills
	{18, 0.7, 0.8}, // Forest Hills
	{19, 0.05, 0.8}, // Taiga Hills

	{20, 0.2, 0.3}, // Extreme Hills Edge
	{21, 2.0, 0.45}, // Jungle
	{22, 2.0, 0.25}, // Jungle Mountains
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

	TextureImage ANVIL_BASE, ANVIL_TOP_DAMAGED_0, ANVIL_TOP_DAMAGED_1,
			ANVIL_TOP_DAMAGED_2, BEACON, BED_FEET_END, BED_FEET_SIDE, BED_FEET_TOP,
			BED_HEAD_END, BED_HEAD_SIDE, BED_HEAD_TOP, BEDROCK, BOOKSHELF, BREWING_STAND,
			BREWING_STAND_BASE, BRICK, CACTUS_BOTTOM, CACTUS_SIDE, CACTUS_TOP,
			CAKE_BOTTOM, CAKE_INNER, CAKE_SIDE, CAKE_TOP, CARROTS_STAGE_0,
			CARROTS_STAGE_1, CARROTS_STAGE_2, CARROTS_STAGE_3, CAULDRON_BOTTOM,
			CAULDRON_INNER, CAULDRON_SIDE, CAULDRON_TOP, CLAY, COAL_BLOCK, COAL_ORE,
			COBBLESTONE, COBBLESTONE_MOSSY, COCOA_STAGE_0, COCOA_STAGE_1, COCOA_STAGE_2,
			COMMAND_BLOCK, COMPARATOR_OFF, COMPARATOR_ON, CRAFTING_TABLE_FRONT,
			CRAFTING_TABLE_SIDE, CRAFTING_TABLE_TOP, DAYLIGHT_DETECTOR_SIDE,
			DAYLIGHT_DETECTOR_TOP, DEADBUSH, DESTROY_STAGE_0, DESTROY_STAGE_1,
			DESTROY_STAGE_2, DESTROY_STAGE_3, DESTROY_STAGE_4, DESTROY_STAGE_5,
			DESTROY_STAGE_6, DESTROY_STAGE_7, DESTROY_STAGE_8, DESTROY_STAGE_9,
			DIAMOND_BLOCK, DIAMOND_ORE, DIRT, DIRT_PODZOL_SIDE, DIRT_PODZOL_TOP,
			DISPENSER_FRONT_HORIZONTAL, DISPENSER_FRONT_VERTICAL, DOOR_IRON_LOWER,
			DOOR_IRON_UPPER, DOOR_WOOD_LOWER, DOOR_WOOD_UPPER, DOUBLE_PLANT_FERN_BOTTOM,
			DOUBLE_PLANT_FERN_TOP, DOUBLE_PLANT_GRASS_BOTTOM, DOUBLE_PLANT_GRASS_TOP,
			DOUBLE_PLANT_PAEONIA_BOTTOM, DOUBLE_PLANT_PAEONIA_TOP,
			DOUBLE_PLANT_ROSE_BOTTOM, DOUBLE_PLANT_ROSE_TOP, DOUBLE_PLANT_SUNFLOWER_BACK,
			DOUBLE_PLANT_SUNFLOWER_BOTTOM, DOUBLE_PLANT_SUNFLOWER_FRONT,
			DOUBLE_PLANT_SUNFLOWER_TOP, DOUBLE_PLANT_SYRINGA_BOTTOM,
			DOUBLE_PLANT_SYRINGA_TOP, DRAGON_EGG, DROPPER_FRONT_HORIZONTAL,
			DROPPER_FRONT_VERTICAL, EMERALD_BLOCK, EMERALD_ORE, ENCHANTING_TABLE_BOTTOM,
			ENCHANTING_TABLE_SIDE, ENCHANTING_TABLE_TOP, END_STONE, ENDFRAME_EYE,
			ENDFRAME_SIDE, ENDFRAME_TOP, FARMLAND_DRY, FARMLAND_WET, FERN, FIRE_LAYER_0,
			FIRE_LAYER_1, FLOWER_ALLIUM, FLOWER_BLUE_ORCHID, FLOWER_DANDELION,
			FLOWER_HOUSTONIA, FLOWER_OXEYE_DAISY, FLOWER_PAEONIA, FLOWER_POT, FLOWER_ROSE,
			FLOWER_TULIP_ORANGE, FLOWER_TULIP_PINK, FLOWER_TULIP_RED, FLOWER_TULIP_WHITE,
			FURNACE_FRONT_OFF, FURNACE_FRONT_ON, FURNACE_SIDE, FURNACE_TOP, GLASS,
			GLASS_PANE_TOP, GLOWSTONE, GOLD_BLOCK, GOLD_ORE, GRASS_SIDE,
			GRASS_SIDE_OVERLAY, GRASS_SIDE_SNOWED, GRASS_TOP, GRAVEL, HARDENED_CLAY,
			HARDENED_CLAY_STAINED_BLACK, HARDENED_CLAY_STAINED_BLUE,
			HARDENED_CLAY_STAINED_BROWN, HARDENED_CLAY_STAINED_CYAN,
			HARDENED_CLAY_STAINED_GRAY, HARDENED_CLAY_STAINED_GREEN,
			HARDENED_CLAY_STAINED_LIGHT_BLUE, HARDENED_CLAY_STAINED_LIME,
			HARDENED_CLAY_STAINED_MAGENTA, HARDENED_CLAY_STAINED_ORANGE,
			HARDENED_CLAY_STAINED_PINK, HARDENED_CLAY_STAINED_PURPLE,
			HARDENED_CLAY_STAINED_RED, HARDENED_CLAY_STAINED_SILVER,
			HARDENED_CLAY_STAINED_WHITE, HARDENED_CLAY_STAINED_YELLOW, HAY_BLOCK_SIDE,
			HAY_BLOCK_TOP, HOPPER_INSIDE, HOPPER_OUTSIDE, HOPPER_TOP, ICE, ICE_PACKED,
			IRON_BARS, IRON_BLOCK, IRON_ORE, ITEMFRAME_BACKGROUND, JUKEBOX_SIDE,
			JUKEBOX_TOP, LADDER, LAPIS_BLOCK, LAPIS_ORE, LAVA_FLOW, LAVA_STILL,
			LEAVES_BIRCH, LEAVES_BIRCH_OPAQUE, LEAVES_JUNGLE, LEAVES_JUNGLE_OPAQUE,
			LEAVES_OAK, LEAVES_OAK_OPAQUE, LEAVES_SPRUCE, LEAVES_SPRUCE_OPAQUE, LEVER,
			LOG_BIRCH, LOG_BIRCH_TOP, LOG_JUNGLE, LOG_JUNGLE_TOP, LOG_OAK, LOG_OAK_TOP,
			LOG_SPRUCE, LOG_SPRUCE_TOP, MELON_SIDE, MELON_STEM_CONNECTED,
			MELON_STEM_DISCONNECTED, MELON_TOP, MOB_SPAWNER, MUSHROOM_BLOCK_INSIDE,
			MUSHROOM_BLOCK_SKIN_BROWN, MUSHROOM_BLOCK_SKIN_RED, MUSHROOM_BLOCK_SKIN_STEM,
			MUSHROOM_BROWN, MUSHROOM_RED, MYCELIUM_SIDE, MYCELIUM_TOP, NETHER_BRICK,
			NETHER_WART_STAGE_0, NETHER_WART_STAGE_1, NETHER_WART_STAGE_2, NETHERRACK,
			NOTEBLOCK, OBSIDIAN, PISTON_BOTTOM, PISTON_INNER, PISTON_SIDE,
			PISTON_TOP_NORMAL, PISTON_TOP_STICKY, PLANKS_BIRCH, PLANKS_JUNGLE, PLANKS_OAK,
			PLANKS_SPRUCE, PORTAL, POTATOES_STAGE_0, POTATOES_STAGE_1, POTATOES_STAGE_2,
			POTATOES_STAGE_3, PUMPKIN_FACE_OFF, PUMPKIN_FACE_ON, PUMPKIN_SIDE,
			PUMPKIN_STEM_CONNECTED, PUMPKIN_STEM_DISCONNECTED, PUMPKIN_TOP,
			QUARTZ_BLOCK_BOTTOM, QUARTZ_BLOCK_CHISELED, QUARTZ_BLOCK_CHISELED_TOP,
			QUARTZ_BLOCK_LINES, QUARTZ_BLOCK_LINES_TOP, QUARTZ_BLOCK_SIDE,
			QUARTZ_BLOCK_TOP, QUARTZ_ORE, RAIL_ACTIVATOR, RAIL_ACTIVATOR_POWERED,
			RAIL_DETECTOR, RAIL_DETECTOR_POWERED, RAIL_GOLDEN, RAIL_GOLDEN_POWERED,
			RAIL_NORMAL, RAIL_NORMAL_TURNED, REDSTONE_BLOCK, REDSTONE_DUST_CROSS,
			REDSTONE_DUST_CROSS_OVERLAY, REDSTONE_DUST_LINE, REDSTONE_DUST_LINE_OVERLAY,
			REDSTONE_LAMP_OFF, REDSTONE_LAMP_ON, REDSTONE_ORE, REDSTONE_TORCH_OFF,
			REDSTONE_TORCH_ON, REEDS, REPEATER_OFF, REPEATER_ON, SAND, SANDSTONE_BOTTOM,
			SANDSTONE_CARVED, SANDSTONE_NORMAL, SANDSTONE_SMOOTH, SANDSTONE_TOP,
			SAPLING_BIRCH, SAPLING_JUNGLE, SAPLING_OAK, SAPLING_SPRUCE, SNOW, SOUL_SAND,
			SPONGE, STONE, STONE_SLAB_SIDE, STONE_SLAB_TOP, STONEBRICK, STONEBRICK_CARVED,
			STONEBRICK_CRACKED, STONEBRICK_MOSSY, TALLGRASS, TNT_BOTTOM, TNT_SIDE,
			TNT_TOP, TORCH_ON, TRAPDOOR, TRIP_WIRE, TRIP_WIRE_SOURCE, VINE, WATER_FLOW,
			WATER_STILL, WATERLILY, WEB, WHEAT_STAGE_0, WHEAT_STAGE_1, WHEAT_STAGE_2,
			WHEAT_STAGE_3, WHEAT_STAGE_4, WHEAT_STAGE_5, WHEAT_STAGE_6, WHEAT_STAGE_7,
			WOOL_COLORED_BLACK, WOOL_COLORED_BLUE, WOOL_COLORED_BROWN, WOOL_COLORED_CYAN,
			WOOL_COLORED_GRAY, WOOL_COLORED_GREEN, WOOL_COLORED_LIGHT_BLUE,
			WOOL_COLORED_LIME, WOOL_COLORED_MAGENTA, WOOL_COLORED_ORANGE,
			WOOL_COLORED_PINK, WOOL_COLORED_PURPLE, WOOL_COLORED_RED, WOOL_COLORED_SILVER,
			WOOL_COLORED_WHITE, WOOL_COLORED_YELLOW;
	std::vector<TextureImage*> textures;
};

void blitFace(Image& image, int face, const Image& texture,
		int xoff = 0, int yoff = 0,
		bool darken = true, double dleft = 0.6, double dright = 0.75);
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
	Image buildImage(double dleft, double dright) const;

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
	Image endportal_texture;

	Image chest[3];
	Image largechest[7];
	Image enderchest[3];

	Image foliagecolors, grasscolors;

	int max_water;
	Image opaque_water[4];
	Image shadow_edge_masks[4];

	// factor to darken the side faces
	// defaults to 0.75 and 0.6
	double dleft, dright;

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

	Image buildImage(const BlockImage& image);

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
	void createSmallerBlock(uint16_t id, uint16_t data, const Image& texture, int y1, int y2);
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
	void createWood(uint16_t id, uint16_t data, const Image& side_texture, const Image& top_texture); // id 17
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
	void createRedstoneWire(bool powered); // id 55
	void createDoor(uint16_t id, const Image& bottom, const Image& top); // id 64, 71
	void createRails(); // id 66
	void createButton(uint16_t id, const Image& tex); // id 77, 143
	void createSnow(); // id 78
	void createIce(); // id 79
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
	void createBrewingStand(); // id 117
	void createCauldron(); // id 118
	void createDragonEgg(); // id 122
	void createBeacon(); // id 138
	void createFlowerPot(); // id 140

	void loadBlocks();
public:
	BlockImages();
	~BlockImages();

	void setSettings(int texture_size, int rotation, bool render_unknown_blocks,
	        bool render_leaves_transparent, const std::string& rendermode);

	bool loadChests(const std::string& normal, const std::string& large,
	        const std::string& ender);
	bool loadColors(const std::string& foliagecolor, const std::string& grasscolor);
	bool loadOther(const std::string& endportal);
	bool loadBlocks(const std::string& block_dir);
	bool loadAll(const std::string& textures_dir);
	bool saveBlocks(const std::string& filename);

	bool isBlockTransparent(uint16_t id, uint16_t data) const;
	bool hasBlock(uint16_t id, uint16_t) const;
	const Image& getBlock(uint16_t id, uint16_t data) const;
	Image getBiomeDependBlock(uint16_t id, uint16_t data, const Biome& biome) const;

	int getMaxWaterNeededOpaque() const;
	const Image& getOpaqueWater(bool south, bool west) const;

	int getBlockImageSize() const;
	int getTextureSize() const;
	int getTileSize() const;
};

}
}

#endif /* TEXTURES_H_ */
