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

#include "render/textures.h"

#include "util.h"

#include <iostream>
#include <map>
#include <cmath>
#include <cstdlib>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace render {

FaceIterator::FaceIterator()
		: is_end(false), size(16), src_x(0), src_y(0), dest_x(0), dest_y(0) {
}

FaceIterator::FaceIterator(int size)
		: is_end(false), size(size), src_x(0), src_y(0), dest_x(0), dest_y(0) {
}

FaceIterator::~FaceIterator() {
}

void FaceIterator::next() {
	if(size == 0) {
		is_end = true;
		return;
	}
	// just iterate over the source pixels
	if (src_x == size - 1 && src_y == size - 1) {
		// set end if we are on bottom right
		is_end = true;
	} else if (src_y == size - 1) {
		// go one to the right (x+1) if we reached the bottom edge of the texture
		src_x++;
		src_y = 0;
	} else {
		// else just go one pixel forward
		src_y++;
	}
}

bool FaceIterator::end() const {
	return is_end;
}

SideFaceIterator::SideFaceIterator()
		: FaceIterator(), side(0), delta(0) {
}

SideFaceIterator::SideFaceIterator(int size, int side)
		: FaceIterator(size), side(side) {
	if (side == RIGHT) {
		delta = size / 2;
		dest_y = delta;
	} else if (side == LEFT) {
		delta = 0;
		dest_y = 0;
	}
}

void SideFaceIterator::next() {
	FaceIterator::next();
	if (src_y == 0 && src_x % 2 == 1) {
		delta += side;
	}
	dest_x = src_x;
	dest_y = src_y + delta;
}

TopFaceIterator::TopFaceIterator()
		: FaceIterator(), next_x(-1), next_y(-1) {
}

TopFaceIterator::TopFaceIterator(int size)
		: FaceIterator(size), next_x(-1), next_y(-1) {
	dest_x = size - 1;
}

void TopFaceIterator::next() {
	FaceIterator::next();
	if (src_y == 0) {
		dest_x = src_x + size - 1;
		dest_y = src_x / 2;
		next_x = dest_x - (src_x % 2 == 0 ? 1 : 0);
		next_y = dest_y + 1;
	} else if (next_x != -1 && next_y != -1) {
		dest_x = next_x;
		dest_y = next_y;
		next_x = -1;
		next_y = -1;
	} else if (src_x % 2 == 0 && src_y == size - 1) {
		dest_y += 1;
	} else {
		if (src_y % 2 == 1) {
			dest_y += 1;
		}
		dest_x -= 1;
	}
}

TextureImage::TextureImage() {
}

TextureImage::TextureImage(const std::string& name)
		: name(name) {
}

TextureImage::~TextureImage() {
}

/**
 * Loads a single block texture image.
 */
bool TextureImage::load(const std::string& path, int size) {
	Image tmp;
	if (!tmp.readPNG(path + "/" + name + ".png"))
		return false;

	// check if this is an animated texture
	// -> use only the first frame
	if (tmp.getWidth() < tmp.getHeight())
		tmp = tmp.clip(0, 0, tmp.getWidth(), tmp.getWidth());

	// we resize the transparent version of the leaves without interpolation,
	// because this can cause half-transparent pixels, which aren't good for performance
	if (name == "leaves" || name == "leaves_jungle" || name == "leaves_spruce")
		tmp.resizeSimple(size, size, *this);
	else
		tmp.resizeInterpolated(size, size, *this);
	return true;
}

const std::string& TextureImage::getName() const {
	return name;
}

Biome& Biome::operator+=(const Biome& other) {
	rainfall += other.rainfall;
	temperature += other.temperature;

	r += other.r;
	g += other.g;
	b += other.b;

	return *this;
}

/**
 * Used to calculate average biome data, to create smooth biome edges.
 */
Biome& Biome::operator/=(int n) {
	rainfall /= n;
	temperature /= n;

	r /= n;
	g /= n;
	b /= n;

	return *this;
}

/**
 * Checks, if two biomes are the same.
 */
bool Biome::operator==(const Biome& other) const {
	double epsilon = 0.1;
	return std::abs(other.rainfall - rainfall) <= epsilon
			&& std::abs(other.temperature - temperature) <= epsilon
			&& r == other.r && g == other.g && b == other.b;
}

uint32_t Biome::getColor(const Image& colors, bool flip_xy) const {
	// x is temperature
	double tmp_temperature = temperature;
	// y is temperature * rainfall
	double tmp_rainfall = rainfall * temperature;

	// check if temperature and rainfall are valid
	if(tmp_temperature > 1)
		tmp_temperature = 1;
	if(tmp_rainfall > 1)
		tmp_rainfall = 1;

	// calculate positions
	int x = 255 - (255 * tmp_temperature);
	int y = 255 - (255 * tmp_rainfall);

	// flip them, if needed
	if (flip_xy) {
		int tmp = x;
		x = 255 - y;
		y = 255 - tmp;
	}

	// return color at this position
	return colors.getPixel(x, y);
}

