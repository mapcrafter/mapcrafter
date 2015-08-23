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

void TopdownBlockImages::createItemStyleBlock(uint16_t id, uint16_t data,
		const RGBAImage& texture) {
	// call parent method because we don't want to rotate the texture
	AbstractBlockImages::setBlockImage(id, data, texture);
}

void TopdownBlockImages::createRotatedBlock(uint16_t id, uint16_t extra_data,
		const RGBAImage& texture) {
	setBlockImage(id, 2 | extra_data, texture);
	setBlockImage(id, 3 | extra_data, texture.rotate(2));
	setBlockImage(id, 4 | extra_data, texture.rotate(3));
	setBlockImage(id, 5 | extra_data, texture.rotate(1));
}

void TopdownBlockImages::createWood(uint16_t id, uint16_t extra_data,
		const RGBAImage& side, const RGBAImage& top) { // id 17, 162
	setBlockImage(id, extra_data | 4, side.rotate(1));
	setBlockImage(id, extra_data | 8, side);
	setBlockImage(id, extra_data, top.rotate(1)); // old format
	setBlockImage(id, extra_data | 4 | 8, top.rotate(1));
}

void TopdownBlockImages::createDispenserDropper(uint16_t id, const RGBAImage& front) { // id 23, 158
	RGBAImage side = resources.getBlockTextures().FURNACE_SIDE;
	RGBAImage top = resources.getBlockTextures().FURNACE_TOP;

	createRotatedBlock(id, 0, top);
	setBlockImage(id, 0, top);
	setBlockImage(id, 1, front);
}

void TopdownBlockImages::createBed() { // id 26
	RGBAImage top = resources.getBlockTextures().BED_FEET_TOP;
	setBlockImage(26, 0, top.rotate(2));
	setBlockImage(26, 1, top.rotate(3));
	setBlockImage(26, 2, top);
	setBlockImage(26, 3, top.rotate(1));

	top = resources.getBlockTextures().BED_HEAD_TOP;
	setBlockImage(26, 8, top.rotate(1));
	setBlockImage(26, 1 | 8, top.rotate(2));
	setBlockImage(26, 2 | 8, top.rotate(3));
	setBlockImage(26, 3 | 8, top);
}

void TopdownBlockImages::createStraightRails(uint16_t id, uint16_t extra_data,
		const RGBAImage& texture) { // id 27, 28
	RGBAImage north_south = texture;
	RGBAImage east_west = texture.rotate(1);
	setBlockImage(id, 0 | extra_data, north_south);
	setBlockImage(id, 1 | extra_data, east_west);
	setBlockImage(id, 2 | extra_data, east_west);
	setBlockImage(id, 3 | extra_data, east_west);
	setBlockImage(id, 4 | extra_data, north_south);
	setBlockImage(id, 5 | extra_data, north_south);
}

void TopdownBlockImages::createTorch(uint16_t id, const RGBAImage& texture) { // id 50, 75, 76
	// TODO also display the torches on walls?
	createItemStyleBlock(id, 5, texture);
}

void TopdownBlockImages::createChest(uint16_t id, const ChestTextures& textures) { // 54, 130
	RGBAImage top = textures[ChestTextures::TOP];
	setBlockImage(id, DATA_NORTH, top.rotate(3));
	setBlockImage(id, DATA_SOUTH, top.rotate(1));
	setBlockImage(id, DATA_EAST, top);
	setBlockImage(id, DATA_WEST, top.rotate(2));
}

void TopdownBlockImages::createDoubleChest(uint16_t id, const DoubleChestTextures& textures) { // 54
	int l = LARGECHEST_DATA_LARGE;
	setBlockImage(id, DATA_NORTH | l | LARGECHEST_DATA_LEFT, textures[DoubleChestTextures::TOP_LEFT].rotate(2));
	setBlockImage(id, DATA_SOUTH | l | LARGECHEST_DATA_LEFT, textures[DoubleChestTextures::TOP_LEFT]);
	setBlockImage(id, DATA_EAST | l | LARGECHEST_DATA_LEFT, textures[DoubleChestTextures::TOP_LEFT].rotate(3));
	setBlockImage(id, DATA_WEST | l | LARGECHEST_DATA_LEFT, textures[DoubleChestTextures::TOP_LEFT].rotate(1));

	setBlockImage(id, DATA_NORTH | l, textures[DoubleChestTextures::TOP_RIGHT].rotate(2));
	setBlockImage(id, DATA_SOUTH | l, textures[DoubleChestTextures::TOP_RIGHT]);
	setBlockImage(id, DATA_EAST | l, textures[DoubleChestTextures::TOP_RIGHT].rotate(3));
	setBlockImage(id, DATA_WEST | l, textures[DoubleChestTextures::TOP_RIGHT].rotate(1));
}

