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

#ifndef RENDERMODES_LIGHTING_H_
#define RENDERMODES_LIGHTING_H_

#include "../rendermode.h"

#include <array>

namespace mapcrafter {
namespace renderer {

/**
 * Lighting rendermode implementation.
 *
 * This does basically the same things Minecraft Overviewer does.
 */

/**
 * This structure defines the four neighbor blocks of a face corner to calculate the lighting
 * color. The neighbor blocks are all specified as relative positions to the block the
 * corner belongs to.
 *
 * The neighbors are defined by specifying the first position and two other directions
 * where the other neighbors are.
 *
 * The addPos-method is used to move all these neighbors in a specified direction.
 */
struct CornerNeighbors {
	mc::BlockPos pos1, pos2, pos3, pos4;
	mc::BlockPos dir1, dir2;
	
	CornerNeighbors();
	CornerNeighbors(const mc::BlockPos& pos1, const mc::BlockPos& dir1, const mc::BlockPos& dir2);
	
	CornerNeighbors addPos(const mc::BlockPos& pos) const;
};

/**
 * This structure defines the four corners of a block face.
 *
 * The four corners are defined by specifying the first corner. The other corners are
 * calculated by moving this corner to the directions of his neighbors.
 */
struct FaceCorners {
	CornerNeighbors corner1, corner2, corner3, corner4;
	
	FaceCorners();
	FaceCorners(const CornerNeighbors& corner1);
};

extern const FaceCorners CORNERS_LEFT, CORNERS_RIGHT, CORNERS_TOP, CORNERS_BOTTOM;

struct LightingData {
	uint8_t block, sky;
};

typedef double LightingColor;

// corner colors of a face
// - defined as array with corners top left / top right / bottom left / bottom right
typedef std::array<LightingColor, 4> CornerColors;

void drawBottomTriangle(RGBAImage& image, int size, double c1, double c2, double c3);
void drawTopTriangle(RGBAImage& image, int size, double c1, double c2, double c3);

class LightingRenderMode : public RenderMode {
public:
	LightingRenderMode(bool day, double lighting_intensity, bool dimension_end);
	virtual ~LightingRenderMode();

	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

private:
	bool day;
	double lighting_intensity;
	bool dimension_end;

	bool render_view_initialized;
	bool isometric_render_view;

	/**
	 * Draws the shade of the corners by drawing two triangles with the supplied colors.
	 */
	void createShade(RGBAImage& image, const CornerColors& corners) const;
	
	/**
	 * Calculates the color of the light of a block.
	 *
	 * This uses the formula 0.8**(15 - max(block_light, sky_light))
	 * When calculating nightlight, the skylight is reduced by 11.
	 */
	LightingColor calculateLightingColor(const LightingData& light) const;

	/**
	 * Estimates the light of a block from its neighbors.
	 */
	LightingData estimateLight(const mc::BlockPos& pos);

	/**
	 * Returns the light of a block (sky/block light). This also means that the light is
	 * estimated if the block is a special transparent block.
	 */
	LightingData getBlockLight(const mc::BlockPos& pos);

	/**
	 * Returns the lighting color of a block.
	 */
	LightingColor getLightingColor(const mc::BlockPos& pos);

	/**
	 * Returns the lighting color of a corner by calculating the average lighting color of
	 * the four neighbor blocks.
	 */
	LightingColor getCornerColor(const mc::BlockPos& pos, const CornerNeighbors& corner);

	/**
	 * Returns the corner lighting colors of a block face.
	 */
	CornerColors getCornerColors(const mc::BlockPos& pos, const FaceCorners& corners);

	/**
	 * Adds smooth lighting to the left face of a block image, but only a part of the
	 * face (specify y_start, y_end, used for slab lighting for example).
	 */
	void lightLeft(RGBAImage& image, const CornerColors& colors, int y_start, int y_end);

	/**
	 * Adds smooth lighting to the left face of a block image.
	 */
	void lightLeft(RGBAImage& image, const CornerColors& colors);

	/**
	 * Adds smooth lighting to the right face of a block image, but only a part of the
	 * face (specify y_start, y_end, used for slab lighting for example).
	 */
	void lightRight(RGBAImage& image, const CornerColors& colors, int y_start, int y_end);

	/**
	 * Adds smooth lighting to the right face of a block image.
	 */
	void lightRight(RGBAImage& image, const CornerColors& colors);

	/**
	 * Adds smooth lighting to the top face of a block image.
	 */
	void lightTop(RGBAImage& image, const CornerColors& colors, int yoff = 0);
	
	/**
	 * Applies the smooth lighting to a block by adding lighting to the top, left and
	 * right face (if not covered by another, not transparent, block).
	 */
	void doSmoothLight(RGBAImage& image, const mc::BlockPos& pos,
			uint16_t id, uint16_t data);

	/**
	 * Applies the smooth lighting to a slab (not double slabs).
	 */
	void doSlabLight(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	/**
	 * Applies a simple lighting to a block by coloring the whole block with the lighting
	 * color of the block.
	 */
	void doSimpleLight(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);
};

} /* namespace render */
} /* namespace mapcrafter */

#endif /* RENDERMODES_LIGHTING_H_ */