// generated code by texture_code.py
BlockTextures::BlockTextures()
		: ACTIVATOR_RAIL("activatorRail"), ACTIVATOR_RAIL_POWERED(
		        "activatorRail_powered"), ANVIL_BASE("anvil_base"), ANVIL_TOP(
		        "anvil_top"), ANVIL_TOP_DAMAGED_1("anvil_top_damaged_1"), ANVIL_TOP_DAMAGED_2(
		        "anvil_top_damaged_2"), BEACON("beacon"), BED_FEET_END("bed_feet_end"), BED_FEET_SIDE(
		        "bed_feet_side"), BED_FEET_TOP("bed_feet_top"), BED_HEAD_END(
		        "bed_head_end"), BED_HEAD_SIDE("bed_head_side"), BED_HEAD_TOP(
		        "bed_head_top"), BEDROCK("bedrock"), BLOCK_DIAMOND("blockDiamond"), BLOCK_EMERALD(
		        "blockEmerald"), BLOCK_GOLD("blockGold"), BLOCK_IRON("blockIron"), BLOCK_LAPIS(
		        "blockLapis"), BLOCK_REDSTONE("blockRedstone"), BOOKSHELF("bookshelf"), BREWING_STAND(
		        "brewingStand"), BREWING_STAND_BASE("brewingStand_base"), BRICK("brick"), CACTUS_BOTTOM(
		        "cactus_bottom"), CACTUS_SIDE("cactus_side"), CACTUS_TOP("cactus_top"), CAKE_BOTTOM(
		        "cake_bottom"), CAKE_INNER("cake_inner"), CAKE_SIDE("cake_side"), CAKE_TOP(
		        "cake_top"), CARROTS_0("carrots_0"), CARROTS_1("carrots_1"), CARROTS_2(
		        "carrots_2"), CARROTS_3("carrots_3"), CAULDRON_BOTTOM("cauldron_bottom"), CAULDRON_INNER(
		        "cauldron_inner"), CAULDRON_SIDE("cauldron_side"), CAULDRON_TOP(
		        "cauldron_top"), CLAY("clay"), CLOTH_0("cloth_0"), CLOTH_1("cloth_1"), CLOTH_10(
		        "cloth_10"), CLOTH_11("cloth_11"), CLOTH_12("cloth_12"), CLOTH_13(
		        "cloth_13"), CLOTH_14("cloth_14"), CLOTH_15("cloth_15"), CLOTH_2(
		        "cloth_2"), CLOTH_3("cloth_3"), CLOTH_4("cloth_4"), CLOTH_5("cloth_5"), CLOTH_6(
		        "cloth_6"), CLOTH_7("cloth_7"), CLOTH_8("cloth_8"), CLOTH_9("cloth_9"), COCOA_0(
		        "cocoa_0"), COCOA_1("cocoa_1"), COCOA_2("cocoa_2"), COMMAND_BLOCK(
		        "commandBlock"), COMPARATOR("comparator"), COMPARATOR_LIT(
		        "comparator_lit"), CROPS_0("crops_0"), CROPS_1("crops_1"), CROPS_2(
		        "crops_2"), CROPS_3("crops_3"), CROPS_4("crops_4"), CROPS_5("crops_5"), CROPS_6(
		        "crops_6"), CROPS_7("crops_7"), DAYLIGHT_DETECTOR_SIDE(
		        "daylightDetector_side"), DAYLIGHT_DETECTOR_TOP("daylightDetector_top"), DEADBUSH(
		        "deadbush"), DESTROY_0("destroy_0"), DESTROY_1("destroy_1"), DESTROY_2(
		        "destroy_2"), DESTROY_3("destroy_3"), DESTROY_4("destroy_4"), DESTROY_5(
		        "destroy_5"), DESTROY_6("destroy_6"), DESTROY_7("destroy_7"), DESTROY_8(
		        "destroy_8"), DESTROY_9("destroy_9"), DETECTOR_RAIL("detectorRail"), DETECTOR_RAIL_ON(
		        "detectorRail_on"), DIRT("dirt"), DISPENSER_FRONT("dispenser_front"), DISPENSER_FRONT_VERTICAL(
		        "dispenser_front_vertical"), DOOR_IRON_LOWER("doorIron_lower"), DOOR_IRON_UPPER(
		        "doorIron_upper"), DOOR_WOOD_LOWER("doorWood_lower"), DOOR_WOOD_UPPER(
		        "doorWood_upper"), DRAGON_EGG("dragonEgg"), DROPPER_FRONT(
		        "dropper_front"), DROPPER_FRONT_VERTICAL("dropper_front_vertical"), ENCHANTMENT_BOTTOM(
		        "enchantment_bottom"), ENCHANTMENT_SIDE("enchantment_side"), ENCHANTMENT_TOP(
		        "enchantment_top"), ENDFRAME_EYE("endframe_eye"), ENDFRAME_SIDE(
		        "endframe_side"), ENDFRAME_TOP("endframe_top"), FARMLAND_DRY(
		        "farmland_dry"), FARMLAND_WET("farmland_wet"), FENCE_IRON("fenceIron"), FERN(
		        "fern"), FIRE_0("fire_0"), FIRE_1("fire_1"), FLOWER("flower"), FLOWER_POT(
		        "flowerPot"), FURNACE_FRONT("furnace_front"), FURNACE_FRONT_LIT(
		        "furnace_front_lit"), FURNACE_SIDE("furnace_side"), FURNACE_TOP(
		        "furnace_top"), GLASS("glass"), GOLDEN_RAIL("goldenRail"), GOLDEN_RAIL_POWERED(
		        "goldenRail_powered"), GRASS_SIDE("grass_side"), GRASS_SIDE_OVERLAY(
		        "grass_side_overlay"), GRASS_TOP("grass_top"), GRAVEL("gravel"), HELLROCK(
		        "hellrock"), HELLSAND("hellsand"), HOPPER("hopper"), HOPPER_INSIDE(
		        "hopper_inside"), HOPPER_TOP("hopper_top"), ICE("ice"), ITEMFRAME_BACK(
		        "itemframe_back"), JUKEBOX_TOP("jukebox_top"), LADDER("ladder"), LAVA(
		        "lava"), LAVA_FLOW("lava_flow"), LEAVES("leaves"), LEAVES_JUNGLE(
		        "leaves_jungle"), LEAVES_JUNGLE_OPAQUE("leaves_jungle_opaque"), LEAVES_OPAQUE(
		        "leaves_opaque"), LEAVES_SPRUCE("leaves_spruce"), LEAVES_SPRUCE_OPAQUE(
		        "leaves_spruce_opaque"), LEVER("lever"), LIGHTGEM("lightgem"), MELON_SIDE(
		        "melon_side"), MELON_TOP("melon_top"), MOB_SPAWNER("mobSpawner"), MUSHROOM_BROWN(
		        "mushroom_brown"), MUSHROOM_INSIDE("mushroom_inside"), MUSHROOM_RED(
		        "mushroom_red"), MUSHROOM_SKIN_BROWN("mushroom_skin_brown"), MUSHROOM_SKIN_RED(
		        "mushroom_skin_red"), MUSHROOM_SKIN_STEM("mushroom_skin_stem"), MUSIC_BLOCK(
		        "musicBlock"), MYCEL_SIDE("mycel_side"), MYCEL_TOP("mycel_top"), NETHER_BRICK(
		        "netherBrick"), NETHER_STALK_0("netherStalk_0"), NETHER_STALK_1(
		        "netherStalk_1"), NETHER_STALK_2("netherStalk_2"), NETHERQUARTZ(
		        "netherquartz"), OBSIDIAN("obsidian"), ORE_COAL("oreCoal"), ORE_DIAMOND(
		        "oreDiamond"), ORE_EMERALD("oreEmerald"), ORE_GOLD("oreGold"), ORE_IRON(
		        "oreIron"), ORE_LAPIS("oreLapis"), ORE_REDSTONE("oreRedstone"), PISTON_BOTTOM(
		        "piston_bottom"), PISTON_INNER_TOP("piston_inner_top"), PISTON_SIDE(
		        "piston_side"), PISTON_TOP("piston_top"), PISTON_TOP_STICKY(
		        "piston_top_sticky"), PORTAL("portal"), POTATOES_0("potatoes_0"), POTATOES_1(
		        "potatoes_1"), POTATOES_2("potatoes_2"), POTATOES_3("potatoes_3"), PUMPKIN_FACE(
		        "pumpkin_face"), PUMPKIN_JACK("pumpkin_jack"), PUMPKIN_SIDE(
		        "pumpkin_side"), PUMPKIN_TOP("pumpkin_top"), QUARTZBLOCK_BOTTOM(
		        "quartzblock_bottom"), QUARTZBLOCK_CHISELED("quartzblock_chiseled"), QUARTZBLOCK_CHISELED_TOP(
		        "quartzblock_chiseled_top"), QUARTZBLOCK_LINES("quartzblock_lines"), QUARTZBLOCK_LINES_TOP(
		        "quartzblock_lines_top"), QUARTZBLOCK_SIDE("quartzblock_side"), QUARTZBLOCK_TOP(
		        "quartzblock_top"), RAIL("rail"), RAIL_TURN("rail_turn"), REDSTONE_DUST_CROSS(
		        "redstoneDust_cross"), REDSTONE_DUST_CROSS_OVERLAY(
		        "redstoneDust_cross_overlay"), REDSTONE_DUST_LINE("redstoneDust_line"), REDSTONE_DUST_LINE_OVERLAY(
		        "redstoneDust_line_overlay"), REDSTONE_LIGHT("redstoneLight"), REDSTONE_LIGHT_LIT(
		        "redstoneLight_lit"), REDTORCH("redtorch"), REDTORCH_LIT("redtorch_lit"), REEDS(
		        "reeds"), REPEATER("repeater"), REPEATER_LIT("repeater_lit"), ROSE(
		        "rose"), SAND("sand"), SANDSTONE_BOTTOM("sandstone_bottom"), SANDSTONE_CARVED(
		        "sandstone_carved"), SANDSTONE_SIDE("sandstone_side"), SANDSTONE_SMOOTH(
		        "sandstone_smooth"), SANDSTONE_TOP("sandstone_top"), SAPLING("sapling"), SAPLING_BIRCH(
		        "sapling_birch"), SAPLING_JUNGLE("sapling_jungle"), SAPLING_SPRUCE(
		        "sapling_spruce"), SNOW("snow"), SNOW_SIDE("snow_side"), SPONGE("sponge"), STEM_BENT(
		        "stem_bent"), STEM_STRAIGHT("stem_straight"), STONE("stone"), STONE_MOSS(
		        "stoneMoss"), STONEBRICK("stonebrick"), STONEBRICKSMOOTH(
		        "stonebricksmooth"), STONEBRICKSMOOTH_CARVED("stonebricksmooth_carved"), STONEBRICKSMOOTH_CRACKED(
		        "stonebricksmooth_cracked"), STONEBRICKSMOOTH_MOSSY(
		        "stonebricksmooth_mossy"), STONESLAB_SIDE("stoneslab_side"), STONESLAB_TOP(
		        "stoneslab_top"), TALLGRASS("tallgrass"), THINGLASS_TOP("thinglass_top"), TNT_BOTTOM(
		        "tnt_bottom"), TNT_SIDE("tnt_side"), TNT_TOP("tnt_top"), TORCH("torch"), TRAPDOOR(
		        "trapdoor"), TREE_BIRCH("tree_birch"), TREE_JUNGLE("tree_jungle"), TREE_SIDE(
		        "tree_side"), TREE_SPRUCE("tree_spruce"), TREE_TOP("tree_top"), TRIP_WIRE(
		        "tripWire"), TRIP_WIRE_SOURCE("tripWireSource"), VINE("vine"), WATER(
		        "water"), WATER_FLOW("water_flow"), WATERLILY("waterlily"), WEB("web"), WHITE_STONE(
		        "whiteStone"), WOOD("wood"), WOOD_BIRCH("wood_birch"), WOOD_JUNGLE(
		        "wood_jungle"), WOOD_SPRUCE("wood_spruce"), WORKBENCH_FRONT(
		        "workbench_front"), WORKBENCH_SIDE("workbench_side"), WORKBENCH_TOP(
		        "workbench_top"), textures( { &ACTIVATOR_RAIL, &ACTIVATOR_RAIL_POWERED,
		        &ANVIL_BASE, &ANVIL_TOP, &ANVIL_TOP_DAMAGED_1, &ANVIL_TOP_DAMAGED_2,
		        &BEACON, &BED_FEET_END, &BED_FEET_SIDE, &BED_FEET_TOP, &BED_HEAD_END,
		        &BED_HEAD_SIDE, &BED_HEAD_TOP, &BEDROCK, &BLOCK_DIAMOND, &BLOCK_EMERALD,
		        &BLOCK_GOLD, &BLOCK_IRON, &BLOCK_LAPIS, &BLOCK_REDSTONE, &BOOKSHELF,
		        &BREWING_STAND, &BREWING_STAND_BASE, &BRICK, &CACTUS_BOTTOM, &CACTUS_SIDE,
		        &CACTUS_TOP, &CAKE_BOTTOM, &CAKE_INNER, &CAKE_SIDE, &CAKE_TOP, &CARROTS_0,
		        &CARROTS_1, &CARROTS_2, &CARROTS_3, &CAULDRON_BOTTOM, &CAULDRON_INNER,
		        &CAULDRON_SIDE, &CAULDRON_TOP, &CLAY, &CLOTH_0, &CLOTH_1, &CLOTH_10,
		        &CLOTH_11, &CLOTH_12, &CLOTH_13, &CLOTH_14, &CLOTH_15, &CLOTH_2, &CLOTH_3,
		        &CLOTH_4, &CLOTH_5, &CLOTH_6, &CLOTH_7, &CLOTH_8, &CLOTH_9, &COCOA_0,
		        &COCOA_1, &COCOA_2, &COMMAND_BLOCK, &COMPARATOR, &COMPARATOR_LIT,
		        &CROPS_0, &CROPS_1, &CROPS_2, &CROPS_3, &CROPS_4, &CROPS_5, &CROPS_6,
		        &CROPS_7, &DAYLIGHT_DETECTOR_SIDE, &DAYLIGHT_DETECTOR_TOP, &DEADBUSH,
		        &DESTROY_0, &DESTROY_1, &DESTROY_2, &DESTROY_3, &DESTROY_4, &DESTROY_5,
		        &DESTROY_6, &DESTROY_7, &DESTROY_8, &DESTROY_9, &DETECTOR_RAIL,
		        &DETECTOR_RAIL_ON, &DIRT, &DISPENSER_FRONT, &DISPENSER_FRONT_VERTICAL,
		        &DOOR_IRON_LOWER, &DOOR_IRON_UPPER, &DOOR_WOOD_LOWER, &DOOR_WOOD_UPPER,
		        &DRAGON_EGG, &DROPPER_FRONT, &DROPPER_FRONT_VERTICAL, &ENCHANTMENT_BOTTOM,
		        &ENCHANTMENT_SIDE, &ENCHANTMENT_TOP, &ENDFRAME_EYE, &ENDFRAME_SIDE,
		        &ENDFRAME_TOP, &FARMLAND_DRY, &FARMLAND_WET, &FENCE_IRON, &FERN, &FIRE_0,
		        &FIRE_1, &FLOWER, &FLOWER_POT, &FURNACE_FRONT, &FURNACE_FRONT_LIT,
		        &FURNACE_SIDE, &FURNACE_TOP, &GLASS, &GOLDEN_RAIL, &GOLDEN_RAIL_POWERED,
		        &GRASS_SIDE, &GRASS_SIDE_OVERLAY, &GRASS_TOP, &GRAVEL, &HELLROCK,
		        &HELLSAND, &HOPPER, &HOPPER_INSIDE, &HOPPER_TOP, &ICE, &ITEMFRAME_BACK,
		        &JUKEBOX_TOP, &LADDER, &LAVA, &LAVA_FLOW, &LEAVES, &LEAVES_JUNGLE,
		        &LEAVES_JUNGLE_OPAQUE, &LEAVES_OPAQUE, &LEAVES_SPRUCE,
		        &LEAVES_SPRUCE_OPAQUE, &LEVER, &LIGHTGEM, &MELON_SIDE, &MELON_TOP,
		        &MOB_SPAWNER, &MUSHROOM_BROWN, &MUSHROOM_INSIDE, &MUSHROOM_RED,
		        &MUSHROOM_SKIN_BROWN, &MUSHROOM_SKIN_RED, &MUSHROOM_SKIN_STEM,
		        &MUSIC_BLOCK, &MYCEL_SIDE, &MYCEL_TOP, &NETHER_BRICK, &NETHER_STALK_0,
		        &NETHER_STALK_1, &NETHER_STALK_2, &NETHERQUARTZ, &OBSIDIAN, &ORE_COAL,
		        &ORE_DIAMOND, &ORE_EMERALD, &ORE_GOLD, &ORE_IRON, &ORE_LAPIS,
		        &ORE_REDSTONE, &PISTON_BOTTOM, &PISTON_INNER_TOP, &PISTON_SIDE,
		        &PISTON_TOP, &PISTON_TOP_STICKY, &PORTAL, &POTATOES_0, &POTATOES_1,
		        &POTATOES_2, &POTATOES_3, &PUMPKIN_FACE, &PUMPKIN_JACK, &PUMPKIN_SIDE,
		        &PUMPKIN_TOP, &QUARTZBLOCK_BOTTOM, &QUARTZBLOCK_CHISELED,
		        &QUARTZBLOCK_CHISELED_TOP, &QUARTZBLOCK_LINES, &QUARTZBLOCK_LINES_TOP,
		        &QUARTZBLOCK_SIDE, &QUARTZBLOCK_TOP, &RAIL, &RAIL_TURN,
		        &REDSTONE_DUST_CROSS, &REDSTONE_DUST_CROSS_OVERLAY, &REDSTONE_DUST_LINE,
		        &REDSTONE_DUST_LINE_OVERLAY, &REDSTONE_LIGHT, &REDSTONE_LIGHT_LIT,
		        &REDTORCH, &REDTORCH_LIT, &REEDS, &REPEATER, &REPEATER_LIT, &ROSE, &SAND,
		        &SANDSTONE_BOTTOM, &SANDSTONE_CARVED, &SANDSTONE_SIDE, &SANDSTONE_SMOOTH,
		        &SANDSTONE_TOP, &SAPLING, &SAPLING_BIRCH, &SAPLING_JUNGLE,
		        &SAPLING_SPRUCE, &SNOW, &SNOW_SIDE, &SPONGE, &STEM_BENT, &STEM_STRAIGHT,
		        &STONE, &STONE_MOSS, &STONEBRICK, &STONEBRICKSMOOTH,
		        &STONEBRICKSMOOTH_CARVED, &STONEBRICKSMOOTH_CRACKED,
		        &STONEBRICKSMOOTH_MOSSY, &STONESLAB_SIDE, &STONESLAB_TOP, &TALLGRASS,
		        &THINGLASS_TOP, &TNT_BOTTOM, &TNT_SIDE, &TNT_TOP, &TORCH, &TRAPDOOR,
		        &TREE_BIRCH, &TREE_JUNGLE, &TREE_SIDE, &TREE_SPRUCE, &TREE_TOP,
		        &TRIP_WIRE, &TRIP_WIRE_SOURCE, &VINE, &WATER, &WATER_FLOW, &WATERLILY,
		        &WEB, &WHITE_STONE, &WOOD, &WOOD_BIRCH, &WOOD_JUNGLE, &WOOD_SPRUCE,
		        &WORKBENCH_FRONT, &WORKBENCH_SIDE, &WORKBENCH_TOP }) {
}

BlockTextures::~BlockTextures() {
}

/**
 * Loads all block textures from the 'blocks' directory.
 */
bool BlockTextures::load(const std::string& block_dir, int size) {
	if (!fs::exists(block_dir) || !fs::is_directory(block_dir)) {
		std::cerr << "Error: Directory 'blocks' with block textures does not exist." << std::endl;
		return false;
	}

	// go through all textures and load them
	for (size_t i = 0; i < textures.size(); i++) {
		if (!textures[i]->load(block_dir, size))
			std::cerr << "Warning: Unable to load block texture "
				<< textures[i]->getName() << ".png ." << std::endl;
	}
	return true;
}

/**
 * Blits a face on a block image.
 */
void blitFace(Image& image, int face, const Image& texture, int xoff, int yoff,
		bool darken, double darken_left, double darken_right) {
	double d = 1;
	if (darken) {
		if (face == FACE_SOUTH || face == FACE_NORTH)
			d = darken_left;
		else if (face == FACE_WEST || face == FACE_EAST)
			d = darken_right;
	}

	int size = texture.getWidth();

	if (face == FACE_BOTTOM || face == FACE_TOP) {
		if (face == FACE_BOTTOM)
			yoff += size;
		for (TopFaceIterator it(size); !it.end(); it.next()) {
			uint32_t pixel = texture.getPixel(it.src_x, it.src_y);
			image.blendPixel(rgba_multiply(pixel, d, d, d), it.dest_x + xoff,
					it.dest_y + yoff);
		}
	} else {
		int itside = SideFaceIterator::LEFT;
		if (face == FACE_NORTH || face == FACE_SOUTH)
			itside = SideFaceIterator::RIGHT;

		if (face == FACE_EAST || face == FACE_SOUTH)
			xoff += size;
		if (face == FACE_WEST || face == FACE_SOUTH)
			yoff += size / 2;
		for (SideFaceIterator it(size, itside); !it.end(); it.next()) {
			uint32_t pixel = texture.getPixel(it.src_x, it.src_y);
			image.blendPixel(rgba_multiply(pixel, d, d, d), it.dest_x + xoff,
					it.dest_y + yoff);
		}
	}
}

