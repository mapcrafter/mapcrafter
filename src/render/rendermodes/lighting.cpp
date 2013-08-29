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

#include "../textures.h"

#include <cmath>

namespace mapcrafter {
namespace render {

// corner definitions of the faces
extern const FaceCorners CORNERS_LEFT = FaceCorners(CornerNeighbors(
		mc::DIR_WEST + mc::DIR_NORTH + mc::DIR_TOP,
		mc::DIR_SOUTH,
		mc::DIR_BOTTOM));

extern const FaceCorners CORNERS_RIGHT = FaceCorners(CornerNeighbors(
		mc::DIR_SOUTH + mc::DIR_WEST + mc::DIR_TOP,
		mc::DIR_EAST,
		mc::DIR_BOTTOM));

extern const FaceCorners CORNERS_TOP = FaceCorners(CornerNeighbors(
		mc::DIR_TOP + mc::DIR_NORTH + mc::DIR_WEST,
		mc::DIR_EAST,
		mc::DIR_SOUTH));

extern const FaceCorners CORNERS_BOTTOM = FaceCorners(CornerNeighbors(
		mc::DIR_NORTH + mc::DIR_WEST,
		mc::DIR_EAST,
		mc::DIR_SOUTH));

CornerNeighbors::CornerNeighbors() {
}

CornerNeighbors::CornerNeighbors(const mc::BlockPos& pos1, const mc::BlockPos& dir1, const mc::BlockPos& dir2)
	: pos1(pos1),
	  pos2(pos1 + dir1),
	  pos3(pos1 + dir2),
	  pos4(pos1 + dir1 + dir2),

