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

#include "../../../util.h"

namespace mapcrafter {
namespace renderer {

TopdownBlockImages::TopdownBlockImages() {
}

TopdownBlockImages::~TopdownBlockImages() {
}

int TopdownBlockImages::getMaxWaterNeededOpaque() const {
	return 1;
}

const RGBAImage& TopdownBlockImages::getOpaqueWater(bool south, bool west) const {
	return unknown_block;
}

int TopdownBlockImages::getBlockSize() const {
	return texture_size;
}

uint16_t TopdownBlockImages::filterBlockData(uint16_t id, uint16_t data) const {
	if (id == 6)
		return data & (0xff00 | 0b00000011);
	else if (id >= 8 && id <= 11) // water
		return 0;
	else if (id == 18 || id == 161) // leaves
		return data & (0xff00 | 0b00000011);
	else if (id == 26) // bed
		return data & (0xff00 | 0b00001011);
	else if (id == 54 || id == 130) { // chests
		// at first get the direction of the chest and rotate if needed
		uint16_t dir_rotate = (data >> 4) & 0xf;
		uint16_t dir = util::rotateShiftLeft(dir_rotate, rotation, 4) << 4;
		// then get the neighbor chests
		uint16_t neighbors = (data >> 4) & 0xf0;

		// if no neighbors, this is a small chest
		// the data contains only the direction
		if (neighbors == 0 || id == 130)
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
	} else if (id == 55) { // redstone wire, tripwire
		// check if powered
		if ((data & 0b1111) != 0)
			return (data & ~(0b1111)) | REDSTONE_POWERED;
		return data & ~(0b1111);
	} else if (id == 60) // farmland
		return data & 0xff00;
	else if (id == 64 || id == 71) // doors
		return data & 0b1111110000;
	else if (id == 78) // snow
		return 0;
	else if (id == 81 || id == 83 || id == 92) // cactus, sugar cane, cake
		return data & 0xff00;
	else if (id == 84) // jukebox
		return 0;
	else if (id == 93 || id == 94) // redstone repeater
		return data & (0xff00 | 0b00000011);
	else if (id == 117) // brewing stand
		return data & 0xff00;
	else if (id == 119 || id == 120) // end portal, end portal frame
		return data & 0xff00;
	else if (id == 127)
		return data & 0b1100;
	else if (id == 131) // trip wire hook
		return data & 0b11;
	else if (id == 132) // trip wire
		return data & ~0xf;
	// the light sensor shouldn't have any data, but I had problems with it...
	else if (id == 151)
		return 0;
	return data;
}

bool TopdownBlockImages::checkImageTransparency(const RGBAImage& block) const {
	for (int x = 0; x < block.getWidth(); x++)
		for (int y = 0; y < block.getHeight(); y++)
			if (rgba_alpha(block.getPixel(x, y)) < 255)
				return true;
	return false;
}

RGBAImage TopdownBlockImages::createUnknownBlock() const {
	RGBAImage unknown_block(texture_size, texture_size);
	unknown_block.fill(rgba(255, 0, 0, 255), 0, 0, texture_size, texture_size);
	return unknown_block;
}

RGBAImage TopdownBlockImages::createBiomeBlock(uint16_t id, uint16_t data,
		const Biome& biome_data) const {if (!block_images.count(id | (data << 16)))
			return unknown_block;
	uint32_t color;
	// leaves have the foliage colors
	// for birches, the color x/y coordinate is flipped
	if (id == 18)
		color = biome_data.getColor(resources.getGrassColors(), (data & 0b11) == 2);
	else
		color = biome_data.getColor(resources.getFoliageColors(), false);

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

	setBlockImage(8, 0, t.WATER_STILL);
	setBlockImage(9, 0, t.WATER_STILL);
	setBlockImage(10, 0, t.LAVA_STILL);
	setBlockImage(11, 0, t.LAVA_STILL);

	setBlockImage(12, 0, t.SAND); // sand
	setBlockImage(12, 1, t.RED_SAND); // red sand
	setBlockImage(13, 0, t.GRAVEL); // gravel

	setBlockImage(17, 0, t.LOG_OAK_TOP); // oak
	setBlockImage(17, 1, t.LOG_SPRUCE_TOP); // pine/spruce
	setBlockImage(17, 2, t.LOG_BIRCH_TOP); // birch
	setBlockImage(17, 3, t.LOG_JUNGLE_TOP); // jungle

	setBlockImage(18, 0, t.LEAVES_OAK); // oak
	setBlockImage(18, 1, t.LEAVES_SPRUCE); // pine/spruce
	setBlockImage(18, 2, t.LEAVES_OAK); // birch
	setBlockImage(18, 3, t.LEAVES_JUNGLE); // jungle

	setBlockImage(31, 0, empty_texture); // dead bush style
	setBlockImage(31, 1, empty_texture); // tall grass
	setBlockImage(31, 2, empty_texture); // fern
	setBlockImage(32, 0, empty_texture); // dead bush

	setBlockImage(48, 0, t.COBBLESTONE_MOSSY); // moss stone

	setBlockImage(78, 0, t.SNOW); // snow

	setBlockImage(82, 0, t.CLAY); // clay block

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

	setBlockImage(161, 0, t.LEAVES_ACACIA); // acacia
	setBlockImage(161, 1, t.LEAVES_BIG_OAK); // dark oak
	setBlockImage(162, 0, t.LOG_ACACIA_TOP); // acacia
	setBlockImage(162, 1, t.LOG_BIG_OAK_TOP); // acacia (placeholder)
	setBlockImage(162, 2, t.LOG_ACACIA_TOP); // dark wood
	setBlockImage(162, 3, t.LOG_BIG_OAK_TOP); // dark wood (placeholder)

	setBlockImage(172, 0, t.HARDENED_CLAY); // hardened clay
}

void TopdownBlockImages::createBiomeBlocks() {
	for (auto it = block_images.begin(); it != block_images.end(); ++it) {
		uint16_t id = it->first & 0xffff;
		uint16_t data = (it->first & 0xffff0000) >> 16;

		// check if this is a biome block
		if (!Biome::isBiomeBlock(id, data))
			continue;

		for (size_t i = 0; i < BIOMES_SIZE; i++) {
			Biome biome = BIOMES[i];
			uint64_t b = biome.getID();
			biome_images[id | ((uint64_t) data << 16) | (b << 32)] =
					createBiomeBlock(id, data, biome);
		}
	}
}

} /* namespace renderer */
} /* namespace mapcrafter */