/**
 * Blits the two faces (like a cross from top) to make an item-style block.
 */
void blitItemStyleBlock(Image& image, const Image& north_south, const Image& east_west) {
	int size = MAX(north_south.getWidth(), east_west.getWidth());
	SideFaceIterator it(size, SideFaceIterator::RIGHT);
	for (; !it.end(); it.next()) {
		if (it.src_x > size / 2) {
			uint32_t pixel = east_west.getPixel(it.src_x, it.src_y);
			image.blendPixel(pixel, size / 2 + it.dest_x, size / 4 + it.dest_y);
		}
	}
	it = SideFaceIterator(size, SideFaceIterator::LEFT);
	for (; !it.end(); it.next()) {
		uint32_t pixel = north_south.getPixel(it.src_x, it.src_y);
		image.blendPixel(pixel, size / 2 + it.dest_x, size / 4 + it.dest_y);
	}
	it = SideFaceIterator(size, SideFaceIterator::RIGHT);
	for (; !it.end(); it.next()) {
		if (it.src_x <= size / 2) {
			uint32_t pixel = east_west.getPixel(it.src_x, it.src_y);
			image.blendPixel(pixel, size / 2 + it.dest_x, size / 4 + it.dest_y);
		}
	}
}

/**
 * This function rotates the north-, south-, east- and west-oriented block images
 * by swapping their images. Used for special block images: stairs, rails
 */
void rotateImages(Image& north, Image& south, Image& east, Image& west, int rotation) {
	std::map<int, Image> images;
	images[rotate_shift_r(FACE_NORTH, rotation, 4)] = north;
	images[rotate_shift_r(FACE_SOUTH, rotation, 4)] = south;
	images[rotate_shift_r(FACE_EAST, rotation, 4)] = east;
	images[rotate_shift_r(FACE_WEST, rotation, 4)] = west;

	north = images[FACE_NORTH];
	south = images[FACE_SOUTH];
	east = images[FACE_EAST];
	west = images[FACE_WEST];
}

BlockImage::BlockImage(int type)
		: type(type) {
	for (int i = 0; i < 6; i++) {
		x_offsets[i] = 0;
		y_offsets[i] = 0;
	}
}

BlockImage::~BlockImage() {
}

/**
 * Sets a face of a block image. You can use this method also to set more than one face
 * to the same texture.
 */
BlockImage& BlockImage::setFace(int face, const Image& texture, int xoff, int yoff) {
	for (int i = 0; i < 6; i++)
		if (face & (1 << i)) {
			faces[i] = texture;
			x_offsets[i] = xoff;
			y_offsets[i] = yoff;
		}
	return *this;
}

/**
 * Returns the texture of a face.
 */
const Image& BlockImage::getFace(int face) const {
	for (int i = 0; i < 6; i++)
		if (face == (1 << i))
			return faces[i];
	return empty_image;
}

int BlockImage::getXOffset(int face) const {
	for (int i = 0; i < 6; i++)
		if (face == (1 << i))
			return x_offsets[i];
	return 0;
}

int BlockImage::getYOffset(int face) const {
	for (int i = 0; i < 6; i++)
		if (face == (1 << i))
			return y_offsets[i];
	return 0;
}

/**
 * Returns this block count*90 degrees rotated.
 */
BlockImage BlockImage::rotate(int count) const {
	count = count % 4;
	if (count == 0)
		return BlockImage(*this);

	BlockImage rotated(type);
	for (int i = 0; i < 4; i++) {
		int face = 1 << i;
		int new_face = rotate_shift_l(face, count, 4);
		rotated.setFace(new_face, getFace(face), getXOffset(face), getYOffset(face));
	}

	Image top = getFace(FACE_TOP).rotate(count);
	Image bottom = getFace(FACE_BOTTOM).rotate(count);
	rotated.setFace(FACE_TOP, top, getXOffset(FACE_TOP), getYOffset(FACE_TOP));
	rotated.setFace(FACE_BOTTOM, bottom, getXOffset(FACE_BOTTOM), getYOffset(FACE_BOTTOM));
	return rotated;
}

/**
 * Creates the block image from the textures.
 */
Image BlockImage::buildImage(double dleft, double dright) const {
	Image image;

	int size = 0;
	for(int i = 0; i < 6; i++)
		size = MAX(size, faces[i].getWidth());
	image.setSize(size * 2, size * 2);

	if (type == NORMAL) {
		int order[] = {FACE_BOTTOM, FACE_NORTH, FACE_EAST, FACE_WEST, FACE_SOUTH, FACE_TOP};

		for(int i = 0; i < 6; i++) {
			int face = order[i];
			int xoff = getXOffset(face), yoff = getYOffset(face);
			if (face == FACE_NORTH || face == FACE_EAST)
				blitFace(image, face, getFace(face).flip(true, false),
						xoff, yoff, true, dleft, dright);
			else
				blitFace(image, face, getFace(face),
						xoff, yoff, true, dleft, dright);
		}
	} else if (type == ITEM_STYLE) {
		blitItemStyleBlock(image, getFace(FACE_NORTH), getFace(FACE_EAST));
	}

	return image;
}

BlockImages::BlockImages()
		: texture_size(16), rotation(0), render_unknown_blocks(false),
		  render_leaves_transparent(false), max_water(99),
		  dleft(0.75), dright(0.6) {
}

BlockImages::~BlockImages() {
}

void BlockImages::setSettings(int texture_size, int rotation, bool render_unknown_blocks,
        bool render_leaves_transparent, const std::string& rendermode) {
	this->texture_size = texture_size;
	this->rotation = rotation;
	this->render_unknown_blocks = render_unknown_blocks;
	this->render_leaves_transparent = render_leaves_transparent;

	if (rendermode == "daylight" || rendermode == "nightlight") {
		dleft = 0.95;
		dright = 0.8;
	}
}

/**
 * This function converts the chest image to usable chest textures and stores them
 * in the textures array.
 */
bool loadChestTexture(const Image& image, Image* textures, int texture_size) {
	if (image.getWidth() != image.getHeight())
		return false;
	// if the image is 64px wide, the chest images are 14x14
	int ratio = image.getHeight() / 64;
	int size = ratio * 14;

	Image front = image.clip(size, 29 * ratio, size, size);
	front.alphablit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 6 * ratio, 3 * ratio);
	Image side = image.clip(0, 29 * ratio, size, size);
	side.alphablit(image.clip(0, size, size, 4 * ratio), 0, 0);
	Image top = image.clip(size, 0, size, size);

	// resize the chest images to texture size
	front.resizeInterpolated(texture_size, texture_size, textures[CHEST_FRONT]);
	side.resizeInterpolated(texture_size, texture_size, textures[CHEST_SIDE]);
	top.resizeInterpolated(texture_size, texture_size, textures[CHEST_TOP]);

	return true;
}

/**
 * This function converts the large chest image to usable chest textures and stores them
 * in the textures array.
 */
bool loadLargeChestTexture(const Image& image, Image* textures, int texture_size) {
	if (image.getWidth() != image.getHeight() * 2)
		return false;
	int ratio = image.getHeight() / 64;
	int size = ratio * 14;

	// note here that a whole chest is 30*ratio pixels wide, but our
	// chest textures are only 14x14 * ratio pixels, so we need to omit two rows in the middle
	// => the second image starts not at x*size, it starts at x*size+2*ratio
	Image front_left = image.clip(size, 29 * ratio, size, size);
	front_left.alphablit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front_left.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 13 * ratio,
	        3 * ratio);
	Image front_right = image.clip(2 * size + 2 * ratio, 29 * ratio, size, size);
	front_right.alphablit(image.clip(2 * size + 2 * ratio, size, size, 4 * ratio), 0, 0);
	front_right.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), -ratio,
	        3 * ratio);

	Image side = image.clip(0, 29 * ratio, size, size);
	side.alphablit(image.clip(0, size, size, 4 * ratio), 0, 0);

	Image top_left = image.clip(size, 0, size, size);
	Image top_right = image.clip(2 * size + 2 * ratio, 0, size, size);

	Image back_left = image.clip(4 * size + 2, 29 * ratio, size, size);
	back_left.alphablit(image.clip(4 * size + 2, size, size, 4 * ratio), 0, 0);
	Image back_right = image.clip(5 * size + 4, 29 * ratio, size, size);
	back_right.alphablit(image.clip(5 * size + 4, size, size, 4 * ratio), 0, 0);

	// resize the chest images to texture size
	front_left.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_FRONT_LEFT]);
	front_right.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_FRONT_RIGHT]);
	side.resizeInterpolated(texture_size, texture_size, textures[LARGECHEST_SIDE]);
	top_left.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_TOP_LEFT]);
	top_right.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_TOP_RIGHT]);
	back_left.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_BACK_LEFT]);
	back_right.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_BACK_RIGHT]);

	return true;
}

bool BlockImages::loadChests(const std::string& normal, const std::string& large,
        const std::string& ender) {
	Image img_chest, img_largechest, img_enderchest;
	if (!img_chest.readPNG(normal) || !img_largechest.readPNG(large)
	        || !img_enderchest.readPNG(ender))
		return false;

	if (!loadChestTexture(img_chest, chest, texture_size)
	        || !loadChestTexture(img_enderchest, enderchest, texture_size)
	        || !loadLargeChestTexture(img_largechest, largechest, texture_size))
		return false;
	return true;
}

bool BlockImages::loadColors(const std::string& foliagecolor,
        const std::string& grasscolor) {
	return foliagecolors.readPNG(foliagecolor) && grasscolors.readPNG(grasscolor);
}

bool BlockImages::loadOther(const std::string& fire, const std::string& endportal) {
	Image fire_img, endportal_img;
	if(!fire_img.readPNG(fire) || !endportal_img.readPNG(endportal))
		return false;
	fire_img.resizeInterpolated(texture_size, texture_size, fire_texture);
	endportal_img.resizeInterpolated(texture_size, texture_size, endportal_texture);
	return true;
}

bool BlockImages::loadBlocks(const std::string& block_dir) {
	if (!textures.load(block_dir, texture_size))
		return false;

	empty_texture.setSize(texture_size, texture_size);
	unknown_block.setSize(texture_size, texture_size);
	if (render_unknown_blocks)
		unknown_block.fill(rgba(255, 0, 0, 255), 0, 0, texture_size, texture_size);

	loadBlocks();
	testWaterTransparency();
	createBiomeBlocks();
	return true;
}

bool BlockImages::loadAll(const std::string& textures_dir) {
	if (!loadChests(textures_dir + "/chest.png", textures_dir + "/largechest.png",
			textures_dir + "/enderchest.png")) {
		std::cerr << "Error: Unable to load chest.png, largechest.png or enderchest.png!"
				<< std::endl;
		return false;
	} else if (!loadColors(textures_dir + "/foliagecolor.png",
			textures_dir + "/grasscolor.png")) {
		std::cerr << "Error: Unable to load foliagecolor.png or grasscolor.png!"
				<< std::endl;
		return false;
	} else if (!loadOther(textures_dir + "/fire.png",
			textures_dir + "/endportal.png")) {
		std::cerr << "Error: Unable to load fire.png or endportal.png!" << std::endl;
		return false;
	} else if (!loadBlocks(textures_dir + "/blocks")) {
		std::cerr << "Error: Unable to load block textures!" << std::endl;
		return false;
	}
	return true;
}

/**
 * Comparator to sort the block int's with id and data.
 */
struct block_comparator {
	bool operator()(uint32_t b1, uint32_t b2) const {
		uint16_t id1 = b1 & 0xffff;
		uint16_t id2 = b2 & 0xffff;
		if (id1 != id2)
			return id1 < id2;
		uint16_t data1 = (b1 & 0xffff0000) >> 16;
		uint16_t data2 = (b2 & 0xffff0000) >> 16;
		return data1 < data2;
	}
};

bool BlockImages::saveBlocks(const std::string& filename) {
	std::map<uint32_t, Image, block_comparator> blocks_sorted;
	for (std::unordered_map<uint32_t, Image>::const_iterator it = block_images.begin();
	        it != block_images.end(); ++it) {
		uint16_t data = (it->first & 0xffff0000) >> 16;
		if ((data & (EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM)) == 0)
			blocks_sorted[it->first] = it->second;
	}

	std::vector<Image> blocks;
	for (std::map<uint32_t, Image>::const_iterator it = blocks_sorted.begin();
	        it != blocks_sorted.end(); ++it)
		blocks.push_back(it->second);

	blocks.push_back(opaque_water[0]);
	blocks.push_back(opaque_water[1]);
	blocks.push_back(opaque_water[2]);
	blocks.push_back(opaque_water[3]);

	/*
	for (std::unordered_map<uint64_t, Image>::const_iterator it = biome_images.begin();
			it != biome_images.end(); ++it)
		blocks.push_back(it->second);
	*/

	int blocksize = getBlockImageSize();
	int width = 16;
	int height = std::ceil(blocks.size() / (double) width);
	Image img(width * blocksize, height * blocksize);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int offset = y * width + x;
			if ((size_t) offset >= blocks.size())
				break;
			img.alphablit(blocks.at(offset), x * blocksize, y * blocksize);
		}
	}
	std::cout << block_images.size() << " blocks" << std::endl;
	std::cout << "all: " << blocks.size() << std::endl;

	/*
	srand(time(NULL));
	BlockImage test(BlockImage::ITEM_STYLE);
	//for(int i = 0; i < 6; i++)
	//	test.setFace(1 << i, getTexture(rand() % 15, rand() % 15));
	test.setFace(FACE_NORTH | FACE_SOUTH, getTexture(rand() % 15, rand() % 15));
	test.setFace(FACE_EAST | FACE_WEST, getTexture(rand() % 15, rand() % 15));

	Image testimg(32*5, 32);
	for(int i = 0; i < 5; i++) {
		BlockImage block = test.rotate(i);
		Image test = block.buildImage();
		testimg.simpleblit(test, i*32, 0);
	}
	testimg.writePNG("test.png");
	*/

	/*
	Image terrain(texture_size * 16, texture_size * 16);
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			terrain.simpleblit(getTexture(x, y), texture_size * x, texture_size * y);
		}
	}
	terrain.writePNG("test.png");
	*/

	return img.writePNG(filename);
}

