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

#include "lighting.h"

#include "../blockimages.h"
#include "../image.h"
#include "../renderviews/isometric/blockimages.h"
#include "../renderviews/topdown/blockimages.h"
#include "../../mc/chunk.h"
#include "../../mc/pos.h"
#include "../../util.h"

#include <cmath>
#include "../../config/configsections/map.h"
#include "../../config/configsections/world.h"

namespace mapcrafter {
namespace renderer {

// TODO
// Find out why the mc::DIR_* constants are all set to (0, 0, 0) !

// corner definitions of the faces
extern const FaceCorners CORNERS_LEFT = FaceCorners(CornerNeighbors(
		/*mc::DIR_WEST + mc::DIR_NORTH + mc::DIR_TOP*/ mc::BlockPos(-1, -1, 1),
		/*mc::DIR_SOUTH*/ mc::BlockPos(0, 1, 0),
		/*mc::DIR_BOTTOM*/ mc::BlockPos(0, 0, -1)));

extern const FaceCorners CORNERS_RIGHT = FaceCorners(CornerNeighbors(
		/*mc::DIR_SOUTH + mc::DIR_WEST + mc::DIR_TOP*/ mc::BlockPos(-1, 1, 1),
		/*mc::DIR_EAST*/ mc::BlockPos(1, 0, 0),
		/*mc::DIR_BOTTOM*/ mc::BlockPos(0, 0, -1)));

extern const FaceCorners CORNERS_TOP = FaceCorners(CornerNeighbors(
		/*mc::DIR_TOP + mc::DIR_NORTH + mc::DIR_WEST*/ mc::BlockPos(-1, -1, 1),
		/*mc::DIR_EAST*/ mc::BlockPos(1, 0, 0),
		/*mc::DIR_SOUTH*/ mc::BlockPos(0, 1, 0)));

extern const FaceCorners CORNERS_BOTTOM = FaceCorners(CornerNeighbors(
		/*mc::DIR_NORTH + mc::DIR_WEST*/ mc::BlockPos(-1, -1, 0),
		/*mc::DIR_EAST*/ mc::BlockPos(1, 0, 0),
		/*mc::DIR_SOUTH*/ mc::BlockPos(0, 1, 0)));

CornerNeighbors::CornerNeighbors() {
}

CornerNeighbors::CornerNeighbors(const mc::BlockPos& pos1, const mc::BlockPos& dir1,
		const mc::BlockPos& dir2)
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

LightingRenderMode::LightingRenderMode(bool day, double lighting_intensity,
		bool dimension_end)
	: day(day), lighting_intensity(lighting_intensity), dimension_end(dimension_end),
	  render_view_initialized(false), isometric_render_view(true) {
}

LightingRenderMode::~LightingRenderMode() {
}

bool LightingRenderMode::isHidden(const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	return false;
}

void LightingRenderMode::draw(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	// TODO BIG ONE!
	// split render modes up to some kind of frontend/backend
	if (!render_view_initialized) {
		render_view_initialized = true;
		isometric_render_view = dynamic_cast<IsometricBlockImages*>(images) != nullptr;
	}

	bool transparent = images->isBlockTransparent(id, data);

	bool water = (id == 8 || id == 9) && (data & util::binary<1111>::value) == 0;
	int texture_size = image.getHeight() / 2;

	if (isometric_render_view) {
		if (id == 78 && (data & util::binary<1111>::value) == 0) {
			// flat snow gets also smooth lighting
			int height = ((data & util::binary<1111>::value) + 1) / 8.0 * texture_size;
			lightTop(image, getCornerColors(pos, CORNERS_BOTTOM),
					texture_size - height);
			lightLeft(image, getCornerColors(pos, CORNERS_LEFT),
					texture_size - height, texture_size);
			lightRight(image, getCornerColors(pos, CORNERS_RIGHT),
					texture_size - height, texture_size);
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
	} else {
		if (id == 44 || id == 126) {
		} else if (transparent && !water && id != 79) {
		} else {
			lightTop(image, getCornerColors(pos, CORNERS_TOP));
		}
	}
}

/**
 * Draws the bottom triangle.
 * This is the triangle with corners top left, bottom left and bottom right.
 */
void drawBottomTriangle(RGBAImage& image, int size, double c1, double c2,
		double c3) {
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
			//color += (1-color)*(1-LIGHTNING_INTENSITY);
			image.pixel(x, y) = rgba(0, 0, 0, color * 255);
		}
	}
}

/**
 * Draws the top triangle.
 * This is the triangle with corners top left, top right and bottom right.
 */
void drawTopTriangle(RGBAImage& image, int size, double c1, double c2, double c3) {
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
			//color += (1-color)*(1-LIGHTNING_INTENSITY);
			image.pixel(size-1-x, size-1-y) = rgba(0, 0, 0, color * 255);
		}
	}
}

