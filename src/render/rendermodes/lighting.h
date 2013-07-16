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

#ifndef LIGHT_H_
#define LIGHT_H_

#include "base.h"

#include <array>

namespace mapcrafter {
namespace render {

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

// corner colors of a face - defined as array with corners topleft/topright/bottomleft/bottomright
typedef std::array<LightingColor, 4> CornerColors;

class LightingRendermode : public Rendermode {
private:
	bool day;

	void createShade(Image& image, const CornerColors& corners) const;
	
	LightingColor calculateLightingColor(uint8_t block_light, uint8_t sky_light) const;
	void estimateBlockLight(mc::Block& block, const mc::BlockPos& pos) const;
	LightingData getBlockLight(const mc::BlockPos& pos) const;

	LightingColor getLightingColor(const mc::BlockPos& pos) const;
	LightingColor getCornerColor(const mc::BlockPos& pos, const CornerNeighbors& corner) const;
	CornerColors getCornerColors(const mc::BlockPos& pos, const FaceCorners& corners) const;
	
	void lightLeft(Image& image, const CornerColors& colors,
			bool top = true, bool bottom = true) const;
	void lightRight(Image& image, const CornerColors& colors,
			bool top = true, bool bottom = true) const;
	void lightTop(Image& image, const CornerColors& colors, int yoff = 0) const;
	
	void doSlabLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	void doSimpleLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);
	void doSmoothLight(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);
public:
	LightingRendermode(RenderState& state, bool day);
	virtual ~LightingRendermode();

	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	virtual void draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* LIGHT_H_ */