/**
 * This method filters unnecessary block data, for example the leaves decay counter.
 */
uint16_t BlockImages::filterBlockData(uint16_t id, uint16_t data) const {
	if (id == 6)
		return data & (0xff00 | 0b00000011);
	else if (id >= 8 && id <= 11) // water, lava
		return data & (0xff00 | 0b11110111);
	else if (id == 18) // leaves
		return data & (0xff00 | 0b00000011);
	else if (id == 26) // bed
		return data & (0xff00 | 0b00001011);
	else if (id == 54 || id == 95 || id == 130) { // chests
		// at first get the direction of the chest and rotate if needed
		uint16_t dir_rotate = (data >> 4) & 0xf;
		uint16_t dir = rotate_shift_l(dir_rotate, rotation, 4) << 4;
		// then get the neighbor chests
		uint16_t neighbors = (data >> 4) & 0xf0;

		// if no neighbors, this is a small chest
		// the data contains only the direction
		if (neighbors == 0 || id == 95 || id == 130)
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
	} else if (id == 60) // farmland
		return data & 0xff00;
	else if (id == 64 || id == 71) // doors
		return data & 0b1111110000;
	else if (id == 81 || id == 83 || id == 92) // cactus, sugar cane, cake
		return data & 0xff00;
	else if (id == 93 || id == 94) // redstone repeater
		return data & (0xff00 | 0b00000011);
	else if (id == 117) // brewing stand
		return data & 0xff00;
	else if (id == 119 || id == 120) // end portal, end portal frame
		return data & 0xff00;
	// the light sensor shouldn't have any data, but I had problems with it...
	else if (id == 151)
		return 0;
	return data;
}

/**
 * Checks, if a block images has transparent pixels.
 */
bool BlockImages::checkImageTransparency(const Image& image) const {
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (ALPHA(image.getPixel(it.dest_x, it.dest_y + texture_size/2)) < 255)
			return true;
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (ALPHA(image.getPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2))
		        < 255)
			return true;
	}
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		if (ALPHA(image.getPixel(it.dest_x, it.dest_y)) < 255)
			return true;
	}
	return false;
}

/**
 * This method adds to the block image the dark shadow edges by blitting the shadow edge
 * masks and then stores the block image with the special data.
 */
void BlockImages::addBlockShadowEdges(uint16_t id, uint16_t data, const Image& block) {
	for (int n = 0; n <= 1; n++)
		for (int e = 0; e <= 1; e++)
			for (int b = 0; b <= 1; b++) {
				Image image = block;
				uint16_t extra_data = 0;
				if (n) {
					image.alphablit(shadow_edge_masks[0], 0, 0);
					extra_data |= EDGE_NORTH;
				}
				if (e) {
					image.alphablit(shadow_edge_masks[1], 0, 0);
					extra_data |= EDGE_EAST;
				}
				if (b) {
					image.alphablit(shadow_edge_masks[2], 0, 0);
					extra_data |= EDGE_BOTTOM;
				}
				block_images[id | ((data | extra_data) << 16)] = image;
			}
}

/**
 * Sets a block image in the block image list (and rotates it if necessary (later)).
 */
void BlockImages::setBlockImage(uint16_t id, uint16_t data, const BlockImage& block) {
	setBlockImage(id, data, buildImage(block.rotate(rotation)));
}

/**
 * Sets a block image in the block image list.
 */
void BlockImages::setBlockImage(uint16_t id, uint16_t data, const Image& block) {
	block_images[id | (data << 16)] = block;

	// check if block contains transparency
	if (checkImageTransparency(block))
		block_transparency.insert(id | (data << 16));
	// if block is not transparent, add shadow edges
	else
		addBlockShadowEdges(id, data, block);
}

Image BlockImages::createBiomeBlock(uint16_t id, uint16_t data,
        const Biome& biome_data) const {
	if (!block_images.count(id | (data << 16)))
		return unknown_block;

	uint32_t color;
	// leaves have the foliage colors
	// for birches, the color x/y coordinate is flipped
	if (id == 18)
		color = biome_data.getColor(foliagecolors, (data & 0b11) == 2);
	else
		color = biome_data.getColor(grasscolors, false);

	double r = (double) RED(color) / 255;
	double g = (double) GREEN(color) / 255;
	double b = (double) BLUE(color) / 255;
	
	// multiply with fixed biome color values
	// necessary for the Swampland biome
	r *= (double) biome_data.r / 255;
	g *= (double) biome_data.g / 255;
	b *= (double) biome_data.b / 255;

	// grass block needs something special
	if (id == 2) {
		Image block = block_images.at(id | (data << 16));
		Image side = textures.GRASS_SIDE_OVERLAY.colorize(r, g, b);

		// blit the side overlay over the block
		blitFace(block, FACE_WEST, side, 0, 0, false);
		blitFace(block, FACE_SOUTH, side, 0, 0, false);
		
		// now tint the top of the block
		for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
			uint32_t pixel = block.getPixel(it.dest_x, it.dest_y);
			block.setPixel(it.dest_x, it.dest_y, rgba_multiply(pixel, r, g, b));
		}

		return block;
	}

	return block_images.at(id | (data << 16)).colorize(r, g, b);
}

void BlockImages::createBiomeBlocks() {
	for (std::unordered_map<uint32_t, Image>::iterator it = block_images.begin();
			it != block_images.end(); ++it) {
		uint16_t id = it->first & 0xffff;
		uint16_t data = (it->first & 0xffff0000) >> 16;

		// grass block, leaves, grass, vines, lily pad
		if (id != 2 && id != 18 && id != 31 && id != 106 && id != 111)
			continue;

		for (uint64_t b = 0; b < BIOMES_SIZE; b++) {
			Biome biome = BIOMES[b];
			biome_images[id | ((uint64_t) data << 16) | (b << 32)] =
					createBiomeBlock(id, data, biome);
		}
	}
}

/**
 * This method is very important for the rendering performance. It preblits transparent
 * water blocks until they are nearly opaque.
 */
void BlockImages::testWaterTransparency() {
	// just use the Ocean biome watercolor
	Image water = textures.WATER.colorize(0, 0.39, 0.89);

	// opaque_water[0] is water block when water texture is only on the top
	opaque_water[0].setSize(getBlockImageSize(), getBlockImageSize());
	blitFace(opaque_water[0], FACE_TOP, water, 0, 0, false);
	// same, water top and south (right)
	opaque_water[1] = opaque_water[0];
	// water top and west (left)
	opaque_water[2] = opaque_water[0];
	// water top, south and west
	opaque_water[3] = opaque_water[0];

	// now blit actual faces
	blitFace(opaque_water[1], FACE_SOUTH, water, 0, 0, false);
	blitFace(opaque_water[2], FACE_WEST, water, 0, 0, false);
	blitFace(opaque_water[3], FACE_SOUTH, water, 0, 0, false);
	blitFace(opaque_water[3], FACE_WEST, water, 0, 0, false);

	for (max_water = 2; max_water < 10; max_water++) {
		// make a copy of the first images
		Image tmp = opaque_water[0];
		// blit it over
		tmp.alphablit(tmp, 0, 0);

		// then check alpha
		uint16_t min_alpha = 255;
		for (TopFaceIterator it(texture_size); !it.end(); it.next())
			min_alpha = MIN(min_alpha, ALPHA(tmp.getPixel(it.dest_x, it.dest_y)));

		// images are "enough" opaque
		if (min_alpha == 255) {
			// do a last blit
			blitFace(opaque_water[0], FACE_TOP, water, 0, 0, false);
			blitFace(opaque_water[1], FACE_TOP, water, 0, 0, false);
			blitFace(opaque_water[2], FACE_TOP, water, 0, 0, false);
			blitFace(opaque_water[3], FACE_TOP, water, 0, 0, false);

			blitFace(opaque_water[1], FACE_SOUTH, water, 0, 0, true, dleft, dright);
			blitFace(opaque_water[2], FACE_WEST, water, 0, 0, true, dleft, dright);
			blitFace(opaque_water[3], FACE_SOUTH, water, 0, 0, true, dleft, dright);
			blitFace(opaque_water[3], FACE_WEST, water, 0, 0, true, dleft, dright);
			break;
		// when images are too transparent
		} else {
			// blit all images over
			for (int i = 0; i < 4; i++)
				opaque_water[i].alphablit(opaque_water[i], 0, 0);
		}
	}
}

uint32_t BlockImages::darkenLeft(uint32_t pixel) const {
	return rgba_multiply(pixel, dleft, dleft, dleft);
}

uint32_t BlockImages::darkenRight(uint32_t pixel) const {
	return rgba_multiply(pixel, dright, dright, dright);
}

Image BlockImages::buildImage(const BlockImage& image) {
	return image.buildImage(dleft, dright);
}

BlockImage BlockImages::buildSmallerBlock(const Image& left_texture,
        const Image& right_texture, const Image& top_texture, int y1, int y2) {
	Image left = left_texture;
	Image right = right_texture;
	left.fill(0, 0, 0, texture_size, texture_size - y2);
	right.fill(0, 0, 0, texture_size, texture_size - y2);

	left.fill(0, 0, texture_size - y1, texture_size, y1);
	right.fill(0, 0, texture_size - y1, texture_size, y1);

	BlockImage block;
	block.setFace(FACE_EAST | FACE_WEST, left);
	block.setFace(FACE_NORTH | FACE_SOUTH, right);
	block.setFace(FACE_TOP, top_texture, 0, texture_size - y2);
	return block;
}

Image BlockImages::buildStairsSouth(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);

	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_x > texture_size / 2 ? 0 : texture_size / 2;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_x >= texture_size / 2 || it.src_y >= texture_size / 2 - 1) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
	}
	return block;
}

Image BlockImages::buildStairsNorth(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_x >= texture_size / 2 ? texture_size / 2 : 0;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_x <= texture_size / 2 || it.src_y >= texture_size / 2) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? texture_size : texture_size / 2;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 : texture_size / 4;
		uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}
	return block;
}

Image BlockImages::buildStairsWest(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_y > texture_size / 2 ? 0 : texture_size / 2;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
	}

	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_x <= texture_size / 2 || it.src_y >= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}
	return block;
}

Image BlockImages::buildStairsEast(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_y > texture_size / 2 ? texture_size / 2 : 0;
		// fix to prevent a transparent gap
		if (it.src_y == texture_size / 2 && it.src_x % 2 == 0)
			y = texture_size / 2;
		if (it.src_y == texture_size / 2 - 1 && it.src_x % 2 == 0)
			y = texture_size / 2;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? 0 : texture_size / 2;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 : texture_size / 4;
		uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_x >= texture_size / 2 || it.src_y >= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockImages::buildUpsideDownStairsNorth(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_TOP, texture, 0, 0, true, dleft, dright);

	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? texture_size / 2 : texture_size;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 - 4 : texture_size / 2;
		uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x < texture_size / 2) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockImages::buildUpsideDownStairsSouth(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_SOUTH, texture, 0, 0, true, dleft, dright);
	blitFace(block, FACE_TOP, texture, 0, 0, true, dleft, dright);
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x >= texture_size / 2) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockImages::buildUpsideDownStairsEast(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_TOP, texture, 0, 0, true, dleft, dright);

	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? texture_size / 2 : 0;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 - 4 : texture_size / 2;
		uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}

	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x >= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockImages::buildUpsideDownStairsWest(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_WEST, texture, 0, 0, true, dleft, dright);
	blitFace(block, FACE_TOP, texture, 0, 0, true, dleft, dright);
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x <= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

void BlockImages::buildCustomTextures() {
	shadow_edge_masks[0].setSize(getBlockImageSize(), getBlockImageSize());
	shadow_edge_masks[1].setSize(getBlockImageSize(), getBlockImageSize());
	shadow_edge_masks[2].setSize(getBlockImageSize(), getBlockImageSize());

	uint32_t darkness = rgba(0, 0, 0, 64);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		if (it.src_x < 1)
			shadow_edge_masks[0].setPixel(it.dest_x, it.dest_y, darkness);
		if (it.src_y < 1)
			shadow_edge_masks[1].setPixel(it.dest_x, it.dest_y, darkness);
		if (it.src_x == texture_size - 1 || it.src_y == texture_size - 1)
			shadow_edge_masks[2].setPixel(it.dest_x, it.dest_y + texture_size, darkness);
	}
}

void BlockImages::createBlock(uint16_t id, uint16_t data, const Image& texture) {
	createBlock(id, data, texture, texture);
}