	  dir1(dir1), dir2(dir2) {
}

CornerNeighbors CornerNeighbors::addPos(const mc::BlockPos& pos) const {
	return CornerNeighbors(this->pos1 + pos, dir1, dir2);
}

FaceCorners::FaceCorners() {
}

FaceCorners::FaceCorners(const CornerNeighbors& corner1)
	: corner1(corner1),
	  corner2(corner1.addPos(corner1.dir1)),
	  corner3(corner1.addPos(corner1.dir2)),
	  corner4(corner1.addPos(corner1.dir1 + corner1.dir2)) {
}

/**
 * Draws the bottom triangle.
 * This is the triangle with corners top left, bottom left and bottom right.
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

/**
 * Draws the top triangle.
 * This is the triangle with corners top left, top right and bottom right.
 */
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

/**
 * Draws the shade of the corners by drawing two triangles with the supplied colors.
 */
void LightingRendermode::createShade(Image& image, const CornerColors& corners) const {
	int size = image.getWidth();
	drawBottomTriangle(image, size, corners[0], corners[2], corners[3]);
	drawTopTriangle(image, size, corners[3], corners[1], corners[0]);
}

/**
 * Calculates the color of the light of a block.
 *
 * This uses the formula 0.8**(15 - max(block_light, sky_light))
 * When calculating nightlight, the skylight is reduced by 11.
 */
LightingColor LightingRendermode::calculateLightingColor(uint8_t block_light, uint8_t sky_light) const {
	if (day)
		return pow(0.8, 15 - std::max(block_light, sky_light));
	return pow(0.8, 15 - std::max(block_light+0, sky_light - 11));
}

bool isSpecialTransparent(uint16_t id) {
	// blocks which are transparent but don't have correct lighting data

	// they need skylight from the block above
	// (or the block above above, if the block above is also one of this blocks)

	// they need an average blocklight from near blocks
	return id == 44      // slabs
			|| id == 53  // oak wood stairs
			|| id == 64 || id == 71 // doors
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

/**
 * Estimates the light of a block from its neighbors.
 */
void LightingRendermode::estimateBlockLight(mc::Block& block, const mc::BlockPos& pos) const {
	assert(state.isValid());

	// get the sky light from the block above
	mc::BlockPos off(0, 0, 0);
	mc::Block above;
	while (++off.y) {
		above = state.getBlock(pos + off, mc::GET_ID | mc::GET_DATA | mc::GET_SKY_LIGHT);
		if (isSpecialTransparent(above.id))
			continue;
		if (above.id == 0 || state.images->isBlockTransparent(above.id, above.data))
			block.sky_light = above.sky_light;
		else
			block.sky_light = 15;
		break;
	}

	// get the block light from the neighbor blocks
	int block_lights = 0;
	int block_lights_count = 0;
	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++)
			for (int dy = -1; dy <= 1; dy++) {
				mc::Block other = state.getBlock(pos + mc::BlockPos(dx, dz, dy), mc::GET_ID | mc::GET_DATA | mc::GET_BLOCK_LIGHT);
				if ((other.id == 0 || state.images->isBlockTransparent(other.id, other.data))
						&& !isSpecialTransparent(other.id)) {
					block_lights += other.block_light;
					block_lights_count++;
				}
			}

	if (block_lights_count > 0)
		block.block_light = block_lights / block_lights_count;
}

/**
 * Returns the light of a block (sky/block light). This also means that the light is
 * estimated if this is a special transparent block.
 */
LightingData LightingRendermode::getBlockLight(const mc::BlockPos& pos) const {
	mc::Block block = state.getBlock(pos, mc::GET_ID | mc::GET_LIGHT);
	if (isSpecialTransparent(block.id))
		estimateBlockLight(block, pos);
	return {block.block_light, block.sky_light};
}

/**
 * Returns the lighting color of a block.
 */
LightingColor LightingRendermode::getLightingColor(const mc::BlockPos& pos) const {
	LightingData lighting = getBlockLight(pos);
	return calculateLightingColor(lighting.block, lighting.sky);
}

/**
 * Returns the lighting color of a corner by calculating the average lighting color of
 * the four neighbor blocks.
 */
LightingColor LightingRendermode::getCornerColor(const mc::BlockPos& pos, const CornerNeighbors& corner) const {
	LightingColor color = 0;
	color += getLightingColor(pos + corner.pos1) * 0.25;
	color += getLightingColor(pos + corner.pos2) * 0.25;
	color += getLightingColor(pos + corner.pos3) * 0.25;
	color += getLightingColor(pos + corner.pos4) * 0.25;
	return color;
}

/**
 * Returns the corner lighting colors of a block face.
 */
CornerColors LightingRendermode::getCornerColors(const mc::BlockPos& pos, const FaceCorners& corners) const {
	CornerColors colors = {{
		getCornerColor(pos, corners.corner1),
		getCornerColor(pos, corners.corner2),
		getCornerColor(pos, corners.corner3),
		getCornerColor(pos, corners.corner4),
	}};
	return colors;
}

/**
 * Adds smooth lighting to the left face of a block image.
 */
void LightingRendermode::lightLeft(Image& image, const CornerColors& colors,
		bool top, bool bottom) const {
	int size = image.getWidth() / 2;
	Image tex(size, size);
	createShade(tex, colors);

	for (SideFaceIterator it(size, SideFaceIterator::LEFT); !it.end(); it.next()) {
		if ((it.src_y < size/2 && !top) || (it.src_y >= size/2 && !bottom))
			continue;
		uint32_t& pixel = image.pixel(it.dest_x, it.dest_y + size/2);
		if (pixel != 0) {
			uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
	}
}

/**
 * Adds smooth lighting to the right face of a block image.
 */
void LightingRendermode::lightRight(Image& image, const CornerColors& colors,
		bool top, bool bottom) const {
	int size = image.getWidth() / 2;
	Image tex(size, size);
	createShade(tex, colors);

	for (SideFaceIterator it(size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
		if ((it.src_y < size/2 && !top) || (it.src_y >= size/2 && !bottom))
			continue;
		uint32_t& pixel = image.pixel(it.dest_x + size, it.dest_y + size/2);
		if (pixel != 0) {
			uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
	}
}

/**
 * Adds smooth lighting to the top face of a block image.
 */
void LightingRendermode::lightTop(Image& image, const CornerColors& colors, int yoff) const {
	int size = image.getWidth() / 2;
	Image tex(size, size);
	// we need to rotate the corners a bit to make them suitable for the TopFaceIterator
	CornerColors rotated = {{colors[1], colors[3], colors[0], colors[2]}};
	createShade(tex, rotated);

	for (TopFaceIterator it(size); !it.end(); it.next()) {
		uint32_t& pixel = image.pixel(it.dest_x, it.dest_y + yoff);
		if (pixel != 0) {
			uint8_t d = ALPHA(tex.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
	}
}

/**
 * Applies the smooth lighting to a slab (not double slabs).
 */
void LightingRendermode::doSlabLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	// to apply smooth lighting to a slab,
	// we move the top shadow down if this is the bottom slab
	// and we use only the top/bottom part of the side shadows

	// check if the slab is the top or the bottom half of the block
	bool top = data & 0x8;
	bool bottom = !top;
	// set y-offset for the top face shadow
	int yoff = top ? 0 : image.getHeight()/4;

	// light the faces
	mc::Block block;
	block = state.getBlock(pos + mc::DIR_WEST);
	if (block.id == 0 || state.images->isBlockTransparent(block.id, block.data))
		lightLeft(image, getCornerColors(pos, CORNERS_LEFT), top, bottom);

	block = state.getBlock(pos + mc::DIR_SOUTH);
	if (block.id == 0 || state.images->isBlockTransparent(block.id, block.data))
		lightRight(image, getCornerColors(pos, CORNERS_RIGHT), top, bottom);

	block = state.getBlock(pos + mc::DIR_TOP);
	if (block.id == 0 || state.images->isBlockTransparent(block.id, block.data))
		lightTop(image, getCornerColors(pos, CORNERS_TOP), yoff);
}

/**
 * Applies a simple lighting to a block. This colors the whole block with the lighting
 * color of the block.
 */
void LightingRendermode::doSimpleLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	uint8_t factor = getLightingColor(pos) * 255;

	int size = image.getWidth();
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			uint32_t& pixel = image.pixel(x, y);
			if (pixel != 0)
				pixel = rgba_multiply(pixel, factor, factor, factor, 255);
		}
	}
}

/**
 * Applies the smooth lighting to a block.
 */
void LightingRendermode::doSmoothLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	// check if lighting faces are visible
	bool light_left = true, light_right = true, light_top = true;

	bool water = (id == 8 || id == 9) && (data & 0b1111) == 0;
	if (water || id == 79) {
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
		light_left = block.id == 0 || state.images->isBlockTransparent(block.id, block.data);
	}
	if (light_right) {
		block = state.getBlock(pos + mc::DIR_SOUTH);
		light_right = block.id == 0 || state.images->isBlockTransparent(block.id, block.data);
	}
	if (light_top) {
		block = state.getBlock(pos + mc::DIR_TOP);
		light_top = block.id == 0 || state.images->isBlockTransparent(block.id, block.data);
	}

	// do the lighting
	if (light_left)
		lightLeft(image, getCornerColors(pos, CORNERS_LEFT));

	if (light_right)
		lightRight(image, getCornerColors(pos, CORNERS_RIGHT));

	if (light_top)
		lightTop(image, getCornerColors(pos, CORNERS_TOP));
}

bool LightingRendermode::isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	return false;
}

void LightingRendermode::draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	bool transparent = state.images->isBlockTransparent(id, data);
	bool water = (id == 8 || id == 9) && (data & 0b1111) == 0;

	if(id == 78 && (data & 0b1111) == 0) {
		// flat snow gets also smooth lighting
		lightTop(image, getCornerColors(pos, CORNERS_BOTTOM), image.getHeight() / 2);
	} else if (id == 44 || id == 126) {
		// slabs and wooden slabs
		doSlabLight(image, pos, id, data);
	} else if (transparent && !water && id != 79) {
		// transparent blocks (except full water blocks and ice)
		// get simple lighting, they are completely lighted, not per face
		doSimpleLight(image, pos, id, data);
	} else {
		// do smooth lighting for all other blocks
		doSmoothLight(image, pos, id, data);
	}
}

} /* namespace render */
} /* namespace mapcrafter */
