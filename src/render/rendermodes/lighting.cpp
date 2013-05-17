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

#include "lighting.h"

#include "render/textures.h"

#include <cmath>

namespace mapcrafter {
namespace render {

/**
 * Experimental, first test lighting implementation.
 */

void drawBottomTriangle(Image& image, int size, double c1, double c2, double c3) {
	double e1diff = c2 - c1;
	double e2diff = c3 - c1;
	double fy = 0;
	double fyStep = (double) 1 / (size-1);
	for (int y = 0; y < size; y++, fy+=fyStep) {
		double color1 = c1 + e1diff*fy;
		double color2 = c1 + e2diff*fy;
		double colordiff = color2 - color1;
		double fx = 0;
		double fxStep = 0;
		if (y != 0)
			fxStep = (double) 1 / y;
		else
			fx = 1;
		for (int x = 0; x <= y; x++, fx+=fxStep) {
			double color = color1 + colordiff*fx;
			image.pixel(x, y) = rgba(0, 0, 0, color * 255);
		}
	}
}

void drawTopTriangle(Image& image, int size, double c1, double c2, double c3) {
	double e1diff = c2 - c1;
	double e2diff = c3 - c1;
	double fy = 0;
	double fyStep = (double) 1 / (size-1);
	for (int y = 0; y < size; y++, fy+=fyStep) {
		double color1 = c1 + e1diff*fy;
		double color2 = c1 + e2diff*fy;
		double colordiff = color2 - color1;
		double fx = 0;
		double fxStep = 0;
		if (y != 0)
			fxStep = (double) 1 / y;
		else
			fx = 1;
		for (int x = 0; x <= y; x++, fx+=fxStep) {
			double color = color1 + colordiff*fx;
			image.pixel(size-1-x, size-1-y) = rgba(0, 0, 0, color * 255);
		}
	}
}

LightingRendermode::LightingRendermode(RenderState& state, bool day)
		: Rendermode(state), day(day) {
}

LightingRendermode::~LightingRendermode() {

}

double LightingRendermode::calculateLightingFactor(uint8_t block_light, uint8_t sky_light) const {
	if (day)
		return pow(0.8, 15 - std::max(block_light, sky_light));
	return pow(0.8, 15 - std::max(block_light+0, sky_light - 11));
}

double LightingRendermode::getLightingValue(const mc::BlockPos& pos) const {
	mc::Block block = state.getBlock(pos, mc::GET_LIGHT);
	return calculateLightingFactor(block.block_light, block.sky_light);
}

double LightingRendermode::getCornerLighting(const mc::BlockPos& pos, mc::BlockPos p1,
		mc::BlockPos p2, mc::BlockPos extra) const {

	return (getLightingValue(pos + p1)
			+ getLightingValue(pos + p2)
			+ getLightingValue(pos + p1 + extra)
			+ getLightingValue(pos + p2 + extra)) / 4;
}

void LightingRendermode::createShade(Image& image, double c1, double c2, double c3,
		double c4) const {
	int size = image.getWidth();
	drawBottomTriangle(image, size, c1, c3, c4);
	drawTopTriangle(image, size, c4, c2, c1);
}

bool isSpecialTransparent(uint16_t id) {
	// blocks which are transparent but don't have correct lighting data

	// they need skylight from the block above
	// (or the block above above, if the block above is also one of this blocks)

	// they need an average blocklight from near blocks
	return id == 44      // slabs
			|| id == 53  // oak wood stairs
			|| id == 67  // cobble stairs
			|| id == 108 // brick stairs
			|| id == 109 // stone brick stairs
			|| id == 114 // nether brick stairs
			|| id == 126 // wooden slabs
			|| id == 128 // sandstone stairs
			|| id == 134 // spruce wood stairs
			|| id == 135 // birch wood stairs
			|| id == 136 // jungle wood stairs
			|| id == 145 // anvil
			|| id == 156;// quartz stairs
}

void LightingRendermode::doSimpleLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	uint8_t sky_light = 0, block_light = 0;

	if(!isSpecialTransparent(id)) {
		mc::Block block = state.getBlock(pos, mc::GET_LIGHT);
		block_light = block.block_light;
		sky_light = block.sky_light;
	} else {
		// get the skylight from the block above
		mc::BlockPos off(0, 0, 0);
		mc::Block block;
		while (++off.y) {
			block = state.getBlock(pos + off, mc::GET_ID | mc::GET_DATA | mc::GET_SKY_LIGHT);
			if (isSpecialTransparent(block.id))
				continue;
			if (block.id == 0 || state.images.isBlockTransparent(block.id, block.data))
				sky_light = block.sky_light;
			else
				sky_light = 15;
			break;
		}

		// get the blocklight from the neighbor blocks
		int block_lights = 0;
		int block_lights_count = 0;
		for (int dx = -1; dx <= 1; dx++)
			for (int dz = -1; dz <= 1; dz++)
				for (int dy = -1; dy <= 1; dy++) {
					mc::Block block = state.getBlock(pos + mc::BlockPos(dx, dz, dy), mc::GET_ID | mc::GET_DATA | mc::GET_BLOCK_LIGHT);
					if ((block.id == 0 || state.images.isBlockTransparent(block.id, block.data))
							&& !isSpecialTransparent(block.id)) {
						block_lights += block.block_light;
						block_lights_count++;
					}
				}

		if (block_lights_count > 0)
			block_light = block_lights / block_lights_count;
	}