void BlockImages::createBlock(uint16_t id, uint16_t data, const Image& side_texture,
        const Image& top_texture) {
	createBlock(id, data, side_texture, side_texture, top_texture);
}

void BlockImages::createBlock(uint16_t id, uint16_t data, const Image& left_texture,
        const Image& right_texture, const Image& top_texture) {
	BlockImage block;
	block.setFace(FACE_EAST | FACE_WEST, left_texture);
	block.setFace(FACE_NORTH | FACE_SOUTH, right_texture);
	block.setFace(FACE_TOP, top_texture);
	setBlockImage(id, data, block);
}

void BlockImages::createSmallerBlock(uint16_t id, uint16_t data,
        const Image& left_texture, const Image& right_texture, const Image& top_texture,
        int y1, int y2) {
	setBlockImage(id, data,
	        buildSmallerBlock(left_texture, right_texture, top_texture, y1, y2));
}

void BlockImages::createSmallerBlock(uint16_t id, uint16_t data, const Image& side_face,
        const Image& top_texture, int y1, int y2) {
	setBlockImage(id, data,
	        buildSmallerBlock(side_face, side_face, top_texture, y1, y2));
}

void BlockImages::createRotatedBlock(uint16_t id, uint16_t extra_data,
        const Image& front_texture, const Image& side_texture,
        const Image& top_texture) {
	createRotatedBlock(id, extra_data, front_texture, side_texture, side_texture,
	        top_texture);
}

void BlockImages::createRotatedBlock(uint16_t id, uint16_t extra_data,
        const Image& front_texture, const Image& back_texture, const Image& side_texture,
        const Image& top_texture) {
	BlockImage block;
	block.setFace(FACE_NORTH, front_texture.flip(true, false));
	block.setFace(FACE_SOUTH, back_texture);
	block.setFace(FACE_EAST, side_texture.flip(true, false));
	block.setFace(FACE_WEST, side_texture);
	block.setFace(FACE_TOP, top_texture);

	setBlockImage(id, 2 | extra_data, block);
	setBlockImage(id, 3 | extra_data, block.rotate(2));
	setBlockImage(id, 4 | extra_data, block.rotate(3));
	setBlockImage(id, 5 | extra_data, block.rotate(1));
}

void BlockImages::createItemStyleBlock(uint16_t id, uint16_t data,
        const Image& texture) {
	createItemStyleBlock(id, data, texture, texture);
}

void BlockImages::createItemStyleBlock(uint16_t id, uint16_t data,
        const Image& north_south, const Image& east_west) {
	BlockImage block(BlockImage::ITEM_STYLE);
	block.setFace(FACE_NORTH | FACE_SOUTH, north_south);
	block.setFace(FACE_EAST | FACE_WEST, east_west);
	setBlockImage(id, data, block);
}

void BlockImages::createSingleFaceBlock(uint16_t id, uint16_t data, int face,
        const Image& texture) {
	setBlockImage(id, data, BlockImage().setFace(face, texture));
}

void BlockImages::createGrassBlock() { // id 2
	Image dirt = textures.DIRT;

	Image grass = dirt;
	Image grass_mask = textures.GRASS_SIDE_OVERLAY;
	grass.alphablit(grass_mask, 0, 0);

	Image top = textures.GRASS_TOP;

	BlockImage block;
	block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, grass);
	block.setFace(FACE_TOP, top);
	setBlockImage(2, 0, block);
	
	// create the snowy version
	grass = textures.SNOW_SIDE;
	top = textures.SNOW;

	BlockImage block_snow;
	block_snow.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, grass);
	block_snow.setFace(FACE_TOP, top);
	setBlockImage(2, GRASS_SNOW, block_snow);
}

void BlockImages::createWater() { // id 8, 9
	Image water = textures.WATER.colorize(0, 0.39, 0.89);
	for (int data = 0; data < 8; data++) {
		int smaller = data / 8.0 * texture_size;
		Image side_texture = water.move(0, smaller);

		Image block(getBlockImageSize(), getBlockImageSize());
		blitFace(block, FACE_WEST, side_texture, 0, 0, true, dleft, dright);
		blitFace(block, FACE_SOUTH, side_texture, 0, 0, true, dleft, dright);
		blitFace(block, FACE_TOP, water, 0, smaller, true, dleft, dright);
		setBlockImage(8, data, block);
		setBlockImage(9, data, block);
	}

	for (int i = 0; i <= 0b111; i++) {
		bool west = i & 0b100;
		bool south = i & 0b010;
		bool top = i & 0b001;

		Image block(getBlockImageSize(), getBlockImageSize());
		uint16_t extra_data = 0;

		if (top)
			blitFace(block, FACE_TOP, water, 0, 0, true, dleft, dright);
		else
			extra_data |= DATA_TOP;
		if (west)
			blitFace(block, FACE_WEST, water, 0, 0, true, dleft, dright);
		else
			extra_data |= DATA_WEST;

		if (south)
			blitFace(block, FACE_SOUTH, water, 0, 0, true, dleft, dright);
		else
			extra_data |= DATA_SOUTH;

		setBlockImage(8, extra_data, block);
		setBlockImage(9, extra_data, block);
	}
}

void BlockImages::createLava() { // id 10, 11
	Image lava = textures.LAVA;
	for (int data = 0; data < 7; data += 2) {
		int smaller = data / 8.0 * texture_size;
		Image side_texture = lava.move(0, smaller);

		BlockImage block;
		block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, side_texture);
		block.setFace(FACE_TOP, lava, 0, smaller);
		setBlockImage(10, data, block);
		setBlockImage(11, data, block);
	}
}

void BlockImages::createWood(uint16_t id, uint16_t data, const Image& side, const Image& top) { // id 17
	createBlock(id, data | 4, top, side, side);
	createBlock(id, data | 8, side, top, side);
	createBlock(id, data, side, side, top); // old format
	createBlock(id, data | 4 | 8, side, side, top);
}

void BlockImages::createLeaves() { // id 18
	if (render_leaves_transparent) {
		createBlock(18, 0, textures.LEAVES); // oak
		createBlock(18, 1, textures.LEAVES_SPRUCE); // pine/spruce
		createBlock(18, 2, textures.LEAVES); // birch
		createBlock(18, 3, textures.LEAVES_JUNGLE); // jungle
	} else {
		createBlock(18, 0, textures.LEAVES_OPAQUE); // oak
		createBlock(18, 1, textures.LEAVES_SPRUCE_OPAQUE); // pine/spruce
		createBlock(18, 2, textures.LEAVES_OPAQUE); // birch
		createBlock(18, 3, textures.LEAVES_JUNGLE_OPAQUE); // jungle
	}
}

void BlockImages::createDispenserDropper(uint16_t id, const Image& front) { // id 23, 158
	Image side = textures.FURNACE_SIDE;
	Image top = textures.FURNACE_TOP;

	createRotatedBlock(id, 0, front, side, top);
	createBlock(id, 0, side, top);
	createBlock(id, 1, side, front);
}

BlockImage buildBed(const Image& top, const Image& north_south, const Image& east_west,
		int face_skip) {
	BlockImage block;

	block.setFace(FACE_TOP, top, 0, top.getHeight() / 16. * 7.0);
	if (face_skip != FACE_NORTH)
		block.setFace(FACE_NORTH, north_south.flip(true, false));
	if (face_skip != FACE_SOUTH)
		block.setFace(FACE_SOUTH, north_south);
	if (face_skip != FACE_EAST)
		block.setFace(FACE_EAST, east_west.flip(true, false));
	if (face_skip != FACE_WEST)
		block.setFace(FACE_WEST, east_west);

	return block;
}

void BlockImages::createBed() { // id 26
	Image front = textures.BED_FEET_END;
	Image side = textures.BED_FEET_SIDE;
	Image top = textures.BED_FEET_TOP;

	setBlockImage(26, 0, buildBed(top.rotate(1), front, side, FACE_SOUTH));
	setBlockImage(26, 1, buildBed(top.rotate(2), side.flip(true, false), front, FACE_WEST));
	setBlockImage(26, 2, buildBed(top.rotate(3), front, side.flip(true, false), FACE_NORTH));
	setBlockImage(26, 3, buildBed(top, side, front, FACE_EAST));

	front = textures.BED_HEAD_END;
	side = textures.BED_HEAD_SIDE;
	top = textures.BED_HEAD_TOP;

	setBlockImage(26, 8, buildBed(top, front, side, FACE_NORTH));
	setBlockImage(26, 1 | 8, buildBed(top.rotate(1), side.flip(true, false), front, FACE_EAST));
	setBlockImage(26, 2 | 8, buildBed(top.rotate(2), front, side.flip(true, false), FACE_SOUTH));
	setBlockImage(26, 3 | 8, buildBed(top.rotate(3), side, front, FACE_WEST));
}

void BlockImages::createStraightRails(uint16_t id, uint16_t extra_data,
        const Image& texture) { // id 27, 28, 66
	createSingleFaceBlock(id, 0 | extra_data, FACE_BOTTOM, texture.rotate(ROTATE_90));
	createSingleFaceBlock(id, 1 | extra_data, FACE_BOTTOM, texture);

	Image rotated_texture = texture.rotate(ROTATE_90);
	Image north(getBlockImageSize(), getBlockImageSize()),
			south(getBlockImageSize(), getBlockImageSize()),
			east(getBlockImageSize(), getBlockImageSize()),
			west(getBlockImageSize(), getBlockImageSize());

	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		east.setPixel(it.dest_x, it.dest_y + it.src_y,
				texture.getPixel(it.src_x, it.src_y));
		east.setPixel(it.dest_x + 1, it.dest_y + it.src_y,
		        texture.getPixel(it.src_x, it.src_y));

		west.setPixel(it.dest_x, it.dest_y + (texture_size - it.src_y),
		        texture.getPixel(it.src_x, it.src_y));
		west.setPixel(it.dest_x, it.dest_y + (texture_size - it.src_y),
		        texture.getPixel(it.src_x, it.src_y));
	}

	north = east.flip(true, false);
	south = west.flip(true, false);

	rotateImages(north, south, east, west, rotation);

	setBlockImage(id, 2 | extra_data, east);
	setBlockImage(id, 3 | extra_data, west);
	setBlockImage(id, 4 | extra_data, north);
	setBlockImage(id, 5 | extra_data, south);
}

BlockImage buildPiston(int frontface, const Image& front, const Image& back,
		const Image& side, const Image& top) {
	BlockImage block;

	block.setFace(FACE_TOP, top);
	block.setFace(frontface, front);
	if(frontface == FACE_NORTH || frontface == FACE_SOUTH) {
		block.setFace(FACE_EAST, side.flip(true, false));
		block.setFace(FACE_WEST, side);
	} else {
		block.setFace(FACE_NORTH, side.flip(true, false));
		block.setFace(FACE_SOUTH, side);
	}

	if(frontface == FACE_NORTH)
		block.setFace(FACE_SOUTH, back);
	else if(frontface == FACE_SOUTH)
		block.setFace(FACE_NORTH, back);
	else if(frontface == FACE_EAST)
		block.setFace(FACE_WEST, back);
	else
		block.setFace(FACE_EAST, back);

	return block;
}

void BlockImages::createPiston(uint16_t id, bool sticky) { //  id 29, 33
	Image front = sticky ? textures.PISTON_TOP_STICKY : textures.PISTON_TOP;
	Image side = textures.PISTON_SIDE;
	Image back = textures.PISTON_BOTTOM;

	createBlock(id, 0, side.rotate(ROTATE_180), back);
	createBlock(id, 1, side, front);

	setBlockImage(id, 2, buildPiston(FACE_NORTH, front, back, side.rotate(3), side.rotate(3)));
	setBlockImage(id, 3, buildPiston(FACE_EAST, front, back, side.rotate(1), side));
	setBlockImage(id, 4, buildPiston(FACE_WEST, front, back, side.rotate(3), side.rotate(2)));
	setBlockImage(id, 5, buildPiston(FACE_SOUTH, front, back, side.rotate(1), side.rotate(1)));
}

void BlockImages::createSlabs(uint16_t id, bool stone_slabs, bool double_slabs) { // id 43, 44, 125, 126
	std::map<int, Image> slab_textures;
	if (stone_slabs) {
		slab_textures[0x0] = textures.STONESLAB_SIDE;
		slab_textures[0x1] = textures.SANDSTONE_SIDE;
		slab_textures[0x2] = textures.WOOD;
		slab_textures[0x3] = textures.STONEBRICK;
		slab_textures[0x4] = textures.BRICK;
		slab_textures[0x5] = textures.STONEBRICKSMOOTH;
		slab_textures[0x6] = textures.NETHER_BRICK;
		slab_textures[0x7] = textures.QUARTZBLOCK_SIDE;
	} else {
		slab_textures[0x0] = textures.WOOD;
		slab_textures[0x1] = textures.WOOD_SPRUCE;
		slab_textures[0x2] = textures.WOOD_BIRCH;
		slab_textures[0x3] = textures.WOOD_JUNGLE;
	}
	for (std::map<int, Image>::const_iterator it = slab_textures.begin();
	        it != slab_textures.end(); ++it) {
		Image side = it->second;
		Image top = it->second;
		if (it->first == 0 && stone_slabs)
			top = textures.STONESLAB_TOP;
		if (double_slabs) {
			createBlock(id, it->first, side, top);
		} else {
			createSmallerBlock(id, it->first, side, top, 0, texture_size / 2);
			createSmallerBlock(id, 0x8 | it->first, side, top, texture_size / 2, texture_size);
		}
	}
}