void TopdownBlockImages::createRedstoneWire(uint16_t id, uint16_t extra_data, uint8_t r,
		uint8_t g, uint8_t b) { // id 55
	RGBAImage redstone_cross = resources.getBlockTextures().REDSTONE_DUST_CROSS;
	RGBAImage redstone_line = resources.getBlockTextures().REDSTONE_DUST_LINE;

	//uint8_t color = powered ? 50 : 255;
	redstone_cross = redstone_cross.colorize(r, g, b);
	redstone_line = redstone_line.colorize(r, g, b);

	// 1/16 of the texture size
	double s = (double) texture_size / 16;

	// go through all redstone combinations
	for (uint16_t i = 0; i <= 0xff; i++) {
		// the redstone extra data starts at the fifth byte
		// so shift the data to the right
		uint16_t data = i << 4;

		// ignore neighbors a block higher
		if ((data & REDSTONE_TOPNORTH) || (data & REDSTONE_TOPSOUTH)
				|| (data & REDSTONE_TOPEAST) || (data & REDSTONE_TOPWEST))
			continue;

		RGBAImage texture = redstone_cross;
		// remove the connections from the cross image
		// if there is no connection
		if (!(data & REDSTONE_NORTH))
			texture.fill(rgba(0, 0, 0, 0), 0, 0, s*16, s*4);
		if (!(data & REDSTONE_SOUTH))
			texture.fill(rgba(0, 0, 0, 0), 0, s*12, s*16, s*4);

		if (!(data & REDSTONE_EAST))
			texture.fill(rgba(0, 0, 0, 0), s*12, 0, s*4, s*16);
		if (!(data & REDSTONE_WEST))
			texture.fill(rgba(0, 0, 0, 0), 0, 0, s*4, s*16);

		// check if we have a line of restone
		if (data == (REDSTONE_NORTH | REDSTONE_SOUTH))
			texture = redstone_line.rotate(ROTATE_90);
		else if (data == (REDSTONE_EAST | REDSTONE_WEST))
			texture = redstone_line;

		// we can add the block like this without rotation
		// because we calculate the neighbors on our own,
		// it does not depend on the rotation of the map
		AbstractBlockImages::setBlockImage(id, data | extra_data, texture);
	}
}

void TopdownBlockImages::createRails() { // id 66
	const BlockTextures& t = resources.getBlockTextures();
	RGBAImage straight = t.RAIL_NORMAL, curved = t.RAIL_NORMAL_TURNED;

	createStraightRails(66, 0, straight);
	setBlockImage(66, 6, curved.flip(false, false)); // south-east
	setBlockImage(66, 7, curved.flip(true, false)); // south-west
	setBlockImage(66, 8, curved.flip(true, true)); // north-west
	setBlockImage(66, 9, curved.flip(false, true)); // north-east
}

namespace {

RGBAImage buildFenceLike(const RGBAImage& texture, bool north, bool south, bool east, bool west,
		int post_factor = 6, int conn_factor = 2) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;
	int post = std::max(ratio * post_factor, 2.0);
	int post_start = (size - post) / 2;
	int conn = std::max(ratio * conn_factor, 2.0);
	int conn_start = (size - conn) / 2;

	RGBAPixel invisible = rgba(0, 0, 0, 0);
	RGBAPixel visible = rgba(0, 0, 0, 255);
	RGBAImage mask(size, size);
	mask.fill(invisible, 0, 0, size, size);
	mask.fill(visible, post_start, post_start, post, post);

	if (north)
		mask.fill(visible, conn_start, 0, conn, size / 2);
	if (south)
		mask.fill(visible, conn_start, size / 2, conn, size / 2 + 1);
	if (east)
		mask.fill(visible, size / 2, conn_start, size / 2 + 1, conn);
	if (west)
		mask.fill(visible, 0, conn_start, size / 2, conn);

	RGBAImage fence = texture;
	for (int x = 0; x < size; x++)
		for (int y = 0; y < size; y++)
			if (mask.pixel(x, y) == invisible)
				fence.pixel(x, y) = invisible;
	return fence;
}

}

void TopdownBlockImages::createFence(uint16_t id, uint16_t extra_data, const RGBAImage& texture) { // id 85, 113, 188-192
	for (uint8_t i = 0; i < 16; i++) {
		uint16_t data = i << 4;
		// special data set by the tile renderer
		bool north = data & DATA_NORTH;
		bool south = data & DATA_SOUTH;
		bool east = data & DATA_EAST;
		bool west = data & DATA_WEST;

		RGBAImage fence = buildFenceLike(texture, north, south, east, west);
		AbstractBlockImages::setBlockImage(id, data | extra_data, fence);
	}
}