	uint8_t factor = calculateLightingFactor(block_light, sky_light) * 255;

	int size = image.getWidth();
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			uint32_t& pixel = image.pixel(x, y);
			if (pixel != 0) {
				pixel = rgba_multiply(pixel, factor, factor, factor, 255);
			}
		}
	}
}

void LightingRendermode::doSmoothLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	bool light_left = true, light_right = true, light_top = true;

	bool water = (id == 8 || id == 9) && (data & 0b1111) == 0;
	if (water) {
		if (data & DATA_WEST)
			light_left = false;
		if (data & DATA_SOUTH)
			light_right = false;
		if (data & DATA_TOP)
			light_top = false;
	}

	mc::Block block;
	if (light_left) {
		block = state.getBlock(pos + mc::DIR_WEST);
		light_left = block.id == 0 || state.images.isBlockTransparent(block.id, block.data);
	}
	if (light_right) {
		block = state.getBlock(pos + mc::DIR_SOUTH);
		light_right = block.id == 0 || state.images.isBlockTransparent(block.id, block.data);
	}
	if (light_top) {
		block = state.getBlock(pos + mc::DIR_TOP);
		light_top = block.id == 0 || state.images.isBlockTransparent(block.id, block.data);
	}

	mc::BlockPos north = mc::DIR_NORTH;
	mc::BlockPos south = mc::DIR_SOUTH;
	mc::BlockPos east = mc::DIR_EAST;
	mc::BlockPos west = mc::DIR_WEST;
	mc::BlockPos top = mc::DIR_TOP;
	mc::BlockPos bottom = mc::DIR_BOTTOM;

	int size = image.getWidth() / 2;

	if (light_left) {
		Image tex(size, size);
		createShade(tex, getCornerLighting(pos, west+north, west),
						 getCornerLighting(pos, west, west+south),
						 getCornerLighting(pos, west+north+bottom, west+bottom),
						 getCornerLighting(pos, west+bottom, west+south+bottom));

		for (SideFaceIterator it(size, SideFaceIterator::LEFT); !it.end(); it.next()) {
			uint32_t pixel = image.pixel(it.dest_x, it.dest_y + size/2);
			if (pixel != 0) {
				uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
				image.pixel(it.dest_x, it.dest_y + size/2) = rgba_multiply(pixel, d, d, d);
			}
		}
	}

	if (light_right) {
		Image tex(size, size);
		createShade(tex, getCornerLighting(pos, south+west, south),
						 getCornerLighting(pos, south, south+east),
						 getCornerLighting(pos, south+west+bottom, south+bottom),
						 getCornerLighting(pos, south+bottom, south+east+bottom));

		for (SideFaceIterator it(size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
			uint32_t pixel = image.pixel(it.dest_x + size, it.dest_y + size/2);
			if (pixel != 0) {
				uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
				image.pixel(it.dest_x + size, it.dest_y + size/2) = rgba_multiply(pixel, d, d, d);
			}
		}
	}

	if (light_top) {
		Image tex(size, size);
		createShade(tex, getCornerLighting(pos, top+north, top, east),
						 getCornerLighting(pos, top, top+south, east),
						 getCornerLighting(pos, top+north+west, top+west, east),
						 getCornerLighting(pos, top+west, top+south+west, east));

		for (TopFaceIterator it(size); !it.end(); it.next()) {
			uint32_t pixel = image.pixel(it.dest_x, it.dest_y);
			if (pixel != 0) {
				uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
				image.pixel(it.dest_x, it.dest_y) = rgba_multiply(pixel, d, d, d);
			}
		}
	}
}

void LightingRendermode::doSmoothLightSnow(Image& image, const mc::BlockPos& pos) {
	int size = image.getWidth() / 2;
	mc::BlockPos north = mc::DIR_NORTH;
	mc::BlockPos south = mc::DIR_SOUTH;
	mc::BlockPos east = mc::DIR_EAST;
	mc::BlockPos west = mc::DIR_WEST;

	Image tex(size, size);
	createShade(tex, getCornerLighting(pos, north, mc::BlockPos(0, 0, 0), east),
					 getCornerLighting(pos, mc::BlockPos(0, 0, 0), south, east),
					 getCornerLighting(pos, north+west, west, east),
					 getCornerLighting(pos, west, south+west, east));

	for (TopFaceIterator it(size); !it.end(); it.next()) {
		uint32_t pixel = image.pixel(it.dest_x, it.dest_y + size);
		if (pixel != 0) {
			uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
			image.pixel(it.dest_x, it.dest_y + size) = rgba_multiply(pixel, d, d, d);
		}
	}
}

bool LightingRendermode::isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	return false;
}

void LightingRendermode::draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	bool transparent = state.images.isBlockTransparent(id, data);
	bool water = (id == 8 || id == 9) && (data & 0b1111) == 0;

	if(id == 78 && (data & 0b1111) == 0) {
		doSmoothLightSnow(image, pos);
	} else if (transparent && !water) {
		doSimpleLight(image, pos, id, data);
	} else {
		doSmoothLight(image, pos, id, data);
	}
}

} /* namespace render */
} /* namespace mapcrafter */