void BlockImages::createTorch(uint16_t id, const Image& texture) { // id 50, 75, 76
	createSingleFaceBlock(id, 1, FACE_WEST, texture);
	createSingleFaceBlock(id, 2, FACE_EAST, texture);
	createSingleFaceBlock(id, 3, FACE_NORTH, texture);
	createSingleFaceBlock(id, 4, FACE_SOUTH, texture);

	createItemStyleBlock(id, 5, texture);
	createItemStyleBlock(id, 6, texture);
}

void BlockImages::createStairs(uint16_t id, const Image& texture) { // id 53, 67, 108, 109, 114, 128, 134, 135, 136
	Image north = buildStairsNorth(texture), south = buildStairsSouth(texture),
			east = buildStairsEast(texture), west = buildStairsWest(texture);
	rotateImages(north, south, east, west, rotation);

	setBlockImage(id, 0, east);
	setBlockImage(id, 1, west);
	setBlockImage(id, 2, south);
	setBlockImage(id, 3, north);

	north = buildUpsideDownStairsNorth(texture);
	south = buildUpsideDownStairsSouth(texture);
	east = buildUpsideDownStairsEast(texture);
	west = buildUpsideDownStairsWest(texture);
	rotateImages(north, south, east, west, rotation);

	setBlockImage(id, 0 | 4, east);
	setBlockImage(id, 1 | 4, west);
	setBlockImage(id, 2 | 4, south);
	setBlockImage(id, 3 | 4, north);
}

void BlockImages::createChest(uint16_t id, Image* textures) { // id 54, 95, 130
	BlockImage chest;
	chest.setFace(FACE_SOUTH, textures[CHEST_FRONT]);
	chest.setFace(FACE_NORTH | FACE_EAST | FACE_WEST, textures[CHEST_SIDE]);
	chest.setFace(FACE_TOP, textures[CHEST_TOP]);

	setBlockImage(id, DATA_NORTH, buildImage(chest.rotate(2)));
	setBlockImage(id, DATA_SOUTH, buildImage(chest));
	setBlockImage(id, DATA_EAST, buildImage(chest.rotate(3)));
	setBlockImage(id, DATA_WEST, buildImage(chest.rotate(1)));
}

void BlockImages::createDoubleChest(uint16_t id, Image* textures) { // id 54
	BlockImage left, right;

	// left side of the chest, south orientation
	left.setFace(FACE_SOUTH, textures[LARGECHEST_FRONT_LEFT]);
	left.setFace(FACE_NORTH, textures[LARGECHEST_BACK_LEFT].flip(true, false));
	left.setFace(FACE_WEST, textures[LARGECHEST_SIDE]);
	left.setFace(FACE_TOP, textures[LARGECHEST_TOP_LEFT].rotate(3));

	// right side of the chest, south orientation
	right.setFace(FACE_SOUTH, textures[LARGECHEST_FRONT_RIGHT]);
	right.setFace(FACE_NORTH, textures[LARGECHEST_BACK_RIGHT].flip(true, false));
	right.setFace(FACE_EAST, textures[LARGECHEST_SIDE]);
	right.setFace(FACE_TOP, textures[LARGECHEST_TOP_RIGHT].rotate(3));

	int l = LARGECHEST_DATA_LARGE;
	setBlockImage(id, DATA_NORTH | l | LARGECHEST_DATA_LEFT, buildImage(left.rotate(2)));
	setBlockImage(id, DATA_SOUTH | l | LARGECHEST_DATA_LEFT, buildImage(left));
	setBlockImage(id, DATA_EAST | l | LARGECHEST_DATA_LEFT, buildImage(left.rotate(3)));
	setBlockImage(id, DATA_WEST | l | LARGECHEST_DATA_LEFT, buildImage(left.rotate(1)));

	setBlockImage(id, DATA_NORTH | l, buildImage(right.rotate(2)));
	setBlockImage(id, DATA_SOUTH | l, buildImage(right));
	setBlockImage(id, DATA_EAST | l, buildImage(right.rotate(3)));
	setBlockImage(id, DATA_WEST | l, buildImage(right.rotate(1)));
}

void BlockImages::createDoor(uint16_t id, const Image& texture_bottom,
        const Image& texture_top) { // id 64, 71
	// TODO sometimes the texture needs to get x flipped when door is opened
	for (int top = 0; top <= 1; top++) {
		for (int flip_x = 0; flip_x <= 1; flip_x++) {
			for (int d = 0; d < 4; d++) {
				Image texture = (top ? texture_top : texture_bottom);
				if (flip_x)
					texture = texture.flip(true, false);
				BlockImage block;

				uint16_t direction = 0;
				if (d == 0) {
					direction = DOOR_NORTH;
					block.setFace(FACE_NORTH, texture);
				} else if (d == 1) {
					direction = DOOR_SOUTH;
					block.setFace(FACE_SOUTH, texture);
				} else if (d == 2) {
					direction = DOOR_EAST;
					block.setFace(FACE_EAST, texture);
				} else if (d == 3) {
					direction = DOOR_WEST;
					block.setFace(FACE_WEST, texture);
				}
				uint16_t data = (top ? DOOR_TOP : 0) | (flip_x ? DOOR_FLIP_X : 0)
				        | direction;
				setBlockImage(id, data, block);
			}
		}
	}
}

void BlockImages::createRails() { // id 66
	Image texture = textures.RAIL;
	Image corner_texture = textures.RAIL_TURN;

	createStraightRails(66, 0, texture);
	createSingleFaceBlock(66, 6, FACE_BOTTOM, corner_texture.flip(false, true));
	createSingleFaceBlock(66, 7, FACE_BOTTOM, corner_texture);
	createSingleFaceBlock(66, 8, FACE_BOTTOM, corner_texture.flip(true, false));
	createSingleFaceBlock(66, 9, FACE_BOTTOM, corner_texture.flip(true, true));
}

void BlockImages::createButton(uint16_t id, const Image& tex) { // id 77, 143
	Image texture = tex;
	int s = texture.getWidth();
	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			if (x < s / 8 * 2 || x > s / 8 * 6 || y < s / 8 * 3 || y > s / 8 * 5)
				texture.setPixel(x, y, rgba(0, 0, 0, 0));
			else {
				uint32_t p = texture.getPixel(x, y);
				if (x == s / 8 * 2 || x == s / 8 * 6 || y == s / 8 * 3 || y == s / 8 * 5)
					p = rgba_multiply(p, 0.8, 0.8, 0.8);
				texture.setPixel(x, y, p);
			}
		}
	}

	createSingleFaceBlock(id, 1, FACE_WEST, texture);
	createSingleFaceBlock(id, 2, FACE_EAST, texture);
	createSingleFaceBlock(id, 3, FACE_NORTH, texture);
	createSingleFaceBlock(id, 4, FACE_SOUTH, texture);
}

void BlockImages::createSnow() { // id 78
	Image snow = textures.SNOW;
	for (int data = 0; data < 8; data++) {
		int height = data / 8.0 * texture_size;
		setBlockImage(78, data, buildSmallerBlock(snow, snow, snow, 0, height));
	}
}

void BlockImages::createIce() { // id 79
	Image texture = textures.ICE;

	for (int w = 0; w <= 1; w++)
		for (int s = 0; s <= 1; s++) {
			Image block(getBlockImageSize(), getBlockImageSize());
			uint16_t extra_data = 0;
			if (w == 1)
				blitFace(block, FACE_WEST, texture, 0, 0, true, dleft, dright);
			else
				extra_data |= DATA_WEST;

			if (s == 1)
				blitFace(block, FACE_SOUTH, texture, 0, 0, true, dleft, dright);
			else
				extra_data |= DATA_SOUTH;
			blitFace(block, FACE_TOP, texture, 0, 0, true, dleft, dright);
			setBlockImage(79, extra_data, block);
		}
}

void BlockImages::createCactus() { // id 81
	BlockImage block;
	block.setFace(FACE_WEST, textures.CACTUS_SIDE, 2, 0);
	block.setFace(FACE_SOUTH, textures.CACTUS_SIDE, -2, 0);
	block.setFace(FACE_TOP, textures.CACTUS_TOP);
	setBlockImage(81, 0, buildImage(block));
}

/**
 * Creates the texture for a fence. The texture looks like a cross, you can set if you
 * want the left or/and right connection posts.
 */
Image createFenceTexture(bool left, bool right, Image texture) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;

	Image mask(size, size);
	mask.fill(rgba(255, 255, 255, 255), 0, 0, size, size);
	// the main post
	mask.fill(0, 6 * ratio, 0, 4 * ratio, 16 * ratio);
	// if set, left and right connection posts
	// note here when filling the width +2, it's because to make sure we fill enough
	// and don't forget a pixel
	if(left)
		mask.fill(0, 0, 4 * ratio, 6 * ratio, 4 * ratio);
	if(right)
		mask.fill(0, 10 * ratio, 4 * ratio, 6 * ratio + 2, 4 * ratio);

	// then apply mask to the texture
	for(int x = 0; x < size; x++)
		for(int y = 0; y < size; y++)
			if(mask.getPixel(x, y) != 0)
				texture.setPixel(x, y, 0);

	return texture;
}

/**
 * This method creates the fence block images. It generates textures for fences and makes
 * with this textures item style block images.
 */
void BlockImages::createFence(uint16_t id, const Image& texture) { // id 85, 113
	Image fence_empty = createFenceTexture(false, false, texture);
	Image fence_left = createFenceTexture(true, false, texture);
	Image fence_right = createFenceTexture(false, true, texture);
	Image fence_both = createFenceTexture(true, true, texture);

	// go through all neighbor combinations
	for (uint8_t i = 0; i < 16; i++) {
		Image left = fence_empty, right = fence_empty;

		uint16_t data = i << 4;
		// special data set by the tile renderer
		bool north = data & DATA_NORTH;
		bool south = data & DATA_SOUTH;
		bool east = data & DATA_EAST;
		bool west = data & DATA_WEST;

		// now select the needed textures for this neighbors
		if (north && south)
			left = fence_both;
		else if (north)
			left = fence_left;
		else if (south)
			left = fence_right;

		if (east && west)
			right = fence_both;
		else if (east)
			right = fence_right;
		else if (west)
			right = fence_left;

		BlockImage block(BlockImage::ITEM_STYLE);
		block.setFace(FACE_NORTH | FACE_SOUTH, left);
		block.setFace(FACE_EAST | FACE_WEST, right);
		setBlockImage(id, data, buildImage(block));
	}
}

void BlockImages::createPumkin(uint16_t id, const Image& front) { // id 86, 91
	Image side = textures.PUMPKIN_SIDE;
	Image top = textures.PUMPKIN_TOP;
	createBlock(id, 0, side, front, top);
	createBlock(id, 1, front, side, top);
	createBlock(id, 2, side, side, top);
	createBlock(id, 3, side, side, top);
	createBlock(id, 4, side, side, top);
}

void BlockImages::createCake() { // id 92
	BlockImage block;
	block.setFace(FACE_WEST, textures.CAKE_SIDE, 1, 0);
	block.setFace(FACE_SOUTH, textures.CAKE_SIDE, -1, 0);
	block.setFace(FACE_TOP, textures.CAKE_TOP, 0, 9);
	setBlockImage(92, 0, buildImage(block));
}

void BlockImages::createRedstoneRepeater(uint16_t id, const Image& texture) { // id 93, 94
	createSingleFaceBlock(id, 0, FACE_BOTTOM, texture.rotate(ROTATE_270));
	createSingleFaceBlock(id, 1, FACE_BOTTOM, texture);
	createSingleFaceBlock(id, 2, FACE_BOTTOM, texture.rotate(ROTATE_90));
	createSingleFaceBlock(id, 3, FACE_BOTTOM, texture.rotate(ROTATE_180));
}

void BlockImages::createTrapdoor() { // id 96
	Image texture = textures.TRAPDOOR;
	for (uint16_t i = 0; i < 16; i++) {
		if (i & 4) {
			int data = i & 0b00000011;
			if (data == 0x0)
				createSingleFaceBlock(96, i, FACE_SOUTH, texture);
			else if (data == 0x1)
				createSingleFaceBlock(96, i, FACE_NORTH, texture);
			else if (data == 0x2)
				createSingleFaceBlock(96, i, FACE_EAST, texture);
			else if (data == 0x3)
				createSingleFaceBlock(96, i, FACE_WEST, texture);
		} else {
			if (i & 8)
				createSingleFaceBlock(96, i, FACE_TOP, texture);
			else
				createSingleFaceBlock(96, i, FACE_BOTTOM, texture);
		}
	}
}

BlockImage buildHugeMushroom(const Image& pores, const Image& cap = Image(),
		int cap_sides = 0, const Image& stem = Image(), int stem_sides = 0) {
	BlockImage block;
	block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST | FACE_TOP, pores);
	for (int i = 0; i < 6; i++) {
		int side = 1 << i;
		if (cap_sides & side)
			block.setFace(side, cap);
		else if (stem_sides & side)
			block.setFace(side, stem);
	}
	return block;
}

