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

#include "blockimages.h"

#include "../util.h"

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

	original = tmp;

	// check if this is an animated texture
	// -> use only the first frame
	if (tmp.getWidth() < tmp.getHeight())
		tmp = tmp.clip(0, 0, tmp.getWidth(), tmp.getWidth());

	// we resize the transparent version of the leaves without interpolation,
	// because this can cause half-transparent pixels, which aren't good for performance
	// redstone also without interpolation
	if (name == "leaves"
			|| name == "leaves_jungle"
			|| name == "leaves_spruce"
			|| name == "redstoneDust_cross"
			|| name == "redstoneDust_line")
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
		: ANVIL_BASE("anvil_base"), ANVIL_TOP_DAMAGED_0("anvil_top_damaged_0"), ANVIL_TOP_DAMAGED_1(
				"anvil_top_damaged_1"), ANVIL_TOP_DAMAGED_2("anvil_top_damaged_2"), BEACON(
				"beacon"), BED_FEET_END("bed_feet_end"), BED_FEET_SIDE("bed_feet_side"), BED_FEET_TOP(
				"bed_feet_top"), BED_HEAD_END("bed_head_end"), BED_HEAD_SIDE(
				"bed_head_side"), BED_HEAD_TOP("bed_head_top"), BEDROCK("bedrock"), BOOKSHELF(
				"bookshelf"), BREWING_STAND("brewing_stand"), BREWING_STAND_BASE(
				"brewing_stand_base"), BRICK("brick"), CACTUS_BOTTOM("cactus_bottom"), CACTUS_SIDE(
				"cactus_side"), CACTUS_TOP("cactus_top"), CAKE_BOTTOM("cake_bottom"), CAKE_INNER(
				"cake_inner"), CAKE_SIDE("cake_side"), CAKE_TOP("cake_top"), CARROTS_STAGE_0(
				"carrots_stage_0"), CARROTS_STAGE_1("carrots_stage_1"), CARROTS_STAGE_2(
				"carrots_stage_2"), CARROTS_STAGE_3("carrots_stage_3"), CAULDRON_BOTTOM(
				"cauldron_bottom"), CAULDRON_INNER("cauldron_inner"), CAULDRON_SIDE(
				"cauldron_side"), CAULDRON_TOP("cauldron_top"), CLAY("clay"), COAL_BLOCK(
				"coal_block"), COAL_ORE("coal_ore"), COBBLESTONE("cobblestone"), COBBLESTONE_MOSSY(
				"cobblestone_mossy"), COCOA_STAGE_0("cocoa_stage_0"), COCOA_STAGE_1(
				"cocoa_stage_1"), COCOA_STAGE_2("cocoa_stage_2"), COMMAND_BLOCK(
				"command_block"), COMPARATOR_OFF("comparator_off"), COMPARATOR_ON(
				"comparator_on"), CRAFTING_TABLE_FRONT("crafting_table_front"), CRAFTING_TABLE_SIDE(
				"crafting_table_side"), CRAFTING_TABLE_TOP("crafting_table_top"), DAYLIGHT_DETECTOR_SIDE(
				"daylight_detector_side"), DAYLIGHT_DETECTOR_TOP("daylight_detector_top"), DEADBUSH(
				"deadbush"), DESTROY_STAGE_0("destroy_stage_0"), DESTROY_STAGE_1(
				"destroy_stage_1"), DESTROY_STAGE_2("destroy_stage_2"), DESTROY_STAGE_3(
				"destroy_stage_3"), DESTROY_STAGE_4("destroy_stage_4"), DESTROY_STAGE_5(
				"destroy_stage_5"), DESTROY_STAGE_6("destroy_stage_6"), DESTROY_STAGE_7(
				"destroy_stage_7"), DESTROY_STAGE_8("destroy_stage_8"), DESTROY_STAGE_9(
				"destroy_stage_9"), DIAMOND_BLOCK("diamond_block"), DIAMOND_ORE(
				"diamond_ore"), DIRT("dirt"), DIRT_PODZOL_SIDE("dirt_podzol_side"), DIRT_PODZOL_TOP(
				"dirt_podzol_top"), DISPENSER_FRONT_HORIZONTAL(
				"dispenser_front_horizontal"), DISPENSER_FRONT_VERTICAL(
				"dispenser_front_vertical"), DOOR_IRON_LOWER("door_iron_lower"), DOOR_IRON_UPPER(
				"door_iron_upper"), DOOR_WOOD_LOWER("door_wood_lower"), DOOR_WOOD_UPPER(
				"door_wood_upper"), DOUBLE_PLANT_FERN_BOTTOM("doublePlant_fern_bottom"), DOUBLE_PLANT_FERN_TOP(
				"doublePlant_fern_top"), DOUBLE_PLANT_GRASS_BOTTOM(
				"doublePlant_grass_bottom"), DOUBLE_PLANT_GRASS_TOP(
				"doublePlant_grass_top"), DOUBLE_PLANT_PAEONIA_BOTTOM(
				"doublePlant_paeonia_bottom"), DOUBLE_PLANT_PAEONIA_TOP(
				"doublePlant_paeonia_top"), DOUBLE_PLANT_ROSE_BOTTOM(
				"doublePlant_rose_bottom"), DOUBLE_PLANT_ROSE_TOP("doublePlant_rose_top"), DOUBLE_PLANT_SUNFLOWER_BACK(
				"doublePlant_sunflower_back"), DOUBLE_PLANT_SUNFLOWER_BOTTOM(
				"doublePlant_sunflower_bottom"), DOUBLE_PLANT_SUNFLOWER_FRONT(
				"doublePlant_sunflower_front"), DOUBLE_PLANT_SUNFLOWER_TOP(
				"doublePlant_sunflower_top"), DOUBLE_PLANT_SYRINGA_BOTTOM(
				"doublePlant_syringa_bottom"), DOUBLE_PLANT_SYRINGA_TOP(
				"doublePlant_syringa_top"), DRAGON_EGG("dragon_egg"), DROPPER_FRONT_HORIZONTAL(
				"dropper_front_horizontal"), DROPPER_FRONT_VERTICAL(
				"dropper_front_vertical"), EMERALD_BLOCK("emerald_block"), EMERALD_ORE(
				"emerald_ore"), ENCHANTING_TABLE_BOTTOM("enchanting_table_bottom"), ENCHANTING_TABLE_SIDE(
				"enchanting_table_side"), ENCHANTING_TABLE_TOP("enchanting_table_top"), END_STONE(
				"end_stone"), ENDFRAME_EYE("endframe_eye"), ENDFRAME_SIDE(
				"endframe_side"), ENDFRAME_TOP("endframe_top"), FARMLAND_DRY(
				"farmland_dry"), FARMLAND_WET("farmland_wet"), FERN("fern"), FIRE_LAYER_0(
				"fire_layer_0"), FIRE_LAYER_1("fire_layer_1"), FLOWER_ALLIUM(
				"flower_allium"), FLOWER_BLUE_ORCHID("flower_blue_orchid"), FLOWER_DANDELION(
				"flower_dandelion"), FLOWER_HOUSTONIA("flower_houstonia"), FLOWER_OXEYE_DAISY(
				"flower_oxeye_daisy"), FLOWER_PAEONIA("flower_paeonia"), FLOWER_POT(
				"flower_pot"), FLOWER_ROSE("flower_rose"), FLOWER_TULIP_ORANGE(
				"flower_tulip_orange"), FLOWER_TULIP_PINK("flower_tulip_pink"), FLOWER_TULIP_RED(
				"flower_tulip_red"), FLOWER_TULIP_WHITE("flower_tulip_white"), FURNACE_FRONT_OFF(
				"furnace_front_off"), FURNACE_FRONT_ON("furnace_front_on"), FURNACE_SIDE(
				"furnace_side"), FURNACE_TOP("furnace_top"), GLASS("glass"), GLASS_PANE_TOP(
				"glass_pane_top"), GLOWSTONE("glowstone"), GOLD_BLOCK("gold_block"), GOLD_ORE(
				"gold_ore"), GRASS_SIDE("grass_side"), GRASS_SIDE_OVERLAY(
				"grass_side_overlay"), GRASS_SIDE_SNOWED("grass_side_snowed"), GRASS_TOP(
				"grass_top"), GRAVEL("gravel"), HARDENED_CLAY("hardened_clay"), HARDENED_CLAY_STAINED_BLACK(
				"hardened_clay_stained_black"), HARDENED_CLAY_STAINED_BLUE(
				"hardened_clay_stained_blue"), HARDENED_CLAY_STAINED_BROWN(
				"hardened_clay_stained_brown"), HARDENED_CLAY_STAINED_CYAN(
				"hardened_clay_stained_cyan"), HARDENED_CLAY_STAINED_GRAY(
				"hardened_clay_stained_gray"), HARDENED_CLAY_STAINED_GREEN(
				"hardened_clay_stained_green"), HARDENED_CLAY_STAINED_LIGHT_BLUE(
				"hardened_clay_stained_light_blue"), HARDENED_CLAY_STAINED_LIME(
				"hardened_clay_stained_lime"), HARDENED_CLAY_STAINED_MAGENTA(
				"hardened_clay_stained_magenta"), HARDENED_CLAY_STAINED_ORANGE(
				"hardened_clay_stained_orange"), HARDENED_CLAY_STAINED_PINK(
				"hardened_clay_stained_pink"), HARDENED_CLAY_STAINED_PURPLE(
				"hardened_clay_stained_purple"), HARDENED_CLAY_STAINED_RED(
				"hardened_clay_stained_red"), HARDENED_CLAY_STAINED_SILVER(
				"hardened_clay_stained_silver"), HARDENED_CLAY_STAINED_WHITE(
				"hardened_clay_stained_white"), HARDENED_CLAY_STAINED_YELLOW(
				"hardened_clay_stained_yellow"), HAY_BLOCK_SIDE("hay_block_side"), HAY_BLOCK_TOP(
				"hay_block_top"), HOPPER_INSIDE("hopper_inside"), HOPPER_OUTSIDE(
				"hopper_outside"), HOPPER_TOP("hopper_top"), ICE("ice"), ICE_PACKED(
				"ice_packed"), IRON_BARS("iron_bars"), IRON_BLOCK("iron_block"), IRON_ORE(
				"iron_ore"), ITEMFRAME_BACKGROUND("itemframe_background"), JUKEBOX_SIDE(
				"jukebox_side"), JUKEBOX_TOP("jukebox_top"), LADDER("ladder"), LAPIS_BLOCK(
				"lapis_block"), LAPIS_ORE("lapis_ore"), LAVA_FLOW("lava_flow"), LAVA_STILL(
				"lava_still"), LEAVES_BIRCH("leaves_birch"), LEAVES_BIRCH_OPAQUE(
				"leaves_birch_opaque"), LEAVES_JUNGLE("leaves_jungle"), LEAVES_JUNGLE_OPAQUE(
				"leaves_jungle_opaque"), LEAVES_OAK("leaves_oak"), LEAVES_OAK_OPAQUE(
				"leaves_oak_opaque"), LEAVES_SPRUCE("leaves_spruce"), LEAVES_SPRUCE_OPAQUE(
				"leaves_spruce_opaque"), LEVER("lever"), LOG_BIRCH("log_birch"), LOG_BIRCH_TOP(
				"log_birch_top"), LOG_JUNGLE("log_jungle"), LOG_JUNGLE_TOP(
				"log_jungle_top"), LOG_OAK("log_oak"), LOG_OAK_TOP("log_oak_top"), LOG_SPRUCE(
				"log_spruce"), LOG_SPRUCE_TOP("log_spruce_top"), MELON_SIDE("melon_side"), MELON_STEM_CONNECTED(
				"melon_stem_connected"), MELON_STEM_DISCONNECTED(
				"melon_stem_disconnected"), MELON_TOP("melon_top"), MOB_SPAWNER(
				"mob_spawner"), MUSHROOM_BLOCK_INSIDE("mushroom_block_inside"), MUSHROOM_BLOCK_SKIN_BROWN(
				"mushroom_block_skin_brown"), MUSHROOM_BLOCK_SKIN_RED(
				"mushroom_block_skin_red"), MUSHROOM_BLOCK_SKIN_STEM(
				"mushroom_block_skin_stem"), MUSHROOM_BROWN("mushroom_brown"), MUSHROOM_RED(
				"mushroom_red"), MYCELIUM_SIDE("mycelium_side"), MYCELIUM_TOP(
				"mycelium_top"), NETHER_BRICK("nether_brick"), NETHER_WART_STAGE_0(
				"nether_wart_stage_0"), NETHER_WART_STAGE_1("nether_wart_stage_1"), NETHER_WART_STAGE_2(
				"nether_wart_stage_2"), NETHERRACK("netherrack"), NOTEBLOCK("noteblock"), OBSIDIAN(
				"obsidian"), PISTON_BOTTOM("piston_bottom"), PISTON_INNER("piston_inner"), PISTON_SIDE(
				"piston_side"), PISTON_TOP_NORMAL("piston_top_normal"), PISTON_TOP_STICKY(
				"piston_top_sticky"), PLANKS_BIRCH("planks_birch"), PLANKS_JUNGLE(
				"planks_jungle"), PLANKS_OAK("planks_oak"), PLANKS_SPRUCE(
				"planks_spruce"), PORTAL("portal"), POTATOES_STAGE_0("potatoes_stage_0"), POTATOES_STAGE_1(
				"potatoes_stage_1"), POTATOES_STAGE_2("potatoes_stage_2"), POTATOES_STAGE_3(
				"potatoes_stage_3"), PUMPKIN_FACE_OFF("pumpkin_face_off"), PUMPKIN_FACE_ON(
				"pumpkin_face_on"), PUMPKIN_SIDE("pumpkin_side"), PUMPKIN_STEM_CONNECTED(
				"pumpkin_stem_connected"), PUMPKIN_STEM_DISCONNECTED(
				"pumpkin_stem_disconnected"), PUMPKIN_TOP("pumpkin_top"), QUARTZ_BLOCK_BOTTOM(
				"quartz_block_bottom"), QUARTZ_BLOCK_CHISELED("quartz_block_chiseled"), QUARTZ_BLOCK_CHISELED_TOP(
				"quartz_block_chiseled_top"), QUARTZ_BLOCK_LINES("quartz_block_lines"), QUARTZ_BLOCK_LINES_TOP(
				"quartz_block_lines_top"), QUARTZ_BLOCK_SIDE("quartz_block_side"), QUARTZ_BLOCK_TOP(
				"quartz_block_top"), QUARTZ_ORE("quartz_ore"), RAIL_ACTIVATOR(
				"rail_activator"), RAIL_ACTIVATOR_POWERED("rail_activator_powered"), RAIL_DETECTOR(
				"rail_detector"), RAIL_DETECTOR_POWERED("rail_detector_powered"), RAIL_GOLDEN(
				"rail_golden"), RAIL_GOLDEN_POWERED("rail_golden_powered"), RAIL_NORMAL(
				"rail_normal"), RAIL_NORMAL_TURNED("rail_normal_turned"), REDSTONE_BLOCK(
				"redstone_block"), REDSTONE_DUST_CROSS("redstone_dust_cross"), REDSTONE_DUST_CROSS_OVERLAY(
				"redstone_dust_cross_overlay"), REDSTONE_DUST_LINE("redstone_dust_line"), REDSTONE_DUST_LINE_OVERLAY(
				"redstone_dust_line_overlay"), REDSTONE_LAMP_OFF("redstone_lamp_off"), REDSTONE_LAMP_ON(
				"redstone_lamp_on"), REDSTONE_ORE("redstone_ore"), REDSTONE_TORCH_OFF(
				"redstone_torch_off"), REDSTONE_TORCH_ON("redstone_torch_on"), REEDS(
				"reeds"), REPEATER_OFF("repeater_off"), REPEATER_ON("repeater_on"), SAND(
				"sand"), SANDSTONE_BOTTOM("sandstone_bottom"), SANDSTONE_CARVED(
				"sandstone_carved"), SANDSTONE_NORMAL("sandstone_normal"), SANDSTONE_SMOOTH(
				"sandstone_smooth"), SANDSTONE_TOP("sandstone_top"), SAPLING_BIRCH(
				"sapling_birch"), SAPLING_JUNGLE("sapling_jungle"), SAPLING_OAK(
				"sapling_oak"), SAPLING_SPRUCE("sapling_spruce"), SNOW("snow"), SOUL_SAND(
				"soul_sand"), SPONGE("sponge"), STONE("stone"), STONE_SLAB_SIDE(
				"stone_slab_side"), STONE_SLAB_TOP("stone_slab_top"), STONEBRICK(
				"stonebrick"), STONEBRICK_CARVED("stonebrick_carved"), STONEBRICK_CRACKED(
				"stonebrick_cracked"), STONEBRICK_MOSSY("stonebrick_mossy"), TALLGRASS(
				"tallgrass"), TNT_BOTTOM("tnt_bottom"), TNT_SIDE("tnt_side"), TNT_TOP(
				"tnt_top"), TORCH_ON("torch_on"), TRAPDOOR("trapdoor"), TRIP_WIRE(
				"trip_wire"), TRIP_WIRE_SOURCE("trip_wire_source"), VINE("vine"), WATER_FLOW(
				"water_flow"), WATER_STILL("water_still"), WATERLILY("waterlily"), WEB(
				"web"), WHEAT_STAGE_0("wheat_stage_0"), WHEAT_STAGE_1("wheat_stage_1"), WHEAT_STAGE_2(
				"wheat_stage_2"), WHEAT_STAGE_3("wheat_stage_3"), WHEAT_STAGE_4(
				"wheat_stage_4"), WHEAT_STAGE_5("wheat_stage_5"), WHEAT_STAGE_6(
				"wheat_stage_6"), WHEAT_STAGE_7("wheat_stage_7"), WOOL_COLORED_BLACK(
				"wool_colored_black"), WOOL_COLORED_BLUE("wool_colored_blue"), WOOL_COLORED_BROWN(
				"wool_colored_brown"), WOOL_COLORED_CYAN("wool_colored_cyan"), WOOL_COLORED_GRAY(
				"wool_colored_gray"), WOOL_COLORED_GREEN("wool_colored_green"), WOOL_COLORED_LIGHT_BLUE(
				"wool_colored_light_blue"), WOOL_COLORED_LIME("wool_colored_lime"), WOOL_COLORED_MAGENTA(
				"wool_colored_magenta"), WOOL_COLORED_ORANGE("wool_colored_orange"), WOOL_COLORED_PINK(
				"wool_colored_pink"), WOOL_COLORED_PURPLE("wool_colored_purple"), WOOL_COLORED_RED(
				"wool_colored_red"), WOOL_COLORED_SILVER("wool_colored_silver"), WOOL_COLORED_WHITE(
				"wool_colored_white"), WOOL_COLORED_YELLOW("wool_colored_yellow"), textures(
				{ &ANVIL_BASE, &ANVIL_TOP_DAMAGED_0, &ANVIL_TOP_DAMAGED_1,
						&ANVIL_TOP_DAMAGED_2, &BEACON, &BED_FEET_END, &BED_FEET_SIDE,
						&BED_FEET_TOP, &BED_HEAD_END, &BED_HEAD_SIDE, &BED_HEAD_TOP,
						&BEDROCK, &BOOKSHELF, &BREWING_STAND, &BREWING_STAND_BASE, &BRICK,
						&CACTUS_BOTTOM, &CACTUS_SIDE, &CACTUS_TOP, &CAKE_BOTTOM,
						&CAKE_INNER, &CAKE_SIDE, &CAKE_TOP, &CARROTS_STAGE_0,
						&CARROTS_STAGE_1, &CARROTS_STAGE_2, &CARROTS_STAGE_3,
						&CAULDRON_BOTTOM, &CAULDRON_INNER, &CAULDRON_SIDE, &CAULDRON_TOP,
						&CLAY, &COAL_BLOCK, &COAL_ORE, &COBBLESTONE, &COBBLESTONE_MOSSY,
						&COCOA_STAGE_0, &COCOA_STAGE_1, &COCOA_STAGE_2, &COMMAND_BLOCK,
						&COMPARATOR_OFF, &COMPARATOR_ON, &CRAFTING_TABLE_FRONT,
						&CRAFTING_TABLE_SIDE, &CRAFTING_TABLE_TOP,
						&DAYLIGHT_DETECTOR_SIDE, &DAYLIGHT_DETECTOR_TOP, &DEADBUSH,
						&DESTROY_STAGE_0, &DESTROY_STAGE_1, &DESTROY_STAGE_2,
						&DESTROY_STAGE_3, &DESTROY_STAGE_4, &DESTROY_STAGE_5,
						&DESTROY_STAGE_6, &DESTROY_STAGE_7, &DESTROY_STAGE_8,
						&DESTROY_STAGE_9, &DIAMOND_BLOCK, &DIAMOND_ORE, &DIRT,
						&DIRT_PODZOL_SIDE, &DIRT_PODZOL_TOP, &DISPENSER_FRONT_HORIZONTAL,
						&DISPENSER_FRONT_VERTICAL, &DOOR_IRON_LOWER, &DOOR_IRON_UPPER,
						&DOOR_WOOD_LOWER, &DOOR_WOOD_UPPER, &DOUBLE_PLANT_FERN_BOTTOM,
						&DOUBLE_PLANT_FERN_TOP, &DOUBLE_PLANT_GRASS_BOTTOM,
						&DOUBLE_PLANT_GRASS_TOP, &DOUBLE_PLANT_PAEONIA_BOTTOM,
						&DOUBLE_PLANT_PAEONIA_TOP, &DOUBLE_PLANT_ROSE_BOTTOM,
						&DOUBLE_PLANT_ROSE_TOP, &DOUBLE_PLANT_SUNFLOWER_BACK,
						&DOUBLE_PLANT_SUNFLOWER_BOTTOM, &DOUBLE_PLANT_SUNFLOWER_FRONT,
						&DOUBLE_PLANT_SUNFLOWER_TOP, &DOUBLE_PLANT_SYRINGA_BOTTOM,
						&DOUBLE_PLANT_SYRINGA_TOP, &DRAGON_EGG, &DROPPER_FRONT_HORIZONTAL,
						&DROPPER_FRONT_VERTICAL, &EMERALD_BLOCK, &EMERALD_ORE,
						&ENCHANTING_TABLE_BOTTOM, &ENCHANTING_TABLE_SIDE,
						&ENCHANTING_TABLE_TOP, &END_STONE, &ENDFRAME_EYE, &ENDFRAME_SIDE,
						&ENDFRAME_TOP, &FARMLAND_DRY, &FARMLAND_WET, &FERN, &FIRE_LAYER_0,
						&FIRE_LAYER_1, &FLOWER_ALLIUM, &FLOWER_BLUE_ORCHID,
						&FLOWER_DANDELION, &FLOWER_HOUSTONIA, &FLOWER_OXEYE_DAISY,
						&FLOWER_PAEONIA, &FLOWER_POT, &FLOWER_ROSE, &FLOWER_TULIP_ORANGE,
						&FLOWER_TULIP_PINK, &FLOWER_TULIP_RED, &FLOWER_TULIP_WHITE,
						&FURNACE_FRONT_OFF, &FURNACE_FRONT_ON, &FURNACE_SIDE,
						&FURNACE_TOP, &GLASS, &GLASS_PANE_TOP, &GLOWSTONE, &GOLD_BLOCK,
						&GOLD_ORE, &GRASS_SIDE, &GRASS_SIDE_OVERLAY, &GRASS_SIDE_SNOWED,
						&GRASS_TOP, &GRAVEL, &HARDENED_CLAY, &HARDENED_CLAY_STAINED_BLACK,
						&HARDENED_CLAY_STAINED_BLUE, &HARDENED_CLAY_STAINED_BROWN,
						&HARDENED_CLAY_STAINED_CYAN, &HARDENED_CLAY_STAINED_GRAY,
						&HARDENED_CLAY_STAINED_GREEN, &HARDENED_CLAY_STAINED_LIGHT_BLUE,
						&HARDENED_CLAY_STAINED_LIME, &HARDENED_CLAY_STAINED_MAGENTA,
						&HARDENED_CLAY_STAINED_ORANGE, &HARDENED_CLAY_STAINED_PINK,
						&HARDENED_CLAY_STAINED_PURPLE, &HARDENED_CLAY_STAINED_RED,
						&HARDENED_CLAY_STAINED_SILVER, &HARDENED_CLAY_STAINED_WHITE,
						&HARDENED_CLAY_STAINED_YELLOW, &HAY_BLOCK_SIDE, &HAY_BLOCK_TOP,
						&HOPPER_INSIDE, &HOPPER_OUTSIDE, &HOPPER_TOP, &ICE, &ICE_PACKED,
						&IRON_BARS, &IRON_BLOCK, &IRON_ORE, &ITEMFRAME_BACKGROUND,
						&JUKEBOX_SIDE, &JUKEBOX_TOP, &LADDER, &LAPIS_BLOCK, &LAPIS_ORE,
						&LAVA_FLOW, &LAVA_STILL, &LEAVES_BIRCH, &LEAVES_BIRCH_OPAQUE,
						&LEAVES_JUNGLE, &LEAVES_JUNGLE_OPAQUE, &LEAVES_OAK,
						&LEAVES_OAK_OPAQUE, &LEAVES_SPRUCE, &LEAVES_SPRUCE_OPAQUE, &LEVER,
						&LOG_BIRCH, &LOG_BIRCH_TOP, &LOG_JUNGLE, &LOG_JUNGLE_TOP,
						&LOG_OAK, &LOG_OAK_TOP, &LOG_SPRUCE, &LOG_SPRUCE_TOP, &MELON_SIDE,
						&MELON_STEM_CONNECTED, &MELON_STEM_DISCONNECTED, &MELON_TOP,
						&MOB_SPAWNER, &MUSHROOM_BLOCK_INSIDE, &MUSHROOM_BLOCK_SKIN_BROWN,
						&MUSHROOM_BLOCK_SKIN_RED, &MUSHROOM_BLOCK_SKIN_STEM,
						&MUSHROOM_BROWN, &MUSHROOM_RED, &MYCELIUM_SIDE, &MYCELIUM_TOP,
						&NETHER_BRICK, &NETHER_WART_STAGE_0, &NETHER_WART_STAGE_1,
						&NETHER_WART_STAGE_2, &NETHERRACK, &NOTEBLOCK, &OBSIDIAN,
						&PISTON_BOTTOM, &PISTON_INNER, &PISTON_SIDE, &PISTON_TOP_NORMAL,
						&PISTON_TOP_STICKY, &PLANKS_BIRCH, &PLANKS_JUNGLE, &PLANKS_OAK,
						&PLANKS_SPRUCE, &PORTAL, &POTATOES_STAGE_0, &POTATOES_STAGE_1,
						&POTATOES_STAGE_2, &POTATOES_STAGE_3, &PUMPKIN_FACE_OFF,
						&PUMPKIN_FACE_ON, &PUMPKIN_SIDE, &PUMPKIN_STEM_CONNECTED,
						&PUMPKIN_STEM_DISCONNECTED, &PUMPKIN_TOP, &QUARTZ_BLOCK_BOTTOM,
						&QUARTZ_BLOCK_CHISELED, &QUARTZ_BLOCK_CHISELED_TOP,
						&QUARTZ_BLOCK_LINES, &QUARTZ_BLOCK_LINES_TOP, &QUARTZ_BLOCK_SIDE,
						&QUARTZ_BLOCK_TOP, &QUARTZ_ORE, &RAIL_ACTIVATOR,
						&RAIL_ACTIVATOR_POWERED, &RAIL_DETECTOR, &RAIL_DETECTOR_POWERED,
						&RAIL_GOLDEN, &RAIL_GOLDEN_POWERED, &RAIL_NORMAL,
						&RAIL_NORMAL_TURNED, &REDSTONE_BLOCK, &REDSTONE_DUST_CROSS,
						&REDSTONE_DUST_CROSS_OVERLAY, &REDSTONE_DUST_LINE,
						&REDSTONE_DUST_LINE_OVERLAY, &REDSTONE_LAMP_OFF,
						&REDSTONE_LAMP_ON, &REDSTONE_ORE, &REDSTONE_TORCH_OFF,
						&REDSTONE_TORCH_ON, &REEDS, &REPEATER_OFF, &REPEATER_ON, &SAND,
						&SANDSTONE_BOTTOM, &SANDSTONE_CARVED, &SANDSTONE_NORMAL,
						&SANDSTONE_SMOOTH, &SANDSTONE_TOP, &SAPLING_BIRCH,
						&SAPLING_JUNGLE, &SAPLING_OAK, &SAPLING_SPRUCE, &SNOW, &SOUL_SAND,
						&SPONGE, &STONE, &STONE_SLAB_SIDE, &STONE_SLAB_TOP, &STONEBRICK,
						&STONEBRICK_CARVED, &STONEBRICK_CRACKED, &STONEBRICK_MOSSY,
						&TALLGRASS, &TNT_BOTTOM, &TNT_SIDE, &TNT_TOP, &TORCH_ON,
						&TRAPDOOR, &TRIP_WIRE, &TRIP_WIRE_SOURCE, &VINE, &WATER_FLOW,
						&WATER_STILL, &WATERLILY, &WEB, &WHEAT_STAGE_0, &WHEAT_STAGE_1,
						&WHEAT_STAGE_2, &WHEAT_STAGE_3, &WHEAT_STAGE_4, &WHEAT_STAGE_5,
						&WHEAT_STAGE_6, &WHEAT_STAGE_7, &WOOL_COLORED_BLACK,
						&WOOL_COLORED_BLUE, &WOOL_COLORED_BROWN, &WOOL_COLORED_CYAN,
						&WOOL_COLORED_GRAY, &WOOL_COLORED_GREEN, &WOOL_COLORED_LIGHT_BLUE,
						&WOOL_COLORED_LIME, &WOOL_COLORED_MAGENTA, &WOOL_COLORED_ORANGE,
						&WOOL_COLORED_PINK, &WOOL_COLORED_PURPLE, &WOOL_COLORED_RED,
						&WOOL_COLORED_SILVER, &WOOL_COLORED_WHITE, &WOOL_COLORED_YELLOW }) {
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
	images[util::rotate_shift_r(FACE_NORTH, rotation, 4)] = north;
	images[util::rotate_shift_r(FACE_SOUTH, rotation, 4)] = south;
	images[util::rotate_shift_r(FACE_EAST, rotation, 4)] = east;
	images[util::rotate_shift_r(FACE_WEST, rotation, 4)] = west;

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
		int new_face = util::rotate_shift_l(face, count, 4);
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

bool BlockImages::loadOther(const std::string& endportal) {
	Image endportal_img;
	if(!endportal_img.readPNG(endportal))
		return false;
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
	if (!loadChests(textures_dir + "/chest/normal.png", textures_dir + "/chest/normal_double.png",
			textures_dir + "/chest/ender.png")) {
		std::cerr << "Error: Unable to load chest/normal.png, chest/normal_double.png or chest/ender.png" << std::endl;
		std::cerr << "from texture directory '" << textures_dir << "'." << std::endl;
		return false;
	} else if (!loadColors(textures_dir + "/colormap/foliage.png",
			textures_dir + "/colormap/grass.png")) {
		std::cerr << "Error: Unable to load colormap/foliage.png or colormap/grass.png";
		std::cerr << "from texture directory '" << textures_dir << "'." << std::endl;
		return false;
	} else if (!loadOther(textures_dir + "/endportal.png")) {
		std::cerr << "Error: Unable to load endportal.png" << std::endl;
		std::cerr << "from texture directory '" << textures_dir << "'." << std::endl;
		return false;
	} else if (!loadBlocks(textures_dir + "/blocks")) {
		std::cerr << "Error: Unable to load block textures" << std::endl;
		std::cerr << "from texture directory '" << textures_dir << "'." << std::endl;
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
		uint16_t dir = util::rotate_shift_l(dir_rotate, rotation, 4) << 4;
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
	} else if (id == 55) { // redstone wire
		// check if powered
		if ((data & 0b1111) != 0)
			return (data & ~(0b1111)) | REDSTONE_POWERED;
		return data & ~(0b1111);
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
	Image water = textures.WATER_STILL.colorize(0, 0.39, 0.89);

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
	createSmallerBlock(id, data, side_face, side_face, top_texture, y1, y2);
}

void BlockImages::createSmallerBlock(uint16_t id, uint16_t data, const Image& texture,
		int y1, int y2) {
	createSmallerBlock(id, data, texture, texture, texture, y1, y2);
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
	grass = textures.GRASS_SIDE_SNOWED;
	top = textures.SNOW;

	BlockImage block_snow;
	block_snow.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, grass);
	block_snow.setFace(FACE_TOP, top);
	setBlockImage(2, GRASS_SNOW, block_snow);
}

void BlockImages::createWater() { // id 8, 9
	Image water = textures.WATER_STILL.colorize(0, 0.39, 0.89);
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
	Image lava = textures.LAVA_STILL;
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
		createBlock(18, 0, textures.LEAVES_OAK); // oak
		createBlock(18, 1, textures.LEAVES_SPRUCE); // pine/spruce
		createBlock(18, 2, textures.LEAVES_OAK); // birch
		createBlock(18, 3, textures.LEAVES_JUNGLE); // jungle
	} else {
		createBlock(18, 0, textures.LEAVES_OAK_OPAQUE); // oak
		createBlock(18, 1, textures.LEAVES_SPRUCE_OPAQUE); // pine/spruce
		createBlock(18, 2, textures.LEAVES_OAK_OPAQUE); // birch
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
	Image front = sticky ? textures.PISTON_TOP_STICKY : textures.PISTON_TOP_NORMAL;
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
		slab_textures[0x0] = textures.STONE_SLAB_SIDE;
		slab_textures[0x1] = textures.SANDSTONE_NORMAL;
		slab_textures[0x2] = textures.PLANKS_OAK;
		slab_textures[0x3] = textures.COBBLESTONE;
		slab_textures[0x4] = textures.BRICK;
		slab_textures[0x5] = textures.STONEBRICK;
		slab_textures[0x6] = textures.NETHER_BRICK;
		slab_textures[0x7] = textures.QUARTZ_BLOCK_SIDE;
	} else {
		slab_textures[0x0] = textures.PLANKS_OAK;
		slab_textures[0x1] = textures.PLANKS_SPRUCE;
		slab_textures[0x2] = textures.PLANKS_BIRCH;
		slab_textures[0x3] = textures.PLANKS_JUNGLE;
	}
	for (std::map<int, Image>::const_iterator it = slab_textures.begin();
	        it != slab_textures.end(); ++it) {
		Image side = it->second;
		Image top = it->second;
		if (it->first == 0 && stone_slabs)
			top = textures.STONE_SLAB_TOP;
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

void BlockImages::createRedstoneWire(bool powered) { // id 55
	Image redstone_cross = textures.REDSTONE_DUST_CROSS;
	Image redstone_line = textures.REDSTONE_DUST_LINE;

	uint8_t color = powered ? 50 : 255;
	redstone_cross = redstone_cross.colorize(color, 0, 0);
	redstone_line = redstone_line.colorize(color, 0, 0);

	// 1/16 of the texture size
	double s = (double) texture_size / 16;

	// go through all redstone combinations
	for (uint16_t i = 0; i <= 0xff; i++) {
		// the redstone extra data starts at the fifth byte
		// so shift the data to the right
		uint16_t data = i << 4;

		// skip unpossible redstone combinations
		// things like neighbor topnorth but not north
		// what would look like redstone on a wall without a connection
		if (((data & REDSTONE_TOPNORTH) && !(data & REDSTONE_NORTH))
				|| ((data & REDSTONE_TOPSOUTH) && !(data & REDSTONE_SOUTH))
				|| ((data & REDSTONE_TOPEAST) && !(data & REDSTONE_EAST))
				|| ((data & REDSTONE_TOPWEST) && !(data & REDSTONE_WEST)))
			continue;

		BlockImage block;
		Image texture = redstone_cross;
		// remove the connections from the cross image
		// if there is no connection
		if(!(data & REDSTONE_NORTH))
			texture.fill(rgba(0, 0, 0, 0), 0, 0, s*16, s*4);
		if(!(data & REDSTONE_SOUTH))
			texture.fill(rgba(0, 0, 0, 0), 0, s*12, s*16, s*4);

		if(!(data & REDSTONE_EAST))
			texture.fill(rgba(0, 0, 0, 0), s*12, 0, s*4, s*16);
		if(!(data & REDSTONE_WEST))
			texture.fill(rgba(0, 0, 0, 0), 0, 0, s*4, s*16);

		// check if we have a line of restone
		if (data == (REDSTONE_NORTH | REDSTONE_SOUTH))
			texture = redstone_line.rotate(ROTATE_90);
		else if (data == (REDSTONE_EAST | REDSTONE_WEST))
			texture = redstone_line;

		// check if
		if (data & REDSTONE_TOPNORTH)
			block.setFace(FACE_NORTH, redstone_line.rotate(ROTATE_90));
		if (data & REDSTONE_TOPSOUTH)
			block.setFace(FACE_SOUTH, redstone_line.rotate(ROTATE_90));
		if (data & REDSTONE_TOPEAST)
			block.setFace(FACE_EAST, redstone_line.rotate(ROTATE_90));
		if (data & REDSTONE_TOPWEST)
			block.setFace(FACE_WEST, redstone_line.rotate(ROTATE_90));

		// rotate the texture to fit the sky directions
		texture = texture.rotate(ROTATE_270);
		block.setFace(FACE_BOTTOM, texture);

		// set extra data
		if (powered)
			data |= REDSTONE_POWERED;
		// we can add the block like this without rotation
		// because we calculate the neighbors on our own,
		// it does not depend on the rotation of the map
		setBlockImage(55, data, buildImage(block));
	}
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
	Image texture = textures.RAIL_NORMAL;
	Image corner_texture = textures.RAIL_NORMAL_TURNED;

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

void BlockImages::createIce(uint8_t id) { // id 79
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
			setBlockImage(id, extra_data, block);
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
	Image pores = textures.MUSHROOM_BLOCK_INSIDE;
	Image stem = textures.MUSHROOM_BLOCK_SKIN_STEM;

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
	Image texture = textures.PUMPKIN_STEM_DISCONNECTED;

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
	Image texture = textures.PLANKS_OAK;
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

void BlockImages::createBrewingStand() { // id 117
	Image block(getBlockImageSize(), getBlockImageSize());
	blitFace(block, FACE_BOTTOM, textures.BREWING_STAND_BASE);
	blitItemStyleBlock(block, textures.BREWING_STAND, textures.BREWING_STAND);
	setBlockImage(117, 0, block);
}

void BlockImages::createCauldron() { // id 118
	Image side = textures.CAULDRON_SIDE;
	Image water = textures.WATER_STILL;

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

void BlockImages::createDragonEgg() { // id 122
	// create an half circle of the dragon egg texture
	// to create an item style block with this texture

	Image texture = textures.DRAGON_EGG;

	// the formula for an half circle is sqrt(r*r - x*x)
	// the circle would go from -r to +r
	// here we use 1.5 * sqrt(r*r - x*x) in the interval [0;texture_size/2],
	// reflect the quarter circle and remove all pixels which are not in the circle

	int r = (texture_size/2) * (texture_size/2);
	for (int x = 0; x <= texture_size/2; x++) {
		int height = 1.5*sqrt(r - x*x);
		for (int y = 0; y < texture_size-height; y++) {
			texture.setPixel(texture_size/2 + x, y, rgba(0, 0, 0, 0));
			texture.setPixel(texture_size/2 - x - 1, y, rgba(0, 0, 0, 0));
		}
	}

	createItemStyleBlock(122, 0, texture);
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

void BlockImages::createFlowerPot() { // id 140
	double s = (double) textures.FLOWER_POT.original.getWidth() / 16;
	Image tmptex = textures.FLOWER_POT.original.clip(s*5, s*10, s*6, s*6);
	Image pot_texture;
	
	s = (double) texture_size / 16;
	tmptex.resizeInterpolated(s*6, s*6, pot_texture);
	
	int xoff = std::ceil(s*10);
	int yoff = std::ceil(s*16);
	
	Image pot(getBlockImageSize(), getBlockImageSize());
	blitFace(pot, FACE_NORTH, pot_texture, xoff, yoff, true, dleft, dright);
	blitFace(pot, FACE_EAST, pot_texture, xoff, yoff, true, dleft, dright);
	blitFace(pot, FACE_TOP, textures.DIRT.clip(0, 0, s*6, s*6), xoff, yoff+s*3);
	
	Image contents[] = {
		Image(),
		textures.FLOWER_ROSE,
		textures.FLOWER_DANDELION,
		textures.SAPLING_OAK,
		textures.SAPLING_SPRUCE,
		textures.SAPLING_BIRCH,
		textures.SAPLING_JUNGLE,
		textures.MUSHROOM_RED,
		textures.MUSHROOM_BROWN,
		Image(),
		textures.DEADBUSH,
		textures.FERN,
	};
	
	for (int16_t i = 0; i < 11; i++) {
		Image block = pot;
		
		if (i == 9) {
			Image cactus = getBlock(81, 0);
			Image content;
			cactus.resizeSimple(s*16, s*16, content);
			block.alphablit(content, s*8, s*8);
		} else if (i != 0) {
			Image content(texture_size*2, texture_size*2);
			blitItemStyleBlock(content, contents[i], contents[i]);
			block.alphablit(content, 0, s*-3);
		}
		
		blitFace(block, FACE_WEST, pot_texture, xoff, yoff, true, dleft, dright);
		blitFace(block, FACE_SOUTH, pot_texture, xoff, yoff, true, dleft, dright);
		
		setBlockImage(140, i, block);
	}
}

void BlockImages::loadBlocks() {
	buildCustomTextures();
	unknown_block = buildImage(BlockImage().setFace(0b11111, unknown_block));

	BlockTextures& t = textures;

	createBlock(1, 0, t.STONE); // stone
	createGrassBlock(); // id 2
	createBlock(3, 0, t.DIRT); // dirt
	createBlock(4, 0, t.COBBLESTONE); // cobblestone
	// -- wooden planks
	createBlock(5, 0, t.PLANKS_OAK); // oak
	createBlock(5, 1, t.PLANKS_SPRUCE); // pine/spruce
	createBlock(5, 2, t.PLANKS_BIRCH); // birch
	createBlock(5, 3, t.PLANKS_BIRCH); // jungle
	// -- saplings
	createItemStyleBlock(6, 0, t.SAPLING_OAK); // oak
	createItemStyleBlock(6, 1, t.SAPLING_SPRUCE); // spruce
	createItemStyleBlock(6, 2, t.SAPLING_BIRCH); // birch
	createItemStyleBlock(6, 3, t.SAPLING_JUNGLE); // jungle
	// --
	createBlock(7, 0, t.BEDROCK); // bedrock
	createWater(); // id 8, 9
	createLava(); // id 10, 11
	createBlock(12, 0, t.SAND); // sand
	createBlock(13, 0, t.GRAVEL); // gravel
	createBlock(14, 0, t.GOLD_ORE); // gold ore
	createBlock(15, 0, t.IRON_ORE); // iron ore
	createBlock(16, 0, t.COAL_ORE); // coal ore
	// -- wood
	createWood(17, 0, t.LOG_OAK, t.LOG_OAK_TOP); // oak
	createWood(17, 1, t.LOG_SPRUCE, t.LOG_SPRUCE_TOP); // pine/spruce
	createWood(17, 2, t.LOG_BIRCH, t.LOG_BIRCH_TOP); // birch
	createWood(17, 3, t.LOG_JUNGLE, t.LOG_JUNGLE_TOP); // jungle
	createLeaves(); // id 18
	createBlock(19, 0, t.SPONGE); // sponge
	createBlock(20, 0, t.GLASS); // glass
	createBlock(21, 0, t.LAPIS_ORE); // lapis lazuli ore
	createBlock(22, 0, t.LAPIS_BLOCK); // lapis lazuli block
	createDispenserDropper(23, t.DISPENSER_FRONT_HORIZONTAL); // dispenser
	// -- sandstone
	createBlock(24, 0, t.SANDSTONE_NORMAL, t.SANDSTONE_TOP); // normal
	createBlock(24, 1, t.SANDSTONE_CARVED, t.SANDSTONE_TOP); // chiseled
	createBlock(24, 2, t.SANDSTONE_SMOOTH, t.SANDSTONE_TOP); // smooth
	// --
	createBlock(25, 0, t.NOTEBLOCK); // noteblock
	createBed(); // id 26 bed
	createStraightRails(27, 0, t.RAIL_GOLDEN); // id 27 powered rail (unpowered)
	createStraightRails(27, 8, t.RAIL_GOLDEN_POWERED); // id 27 powered rail (powered)
	createStraightRails(28, 0, t.RAIL_ACTIVATOR); // id 28 detector rail
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
	createBlock(35, 0, t.WOOL_COLORED_WHITE); // white
	createBlock(35, 1, t.WOOL_COLORED_ORANGE); // orange
	createBlock(35, 2, t.WOOL_COLORED_MAGENTA); // magenta
	createBlock(35, 3, t.WOOL_COLORED_LIGHT_BLUE); // light blue
	createBlock(35, 4, t.WOOL_COLORED_YELLOW); // yellow
	createBlock(35, 5, t.WOOL_COLORED_LIME); // lime
	createBlock(35, 6, t.WOOL_COLORED_PINK); // pink
	createBlock(35, 7, t.WOOL_COLORED_GRAY); // gray
	createBlock(35, 8, t.WOOL_COLORED_SILVER); // light gray
	createBlock(35, 9, t.WOOL_COLORED_CYAN); // cyan
	createBlock(35, 10, t.WOOL_COLORED_PURPLE); // purple
	createBlock(35, 11, t.WOOL_COLORED_BLUE); // blue
	createBlock(35, 12, t.WOOL_COLORED_BROWN); // brown
	createBlock(35, 13, t.WOOL_COLORED_GREEN); // green
	createBlock(35, 14, t.WOOL_COLORED_RED); // red
	createBlock(35, 15, t.WOOL_COLORED_BLACK); // black
	// --
	createBlock(36, 0, empty_texture); // block moved by piston aka 'block 36'
	createItemStyleBlock(37, 0, t.FLOWER_DANDELION); // dandelion
	createItemStyleBlock(38, 0, t.FLOWER_ROSE); // rose
	createItemStyleBlock(39, 0, t.MUSHROOM_BROWN); // brown mushroom
	createItemStyleBlock(40, 0, t.MUSHROOM_RED); // red mushroom
	createBlock(41, 0, t.GOLD_BLOCK); // block of gold
	createBlock(42, 0, t.IRON_BLOCK); // block of iron
	createSlabs(43, true, true); // double stone slabs
	createSlabs(44, true, false); // normal stone slabs
	createBlock(45, 0, t.BRICK); // bricks
	createBlock(46, 0, t.TNT_SIDE, t.TNT_TOP); // tnt
	createBlock(47, 0, t.BOOKSHELF, t.PLANKS_OAK); // bookshelf
	createBlock(48, 0, t.COBBLESTONE_MOSSY); // moss stone
	createBlock(49, 0, t.OBSIDIAN); // obsidian
	createTorch(50, t.TORCH_ON); // torch
	createItemStyleBlock(51, 0, t.FIRE_LAYER_0); // fire
	createBlock(52, 0, t.MOB_SPAWNER); // monster spawner
	createStairs(53, t.PLANKS_OAK); // oak wood stairs
	createChest(54, chest); // chest
	createDoubleChest(54, largechest); // chest
	createRedstoneWire(true); // id 55
	createRedstoneWire(false);
	createBlock(56, 0, t.DIAMOND_ORE); // diamond ore
	createBlock(57, 0, t.DIAMOND_BLOCK); // block of diamond
	createBlock(58, 0, t.CRAFTING_TABLE_SIDE, t.CRAFTING_TABLE_FRONT, t.CRAFTING_TABLE_TOP); // crafting table
	// -- wheat
	createItemStyleBlock(59, 0, t.WHEAT_STAGE_0); //
	createItemStyleBlock(59, 1, t.WHEAT_STAGE_1); //
	createItemStyleBlock(59, 2, t.WHEAT_STAGE_2); //
	createItemStyleBlock(59, 3, t.WHEAT_STAGE_3); //
	createItemStyleBlock(59, 4, t.WHEAT_STAGE_4); //
	createItemStyleBlock(59, 5, t.WHEAT_STAGE_5); //
	createItemStyleBlock(59, 6, t.WHEAT_STAGE_6); //
	createItemStyleBlock(59, 7, t.WHEAT_STAGE_7); //
	// --
	createBlock(60, 0, t.DIRT, t.FARMLAND_WET); // farmland
	createRotatedBlock(61, 0, t.FURNACE_FRONT_OFF, t.FURNACE_SIDE, t.FURNACE_TOP); // furnace
	createRotatedBlock(62, 0, t.FURNACE_FRONT_ON, t.FURNACE_SIDE, t.FURNACE_TOP); // burning furnace
	// id 63 // sign post
	createDoor(64, t.DOOR_WOOD_LOWER, t.DOOR_WOOD_UPPER); // wooden door
	// -- ladders
	createSingleFaceBlock(65, 2, FACE_SOUTH, t.LADDER);
	createSingleFaceBlock(65, 3, FACE_NORTH, t.LADDER);
	createSingleFaceBlock(65, 4, FACE_EAST, t.LADDER);
	createSingleFaceBlock(65, 5, FACE_WEST, t.LADDER);
	// --
	createRails(); // id 66
	createStairs(67, t.COBBLESTONE); // cobblestone stairs
	// id 68 // wall sign
	// id 69 // lever
	createSmallerBlock(70, 0, t.STONE, t.STONE, 0, 1); // stone pressure plate
	createDoor(71, t.DOOR_IRON_LOWER, t.DOOR_IRON_UPPER); // iron door
	createSmallerBlock(72, 0, t.PLANKS_OAK, t.PLANKS_OAK, 0, 1); // wooden pressure plate
	createBlock(73, 0, t.REDSTONE_ORE); // redstone ore
	createBlock(74, 0, t.REDSTONE_ORE); // glowing redstone ore
	createTorch(75, t.REDSTONE_TORCH_OFF); // redstone torch off
	createTorch(76, t.REDSTONE_TORCH_ON); // redstone torch on
	createButton(77, t.STONE); // stone button
	createSnow(); // id 78
	createIce(79); // ice block
	createBlock(80, 0, t.SNOW); // snow block
	createCactus(); // id 81
	createBlock(82, 0, t.CLAY); // clay block
	createItemStyleBlock(83, 0, t.REEDS); // sugar cane
	createBlock(84, 0, t.NOTEBLOCK, t.JUKEBOX_TOP); // jukebox
	createFence(85, t.PLANKS_OAK); // fence
	createPumkin(86, t.PUMPKIN_FACE_OFF); // pumpkin
	createBlock(87, 0, t.NETHERRACK); // netherrack
	createBlock(88, 0, t.SOUL_SAND); // soul sand
	createBlock(89, 0, t.GLOWSTONE); // glowstone block
	createBlock(90, 0, t.PORTAL); // nether portal block
	createPumkin(91, t.PUMPKIN_FACE_ON); // jack-o-lantern
	createCake(); // id 92
	createRedstoneRepeater(93, t.REPEATER_OFF); // redstone repeater off
	createRedstoneRepeater(94, t.REPEATER_ON); // redstone repeater on
	createChest(95, chest); // locked chest
	createTrapdoor(); // id 96 // trapdoor
	// -- monster egg
	createBlock(97, 0, t.STONE); // stone
	createBlock(97, 1, t.COBBLESTONE); // cobblestone
	createBlock(97, 2, t.STONEBRICK); // stone brick
	// --
	// -- stone bricks
	createBlock(98, 0, t.STONEBRICK); // normal
	createBlock(98, 1, t.STONEBRICK_MOSSY); // mossy
	createBlock(98, 2, t.STONEBRICK_CRACKED); // cracked
	createBlock(98, 3, t.STONEBRICK_CARVED); // chiseled
	// --
	createHugeMushroom(99, t.MUSHROOM_BLOCK_SKIN_BROWN); // huge brown mushroom
	createHugeMushroom(100, t.MUSHROOM_BLOCK_SKIN_RED); // huge red mushroom
	createBarsPane(101, t.IRON_BARS); // iron bars
	createBarsPane(102, t.GLASS); // glass pane
	createBlock(103, 0, t.MELON_SIDE, t.MELON_TOP); // melon
	createStem(104); // pumpkin stem
	createStem(105); // melon stem
	createVines(); // id 106 // vines
	createFenceGate(); // id 107 // fence gate
	createStairs(108, t.BRICK); // brick stairs
	createStairs(109, t.STONEBRICK); // stone brick stairs
	createBlock(110, 0, t.MYCELIUM_SIDE, t.MYCELIUM_TOP); // mycelium
	createSingleFaceBlock(111, 0, FACE_BOTTOM, t.WATERLILY); // lily pad
	createBlock(112, 0, t.NETHER_BRICK); // nether brick
	createFence(113, t.NETHER_BRICK); // nether brick fence
	createStairs(114, t.NETHER_BRICK); // nether brick stairs
	// -- nether wart
	createItemStyleBlock(115, 0, t.NETHER_WART_STAGE_0);
	createItemStyleBlock(115, 1, t.NETHER_WART_STAGE_1);
	createItemStyleBlock(115, 2, t.NETHER_WART_STAGE_1);
	createItemStyleBlock(115, 3, t.NETHER_WART_STAGE_2);
	// --
	createSmallerBlock(116, 0, t.ENCHANTING_TABLE_SIDE,
			t.ENCHANTING_TABLE_TOP, 0, texture_size * 0.75); // enchantment table
	//createItemStyleBlock(117, 0, t.BREWING_STAND); // brewing stand
	createBrewingStand(); // id 117
	createCauldron(); // id 118 // cauldron
	createSmallerBlock(119, 0, endportal_texture, endportal_texture,
			texture_size * 0.25, texture_size * 0.75); // end portal
	createSmallerBlock(120, 0, t.ENDFRAME_SIDE, t.ENDFRAME_TOP, 0,
			texture_size * 0.8125); // end portal frame
	createBlock(121, 0, t.END_STONE); // end stone
	createDragonEgg(); // id 122
	createBlock(123, 0, t.REDSTONE_LAMP_OFF); // redstone lamp inactive
	createBlock(124, 0, t.REDSTONE_LAMP_ON); // redstone lamp active
	createSlabs(125, false, true); // wooden double slabs
	createSlabs(126, false, false); // wooden normal slabs
	// id 127 // cocoa plant
	createStairs(128, t.SANDSTONE_NORMAL); // sandstone stairs
	createBlock(129, 0, t.EMERALD_ORE); // emerald ore
	createChest(130, enderchest); // ender chest
	// id 131 // tripwire hook
	// id 132 // tripwire
	createBlock(133, 0, t.EMERALD_BLOCK); // block of emerald
	createStairs(134, t.PLANKS_SPRUCE); // spruce wood stairs
	createStairs(135, t.PLANKS_BIRCH); // birch wood stairs
	createStairs(136, t.PLANKS_JUNGLE); // jungle wood stairs
	createBlock(137, 0, t.COMMAND_BLOCK); // command block
	createBeacon(); // beacon
	// id 139 // cobblestone wall
	createFlowerPot(); // id 140
	// carrots --
	createItemStyleBlock(141, 0, t.CARROTS_STAGE_0);
	createItemStyleBlock(141, 1, t.CARROTS_STAGE_0);
	createItemStyleBlock(141, 2, t.CARROTS_STAGE_1);
	createItemStyleBlock(141, 3, t.CARROTS_STAGE_1);
	createItemStyleBlock(141, 4, t.CARROTS_STAGE_2);
	createItemStyleBlock(141, 5, t.CARROTS_STAGE_2);
	createItemStyleBlock(141, 6, t.CARROTS_STAGE_2);
	createItemStyleBlock(141, 7, t.CARROTS_STAGE_3);
	// --
	// potatoes --
	createItemStyleBlock(142, 0, t.POTATOES_STAGE_0);
	createItemStyleBlock(142, 1, t.POTATOES_STAGE_0);
	createItemStyleBlock(142, 2, t.POTATOES_STAGE_1);
	createItemStyleBlock(142, 3, t.POTATOES_STAGE_1);
	createItemStyleBlock(142, 4, t.POTATOES_STAGE_2);
	createItemStyleBlock(142, 5, t.POTATOES_STAGE_2);
	createItemStyleBlock(142, 6, t.POTATOES_STAGE_2);
	createItemStyleBlock(142, 7, t.POTATOES_STAGE_3);
	// --
	createButton(143, t.PLANKS_OAK); // wooden button
	// id 144 // head
	// id 145 // anvil
	createChest(146, chest); // trapped chest
	createSmallerBlock(147, 0, t.GOLD_BLOCK, t.GOLD_BLOCK, 0, 1); // weighted pressure plate (light)
	createSmallerBlock(148, 0, t.QUARTZ_BLOCK_LINES, t.QUARTZ_BLOCK_LINES, 0, 1); // weighted pressure plate (heavy)
	createRedstoneRepeater(149, t.COMPARATOR_OFF); // redstone comparator (inactive)
	createRedstoneRepeater(150, t.COMPARATOR_ON); // redstone comparator (active)
	createSmallerBlock(151, 0, t.DAYLIGHT_DETECTOR_SIDE, t.DAYLIGHT_DETECTOR_TOP, 0, 8); // daylight sensor
	createBlock(152, 0, t.REDSTONE_BLOCK); // block of redstone
	createBlock(153, 0, t.QUARTZ_ORE); // nether quartz ore
	// id 154 // hopper
	// block of quartz --
	createBlock(155, 0, t.QUARTZ_BLOCK_SIDE, t.QUARTZ_BLOCK_TOP);
	createBlock(155, 1, t.QUARTZ_BLOCK_CHISELED, t.QUARTZ_BLOCK_CHISELED_TOP);
	createBlock(155, 2, t.QUARTZ_BLOCK_LINES, t.QUARTZ_BLOCK_LINES_TOP);
	createBlock(155, 3, t.QUARTZ_BLOCK_LINES_TOP, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90), t.QUARTZ_BLOCK_LINES);
	createBlock(155, 4, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90), t.QUARTZ_BLOCK_LINES_TOP, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90));
	// --
	createStairs(156, t.QUARTZ_BLOCK_SIDE); // quartz stairs
	createStraightRails(157, 0, t.RAIL_ACTIVATOR); // activator rail
	createDispenserDropper(158, t.DROPPER_FRONT_HORIZONTAL); // dropper
	// stained clay --
	createBlock(159, 0, t.HARDENED_CLAY_STAINED_WHITE);
	createBlock(159, 1, t.HARDENED_CLAY_STAINED_ORANGE);
	createBlock(159, 2, t.HARDENED_CLAY_STAINED_MAGENTA);
	createBlock(159, 3, t.HARDENED_CLAY_STAINED_LIGHT_BLUE);
	createBlock(159, 4, t.HARDENED_CLAY_STAINED_YELLOW);
	createBlock(159, 5, t.HARDENED_CLAY_STAINED_LIME);
	createBlock(159, 6, t.HARDENED_CLAY_STAINED_PINK);
	createBlock(159, 7, t.HARDENED_CLAY_STAINED_GRAY);
	createBlock(159, 8, t.HARDENED_CLAY_STAINED_SILVER);
	createBlock(159, 9, t.HARDENED_CLAY_STAINED_CYAN);
	createBlock(159, 10, t.HARDENED_CLAY_STAINED_PURPLE);
	createBlock(159, 11, t.HARDENED_CLAY_STAINED_BLUE);
	createBlock(159, 12, t.HARDENED_CLAY_STAINED_BROWN);
	createBlock(159, 13, t.HARDENED_CLAY_STAINED_GREEN);
	createBlock(159, 14, t.HARDENED_CLAY_STAINED_RED);
	createBlock(159, 15, t.HARDENED_CLAY_STAINED_BLACK);
	// --

	// hay block --
	createBlock(170, 0, t.HAY_BLOCK_SIDE, t.HAY_BLOCK_TOP); // normal orientation
	createBlock(170, 4, t.HAY_BLOCK_TOP, t.HAY_BLOCK_SIDE.rotate(1), t.HAY_BLOCK_SIDE); // east-west
	createBlock(170, 8, t.HAY_BLOCK_SIDE.rotate(1), t.HAY_BLOCK_TOP, t.HAY_BLOCK_SIDE.rotate(1)); // north-south
	// --
	// carpet --
	createSmallerBlock(171, 0, t.WOOL_COLORED_WHITE, 0, 1);
	createSmallerBlock(171, 1, t.WOOL_COLORED_ORANGE, 0, 1);
	createSmallerBlock(171, 2, t.WOOL_COLORED_MAGENTA, 0, 1);
	createSmallerBlock(171, 3, t.WOOL_COLORED_LIGHT_BLUE, 0, 1);
	createSmallerBlock(171, 4, t.WOOL_COLORED_YELLOW, 0, 1);
	createSmallerBlock(171, 5, t.WOOL_COLORED_LIME, 0, 1);
	createSmallerBlock(171, 6, t.WOOL_COLORED_PINK, 0, 1);
	createSmallerBlock(171, 7, t.WOOL_COLORED_GRAY, 0, 1);
	createSmallerBlock(171, 8, t.WOOL_COLORED_SILVER, 0, 1);
	createSmallerBlock(171, 9, t.WOOL_COLORED_CYAN, 0, 1);
	createSmallerBlock(171, 10, t.WOOL_COLORED_PURPLE, 0, 1);
	createSmallerBlock(171, 11, t.WOOL_COLORED_BLUE, 0, 1);
	createSmallerBlock(171, 12, t.WOOL_COLORED_BROWN, 0, 1);
	createSmallerBlock(171, 13, t.WOOL_COLORED_GREEN, 0, 1);
	createSmallerBlock(171, 14, t.WOOL_COLORED_RED, 0, 1);
	createSmallerBlock(171, 15, t.WOOL_COLORED_BLACK, 0, 1);
	// --
	createBlock(172, 0, t.HARDENED_CLAY); // hardened clay
	createBlock(173, 0, t.COAL_BLOCK); // block of coal
	createIce(174); // packed ice
	//createBlock(174, 0, t.DIRT_PODZOL_SIDE, t.DIRT_PODZOL_SIDE, t.DIRT_PODZOL_TOP); // podzol
}

bool BlockImages::isBlockTransparent(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	// remove edge data
	data &= ~(EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM);

	// special case for doors because they are only used with special data
	// and not with the original minecraft data
	// without this the lighting code for example would need to filter the door data
	// FIXME
	if (id == 64 || id == 71)
		return true;
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
