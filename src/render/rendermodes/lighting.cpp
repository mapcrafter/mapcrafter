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

double LightingRendermode::getLightingValue(const mc::BlockPos& pos) const {
	mc::Block block = state.getBlock(pos, mc::GET_LIGHT);
	if (day)
		return pow(0.8, 15 - std::max(block.block_light, block.sky_light));
	return pow(0.8, 15 - std::max(block.block_light+0, block.sky_light - 11));
}

double LightingRendermode::getLighting(const mc::BlockPos& pos, mc::BlockPos p1,
		mc::BlockPos p2, mc::BlockPos extra) const {

	return (getLightingValue(pos + p1) + getLightingValue(pos + p2)
			+ getLightingValue(pos + p1 + extra) + getLightingValue(pos + p2 + extra)) / 4;

	/*
	mc::Block b1 = state.getBlock(pos + p1, mc::GET_LIGHT);
	mc::Block b2 = state.getBlock(pos + p2, mc::GET_LIGHT);
	mc::Block b3 = state.getBlock(pos + p1 + extra, mc::GET_LIGHT);
	mc::Block b4 = state.getBlock(pos + p2 + extra, mc::GET_LIGHT);
	int block_light = (b1.block_light + b2.block_light + b3.block_light + b4.block_light) / 4;
	int sky_light = (b1.sky_light + b2.sky_light + b3.sky_light + b4.sky_light) / 4;
	if (!day)
		sky_light -= 11;
	double light = pow(0.8, 15 - std::max(block_light, sky_light));

	double a = light;
	//double a = (1 - light) * 0.5;
	//double a = (0.8 - light) * 0.9;
	if (!day)
		a = 1 - light;
	if (a < 0)
		return 0;
	return a;
	*/
}

void LightingRendermode::createShade(Image& image, double c1, double c2, double c3,
		double c4) const {
	int size = image.getWidth();
	drawBottomTriangle(image, size, c1, c3, c4);
	drawTopTriangle(image, size, c4, c2, c1);
}

bool LightingRendermode::isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	return false;
}

void LightingRendermode::draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	bool light_left = true, light_right = true, light_top = true;

	bool water = (id == 8 || id == 9) /*&& (data & 0b1111) == 0*/;
	if (water) {
		if ((data & DATA_WEST) == DATA_WEST)
			light_left = false;
		if ((data & DATA_SOUTH) == DATA_SOUTH)
			light_right = false;
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
		createShade(tex, getLighting(pos, west+north, west),
						 getLighting(pos, west, west+south),
						 getLighting(pos, west+north+bottom, west+bottom),
						 getLighting(pos, west+bottom, west+south+bottom));

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
		createShade(tex, getLighting(pos, south+west, south),
						 getLighting(pos, south, south+east),
						 getLighting(pos, south+west+bottom, south+bottom),
						 getLighting(pos, south+bottom, south+east+bottom));

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
		createShade(tex, getLighting(pos, top+north, top, east),
						 getLighting(pos, top, top+south, east),
						 getLighting(pos, top+north+west, top+west, east),
						 getLighting(pos, top+west, top+south+west, east));

		for (TopFaceIterator it(size); !it.end(); it.next()) {
			uint32_t pixel = image.pixel(it.dest_x, it.dest_y);
			if (pixel != 0) {
				uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
				image.pixel(it.dest_x, it.dest_y) = rgba_multiply(pixel, d, d, d);
			}
		}
	}
}

} /* namespace render */
} /* namespace mapcrafter */