void BlockImages::createHugeMushroom(uint16_t id, const Image& cap) { // id 99, 100
	Image pores = textures.MUSHROOM_INSIDE;
	Image stem = textures.MUSHROOM_SKIN_STEM;

	setBlockImage(id, 0, buildHugeMushroom(pores));
	setBlockImage(id, 1, buildHugeMushroom(pores, cap, FACE_TOP | FACE_WEST | FACE_NORTH));
	setBlockImage(id, 2, buildHugeMushroom(pores, cap, FACE_TOP | FACE_NORTH));
	setBlockImage(id, 3, buildHugeMushroom(pores, cap, FACE_TOP | FACE_NORTH | FACE_EAST));
	setBlockImage(id, 4, buildHugeMushroom(pores, cap, FACE_TOP | FACE_WEST));
	setBlockImage(id, 5, buildHugeMushroom(pores, cap, FACE_TOP));
	setBlockImage(id, 6, buildHugeMushroom(pores, cap, FACE_TOP | FACE_EAST));
	setBlockImage(id, 7, buildHugeMushroom(pores, cap, FACE_TOP | FACE_SOUTH | FACE_WEST));
	setBlockImage(id, 8, buildHugeMushroom(pores, cap, FACE_TOP | FACE_SOUTH));
	setBlockImage(id, 9, buildHugeMushroom(pores, cap, FACE_TOP | FACE_EAST | FACE_SOUTH));
	setBlockImage(id, 10, buildHugeMushroom(pores, cap, 0, stem, 0b1111));
	setBlockImage(id, 14, buildHugeMushroom(pores, cap, 0b111111));
	setBlockImage(id, 15, buildHugeMushroom(pores, cap, 0, stem, 0b111111));
}

void BlockImages::createBarsPane(uint16_t id, const Image& texture_left_right) { // id 101, 102
	Image texture_left = texture_left_right;
	Image texture_right = texture_left_right;
	texture_left.fill(0, texture_size / 2, 0, texture_size / 2, texture_size);
	texture_right.fill(0, 0, 0, texture_size / 2, texture_size);

	createItemStyleBlock(id, 0, texture_left_right);
	for (uint8_t i = 1; i < 16; i++) {
		Image left = empty_texture, right = empty_texture;

		uint16_t data = i << 4;
		bool north = data & DATA_NORTH;
		bool south = data & DATA_SOUTH;
		bool east = data & DATA_EAST;
		bool west = data & DATA_WEST;

		if (north && south)
			left = texture_left_right;
		else if (north)
			left = texture_left;
		else if (south)
			left = texture_right;

		if (east && west)
			right = texture_left_right;
		else if (east)
			right = texture_right;
		else if (west)
			right = texture_left;

		BlockImage block(BlockImage::ITEM_STYLE);
		block.setFace(FACE_NORTH | FACE_SOUTH, left);
		block.setFace(FACE_EAST | FACE_WEST, right);
		setBlockImage(id, data, buildImage(block));
	}
}

void BlockImages::createStem(uint16_t id) { // id 104, 105
	// build here only growing normal stem
	Image texture = textures.STEM_STRAIGHT;

	for (int i = 0; i <= 7; i++) {
		double percentage = 1 - ((double) i / 7);
		int move = percentage * texture_size;

		if (i == 7)
			createItemStyleBlock(id, i, texture.move(0, move).colorize(0.6, 0.7, 0.01));
		else
			createItemStyleBlock(id, i, texture.move(0, move).colorize(0.3, 0.7, 0.01));
	}
}

void BlockImages::createVines() { // id 106
	Image texture = textures.VINE;

	createSingleFaceBlock(106, 0, FACE_TOP, texture);
	for (int i = 1; i < 16; i++) {
		BlockImage block;
		if (i & 1)
			block.setFace(FACE_SOUTH, texture);
		if (i & 2)
			block.setFace(FACE_WEST, texture);
		if (i & 4)
			block.setFace(FACE_NORTH, texture);
		if (i & 8)
			block.setFace(FACE_EAST, texture);
		setBlockImage(106, i, block);
	}
}

/**
 * Creates the texture for a fence gate, opened or closed.
 */
Image createFenceGateTexture(bool opened, Image texture) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;

	Image mask(size, size);
	mask.fill(rgba(255, 255, 255, 255), 0, 0, size, size);

	// left and right post
	mask.fill(0, 0, 0, 4 * ratio, 12 * ratio);
	mask.fill(0, 12 * ratio, 0, 4 * ratio, 12 * ratio);
	// if closed this middle part
	if(!opened)
		mask.fill(0, 4 * ratio, 3 * ratio, 8 * ratio, 6 * ratio);

	// then apply mask to the texture
	for(int x = 0; x < size; x++)
		for(int y = 0; y < size; y++)
			if(mask.getPixel(x, y) != 0)
				texture.setPixel(x, y, 0);

	return texture;
}

void BlockImages::createFenceGate() { // id 107
	Image texture = textures.WOOD;
	Image opened = createFenceGateTexture(true, texture);
	Image closed = createFenceGateTexture(false, texture);

	// go through states opened and closed
	for(int open = 0; open <= 1; open++) {
		Image tex = open ? opened : closed;
		Image north(texture_size * 2, texture_size * 2);
		Image east = north;
		// north and south block images are same
		// (because we ignore the direction of opened fence gates)
		blitFace(north, FACE_NORTH, tex, texture_size * 0.5, texture_size * 0.25, false);
		// also east and west
		blitFace(east, FACE_EAST, tex, -texture_size * 0.5, texture_size * 0.25, false);
		uint8_t extra = open ? 4 : 0;
		if (rotation == 0 || rotation == 2) {
			setBlockImage(107, 0 | extra, north);
			setBlockImage(107, 1 | extra, east);
			setBlockImage(107, 2 | extra, north);
			setBlockImage(107, 3 | extra, east);
		} else {
			setBlockImage(107, 0 | extra, east);
			setBlockImage(107, 1 | extra, north);
			setBlockImage(107, 2 | extra, east);
			setBlockImage(107, 3 | extra, north);
		}
	}
}

void BlockImages::createCauldron() { // id 118
	Image side = textures.CAULDRON_SIDE;
	Image water = textures.WATER;

	for (int i = 0; i < 4; i++) {
		Image block(getBlockImageSize(), getBlockImageSize());
		blitFace(block, FACE_NORTH, side, 0, 0, true, dleft, dright);
		blitFace(block, FACE_EAST, side, 0, 0, true, dleft, dright);
		if (i == 3)
			blitFace(block, FACE_TOP, water, 0, 2, true, dleft, dright);
		else if (i == 2)
			blitFace(block, FACE_TOP, water, 0, 5, true, dleft, dright);
		else if (i == 1)
			blitFace(block, FACE_TOP, water, 0, 9, true, dleft, dright);
		blitFace(block, FACE_SOUTH, side, 0, 0, true, dleft, dright);
		blitFace(block, FACE_WEST, side, 0, 0, true, dleft, dright);
		setBlockImage(118, i, block);
	}
}

void BlockImages::createBeacon() { // id 138
	Image beacon(texture_size * 2, texture_size * 2);

	// at first create this little block in the middle
	Image beacon_texture;
	textures.BEACON.resizeInterpolated(texture_size * 0.75, texture_size * 0.75,
			beacon_texture);
	Image smallblock(texture_size * 2, texture_size * 2);
	blitFace(smallblock, FACE_WEST, beacon_texture, 0, 0, true, dleft, dright);
	blitFace(smallblock, FACE_SOUTH, beacon_texture, 0, 0, true, dleft, dright);
	blitFace(smallblock, FACE_TOP, beacon_texture, 0, 0, true, dleft, dright);

	// then create the obsidian ground
	Image obsidian_texture = textures.OBSIDIAN;
	Image obsidian = buildImage(buildSmallerBlock(obsidian_texture, obsidian_texture,
			obsidian_texture, 0, texture_size / 4));

	// blit block and obsidian ground
	beacon.simpleblit(obsidian, 0, 0);
	beacon.simpleblit(smallblock, texture_size / 4, texture_size / 4);

	// then blit outside glass
	Image glass_texture = textures.GLASS;
	blitFace(beacon, FACE_WEST, glass_texture, 0, 0, true, dleft, dright);
	blitFace(beacon, FACE_SOUTH, glass_texture, 0, 0, true, dleft, dright);
	blitFace(beacon, FACE_TOP, glass_texture, 0, 0, true, dleft, dright);

	setBlockImage(138, 0, beacon);
}

