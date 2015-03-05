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

#ifndef RENDERMODE_H_
#define RENDERMODE_H_

#include "../mc/worldcache.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace mapcrafter {

// some forward declarations
namespace config {
class MapSection;
class WorldSection;
}

namespace mc {
struct Block;
class BlockPos;
class Chunk;
}

namespace renderer {

class BlockImages;
class RGBAImage;

/**
 * A simple interface to implement different rendermodes.
 */
class RenderMode {
public:
	virtual ~RenderMode() {}

	virtual void initialize(BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk) = 0;

	// is called when the tile renderer starts rendering a tile
	virtual void start() = 0;
	// is called when the tile renderer finished rendering a tile
	virtual void end() = 0;

	// is called to allow the rendermode to hide specific blocks
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;
	// is called to allow the rendermode to change a block image
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;
};

class AbstractRenderMode : public RenderMode {
public:
	AbstractRenderMode();
	virtual ~AbstractRenderMode();

	virtual void initialize(BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk);

	virtual void start();
	virtual void end();

	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);

	BlockImages* images;
	mc::WorldCache* world;
	mc::Chunk** current_chunk;
};

/**
 * A render mode which combines multiple render modes into one.
 *
 * TODO comments blah blah
 */
class MultiplexingRenderMode : public RenderMode {
public:
	virtual ~MultiplexingRenderMode();

	void addRenderMode(RenderMode* render_mode);

	virtual void initialize(BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk);

	virtual void start();
	virtual void end();

	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	std::vector<RenderMode*> render_modes;
};

enum class RenderModeType {
	PLAIN,
	DAYLIGHT,
	NIGHTLIGHT,
	CAVE,
	CAVELIGHT
};

std::ostream& operator<<(std::ostream& out, RenderModeType render_mode);

RenderMode* createMapRenderMode(const config::WorldSection& world_config,
		const config::MapSection& map_config);

} /* namespace render */
} /* namespace mapcrafter */

#endif /* RENDERMODE_H_ */
