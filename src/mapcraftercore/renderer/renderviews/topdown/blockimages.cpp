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

#include "blockimages.h"

#include "../../biomes.h"
#include "../../../util.h"

namespace mapcrafter {
namespace renderer {

TopdownBlockImages::TopdownBlockImages() {
}

TopdownBlockImages::~TopdownBlockImages() {
}

const RGBAImage& TopdownBlockImages::getOpaqueWater(bool south, bool west) const {
	return unknown_block;
}

int TopdownBlockImages::getBlockSize() const {
	return texture_size;
}

uint16_t TopdownBlockImages::filterBlockData(uint16_t id, uint16_t data) const {
	// call super method
	data = AbstractBlockImages::filterBlockData(id, data);
	// for now we don't use the edge data provided by the tile renderer
	// TODO?
	data &= ~(EDGE_EAST | EDGE_NORTH | EDGE_BOTTOM);

	// of some blocks we don't need any data at all
	if ((id >= 10 && id <= 11) // lava
			|| id == 24 // sandstone
			|| id == 50 // torch
			|| id == 51 // fire
			|| id == 78 // snow
			|| id == 84 // jukebox
			|| id == 90 // nether portal
			|| id == 151 || id == 178 // lighting sensor
			)
		return 0;

	if (id == 8 || id == 9) // water
		return data & OPAQUE_WATER; // we just need that one bit
	if (id == 18 || id == 161) // leaves
		return data & (0xff00 | 0b00000011);
	else if (id == 60) // farmland
		return data & 0xff00;
	else if (id == 81 || id == 83 || id == 92) // cactus, sugar cane, cake
		return data & 0xff00;
	else if (id == 119 || id == 120) // end portal, end portal frame
		return data & 0xff00;
	return data;
}

bool TopdownBlockImages::isImageTransparent(const RGBAImage& block) const {
	for (int x = 0; x < block.getWidth(); x++)
		for (int y = 0; y < block.getHeight(); y++)
			if (rgba_alpha(block.getPixel(x, y)) < 255)
				return true;
	return false;
}

void TopdownBlockImages::setBlockImage(uint16_t id, uint16_t data, const RGBAImage& block) {
	AbstractBlockImages::setBlockImage(id, data, block.rotate(rotation));
}

RGBAImage TopdownBlockImages::createUnknownBlock() const {
	RGBAImage unknown_block(texture_size, texture_size);
	unknown_block.fill(rgba(255, 0, 0, 255), 0, 0, texture_size, texture_size);
	return unknown_block;
}

RGBAImage TopdownBlockImages::createBiomeBlock(uint16_t id, uint16_t data,
		const Biome& biome) const {if (!block_images.count(id | (data << 16)))
			return unknown_block;
	uint32_t color;
	// leaves have the foliage colors
	// for birches, the color x/y coordinate is flipped
	if (id == 18)
		color = biome.getColor(resources.getGrassColors(), (data & 0b11) == 2);
	else
		color = biome.getColor(resources.getFoliageColors(), false);

	double r = (double) rgba_red(color) / 255;
	double g = (double) rgba_green(color) / 255;
	double b = (double) rgba_blue(color) / 255;

	/*
	// grass block needs something special
	if (id == 2 && false) {
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
	*/

	return block_images.at(id | (data << 16)).colorize(r, g, b);
}

void TopdownBlockImages::createBlocks() {
	const BlockTextures& t = resources.getBlockTextures();

	setBlockImage(1, 0, t.STONE);
	setBlockImage(2, 0, t.GRASS_TOP);
	setBlockImage(3, 0, t.DIRT);
	setBlockImage(3, 1, t.DIRT);
	setBlockImage(3, 2, t.DIRT_PODZOL_TOP);
	setBlockImage(4, 0, t.COBBLESTONE); // cobblestone
	// -- wooden planks
	setBlockImage(5, 0, t.PLANKS_OAK); // oak
	setBlockImage(5, 1, t.PLANKS_SPRUCE); // pine/spruce
	setBlockImage(5, 2, t.PLANKS_BIRCH); // birch
	setBlockImage(5, 3, t.PLANKS_JUNGLE); // jungle
	setBlockImage(5, 4, t.PLANKS_ACACIA); // acacia
	setBlockImage(5, 5, t.PLANKS_BIG_OAK); // dark oak
	// --
	// -- saplings
	setBlockImage(6, 0, t.SAPLING_OAK); // oak
	setBlockImage(6, 1, t.SAPLING_SPRUCE); // spruce
	setBlockImage(6, 2, t.SAPLING_BIRCH); // birch
	setBlockImage(6, 3, t.SAPLING_JUNGLE); // jungle
	setBlockImage(6, 4, t.SAPLING_ACACIA); // acacia
	setBlockImage(6, 5, t.SAPLING_ROOFED_OAK); // dark oak
	// --
	setBlockImage(7, 0, t.BEDROCK); // bedrock
	setBlockImage(8, 0, t.WATER_STILL.colorize(0, 0.39, 0.89));
	setBlockImage(9, 0, t.WATER_STILL.colorize(0, 0.39, 0.89));
	setBlockImage(10, 0, t.LAVA_STILL);
	setBlockImage(11, 0, t.LAVA_STILL);
	setBlockImage(12, 0, t.SAND); // sand
	setBlockImage(12, 1, t.RED_SAND); // red sand
	setBlockImage(13, 0, t.GRAVEL); // gravel
	setBlockImage(14, 0, t.GOLD_ORE); // gold ore
	setBlockImage(15, 0, t.IRON_ORE); // iron ore
	setBlockImage(16, 0, t.COAL_ORE); // coal ore
	setBlockImage(17, 0, t.LOG_OAK_TOP); // oak
	setBlockImage(17, 1, t.LOG_SPRUCE_TOP); // pine/spruce
	setBlockImage(17, 2, t.LOG_BIRCH_TOP); // birch
	setBlockImage(17, 3, t.LOG_JUNGLE_TOP); // jungle
	setBlockImage(18, 0, t.LEAVES_OAK); // oak
	setBlockImage(18, 1, t.LEAVES_SPRUCE); // pine/spruce
	setBlockImage(18, 2, t.LEAVES_OAK); // birch
	setBlockImage(18, 3, t.LEAVES_JUNGLE); // jungle
	setBlockImage(19, 0, t.SPONGE); // sponge
	setBlockImage(19, 1, t.SPONGE_WET); // wet sponge
	setBlockImage(20, 0, t.GLASS);
	setBlockImage(21, 0, t.LAPIS_ORE); // lapis lazuli ore
	setBlockImage(22, 0, t.LAPIS_BLOCK); // lapis lazuli block
	// id 23 // dispenser
	setBlockImage(24, 0, t.SANDSTONE_TOP); // sandstone
	setBlockImage(25, 0, t.NOTEBLOCK); // noteblock
	// id 26 // bed
	// id 27 // powered rail
	// id 28 // detector rail
	// id 29 // sticky piston
	setBlockImage(30, 0, t.WEB); // cobweb
	// -- tall grass
	setBlockImage(31, 0, t.DEADBUSH); // dead bush style
	setBlockImage(31, 1, t.TALLGRASS); // tall grass
	setBlockImage(31, 2, t.FERN); // fern
	// --
	setBlockImage(32, 0, t.DEADBUSH); // dead bush
	// id 33 // piston
	// id 34 // piston extension
	// -- wool
	setBlockImage(35, 0, t.WOOL_COLORED_WHITE); // white
	setBlockImage(35, 1, t.WOOL_COLORED_ORANGE); // orange
	setBlockImage(35, 2, t.WOOL_COLORED_MAGENTA); // magenta
	setBlockImage(35, 3, t.WOOL_COLORED_LIGHT_BLUE); // light blue
	setBlockImage(35, 4, t.WOOL_COLORED_YELLOW); // yellow
	setBlockImage(35, 5, t.WOOL_COLORED_LIME); // lime
	setBlockImage(35, 6, t.WOOL_COLORED_PINK); // pink
	setBlockImage(35, 7, t.WOOL_COLORED_GRAY); // gray
	setBlockImage(35, 8, t.WOOL_COLORED_SILVER); // light gray
	setBlockImage(35, 9, t.WOOL_COLORED_CYAN); // cyan
	setBlockImage(35, 10, t.WOOL_COLORED_PURPLE); // purple
	setBlockImage(35, 11, t.WOOL_COLORED_BLUE); // blue
	setBlockImage(35, 12, t.WOOL_COLORED_BROWN); // brown
	setBlockImage(35, 13, t.WOOL_COLORED_GREEN); // green
	setBlockImage(35, 14, t.WOOL_COLORED_RED); // red
	setBlockImage(35, 15, t.WOOL_COLORED_BLACK); // black
	// --
	setBlockImage(36, 0, empty_texture); // block moved by piston aka 'block 36'
	setBlockImage(37, 0, t.FLOWER_DANDELION); // dandelion
	// -- poppy -- different flowers
	setBlockImage(38, 0, t.FLOWER_ROSE); // poppy
	setBlockImage(38, 1, t.FLOWER_BLUE_ORCHID); // blue orchid
	setBlockImage(38, 2, t.FLOWER_ALLIUM); // azure bluet
	setBlockImage(38, 3, t.FLOWER_HOUSTONIA); //
	setBlockImage(38, 4, t.FLOWER_TULIP_RED); // red tulip
	setBlockImage(38, 5, t.FLOWER_TULIP_ORANGE); // orange tulip
	setBlockImage(38, 6, t.FLOWER_TULIP_WHITE); // white tulip
	setBlockImage(38, 7, t.FLOWER_TULIP_PINK); // pink tulip
	setBlockImage(38, 8, t.FLOWER_OXEYE_DAISY); // oxeye daisy
	// --
	setBlockImage(39, 0, t.MUSHROOM_BROWN); // brown mushroom
	setBlockImage(40, 0, t.MUSHROOM_RED); // red mushroom
	setBlockImage(41, 0, t.GOLD_BLOCK); // block of gold
	setBlockImage(42, 0, t.IRON_BLOCK); // block of iron
	// id 43 // double stone slabs
	// id 44 // normal stone slabs
	setBlockImage(45, 0, t.BRICK); // bricks
	setBlockImage(46, 0, t.TNT_TOP); // tnt
	setBlockImage(47, 0, t.PLANKS_OAK); // bookshelf
	setBlockImage(48, 0, t.COBBLESTONE_MOSSY); // moss stone
	setBlockImage(49, 0, t.OBSIDIAN); // obsidian
	setBlockImage(50, 0, t.TORCH_ON); // torch
	setBlockImage(51, 0, t.FIRE_LAYER_0); // fire
	setBlockImage(52, 0, t.MOB_SPAWNER); // monster spawner
	// id 53 // oak wood stairs
	// id 54 // chest
	// id 55 // redstone wire
	setBlockImage(56, 0, t.DIAMOND_ORE); // diamond ore
	setBlockImage(57, 0, t.DIAMOND_BLOCK); // block of diamond
	setBlockImage(58, 0, t.CRAFTING_TABLE_TOP); // crafting table
	// -- wheat
	setBlockImage(59, 0, t.WHEAT_STAGE_0); //
	setBlockImage(59, 1, t.WHEAT_STAGE_1); //
	setBlockImage(59, 2, t.WHEAT_STAGE_2); //
	setBlockImage(59, 3, t.WHEAT_STAGE_3); //
	setBlockImage(59, 4, t.WHEAT_STAGE_4); //
	setBlockImage(59, 5, t.WHEAT_STAGE_5); //
	setBlockImage(59, 6, t.WHEAT_STAGE_6); //
	setBlockImage(59, 7, t.WHEAT_STAGE_7); //
	// --
	setBlockImage(60, 0, t.FARMLAND_WET); // farmland
	setBlockImage(61, 0, t.FURNACE_TOP); // furnace
	setBlockImage(62, 0, t.FURNACE_TOP); // burning furnace
	// id 63 // sign post
	// id 64 // wooden door
	// id 65 // ladders
	// id 66 // rails
	// id 67 // cobblestone stairs
	// id 68 // wall sign
	// id 69 // lever
	setBlockImage(70, 0, t.STONE); // stone pressure plate // TODO
	// id 71 // iron door
	setBlockImage(72, 0, t.PLANKS_OAK); // wooden pressure plate // TODO
	setBlockImage(73, 0, t.REDSTONE_ORE); // redstone ore
	setBlockImage(74, 0, t.REDSTONE_ORE); // glowing redstone ore
	setBlockImage(75, 0, t.REDSTONE_TORCH_OFF); // redstone torch off // TODO
	setBlockImage(76, 0, t.REDSTONE_TORCH_OFF); // redstone torch on // TODO
	// id 77 // stone button
	setBlockImage(78, 0, t.SNOW); // snow
	setBlockImage(79, 0, t.ICE); // ice
	setBlockImage(80, 0, t.SNOW); // snow block
	setBlockImage(81, 0, t.CACTUS_TOP); // cactus
	setBlockImage(82, 0, t.CLAY); // clay block
	setBlockImage(83, 0, t.REEDS); // sugar cane
	setBlockImage(84, 0, t.JUKEBOX_TOP.rotate(1)); // jukebox
	// id 85 // oak fence
	setBlockImage(86, 0, t.PUMPKIN_TOP); // pumpkin
	setBlockImage(87, 0, t.NETHERRACK); // netherrack
	setBlockImage(88, 0, t.SOUL_SAND); // soul sand
	setBlockImage(89, 0, t.GLOWSTONE); // glowstone block
	setBlockImage(90, 0, t.PORTAL); // nether portal block // TODO?
	setBlockImage(91, 0, t.PUMPKIN_TOP); // jack-o-lantern
	// id 92 // cake
	// redstone repeater off --
	setBlockImage(93, 0, t.REPEATER_OFF.rotate(ROTATE_270));
	setBlockImage(93, 1, t.REPEATER_OFF);
	setBlockImage(93, 2, t.REPEATER_OFF.rotate(ROTATE_90));
	setBlockImage(93, 3, t.REPEATER_OFF.rotate(ROTATE_180));
	// --
	// redstone repeater off --
	setBlockImage(94, 0, t.REPEATER_ON.rotate(ROTATE_270));
	setBlockImage(94, 1, t.REPEATER_ON);
	setBlockImage(94, 2, t.REPEATER_ON.rotate(ROTATE_90));
	setBlockImage(94, 3, t.REPEATER_ON.rotate(ROTATE_180));
	// --
	// stained glass --
	setBlockImage(95, 0, t.GLASS_WHITE);
	setBlockImage(95, 1, t.GLASS_ORANGE);
	setBlockImage(95, 2, t.GLASS_MAGENTA);
	setBlockImage(95, 3, t.GLASS_LIGHT_BLUE);
	setBlockImage(95, 4, t.GLASS_YELLOW);
	setBlockImage(95, 5, t.GLASS_LIME);
	setBlockImage(95, 6, t.GLASS_PINK);
	setBlockImage(95, 7, t.GLASS_GRAY);
	setBlockImage(95, 8, t.GLASS_SILVER);
	setBlockImage(95, 9, t.GLASS_CYAN);
	setBlockImage(95, 10, t.GLASS_PURPLE);
	setBlockImage(95, 11, t.GLASS_BLUE);
	setBlockImage(95, 12, t.GLASS_BROWN);
	setBlockImage(95, 13, t.GLASS_GREEN);
	setBlockImage(95, 14, t.GLASS_RED);
	setBlockImage(95, 15, t.GLASS_BLACK);
	// --
	// id 96 // trapdoor
	// -- monster egg
	setBlockImage(97, 0, t.STONE); // stone
	setBlockImage(97, 1, t.COBBLESTONE); // cobblestone
	setBlockImage(97, 2, t.STONEBRICK); // stone brick
	// --
	// -- stone bricks
	setBlockImage(98, 0, t.STONEBRICK); // normal
	setBlockImage(98, 1, t.STONEBRICK_MOSSY); // mossy
	setBlockImage(98, 2, t.STONEBRICK_CRACKED); // cracked
	setBlockImage(98, 3, t.STONEBRICK_CARVED); // chiseled
	// --
	// id 99 // huge brown mushroom
	// id 100 // huge red mushroom
	// id 101 // iron bars
	// id 102 // glas pane
	setBlockImage(103, 0, t.MELON_TOP); // melon
	// id 104 // pumpkin stem
	// id 105 // melon stem
	// id 106 // vines
	// id 107 // oak fence gate
	// id 108 // brick stairs
	// id 109 // stone brick stairs
	setBlockImage(110, 0, t.MYCELIUM_TOP); // mycelium
	setBlockImage(111, 0, t.WATERLILY); // lily pad
	setBlockImage(112, 0, t.NETHER_BRICK); // nether brick
	// id 113 // nether brick fence
	// id 114 // nether brick stairs
	// -- nether wart
	setBlockImage(115, 0, t.NETHER_WART_STAGE_0);
	setBlockImage(115, 1, t.NETHER_WART_STAGE_1);
	setBlockImage(115, 2, t.NETHER_WART_STAGE_1);
	setBlockImage(115, 3, t.NETHER_WART_STAGE_2);
	// --
	setBlockImage(116, 0, t.ENCHANTING_TABLE_TOP); // enchantment table
	// id 117 // brewing stand
	// id 118 // cauldron
	setBlockImage(119, 0, resources.getEndportalTexture()); // end portal
	setBlockImage(120, 0, t.ENDFRAME_TOP); // end portal frame
	setBlockImage(121, 0, t.END_STONE); // end stone
	// id 122 // dragon egg
	setBlockImage(123, 0, t.REDSTONE_LAMP_OFF); // redstone lamp inactive
	setBlockImage(124, 0, t.REDSTONE_LAMP_OFF); // redstone lamp active
	// id 125 // double wooden slabs
	// id 126 // normal wooden slabs
	// id 127 // cocoas
	// id 128 // sanstone stairs
	setBlockImage(129, 0, t.EMERALD_ORE);
	// id 130 // ender chest
	// id 131 // tripwire hook
	// id 132 // tripwire
	setBlockImage(133, 0, t.EMERALD_BLOCK); // block of emerald
	// id 134 // spruce wood stairs
	// id 135 // birch wood stairs
	// id 136 // jungle wood stairs
	setBlockImage(137, 0, t.COMMAND_BLOCK);
	// id 138 // beacon
	// id 139 // cobblestone wall
	// id 140 // flower pot
	// carrots --
	setBlockImage(141, 0, t.CARROTS_STAGE_0);
	setBlockImage(141, 1, t.CARROTS_STAGE_0);
	setBlockImage(141, 2, t.CARROTS_STAGE_1);
	setBlockImage(141, 3, t.CARROTS_STAGE_1);
	setBlockImage(141, 4, t.CARROTS_STAGE_2);
	setBlockImage(141, 5, t.CARROTS_STAGE_2);
	setBlockImage(141, 6, t.CARROTS_STAGE_2);
	setBlockImage(141, 7, t.CARROTS_STAGE_3);
	// --
	// potatoes --
	setBlockImage(142, 0, t.POTATOES_STAGE_0);
	setBlockImage(142, 1, t.POTATOES_STAGE_0);
	setBlockImage(142, 2, t.POTATOES_STAGE_1);
	setBlockImage(142, 3, t.POTATOES_STAGE_1);
	setBlockImage(142, 4, t.POTATOES_STAGE_2);
	setBlockImage(142, 5, t.POTATOES_STAGE_2);
	setBlockImage(142, 6, t.POTATOES_STAGE_2);
	setBlockImage(142, 7, t.POTATOES_STAGE_3);
	// --
	// id 143 // wooden button
	// id 144 // head
	// id 145 // anvil
	// id 146 // trapped chest
	setBlockImage(147, 0, t.GOLD_BLOCK); // weighted pressure plate (light) // TODO
	setBlockImage(148, 0, t.GOLD_BLOCK); // weighted pressure plate (heavy) // TODO
	// redstone comparator (inactive) --
	setBlockImage(149, 0, t.COMPARATOR_OFF.rotate(ROTATE_270));
	setBlockImage(149, 1, t.COMPARATOR_OFF);
	setBlockImage(149, 2, t.COMPARATOR_OFF.rotate(ROTATE_90));
	setBlockImage(149, 3, t.COMPARATOR_OFF.rotate(ROTATE_180));
	// --
	// redstone comparator (active)
	setBlockImage(150, 0, t.COMPARATOR_ON.rotate(ROTATE_270));
	setBlockImage(150, 1, t.COMPARATOR_ON);
	setBlockImage(150, 2, t.COMPARATOR_ON.rotate(ROTATE_90));
	setBlockImage(150, 3, t.COMPARATOR_ON.rotate(ROTATE_180));
	// --
	setBlockImage(151, 0, t.DAYLIGHT_DETECTOR_TOP); // daylight sensor
	setBlockImage(152, 0, t.REDSTONE_BLOCK); // block of redstone
	setBlockImage(153, 0, t.QUARTZ_ORE); // quartz ore
	// id 154 // hopper
	// block of quartz --
	setBlockImage(155, 0, t.QUARTZ_BLOCK_TOP);
	setBlockImage(155, 1, t.QUARTZ_BLOCK_CHISELED_TOP);
	setBlockImage(155, 2, t.QUARTZ_BLOCK_LINES_TOP);
	setBlockImage(155, 3, t.QUARTZ_BLOCK_LINES);
	setBlockImage(155, 4, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90));
	// --
	// id 156 // quartz stairs
	// id 157 // activator rail
	// id 158 // dropper
	// stained clay --
	setBlockImage(159, 0, t.HARDENED_CLAY_STAINED_WHITE);
	setBlockImage(159, 1, t.HARDENED_CLAY_STAINED_ORANGE);
	setBlockImage(159, 2, t.HARDENED_CLAY_STAINED_MAGENTA);
	setBlockImage(159, 3, t.HARDENED_CLAY_STAINED_LIGHT_BLUE);
	setBlockImage(159, 4, t.HARDENED_CLAY_STAINED_YELLOW);
	setBlockImage(159, 5, t.HARDENED_CLAY_STAINED_LIME);
	setBlockImage(159, 6, t.HARDENED_CLAY_STAINED_PINK);
	setBlockImage(159, 7, t.HARDENED_CLAY_STAINED_GRAY);
	setBlockImage(159, 8, t.HARDENED_CLAY_STAINED_SILVER);
	setBlockImage(159, 9, t.HARDENED_CLAY_STAINED_CYAN);
	setBlockImage(159, 10, t.HARDENED_CLAY_STAINED_PURPLE);
	setBlockImage(159, 11, t.HARDENED_CLAY_STAINED_BLUE);
	setBlockImage(159, 12, t.HARDENED_CLAY_STAINED_BROWN);
	setBlockImage(159, 13, t.HARDENED_CLAY_STAINED_GREEN);
	setBlockImage(159, 14, t.HARDENED_CLAY_STAINED_RED);
	setBlockImage(159, 15, t.HARDENED_CLAY_STAINED_BLACK);
	// --
	// id 160 // stained glass pane
	setBlockImage(161, 0, t.LEAVES_ACACIA); // acacia leaves
	setBlockImage(161, 1, t.LEAVES_BIG_OAK); // dark oak leaves
	// some more wood ---
	setBlockImage(162, 0, t.LOG_ACACIA_TOP); // acacia
	setBlockImage(162, 1, t.LOG_BIG_OAK_TOP); // acacia (placeholder)
	setBlockImage(162, 2, t.LOG_ACACIA_TOP); // dark wood
	setBlockImage(162, 3, t.LOG_BIG_OAK_TOP); // dark wood (placeholder)
	// --
	// id 163 // acacia wood stairs
	// id 164 // dark oak wood stairs
	setBlockImage(165, 0, t.SLIME); // slime block
	setBlockImage(166, 0, empty_texture); // barrier
	// id 167 // iron trapdoor
	// prismarine --
	setBlockImage(168, 0, t.PRISMARINE_ROUGH);
	setBlockImage(168, 1, t.PRISMARINE_BRICKS);
	setBlockImage(168, 2, t.PRISMARINE_DARK);
	// --
	setBlockImage(169, 0, t.SEA_LANTERN); // sea lantern
	// hay block --
	setBlockImage(170, 0, t.HAY_BLOCK_TOP); // normal orientation
	setBlockImage(170, 4, t.HAY_BLOCK_SIDE); // east-west
	setBlockImage(170, 8, t.HAY_BLOCK_SIDE.rotate(1)); // north-south
	// --
	// carpet --
	setBlockImage(171, 0, t.WOOL_COLORED_WHITE);
	setBlockImage(171, 1, t.WOOL_COLORED_ORANGE);
	setBlockImage(171, 2, t.WOOL_COLORED_MAGENTA);
	setBlockImage(171, 3, t.WOOL_COLORED_LIGHT_BLUE);
	setBlockImage(171, 4, t.WOOL_COLORED_YELLOW);
	setBlockImage(171, 5, t.WOOL_COLORED_LIME);
	setBlockImage(171, 6, t.WOOL_COLORED_PINK);
	setBlockImage(171, 7, t.WOOL_COLORED_GRAY);
	setBlockImage(171, 8, t.WOOL_COLORED_SILVER);
	setBlockImage(171, 9, t.WOOL_COLORED_CYAN);
	setBlockImage(171, 10, t.WOOL_COLORED_PURPLE);
	setBlockImage(171, 11, t.WOOL_COLORED_BLUE);
	setBlockImage(171, 12, t.WOOL_COLORED_BROWN);
	setBlockImage(171, 13, t.WOOL_COLORED_GREEN);
	setBlockImage(171, 14, t.WOOL_COLORED_RED);
	setBlockImage(171, 15, t.WOOL_COLORED_BLACK);
	// --
	setBlockImage(172, 0, t.HARDENED_CLAY); // hardened clay
	setBlockImage(173, 0, t.COAL_BLOCK); // block of coal
	setBlockImage(174, 0, t.ICE_PACKED); // packed ice
	// id 175 // large plants
	// id 176 // standing banner
	// id 177 // wall banner
	setBlockImage(178, 0, t.DAYLIGHT_DETECTOR_INVERTED_TOP); // inverted daylight sensor
	// -- red sandstone
	setBlockImage(179, 0, t.RED_SANDSTONE_TOP); // normal
	setBlockImage(179, 1, t.RED_SANDSTONE_TOP); // chiseled
	setBlockImage(179, 2, t.RED_SANDSTONE_TOP); // smooth
	// --
	// id 180 // red sandstone stairs
	// id 181 // double red sandstone slabs
	// id 182 // normal red sandstone slabs
	// id 183 // spruce fence gate
	// id 184 // birch fence gate
	// id 185 // jungle fence gate
	// id 186 // dark oak fence gate
	// id 186 // acacia fence gate
	// id 188 // spruce fence
	// id 189 // birch fence
	// id 190 // jungle fence
	// id 191 // dark oak fence
	// id 192 // acacia fence
	// id 193 // spruce door
	// id 194 // birch door
	// id 195 // jungle door
	// id 196 // acacia door
	// id 197 // dark oak door
}

int TopdownBlockImages::createOpaqueWater() {
	// just use the Ocean biome watercolor
	RGBAImage water = resources.getBlockTextures().WATER_STILL.colorize(0, 0.39, 0.89);
	RGBAImage opaque_water = water;

	int water_preblit;
	for (water_preblit = 2; water_preblit < 10; water_preblit++) {
		opaque_water.alphaBlit(water, 0, 0);

		// then check alpha
		uint8_t min_alpha = 255;
		for (int x = 0; x < opaque_water.getWidth(); x++) {
			for (int y = 0; y < opaque_water.getHeight(); y++) {
				uint8_t alpha = rgba_alpha(opaque_water.getPixel(x, y));
				if (alpha < min_alpha)
					min_alpha = alpha;
			}
		}

		// images are "enough" opaque
		if (min_alpha == 255)
			break;
	}

	uint16_t id = 8;
	uint16_t data = OPAQUE_WATER;
	block_images[id | (data) << 16] = opaque_water;
	block_images[id | (data | OPAQUE_WATER_SOUTH) << 16] = opaque_water;
	block_images[id | (data | OPAQUE_WATER_WEST) << 16] = opaque_water;
	block_images[id | (data | OPAQUE_WATER_SOUTH | OPAQUE_WATER_WEST) << 16] = opaque_water;

	return water_preblit;
}

} /* namespace renderer */
} /* namespace mapcrafter */