void BlockImages::loadBlocks() {
	buildCustomTextures();
	unknown_block = buildImage(BlockImage().setFace(0b11111, unknown_block));

	BlockTextures& t = textures;

	createBlock(1, 0, t.STONE); // stone
	createGrassBlock(); // id 2
	createBlock(3, 0, t.DIRT); // dirt
	createBlock(4, 0, t.STONEBRICK); // cobblestone
	// -- wooden planks
	createBlock(5, 0, t.WOOD); // oak
	createBlock(5, 1, t.WOOD_SPRUCE); // pine/spruce
	createBlock(5, 2, t.WOOD_BIRCH); // birch
	createBlock(5, 3, t.WOOD_BIRCH); // jungle
	// -- saplings
	createItemStyleBlock(6, 0, t.SAPLING); // oak
	createItemStyleBlock(6, 1, t.SAPLING_SPRUCE); // spruce
	createItemStyleBlock(6, 2, t.SAPLING_BIRCH); // birch
	createItemStyleBlock(6, 3, t.SAPLING_JUNGLE); // jungle
	// --
	createBlock(7, 0, t.BEDROCK); // bedrock
	createWater(); // id 8, 9
	createLava(); // id 10, 11
	createBlock(12, 0, t.SAND); // sand
	createBlock(13, 0, t.GRAVEL); // gravel
	createBlock(14, 0, t.ORE_GOLD); // gold ore
	createBlock(15, 0, t.ORE_IRON); // iron ore
	createBlock(16, 0, t.ORE_COAL); // coal ore
	// -- wood
	createWood(17, 0, t.TREE_SIDE, t.TREE_TOP); // oak
	createWood(17, 1, t.TREE_SPRUCE, t.TREE_TOP); // pine/spruce
	createWood(17, 2, t.TREE_BIRCH, t.TREE_TOP); // birch
	createWood(17, 3, t.TREE_JUNGLE, t.TREE_TOP); // jungle
	createLeaves(); // id 18
	createBlock(19, 0, t.SPONGE); // sponge
	createBlock(20, 0, t.GLASS); // glass
	createBlock(21, 0, t.ORE_LAPIS); // lapis lazuli ore
	createBlock(22, 0, t.BLOCK_LAPIS); // lapis lazuli block
	createDispenserDropper(23, t.DISPENSER_FRONT); // dispenser
	// -- sandstone
	createBlock(24, 0, t.SANDSTONE_SIDE, t.SANDSTONE_TOP); // normal
	createBlock(24, 1, t.SANDSTONE_CARVED, t.SANDSTONE_TOP); // chiseled
	createBlock(24, 2, t.SANDSTONE_SMOOTH, t.SANDSTONE_TOP); // smooth
	// --
	createBlock(25, 0, t.MUSIC_BLOCK); // noteblock
	createBed(); // id 26 bed
	createStraightRails(27, 0, t.GOLDEN_RAIL); // id 27 powered rail (unpowered)
	createStraightRails(27, 8, t.GOLDEN_RAIL_POWERED); // id 27 powered rail (powered)
	createStraightRails(28, 0, t.ACTIVATOR_RAIL); // id 28 detector rail
	createPiston(29, true); // sticky piston
	createItemStyleBlock(30, 0, t.WEB); // cobweb
	// -- tall grass
	createItemStyleBlock(31, 0, t.DEADBUSH); // dead bush style
	createItemStyleBlock(31, 1, t.TALLGRASS); // tall grass
	createItemStyleBlock(31, 2, t.FERN); // fern
	// --
	createItemStyleBlock(32, 0, t.DEADBUSH); // dead bush
	createPiston(33, false); // piston
	// id 34 // piston extension
	// -- wool
	createBlock(35, 0, t.CLOTH_0); // white
	createBlock(35, 1, t.CLOTH_1); // orange
	createBlock(35, 2, t.CLOTH_2); // magenta
	createBlock(35, 3, t.CLOTH_3); // light blue
	createBlock(35, 4, t.CLOTH_4); // yellow
	createBlock(35, 5, t.CLOTH_5); // lime
	createBlock(35, 6, t.CLOTH_6); // pink
	createBlock(35, 7, t.CLOTH_7); // gray
	createBlock(35, 8, t.CLOTH_8); // light gray
	createBlock(35, 9, t.CLOTH_9); // cyan
	createBlock(35, 10, t.CLOTH_10); // purple
	createBlock(35, 11, t.CLOTH_11); // blue
	createBlock(35, 12, t.CLOTH_12); // brown
	createBlock(35, 13, t.CLOTH_13); // green
	createBlock(35, 14, t.CLOTH_14); // red
	createBlock(35, 15, t.CLOTH_15); // black
	// --
	createBlock(36, 0, empty_texture); // block moved by piston aka 'block 36'
	createItemStyleBlock(37, 0, t.FLOWER); // dandelion
	createItemStyleBlock(38, 0, t.ROSE); // rose
	createItemStyleBlock(39, 0, t.MUSHROOM_BROWN); // brown mushroom
	createItemStyleBlock(40, 0, t.MUSHROOM_RED); // red mushroom
	createBlock(41, 0, t.BLOCK_GOLD); // block of gold
	createBlock(42, 0, t.BLOCK_IRON); // block of iron
	createSlabs(43, true, true); // double stone slabs
	createSlabs(44, true, false); // normal stone slabs
	createBlock(45, 0, t.BRICK); // bricks
	createBlock(46, 0, t.TNT_SIDE, t.TNT_TOP); // tnt
	createBlock(47, 0, t.BOOKSHELF, t.WOOD); // bookshelf
	createBlock(48, 0, t.STONE_MOSS); // moss stone
	createBlock(49, 0, t.OBSIDIAN); // obsidian
	createTorch(50, t.TORCH); // torch
	createItemStyleBlock(51, 0, fire_texture); // fire
	createBlock(52, 0, t.MOB_SPAWNER); // monster spawner
	createStairs(53, t.WOOD); // oak wood stairs
	createChest(54, chest); // chest
	createDoubleChest(54, largechest); // chest
	// id 55 // redstone wire
	createBlock(56, 0, t.ORE_DIAMOND); // diamond ore
	createBlock(57, 0, t.BLOCK_DIAMOND); // block of diamond
	createBlock(58, 0, t.WORKBENCH_SIDE, t.WORKBENCH_FRONT, t.WORKBENCH_TOP); // crafting table
	// -- wheat
	createItemStyleBlock(59, 0, t.CROPS_0); //
	createItemStyleBlock(59, 1, t.CROPS_1); //
	createItemStyleBlock(59, 2, t.CROPS_2); //
	createItemStyleBlock(59, 3, t.CROPS_3); //
	createItemStyleBlock(59, 4, t.CROPS_4); //
	createItemStyleBlock(59, 5, t.CROPS_5); //
	createItemStyleBlock(59, 6, t.CROPS_6); //
	createItemStyleBlock(59, 7, t.CROPS_7); //
	// --
	createBlock(60, 0, t.DIRT, t.FARMLAND_WET); // farmland
	createRotatedBlock(61, 0, t.FURNACE_FRONT, t.FURNACE_SIDE, t.FURNACE_TOP); // furnace
	createRotatedBlock(62, 0, t.FURNACE_FRONT_LIT, t.FURNACE_SIDE, t.FURNACE_TOP); // burning furnace
	// id 63 // sign post
	createDoor(64, t.DOOR_WOOD_LOWER, t.DOOR_WOOD_UPPER); // wooden door
	// -- ladders
	createSingleFaceBlock(65, 2, FACE_SOUTH, t.LADDER);
	createSingleFaceBlock(65, 3, FACE_NORTH, t.LADDER);
	createSingleFaceBlock(65, 4, FACE_EAST, t.LADDER);
	createSingleFaceBlock(65, 5, FACE_WEST, t.LADDER);
	// --
	createRails(); // id 66
	createStairs(67, t.STONEBRICK); // cobblestone stairs
	// id 68 // wall sign
	// id 69 // lever
	createSmallerBlock(70, 0, t.STONE, t.STONE, 0, 1); // stone pressure plate
	createDoor(71, t.DOOR_IRON_LOWER, t.DOOR_IRON_UPPER); // iron door
	createSmallerBlock(72, 0, t.WOOD, t.WOOD, 0, 1); // wooden pressure plate
	createBlock(73, 0, t.ORE_REDSTONE); // redstone ore
	createBlock(74, 0, t.ORE_REDSTONE); // glowing redstone ore
	createTorch(75, t.REDTORCH); // redstone torch off
	createTorch(76, t.REDTORCH_LIT); // redstone torch on
	createButton(77, t.STONE); // stone button
	createSnow(); // id 78
	createIce(); // id 79
	createBlock(80, 0, t.SNOW); // snow block
	createCactus(); // id 81
	createBlock(82, 0, t.CLAY); // clay block
	createItemStyleBlock(83, 0, t.REEDS); // sugar cane
	createBlock(84, 0, t.MUSIC_BLOCK, t.JUKEBOX_TOP); // jukebox
	createFence(85, t.WOOD); // fence
	createPumkin(86, t.PUMPKIN_FACE); // pumpkin
	createBlock(87, 0, t.HELLROCK); // netherrack
	createBlock(88, 0, t.HELLSAND); // soul sand
	createBlock(89, 0, t.LIGHTGEM); // glowstone block
	createBlock(90, 0, t.PORTAL); // nether portal block
	createPumkin(91, t.PUMPKIN_JACK); // jack-o-lantern
	createCake(); // id 92
	createRedstoneRepeater(93, t.REPEATER); // redstone repeater off
	createRedstoneRepeater(94, t.REPEATER_LIT); // redstone repeater on
	createChest(95, chest); // locked chest
	createTrapdoor(); // id 96 // trapdoor
	// -- monster egg
	createBlock(97, 0, t.STONE); // stone
	createBlock(97, 1, t.STONEBRICK); // cobblestone
	createBlock(97, 2, t.STONEBRICKSMOOTH); // stone brick
	// --
	// -- stone bricks
	createBlock(98, 0, t.STONEBRICKSMOOTH); // normal
	createBlock(98, 1, t.STONEBRICKSMOOTH_MOSSY); // mossy
	createBlock(98, 2, t.STONEBRICKSMOOTH_CRACKED); // cracked
	createBlock(98, 3, t.STONEBRICKSMOOTH_CARVED); // chiseled
	// --
	createHugeMushroom(99, t.MUSHROOM_SKIN_BROWN); // huge brown mushroom
	createHugeMushroom(100, t.MUSHROOM_SKIN_RED); // huge red mushroom
	createBarsPane(101, t.FENCE_IRON); // iron bars
	createBarsPane(102, t.GLASS); // glass pane
	createBlock(103, 0, t.MELON_SIDE, t.MELON_TOP); // melon
	createStem(104); // pumpkin stem
	createStem(105); // melon stem
	createVines(); // id 106 // vines
	createFenceGate(); // id 107 // fence gate
	createStairs(108, t.BRICK); // brick stairs
	createStairs(109, t.STONEBRICKSMOOTH); // stone brick stairs
	createBlock(110, 0, t.MYCEL_SIDE, t.MYCEL_TOP); // mycelium
	createSingleFaceBlock(111, 0, FACE_BOTTOM, t.WATERLILY); // lily pad
	createBlock(112, 0, t.NETHER_BRICK); // nether brick
	createFence(113, t.NETHER_BRICK); // nether brick fence
	createStairs(114, t.NETHER_BRICK); // nether brick stairs
	// -- nether wart
	createItemStyleBlock(115, 0, t.NETHER_STALK_0); //
	createItemStyleBlock(115, 1, t.NETHER_STALK_1); //
	createItemStyleBlock(115, 2, t.NETHER_STALK_1); //
	createItemStyleBlock(115, 3, t.NETHER_STALK_2); //
	// --
	createSmallerBlock(116, 0, t.ENCHANTMENT_SIDE, t.ENCHANTMENT_TOP, 0, texture_size * 0.75); // enchantment table
	createItemStyleBlock(117, 0, t.BREWING_STAND); // brewing stand
	createCauldron(); // id 118 // cauldron
	createSmallerBlock(119, 0, endportal_texture, endportal_texture,
			texture_size * 0.25, texture_size * 0.75); // end portal
	createSmallerBlock(120, 0, t.ENDFRAME_SIDE, t.ENDFRAME_TOP, 0,
			texture_size * 0.8125); // end portal frame
	createBlock(121, 0, t.WHITE_STONE); // end stone
	// id 122 // dragon egg
	createBlock(123, 0, t.REDSTONE_LIGHT); // redstone lamp inactive
	createBlock(124, 0, t.REDSTONE_LIGHT_LIT); // redstone lamp active
	createSlabs(125, false, true); // wooden double slabs
	createSlabs(126, false, false); // wooden normal slabs
	// id 127 // cocoa plant
	createStairs(128, t.SANDSTONE_SIDE); // sandstone stairs
	createBlock(129, 0, t.ORE_EMERALD); // emerald ore
	createChest(130, enderchest); // ender chest
	// id 131 // tripwire hook
	// id 132 // tripwire
	createBlock(133, 0, t.BLOCK_EMERALD); // block of emerald
	createStairs(134, t.WOOD_SPRUCE); // spruce wood stairs
	createStairs(135, t.WOOD_BIRCH); // birch wood stairs
	createStairs(136, t.WOOD_JUNGLE); // jungle wood stairs
	createBlock(137, 0, t.COMMAND_BLOCK); // command block
	createBeacon(); // beacon
	// id 139 // cobblestone wall
	// id 140 // flower pot
	// carrots --
	createItemStyleBlock(141, 0, t.CARROTS_0);
	createItemStyleBlock(141, 1, t.CARROTS_0);
	createItemStyleBlock(141, 2, t.CARROTS_1);
	createItemStyleBlock(141, 3, t.CARROTS_1);
	createItemStyleBlock(141, 4, t.CARROTS_2);
	createItemStyleBlock(141, 5, t.CARROTS_2);
	createItemStyleBlock(141, 6, t.CARROTS_2);
	createItemStyleBlock(141, 7, t.CARROTS_3);
	// --
	// potatoes --
	createItemStyleBlock(142, 0, t.POTATOES_0);
	createItemStyleBlock(142, 1, t.POTATOES_0);
	createItemStyleBlock(142, 2, t.POTATOES_1);
	createItemStyleBlock(142, 3, t.POTATOES_1);
	createItemStyleBlock(142, 4, t.POTATOES_2);
	createItemStyleBlock(142, 5, t.POTATOES_2);
	createItemStyleBlock(142, 6, t.POTATOES_2);
	createItemStyleBlock(142, 7, t.POTATOES_3);
	// --
	createButton(143, t.WOOD); // wooden button
	// id 144 // head
	// id 145 // anvil
	createChest(146, chest); // trapped chest
	createSmallerBlock(147, 0, t.BLOCK_GOLD, t.BLOCK_GOLD, 0, 1); // weighted pressure plate (light)
	createSmallerBlock(148, 0, t.QUARTZBLOCK_LINES, t.QUARTZBLOCK_LINES, 0, 1); // weighted pressure plate (heavy)
	createRedstoneRepeater(149, t.COMPARATOR); // redstone comparator (inactive)
	createRedstoneRepeater(150, t.COMPARATOR_LIT); // redstone comparator (active)
	createSmallerBlock(151, 0, t.DAYLIGHT_DETECTOR_SIDE, t.DAYLIGHT_DETECTOR_TOP, 0, 8); // daylight sensor
	createBlock(152, 0, t.BLOCK_REDSTONE); // block of redstone
	createBlock(153, 0, t.NETHERQUARTZ); // nether quartz ore
	// id 154 // hopper
	// block of quartz --
	createBlock(155, 0, t.QUARTZBLOCK_SIDE, t.QUARTZBLOCK_TOP);
	createBlock(155, 1, t.QUARTZBLOCK_CHISELED, t.QUARTZBLOCK_CHISELED_TOP);
	createBlock(155, 2, t.QUARTZBLOCK_LINES, t.QUARTZBLOCK_LINES_TOP);
	createBlock(155, 3, t.QUARTZBLOCK_LINES_TOP, t.QUARTZBLOCK_LINES.rotate(ROTATE_90), t.QUARTZBLOCK_LINES);
	createBlock(155, 4, t.QUARTZBLOCK_LINES.rotate(ROTATE_90), t.QUARTZBLOCK_LINES_TOP, t.QUARTZBLOCK_LINES.rotate(ROTATE_90));
	// --
	createStairs(156, t.QUARTZBLOCK_SIDE); // quartz stairs
	createStraightRails(157, 0, t.ACTIVATOR_RAIL); // activator rail
	createDispenserDropper(158, t.DISPENSER_FRONT); // dropper
}

bool BlockImages::isBlockTransparent(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	// remove edge data
	data &= ~(EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM);
	if (block_images.count(id | (data << 16)) == 0)
		return !render_unknown_blocks;
	return block_transparency.count(id | (data << 16)) != 0;
}

bool BlockImages::hasBlock(uint16_t id, uint16_t data) const {
	return block_images.count(id | (data << 16)) != 0;
}

const Image& BlockImages::getBlock(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	if (!hasBlock(id, data))
		return unknown_block;
	return block_images.at(id | (data << 16));
}

Image BlockImages::getBiomeDependBlock(uint16_t id, uint16_t data,
        const Biome& biome) const {
	data = filterBlockData(id, data);
	// just return the block if biome is invalid
	// special case for the snowy grass block
	if (biome.id >= BIOMES_SIZE || (id == 2 && (data & GRASS_SNOW)))
		return getBlock(id, data);

	if (!hasBlock(id, data))
		return unknown_block;

	// check if this biome block is precalculated
	if (biome == BIOMES[biome.id]) {
		int64_t key = id | (((int64_t) data) << 16) | (((int64_t) biome.id) << 32);
		if (!biome_images.count(key))
			return unknown_block;
		return biome_images.at(key);
	}

	// create the block if not
	return createBiomeBlock(id, data, biome);
}

int BlockImages::getMaxWaterNeededOpaque() const {
	return max_water;
}

const Image& BlockImages::getOpaqueWater(bool south, bool west) const {
	int index = ((south ? 0 : 1) | ((west ? 0 : 1) << 1));
	return opaque_water[index];
}

int BlockImages::getBlockImageSize() const {
	return texture_size * 2;
}

int BlockImages::getTextureSize() const {
	return texture_size;
}

int BlockImages::getTileSize() const {
	return texture_size * 2 * 16;
}

}
}
