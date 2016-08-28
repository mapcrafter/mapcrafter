/*
 * Copyright 2012-2016 Moritz Hilscher
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

LightingData::LightingData(uint8_t block_light, uint8_t sky_light)
	: block_light(block_light), sky_light(sky_light) {
}

LightingData::~LightingData() {
}

uint8_t LightingData::getBlockLight() const {
	return block_light;
}

uint8_t LightingData::getSkyLight() const {
	return sky_light;
}

uint8_t LightingData::getLightLevel(bool day) const {
	if (day)
		return std::max(block_light, sky_light);
	return std::max(block_light + 0, sky_light - 11);
}

namespace {

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
			|| id == 203 // purpur stairs
			|| id == 205 // purpur slabs
			|| id == 208 // grass paths
			;
}

}

LightingData LightingData::estimate(const mc::Block& block,
		BlockImages* images, mc::WorldCache* world, mc::Chunk* current_chunk) {
	// estimate the light if this is a special block
	if (!isSpecialTransparent(block.id))
		return LightingData(block.block_light, block.sky_light);

	uint8_t block_light = 0, sky_light = 0;

	// get the sky light from the block above
	mc::BlockPos off(0, 0, 0);
	mc::Block above;
	while (++off.y) {
		above = world->getBlock(block.pos + off, current_chunk,
				mc::GET_ID | mc::GET_DATA | mc::GET_SKY_LIGHT);
		if (isSpecialTransparent(above.id))
			continue;
		if (above.id == 0 || images->isBlockTransparent(above.id, above.data))
			sky_light = above.sky_light;
		else
			sky_light = 15;
		break;
	}

	// get the block light from the neighbor blocks
	int block_lights = 0;
	int block_lights_count = 0;
	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++)
			for (int dy = -1; dy <= 1; dy++) {
				mc::Block other = world->getBlock(block.pos + mc::BlockPos(dx, dz, dy),
						current_chunk, mc::GET_ID | mc::GET_DATA | mc::GET_BLOCK_LIGHT);
				if ((other.id == 0
						|| images->isBlockTransparent(other.id, other.data))
						&& !isSpecialTransparent(other.id)) {
					block_lights += other.block_light;
					block_lights_count++;
				}
			}

	if (block_lights_count > 0)
		block_light = block_lights / block_lights_count;
	return LightingData(block_light, sky_light);
}


LightingRenderer::~LightingRenderer() {
}

void LightingRenderer::lightAllSimple(RGBAImage& image, LightingColor color) const {
	uint8_t factor = color * 255;

	int size = image.getWidth();
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			uint32_t& pixel = image.pixel(x, y);
			if (pixel != 0)
				pixel = rgba_multiply(pixel, factor, factor, factor, 255);
		}
	}
}

const OverlayRendererType LightingRenderer::TYPE = OverlayRendererType::LIGHTING;

void LightingRenderer::drawBottomTriangle(RGBAImage& image, int size, double c1,
		double c2, double c3) const {
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

void LightingRenderer::drawTopTriangle(RGBAImage& image, int size, double c1,
		double c2, double c3) const {
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

void LightingRenderer::createShade(RGBAImage& image, const CornerColors& corners) const {
	int size = image.getWidth();
	drawBottomTriangle(image, size, corners[0], corners[2], corners[3]);
	drawTopTriangle(image, size, corners[3], corners[1], corners[0]);
}

LightingOverlay::LightingOverlay(std::shared_ptr<config::ConfigSection> overlay_config,
		bool simulate_sun_light)
	: AbstractOverlay(overlay_config),
	day(config->isDay()),
	lighting_intensity(config->getIntensity()),
	lighting_water_intensity(config->getWaterIntensity()),
	simulate_sun_light(simulate_sun_light) {
}

LightingOverlay::~LightingOverlay() {
}

void LightingOverlay::drawOverlay(const RGBAImage& block, RGBAImage& overlay,
		const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	bool transparent = images->isBlockTransparent(id, data);

	bool water = (id == 8 || id == 9) && (data & util::binary<1111>::value) == 0;
	int texture_size = block.getHeight() / 2;

	if ((id == 78 && (data & util::binary<1111>::value) == 0) || id == 208) {
		// flat snow and grass paths also get smooth lighting
		int height = ((data & util::binary<1111>::value) + 1) / 8.0 * texture_size;
		if (id == 208)
			height = texture_size * 15.0 / 16.0;
		renderer->lightTop(overlay, getCornerColors(pos, CORNERS_BOTTOM),
				texture_size - height);
		renderer->lightLeft(overlay, getCornerColors(pos, CORNERS_LEFT),
				texture_size - height, texture_size);
		renderer->lightRight(overlay, getCornerColors(pos, CORNERS_RIGHT),
				texture_size - height, texture_size);
	} else if (id == 44 || id == 126) {
		// slabs and wooden slabs
		doSlabLight(overlay, pos, id, data);
	} else if (transparent && !water && id != 79) {
		// transparent blocks (except full water blocks, ice, grass paths)
		// get simple lighting, they are completely lighted, not per face
		doSimpleLight(overlay, pos, id, data);
	} else {
		// do smooth lighting for all other blocks
		doSmoothLight(overlay, pos, id, data);
	}
}

LightingColor LightingOverlay::calculateLightingColor(const LightingData& light) const {
	return pow(0.8, 15 - light.getLightLevel(day));
}

LightingData LightingOverlay::getBlockLight(const mc::BlockPos& pos) {
	mc::Block block = getBlock(pos, mc::GET_ID | mc::GET_DATA | mc::GET_LIGHT);
	LightingData light = LightingData::estimate(block, images, world, *current_chunk);

	// TODO also move this to LightingData class?
	// lighting fix for The End
	// The End has no sun light set -> lighting looks ugly
	// just emulate the sun light for transparent blocks
	if (simulate_sun_light) {
		uint8_t sky = 15;
		if (block.id != 0 && !images->isBlockTransparent(block.id, block.data))
			sky = 0;
		return LightingData(sky, light.getBlockLight());
	}
	return light;
}

LightingColor LightingOverlay::getLightingColor(const mc::BlockPos& pos, double intensity) {
	LightingData lighting = getBlockLight(pos);
	LightingColor color = calculateLightingColor(lighting);
	return color + (1-color)*(1-intensity);
}

LightingColor LightingOverlay::getCornerColor(const mc::BlockPos& pos,
		const CornerNeighbors& corner, double intensity) {
	LightingColor color = 0;
	color += getLightingColor(pos + corner.pos1, intensity) * 0.25;
	color += getLightingColor(pos + corner.pos2, intensity) * 0.25;
	color += getLightingColor(pos + corner.pos3, intensity) * 0.25;
	color += getLightingColor(pos + corner.pos4, intensity) * 0.25;
	return color;
}

CornerColors LightingOverlay::getCornerColors(const mc::BlockPos& pos,
		const FaceCorners& corners, double intensity) {
	if (intensity < 0)
		intensity = lighting_intensity;
	CornerColors colors = {{
		getCornerColor(pos, corners.corner1, intensity),
		getCornerColor(pos, corners.corner2, intensity),
		getCornerColor(pos, corners.corner3, intensity),
		getCornerColor(pos, corners.corner4, intensity),
	}};
	return colors;
}

void LightingOverlay::doSmoothLight(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	// check if lighting faces are visible
	bool light_left = true, light_right = true, light_top = true;
	bool water = (id == 8 || id == 9) && (data & util::binary<1111>::value) == 0;
	bool under_water[3] = {false, false, false};

	if (water) {
		// DATA_{WEST,SOUTH,TOP} means for non-opaque water that there are water faces
		// on these sides, e.g. we need lighting on those sides
		if (!(data & DATA_WEST))
			light_left = false;
		if (!(data & DATA_SOUTH))
			light_right = false;
		if (!(data & DATA_TOP))
			light_top = false;
	}

	if (id == 79) {
		// DATA_{WEST,SOUTH} means for ice blocks that there are no faces on this side
		if (data & DATA_WEST)
			light_left = false;
		if (data & DATA_SOUTH)
			light_right = false;
	}

	mc::Block block;
	if (light_left) {
		block = getBlock(pos + mc::DIR_WEST);
		under_water[0] = block.id == 8 || block.id == 9;
		light_left = block.id == 0 || images->isBlockTransparent(block.id, block.data);
	}
	if (light_right) {
		block = getBlock(pos + mc::DIR_SOUTH);
		under_water[1] = block.id == 8 || block.id == 9;
		light_right = block.id == 0 || images->isBlockTransparent(block.id, block.data);
	}
	if (light_top) {
		block = getBlock(pos + mc::DIR_TOP);
		under_water[2] = block.id == 8 || block.id == 9;
		light_top = block.id == 0 || images->isBlockTransparent(block.id, block.data);
	}

	double intensity = lighting_intensity;
	double water_intensity = lighting_water_intensity; // 0.65* lighting_intensity;

	// do the lighting
	if (light_left)
		renderer->lightLeft(image, getCornerColors(pos, CORNERS_LEFT, under_water[0] ? water_intensity : intensity));

	if (light_right)
		renderer->lightRight(image, getCornerColors(pos, CORNERS_RIGHT, under_water[1] ? water_intensity : intensity));

	if (light_top)
		renderer->lightTop(image, getCornerColors(pos, CORNERS_TOP, under_water[2] ? water_intensity : intensity));
}

void LightingOverlay::doSlabLight(RGBAImage& image, const mc::BlockPos& pos,
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
		renderer->lightLeft(image, getCornerColors(pos, CORNERS_LEFT), ystart, yend);

	block = getBlock(pos + mc::DIR_SOUTH);
	if (block.id == 0 || images->isBlockTransparent(block.id, block.data))
		renderer->lightRight(image, getCornerColors(pos, CORNERS_RIGHT), ystart, yend);

	block = getBlock(pos + mc::DIR_TOP);
	if (block.id == 0 || images->isBlockTransparent(block.id, block.data))
		renderer->lightTop(image, getCornerColors(pos, CORNERS_TOP), yoff);
}

void LightingOverlay::doSimpleLight(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	uint8_t factor = getLightingColor(pos, lighting_intensity) * 255;

	int size = image.getWidth();
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			uint32_t& pixel = image.pixel(x, y);
			pixel = rgba(0, 0, 0, 255 - factor);
			/*
			if (pixel != 0)
				pixel = rgba_multiply(pixel, factor, factor, factor, 255);
			*/
		}
	}
}

} /* namespace render */
} /* namespace mapcrafter */