void LightingRenderMode::createShade(RGBAImage& image,
		const CornerColors& corners) const {
	int size = image.getWidth();
	drawBottomTriangle(image, size, corners[0], corners[2], corners[3]);
	drawTopTriangle(image, size, corners[3], corners[1], corners[0]);
}

LightingColor LightingRenderMode::calculateLightingColor(
		const LightingData& light) const {
	if (day)
		return pow(0.8, 15 - std::max(light.block, light.sky));
	return pow(0.8, 15 - std::max(light.block+0, light.sky - 11));
}

bool isSpecialTransparent(uint16_t id) {
	// blocks which are transparent but don't have correct lighting data

	// they need skylight from the block above
	// (or the block above above, if the block above is also one of this blocks)

	// they need an average blocklight from near blocks
	return id == 44     // stone slabs
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
			|| id == 156 // quartz stairs
			|| id == 163 // acacia wood stairs
			|| id == 164 // dark oak wood stairs
			|| id == 180 // red sandstone stairs
			|| id == 182 // stone2 slabs
			;
}

LightingData LightingRenderMode::estimateLight(const mc::BlockPos& pos) {
	LightingData light;
	// get the sky light from the block above
	mc::BlockPos off(0, 0, 0);
	mc::Block above;
	while (++off.y) {
		above = getBlock(pos + off, mc::GET_ID | mc::GET_DATA | mc::GET_SKY_LIGHT);
		if (isSpecialTransparent(above.id))
			continue;
		if (above.id == 0 || images->isBlockTransparent(above.id, above.data))
			light.sky = above.sky_light;
		else
			light.sky = 15;
		break;
	}

	// get the block light from the neighbor blocks
	int block_lights = 0;
	int block_lights_count = 0;
	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++)
			for (int dy = -1; dy <= 1; dy++) {
				mc::Block other = getBlock(pos + mc::BlockPos(dx, dz, dy),
						mc::GET_ID | mc::GET_DATA | mc::GET_BLOCK_LIGHT);
				if ((other.id == 0
						|| images->isBlockTransparent(other.id, other.data))
						&& !isSpecialTransparent(other.id)) {
					block_lights += other.block_light;
					block_lights_count++;
				}
			}

	if (block_lights_count > 0)
		light.block = block_lights / block_lights_count;
	return light;
}

LightingData LightingRenderMode::getBlockLight(const mc::BlockPos& pos) {
	LightingData light;
	mc::Block block = getBlock(pos, mc::GET_ID | mc::GET_DATA | mc::GET_LIGHT);
	if (isSpecialTransparent(block.id)) {
		light = estimateLight(pos);
	} else {
		light.block = block.block_light,
		light.sky = block.sky_light;
	}

	// lighting fix for The End
	// The End has no sun light set -> lighting looks ugly
	// just emulate the sun light for transparent blocks
	if (dimension_end) {
		light.sky = 15;
		if (block.id != 0 && !images->isBlockTransparent(block.id, block.data))
			light.sky = 0;
	}
	return light;
}

LightingColor LightingRenderMode::getLightingColor(const mc::BlockPos& pos) {
	LightingData lighting = getBlockLight(pos);
	LightingColor color = calculateLightingColor(lighting);
	return color + (1-color)*(1-lighting_intensity);
}

LightingColor LightingRenderMode::getCornerColor(const mc::BlockPos& pos,
		const CornerNeighbors& corner) {
	LightingColor color = 0;
	color += getLightingColor(pos + corner.pos1) * 0.25;
	color += getLightingColor(pos + corner.pos2) * 0.25;
	color += getLightingColor(pos + corner.pos3) * 0.25;
	color += getLightingColor(pos + corner.pos4) * 0.25;
	return color;
}

CornerColors LightingRenderMode::getCornerColors(const mc::BlockPos& pos,
		const FaceCorners& corners) {
	CornerColors colors = {{
		getCornerColor(pos, corners.corner1),
		getCornerColor(pos, corners.corner2),
		getCornerColor(pos, corners.corner3),
		getCornerColor(pos, corners.corner4),
	}};
	return colors;
}