void TopdownBlockImages::createCake() {
	RGBAImage texture = resources.getBlockTextures().CAKE_TOP;

	for (int data = 0; data <= 6; data++) {
		int eaten = (double) data / 7 * texture_size;
		RGBAImage cake = texture;
		cake.fill(0, 0, 0, eaten, texture_size);
		setBlockImage(92, data, cake);
	}
}

void TopdownBlockImages::createHugeMushroom(uint16_t id, const RGBAImage& cap) { // id 99, 100
	RGBAImage pores = resources.getBlockTextures().MUSHROOM_BLOCK_INSIDE;
	RGBAImage stem = resources.getBlockTextures().MUSHROOM_BLOCK_SKIN_STEM;

	setBlockImage(id, 0, pores);
	setBlockImage(id, 1, cap);
	setBlockImage(id, 2, cap);
	setBlockImage(id, 3, cap);
	setBlockImage(id, 4, cap);
	setBlockImage(id, 5, cap);
	setBlockImage(id, 6, cap);
	setBlockImage(id, 7, cap);
	setBlockImage(id, 8, cap);
	setBlockImage(id, 9, cap);
	setBlockImage(id, 10, pores);
	setBlockImage(id, 14, cap);
	setBlockImage(id, 15, stem);
}

void TopdownBlockImages::createVines() { // id 106
	RGBAImage texture = resources.getBlockTextures().VINE;
	setBlockImage(106, 0, texture);
	setBlockImage(106, 42, empty_texture);
}

void TopdownBlockImages::createFenceGate(uint8_t id, RGBAImage texture) { // id 107, 183-187
	// go through states opened and closed
	for (int open = 0; open <= 1; open++) {
		// north/south, east/west block images are same
		// (because we ignore the direction of opened fence gates)
		RGBAImage north = buildFenceLike(texture, false, false, true, true, 2, 2);
		RGBAImage east = buildFenceLike(texture, true, true, false, false, 2, 2);
		// create the opening-part if it's open
		if (open) {
			double ratio = (double) texture_size / 16;
			north.fill(0, ratio * 4, ratio * 4, ratio * 8, ratio * 8);
			east.fill(0, ratio * 4, ratio * 4, ratio * 8, ratio * 8);
		}

		uint8_t extra = open ? 4 : 0;
		if (rotation == 0 || rotation == 2) {
			setBlockImage(id, 0 | extra, north);
			setBlockImage(id, 1 | extra, east);
			setBlockImage(id, 2 | extra, north);
			setBlockImage(id, 3 | extra, east);
		} else {
			setBlockImage(id, 0 | extra, east);
			setBlockImage(id, 1 | extra, north);
			setBlockImage(id, 2 | extra, east);
			setBlockImage(id, 3 | extra, north);
		}
	}
}

void TopdownBlockImages::createCocoas() { // id 127
	const BlockTextures& t = resources.getBlockTextures();
	double ratio = (double) texture_size / 16;

	for (int stage = 0; stage <= 2; stage++) {
		RGBAImage cocoa;
		if (stage == 0)
			cocoa = t.COCOA_STAGE_0.clip(0, 0, ratio*4, ratio*4);
		else if (stage == 1)
			cocoa = t.COCOA_STAGE_1.clip(0, 0, ratio*6, ratio*6);
		else
			cocoa = t.COCOA_STAGE_2.clip(0, 0, ratio*7, ratio*7);
		int cocoa_size = cocoa.getWidth();

		for (int dir = 0; dir <= 3; dir++) {
			bool north = (dir == 2), south = (dir == 0), east = (dir == 3), west = (dir == 1);
			int x = 0;
			int y = 0;
			if (north || south)
				x = (texture_size - cocoa_size) / 2;
			if (east || west)
				y = (texture_size - cocoa_size) / 2;
			if (south)
				y = texture_size - cocoa_size;
			if (east)
				x = texture_size - cocoa_size;

			RGBAImage block(texture_size, texture_size);
			block.alphaBlit(cocoa, x, y);
			setBlockImage(127, dir | (stage << 2), block);
		}
	}
}

void TopdownBlockImages::createTripwireHook() { // id 131
	RGBAImage tripwire = resources.getBlockTextures().REDSTONE_DUST_LINE.colorize((uint8_t) 192, 192, 192);

	// TODO also render that part on the wall?
	setBlockImage(131, 0, tripwire.rotate(1)); // trip wire hook on the north side
	setBlockImage(131, 1, tripwire.rotate(2)); // on the east side
	setBlockImage(131, 2, tripwire.rotate(3)); // on the south side
	setBlockImage(131, 3, tripwire); // on the west side
}