void LightingRenderMode::lightLeft(RGBAImage& image, const CornerColors& colors,
		int y_start, int y_end) {
	int size = image.getWidth() / 2;
	RGBAImage tex(size, size);
	createShade(tex, colors);

	for (SideFaceIterator it(size, SideFaceIterator::LEFT); !it.end(); it.next()) {
		if (it.src_y < y_start || it.src_y > y_end)
			continue;
		uint32_t& pixel = image.pixel(it.dest_x, it.dest_y + size/2);
		if (pixel != 0) {
			uint8_t d = rgba_alpha(tex.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
	}
}

void LightingRenderMode::lightLeft(RGBAImage& image, const CornerColors& colors) {
	lightLeft(image, colors, 0, image.getHeight() / 2);
}

void LightingRenderMode::lightRight(RGBAImage& image, const CornerColors& colors,
		int y_start, int y_end) {
	int size = image.getWidth() / 2;
	RGBAImage tex(size, size);
	createShade(tex, colors);

	for (SideFaceIterator it(size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
		if (it.src_y < y_start || it.src_y > y_end)
			continue;
		uint32_t& pixel = image.pixel(it.dest_x + size, it.dest_y + size/2);
		if (pixel != 0) {
			uint8_t d = rgba_alpha(tex.pixel(it.src_x, it.src_y));
			pixel = rgba_multiply(pixel, d, d, d);
		}
	}
}

void LightingRenderMode::lightRight(RGBAImage& image, const CornerColors& colors) {
	lightRight(image, colors, 0, image.getHeight() / 2);
}

void LightingRenderMode::lightTop(RGBAImage& image, const CornerColors& colors,
		int yoff) {
	if (isometric_render_view) {
		int size = image.getWidth() / 2;
		RGBAImage tex(size, size);
		// we need to rotate the corners a bit to make them suitable for the TopFaceIterator
		CornerColors rotated = {{colors[1], colors[3], colors[0], colors[2]}};
		createShade(tex, rotated);

		for (TopFaceIterator it(size); !it.end(); it.next()) {
			uint32_t& pixel = image.pixel(it.dest_x, it.dest_y + yoff);
			if (pixel != 0) {
				uint8_t d = rgba_alpha(tex.pixel(it.src_x, it.src_y));
				pixel = rgba_multiply(pixel, d, d, d);
			}
		}
	} else {
		int size = image.getWidth();
		RGBAImage tex(size, size);
		createShade(tex, colors);
		for (int x = 0; x < size; x++)
			for (int y = 0; y < size; y++) {
				uint32_t& pixel = image.pixel(x, y);
				if (pixel != 0) {
					uint8_t d = rgba_alpha(tex.pixel(x, y));
					pixel = rgba_multiply(pixel, d, d, d);
				}
			}
	}
}

void LightingRenderMode::doSmoothLight(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	// check if lighting faces are visible
	bool light_left = true, light_right = true, light_top = true;
	bool water = (id == 8 || id == 9) && (data & util::binary<1111>::value) == 0;

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
		block = getBlock(pos + mc::DIR_WEST);
		light_left = block.id == 0 || images->isBlockTransparent(block.id, block.data);
	}
	if (light_right) {
		block = getBlock(pos + mc::DIR_SOUTH);
		light_right = block.id == 0 || images->isBlockTransparent(block.id, block.data);
	}
	if (light_top) {
		block = getBlock(pos + mc::DIR_TOP);
		light_top = block.id == 0 || images->isBlockTransparent(block.id, block.data);
	}

	// do the lighting
	if (light_left)
		lightLeft(image, getCornerColors(pos, CORNERS_LEFT));

	if (light_right)
		lightRight(image, getCornerColors(pos, CORNERS_RIGHT));

	if (light_top)
		lightTop(image, getCornerColors(pos, CORNERS_TOP));
}

void LightingRenderMode::doSlabLight(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	// to apply smooth lighting to a slab,
	// we move the top shadow down if this is the bottom slab
	// and we use only the top/bottom part of the side shadows

	// check if the slab is the top or the bottom half of the block
	bool top = data & 0x8;
	// set y-offset for the top face shadow
	int yoff = top ? 0 : image.getHeight()/4;
	// set limits for the sides where it should apply lighting
	int ystart = yoff;
	int yend = yoff + image.getHeight()/4;

	// light the faces
	mc::Block block;
	block = getBlock(pos + mc::DIR_WEST);
	if (block.id == 0 || images->isBlockTransparent(block.id, block.data))
		lightLeft(image, getCornerColors(pos, CORNERS_LEFT), ystart, yend);

	block = getBlock(pos + mc::DIR_SOUTH);
	if (block.id == 0 || images->isBlockTransparent(block.id, block.data))
		lightRight(image, getCornerColors(pos, CORNERS_RIGHT), ystart, yend);

	block = getBlock(pos + mc::DIR_TOP);
	if (block.id == 0 || images->isBlockTransparent(block.id, block.data))
		lightTop(image, getCornerColors(pos, CORNERS_TOP), yoff);
}

void LightingRenderMode::doSimpleLight(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
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

} /* namespace render */
} /* namespace mapcrafter */