void TopdownBlockImages::createLargePlant(uint16_t data, const RGBAImage& texture,
		const RGBAImage& top_texture) { // id 175
	createItemStyleBlock(175, data, texture);
	createItemStyleBlock(175, data | LARGEPLANT_TOP, top_texture);
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
			//|| id == 50 || id == 75 || id == 76 // torch // TODO
			|| id == 51 // fire
			|| id == 78 // snow
			|| id == 79 // ice
			|| id == 84 // jukebox
			|| id == 90 // nether portal
			|| id == 117 // brewing stand
			|| id == 151 || id == 178 // lighting sensor
			|| id == 154 // hopper
			|| id == 174 // packed ice
			)
		return 0;

	if (id == 8 || id == 9) // water
		return data & OPAQUE_WATER; // we just need that one bit
	if (id == 18 || id == 161) // leaves
		return data & (0xff00 | 0b00000011);
	else if (id == 26) // bed
		return data & (0xff00 | util::binary<1011>::value);
	else if (id == 43 || id == 44 || id == 125 || id == 126 || id == 181 || id == 182)
		return data & ~8;
	else if (id == 55)
		// unset unnecessary neighbor information for this render view
		return data & ~(REDSTONE_TOPNORTH | REDSTONE_TOPSOUTH | REDSTONE_TOPEAST | REDSTONE_TOPWEST);
	else if (id == 60) // farmland
		return data & 0xff00;
	else if (id == 93 || id == 94) // redstone repeater
		return data & (0xff00 | util::binary<11>::value);
	else if (id == 81 || id == 83) // cactus, sugar cane
		return data & 0xff00;
	else if (id == 106) { // vines
		// data must be 0 or vine won't be visible
		if ((data & 0xff) != 0)
			return (data & ~0xff) | 42;
	} else if (id == 119 || id == 120) // end portal, end portal frame
		return data & 0xff00;
	else if (id == 131) // trip wire hook
		return data & util::binary<11>::value;
	else if (id == 132) // trip wire
		return data & ~0xf;
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
	// LOG(INFO) << id << " " << data << " " << rotation;
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
	RGBAImage water = t.WATER_STILL.colorize(0, 0.39, 0.89);

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
	createItemStyleBlock(6, 0, t.SAPLING_OAK); // oak
	createItemStyleBlock(6, 1, t.SAPLING_SPRUCE); // spruce
	createItemStyleBlock(6, 2, t.SAPLING_BIRCH); // birch
	createItemStyleBlock(6, 3, t.SAPLING_JUNGLE); // jungle
	createItemStyleBlock(6, 4, t.SAPLING_ACACIA); // acacia
	createItemStyleBlock(6, 5, t.SAPLING_ROOFED_OAK); // dark oak
	// --
	setBlockImage(7, 0, t.BEDROCK); // bedrock
	setBlockImage(8, 0, water);
	setBlockImage(9, 0, water);
	setBlockImage(10, 0, t.LAVA_STILL);
	setBlockImage(11, 0, t.LAVA_STILL);
	setBlockImage(12, 0, t.SAND); // sand
	setBlockImage(12, 1, t.RED_SAND); // red sand
	setBlockImage(13, 0, t.GRAVEL); // gravel
	setBlockImage(14, 0, t.GOLD_ORE); // gold ore
	setBlockImage(15, 0, t.IRON_ORE); // iron ore
	setBlockImage(16, 0, t.COAL_ORE); // coal ore
	// -- wood
	createWood(17, 0, t.LOG_OAK, t.LOG_OAK_TOP); // oak
	createWood(17, 1, t.LOG_SPRUCE, t.LOG_SPRUCE_TOP); // pine/spruce
	createWood(17, 2, t.LOG_BIRCH, t.LOG_BIRCH_TOP); // birch
	createWood(17, 3, t.LOG_JUNGLE, t.LOG_JUNGLE_TOP); // jungle
	// --
	setBlockImage(18, 0, t.LEAVES_OAK); // oak
	setBlockImage(18, 1, t.LEAVES_SPRUCE); // pine/spruce
	setBlockImage(18, 2, t.LEAVES_OAK); // birch
	setBlockImage(18, 3, t.LEAVES_JUNGLE); // jungle
	setBlockImage(19, 0, t.SPONGE); // sponge
	setBlockImage(19, 1, t.SPONGE_WET); // wet sponge
	setBlockImage(20, 0, t.GLASS);
	setBlockImage(21, 0, t.LAPIS_ORE); // lapis lazuli ore
	setBlockImage(22, 0, t.LAPIS_BLOCK); // lapis lazuli block
	createDispenserDropper(23, t.DISPENSER_FRONT_HORIZONTAL); // dispenser
	setBlockImage(24, 0, t.SANDSTONE_TOP); // sandstone
	setBlockImage(25, 0, t.NOTEBLOCK); // noteblock
	createBed(); // id 26 // bed
	createStraightRails(27, 0, t.RAIL_GOLDEN); // powered rail (unpowered)
	createStraightRails(27, 8, t.RAIL_GOLDEN_POWERED); // powered rail (powered);
	createStraightRails(28, 0, t.RAIL_ACTIVATOR); // detector rail
	// id 29 // sticky piston
	createItemStyleBlock(30, 0, t.WEB); // cobweb
	// -- tall grass
	createItemStyleBlock(31, 0, t.DEADBUSH); // dead bush style
	createItemStyleBlock(31, 1, t.TALLGRASS); // tall grass
	createItemStyleBlock(31, 2, t.FERN); // fern
	// --
	createItemStyleBlock(32, 0, t.DEADBUSH); // dead bush
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
	createItemStyleBlock(37, 0, t.FLOWER_DANDELION); // dandelion
	// -- poppy -- different flowers
	createItemStyleBlock(38, 0, t.FLOWER_ROSE); // poppy
	createItemStyleBlock(38, 1, t.FLOWER_BLUE_ORCHID); // blue orchid
	createItemStyleBlock(38, 2, t.FLOWER_ALLIUM); // azure bluet
	createItemStyleBlock(38, 3, t.FLOWER_HOUSTONIA); //
	createItemStyleBlock(38, 4, t.FLOWER_TULIP_RED); // red tulip
	createItemStyleBlock(38, 5, t.FLOWER_TULIP_ORANGE); // orange tulip
	createItemStyleBlock(38, 6, t.FLOWER_TULIP_WHITE); // white tulip
	createItemStyleBlock(38, 7, t.FLOWER_TULIP_PINK); // pink tulip
	createItemStyleBlock(38, 8, t.FLOWER_OXEYE_DAISY); // oxeye daisy
	// --
	setBlockImage(39, 0, t.MUSHROOM_BROWN); // brown mushroom
	setBlockImage(40, 0, t.MUSHROOM_RED); // red mushroom
	setBlockImage(41, 0, t.GOLD_BLOCK); // block of gold
	setBlockImage(42, 0, t.IRON_BLOCK); // block of iron
	// double stone slabs --
	setBlockImage(43, 0, t.STONE_SLAB_TOP);
	setBlockImage(43, 1, t.SANDSTONE_TOP);
	setBlockImage(43, 2, t.PLANKS_OAK);
	setBlockImage(43, 3, t.COBBLESTONE);
	setBlockImage(43, 4, t.BRICK);
	setBlockImage(43, 5, t.STONEBRICK);
	setBlockImage(43, 6, t.NETHER_BRICK);
	setBlockImage(43, 7, t.QUARTZ_BLOCK_SIDE);
	// --
	// normal stone slabs --
	setBlockImage(44, 0, t.STONE_SLAB_TOP);
	setBlockImage(44, 1, t.SANDSTONE_TOP);
	setBlockImage(44, 2, t.PLANKS_OAK);
	setBlockImage(44, 3, t.COBBLESTONE);
	setBlockImage(44, 4, t.BRICK);
	setBlockImage(44, 5, t.STONEBRICK);
	setBlockImage(44, 6, t.NETHER_BRICK);
	setBlockImage(44, 7, t.QUARTZ_BLOCK_SIDE);
	// --
	setBlockImage(45, 0, t.BRICK); // bricks
	setBlockImage(46, 0, t.TNT_TOP); // tnt
	setBlockImage(47, 0, t.PLANKS_OAK); // bookshelf
	setBlockImage(48, 0, t.COBBLESTONE_MOSSY); // moss stone
	setBlockImage(49, 0, t.OBSIDIAN); // obsidian
	createTorch(50, t.TORCH_ON); // torch
	createItemStyleBlock(51, 0, t.FIRE_LAYER_0); // fire
	setBlockImage(52, 0, t.MOB_SPAWNER); // monster spawner
	// id 53 // oak wood stairs
	createChest(54, resources.getNormalChest()); // chest
	createDoubleChest(54, resources.getNormalDoubleChest()); // chest
	createRedstoneWire(55, 0, 48, 0, 0); // redstone wire not powered
	createRedstoneWire(55, REDSTONE_POWERED, 192, 0, 0); // redstone wire powered
	setBlockImage(56, 0, t.DIAMOND_ORE); // diamond ore
	setBlockImage(57, 0, t.DIAMOND_BLOCK); // block of diamond
	setBlockImage(58, 0, t.CRAFTING_TABLE_TOP); // crafting table
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
	setBlockImage(60, 0, t.FARMLAND_WET); // farmland
	createRotatedBlock(61, 0, t.FURNACE_TOP); // furnace
	createRotatedBlock(62, 0, t.FURNACE_TOP); // burning furnace
	// id 63 // sign post
	// id 64 // wooden door
	// id 65 // ladders
	createRails(); // id 66
	// id 67 // cobblestone stairs
	// id 68 // wall sign
	// id 69 // lever
	setBlockImage(70, 0, t.STONE); // stone pressure plate // TODO
	// id 71 // iron door
	setBlockImage(72, 0, t.PLANKS_OAK); // wooden pressure plate // TODO
	setBlockImage(73, 0, t.REDSTONE_ORE); // redstone ore
	setBlockImage(74, 0, t.REDSTONE_ORE); // glowing redstone ore
	createTorch(75, t.REDSTONE_TORCH_OFF); // redstone torch off
	createTorch(76, t.REDSTONE_TORCH_ON); // redstone torch on
	// id 77 // stone button
	setBlockImage(78, 0, t.SNOW); // snow
	setBlockImage(79, 0, t.ICE); // ice
	setBlockImage(80, 0, t.SNOW); // snow block
	setBlockImage(81, 0, t.CACTUS_TOP); // cactus
	setBlockImage(82, 0, t.CLAY); // clay block
	createItemStyleBlock(83, 0, t.REEDS); // sugar cane
	setBlockImage(84, 0, t.JUKEBOX_TOP.rotate(1)); // jukebox
	createFence(85, 0, t.PLANKS_OAK); // oak fence
	setBlockImage(86, 0, t.PUMPKIN_TOP); // pumpkin
	setBlockImage(87, 0, t.NETHERRACK); // netherrack
	setBlockImage(88, 0, t.SOUL_SAND); // soul sand
	setBlockImage(89, 0, t.GLOWSTONE); // glowstone block
	setBlockImage(90, 0, t.PORTAL); // nether portal block // TODO?
	setBlockImage(91, 0, t.PUMPKIN_TOP); // jack-o-lantern
	createCake(); // id 92 // cake
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
	createHugeMushroom(99, t.MUSHROOM_BLOCK_SKIN_BROWN); // huge brown mushroom
	createHugeMushroom(100, t.MUSHROOM_BLOCK_SKIN_RED); // huge red mushroom
	// id 101 // iron bars
	// id 102 // glas pane
	setBlockImage(103, 0, t.MELON_TOP); // melon
	// id 104 // pumpkin stem
	// id 105 // melon stem
	// id 106 // vines
	createVines(); // id 106
	createFenceGate(107, t.PLANKS_OAK); // oak fence gate
	// id 108 // brick stairs
	// id 109 // stone brick stairs
	setBlockImage(110, 0, t.MYCELIUM_TOP); // mycelium
	// -- lily pad
	setBlockImage(111, 0, t.WATERLILY);
	setBlockImage(111, 1, t.WATERLILY.rotate(3));
	setBlockImage(111, 2, t.WATERLILY.rotate(2));
	setBlockImage(111, 3, t.WATERLILY.rotate(1));
	// --
	setBlockImage(112, 0, t.NETHER_BRICK); // nether brick
	createFence(113, 0, t.NETHER_BRICK); // nether brick fence
	// id 114 // nether brick stairs
	// -- nether wart
	createItemStyleBlock(115, 0, t.NETHER_WART_STAGE_0);
	createItemStyleBlock(115, 1, t.NETHER_WART_STAGE_1);
	createItemStyleBlock(115, 2, t.NETHER_WART_STAGE_1);
	createItemStyleBlock(115, 3, t.NETHER_WART_STAGE_2);
	// --
	setBlockImage(116, 0, t.ENCHANTING_TABLE_TOP); // enchantment table
	// -- brewing stand
	RGBAImage brewing_stand = t.BREWING_STAND_BASE;
	brewing_stand.alphaBlit(t.BREWING_STAND, 0, 0);
	AbstractBlockImages::setBlockImage(117, 0, brewing_stand);
	// --
	// -- cauldron
	RGBAImage cauldron = t.CAULDRON_INNER, cauldron_water = cauldron;
	cauldron.alphaBlit(t.CAULDRON_TOP, 0, 0);
	cauldron_water.alphaBlit(water, 0, 0);
	cauldron_water.alphaBlit(t.CAULDRON_TOP, 0, 0);
	setBlockImage(118, 0, cauldron);
	setBlockImage(118, 1, cauldron_water);
	setBlockImage(118, 2, cauldron_water);
	setBlockImage(118, 3, cauldron_water);
	// --
	setBlockImage(119, 0, resources.getEndportalTexture()); // end portal
	setBlockImage(120, 0, t.ENDFRAME_TOP); // end portal frame
	setBlockImage(121, 0, t.END_STONE); // end stone
	setBlockImage(122, 0, t.DRAGON_EGG); // dragon egg
	createItemStyleBlock(123, 0, t.REDSTONE_LAMP_OFF); // redstone lamp inactive
	createItemStyleBlock(124, 0, t.REDSTONE_LAMP_ON); // redstone lamp active
	// // double wooden slabs
	setBlockImage(125, 0, t.PLANKS_OAK);
	setBlockImage(125, 1, t.PLANKS_SPRUCE);
	setBlockImage(125, 2, t.PLANKS_BIRCH);
	setBlockImage(125, 3, t.PLANKS_JUNGLE);
	setBlockImage(125, 4, t.PLANKS_ACACIA);
	setBlockImage(125, 5, t.PLANKS_BIG_OAK);
	// --
	// normal wooden slabs --
	setBlockImage(126, 0, t.PLANKS_OAK);
	setBlockImage(126, 1, t.PLANKS_SPRUCE);
	setBlockImage(126, 2, t.PLANKS_BIRCH);
	setBlockImage(126, 3, t.PLANKS_JUNGLE);
	setBlockImage(126, 4, t.PLANKS_ACACIA);
	setBlockImage(126, 5, t.PLANKS_BIG_OAK);
	// --
	createCocoas(); // id 127 // cocoas
	// id 128 // sanstone stairs
	setBlockImage(129, 0, t.EMERALD_ORE);
	createChest(130, resources.getEnderChest()); // ender chest
	createTripwireHook(); // id 131 // tripwire hook
	createRedstoneWire(132, 0, 192, 192, 192); // tripwire
	setBlockImage(133, 0, t.EMERALD_BLOCK); // block of emerald
	// id 134 // spruce wood stairs
	// id 135 // birch wood stairs
	// id 136 // jungle wood stairs
	setBlockImage(137, 0, t.COMMAND_BLOCK);
	// -- beacon
	RGBAImage beacon = t.OBSIDIAN, beacon_block;
	int beacon_size = texture_size / 16.0 * 10;
	if (beacon_size % 2)
		beacon_size--; // odd sizes suck
	t.BEACON.clip(1, 1, texture_size - 2, texture_size - 2).resize(beacon_block, beacon_size, beacon_size);
	beacon.alphaBlit(beacon_block, (texture_size - beacon_size) / 2, (texture_size - beacon_size) / 2);
	beacon.alphaBlit(t.GLASS, 0, 0);
	setBlockImage(138, 0, beacon);
	// --
	createFence(139, 0, t.COBBLESTONE); // cobblestone wall
	createFence(139, 1, t.COBBLESTONE_MOSSY); // cobblestone wall mossy
	// id 140 // flower pot
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
	// id 143 // wooden button
	// id 144 // head
	// -- anvil
	setBlockImage(145, 0, t.ANVIL_TOP_DAMAGED_0);
	setBlockImage(145, 1, t.ANVIL_TOP_DAMAGED_0.rotate(1));
	setBlockImage(145, 2, t.ANVIL_TOP_DAMAGED_0);
	setBlockImage(145, 3, t.ANVIL_TOP_DAMAGED_0.rotate(1));
	setBlockImage(145, 4, t.ANVIL_TOP_DAMAGED_1);
	setBlockImage(145, 5, t.ANVIL_TOP_DAMAGED_1.rotate(1));
	setBlockImage(145, 6, t.ANVIL_TOP_DAMAGED_1);
	setBlockImage(145, 7, t.ANVIL_TOP_DAMAGED_1.rotate(1));
	setBlockImage(145, 8, t.ANVIL_TOP_DAMAGED_2);
	setBlockImage(145, 9, t.ANVIL_TOP_DAMAGED_2.rotate(1));
	setBlockImage(145, 10, t.ANVIL_TOP_DAMAGED_2);
	setBlockImage(145, 11, t.ANVIL_TOP_DAMAGED_2.rotate(1));
	// --
	createChest(146, resources.getTrappedChest()); // trapped chest
	createDoubleChest(146, resources.getTrappedDoubleChest()); // double trapped chest
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
	// -- hopper
	RGBAImage hopper = t.HOPPER_INSIDE;
	hopper.alphaBlit(t.HOPPER_TOP, 0, 0);
	setBlockImage(154, 0, hopper);
	// --
	// block of quartz --
	setBlockImage(155, 0, t.QUARTZ_BLOCK_TOP);
	setBlockImage(155, 1, t.QUARTZ_BLOCK_CHISELED_TOP);
	setBlockImage(155, 2, t.QUARTZ_BLOCK_LINES_TOP);
	setBlockImage(155, 3, t.QUARTZ_BLOCK_LINES);
	setBlockImage(155, 4, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90));
	// --
	// id 156 // quartz stairs
	createStraightRails(157, 0, t.RAIL_ACTIVATOR); // activator rail
	createDispenserDropper(158, t.DROPPER_FRONT_HORIZONTAL); // dropper
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
	// some more wood --
	createWood(162, 0, t.LOG_ACACIA, t.LOG_ACACIA_TOP); // acacia
	createWood(162, 1, t.LOG_BIG_OAK, t.LOG_BIG_OAK_TOP); // acacia (placeholder)
	createWood(162, 2, t.LOG_ACACIA, t.LOG_ACACIA_TOP); // dark wood
	createWood(162, 3, t.LOG_BIG_OAK, t.LOG_BIG_OAK_TOP); // dark wood (placeholder)
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
	// large plants, id 175 --
	// the top texture of the sunflower is a bit modified
	RGBAImage sunflower_top = t.DOUBLE_PLANT_SUNFLOWER_TOP;
	sunflower_top.alphaBlit(t.DOUBLE_PLANT_SUNFLOWER_FRONT, 0, -texture_size * 0.25);
	createLargePlant(0, t.DOUBLE_PLANT_SUNFLOWER_BOTTOM, sunflower_top);
	createLargePlant(1, t.DOUBLE_PLANT_SYRINGA_BOTTOM, t.DOUBLE_PLANT_SYRINGA_TOP);
	createLargePlant(2, t.DOUBLE_PLANT_GRASS_BOTTOM, t.DOUBLE_PLANT_GRASS_TOP);
	createLargePlant(3, t.DOUBLE_PLANT_FERN_BOTTOM, t.DOUBLE_PLANT_FERN_TOP);
	createLargePlant(4, t.DOUBLE_PLANT_ROSE_BOTTOM, t.DOUBLE_PLANT_ROSE_TOP);
	createLargePlant(5, t.DOUBLE_PLANT_PAEONIA_BOTTOM, t.DOUBLE_PLANT_PAEONIA_TOP);
	// --
	// id 176 // standing banner
	// id 177 // wall banner
	setBlockImage(178, 0, t.DAYLIGHT_DETECTOR_INVERTED_TOP); // inverted daylight sensor
	// -- red sandstone
	setBlockImage(179, 0, t.RED_SANDSTONE_TOP); // normal
	setBlockImage(179, 1, t.RED_SANDSTONE_TOP); // chiseled
	setBlockImage(179, 2, t.RED_SANDSTONE_TOP); // smooth
	// --
	// id 180 // red sandstone stairs
	// double red sandstone slabs --
	setBlockImage(181, 0, t.RED_SANDSTONE_TOP);
	// --
	// normal red sandstone slabs --
	setBlockImage(181, 0, t.RED_SANDSTONE_TOP);
	// --
	createFenceGate(183, t.PLANKS_SPRUCE); // spruce fence gate
	createFenceGate(184, t.PLANKS_BIRCH); // birch fence gate
	createFenceGate(185, t.PLANKS_JUNGLE); // jungle fence gate
	createFenceGate(186, t.PLANKS_BIG_OAK); // dark oak fence gate
	createFenceGate(187, t.PLANKS_ACACIA); // acacia fence gate
	createFence(188, 0, t.PLANKS_SPRUCE); // spruce fence
	createFence(189, 0, t.PLANKS_BIRCH); // birch fence
	createFence(190, 0, t.PLANKS_JUNGLE); // jungle fence
	createFence(191, 0, t.PLANKS_BIG_OAK); // dark oak fence
	createFence(192, 0, t.PLANKS_ACACIA); // acacia fence
	// id 193 // spruce door
	// id 194 // birch door
	// id 195 // jungle door
	// id 196 // acacia door
	// id 197 // dark oak door
	// id 198 // end rod
	// id 199 // chrous plant
	// id 200 // chorus flower
	// id 201 // purpur block
	// id 202 // purpur pillar
	// id 203 // purpur stairs
	// id 204 // purpur double slab
	// id 205 // purpur slab
	// id 206 // end stone bricks
	// id 207 // beetroot seeds
	// id 208 // grass path
	// id 209 // end gateway
	// id 210 // repeating command block
	// id 211 // chain command block
	// id 255 // structure block
}

int TopdownBlockImages::createOpaqueWater() {
	// TODO pre-blit each water block of water depth x ?
	// TODO min_alpha >= 250 ?
	
	// just use the Ocean biome watercolor
	RGBAImage water = resources.getBlockTextures().WATER_STILL.colorize(0, 0.39, 0.89);
	RGBAImage opaque_water = water;

	int water_preblit;
	for (water_preblit = 2; water_preblit < 100; water_preblit++) {
		// blit another layer of water
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
		if (min_alpha >= 250)
			break;
	}

	LOG(DEBUG) << "pre-blit water (topdown): " << water_preblit;

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
