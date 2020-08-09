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

#ifndef RENDERMODE_H_
#define RENDERMODE_H_

#include "renderview.h"
// TODO forward-declare once template is gone!
#include "../renderer/blockimages.h"
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
class BlockImage;
class RGBAImage;

/**
 * A simple interface to implement different render modes.
 */
class RenderMode {
public:
	virtual ~RenderMode() {}

	/**
	 * Sets stuff (block images and world cache) that is required for the render mode
	 * to operate. There is a pointer to the current chunk that is used by the tile
	 * renderer, that way you (mostly) don't need to access the world cache.
	 *
	 * The render view is required because some render modes need render view specific
	 * methods to modify the block images.
	 */
	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk) = 0;

	/**
	 * This method is called by the tile renderer to check if a block should be hidden.
	 */
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

	virtual bool isHidden(const mc::BlockPos& pos, const BlockImage& block_image) { return false; }

	/**
	 * This method is called by the tile renderer so you can modify block images that
	 * are about to be rendered.
	 */
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id,
			uint16_t data) = 0;

	virtual void draw(RGBAImage& image, const BlockImage& block_image,
			const mc::BlockPos& pos, uint16_t id) {}
};

/**
 * The base render mode class already implements handling of the initialize-method and
 * some other stuff (a comfortable getBlock-method that takes the current_chunk into
 * account).
 */
class BaseRenderMode : public RenderMode {
public:
	BaseRenderMode();
	virtual ~BaseRenderMode();

	/**
	 * Stores the supplied stuff from the tile renderer and creates the render mode
	 * renderer with the render view.
	 */
	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk);

	/**
	 * Dummy implementation of interface method. Returns false as default.
	 */
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);

	/**
	 * Dummy implementation of interface method.
	 */
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID);

	BlockImages* images;
	RenderedBlockImages* block_images;
	mc::WorldCache* world;
	mc::Chunk** current_chunk;
};

/**
 * This is a class for a render mode that combines multiple render modes into one.
 */
class MultiplexingRenderMode : public RenderMode {
public:
	virtual ~MultiplexingRenderMode();

	/**
	 * Adds a render mode. The supplied render mode is destroyed when this multiplexing
	 * render mode is destroyed.
	 */
	void addRenderMode(RenderMode* render_mode);

	/**
	 * Passes the supplied render data to the render modes.
	 */
	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk);

	/**
	 * Calls this method of each render mode and returns true if one render mode returns
	 * true (= false is default).
	 */
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual bool isHidden(const mc::BlockPos& pos, const BlockImage& block_image);

	/**
	 * Calls this method of each render mode.
	 */
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual void draw(RGBAImage& image, const BlockImage& block_image, const mc::BlockPos& pos, uint16_t id);

protected:
	std::vector<RenderMode*> render_modes;
};

/**
 * Types of (of other base render modes composed) render modes that are available for
 * the user.
 */
enum class RenderModeType {
	PLAIN,
	DAYLIGHT,
	NIGHTLIGHT,
	CAVE,
	CAVELIGHT
};

enum class OverlayType {
	NONE,
	SLIME,
	SPAWNDAY,
	SPAWNNIGHT,
};

std::ostream& operator<<(std::ostream& out, RenderModeType render_mode);
std::ostream& operator<<(std::ostream& out, OverlayType overlay);

/**
 * Creates the render mode for a map config section.
 */
RenderMode* createRenderMode(const config::WorldSection& world_config,
		const config::MapSection& map_config, int rotation);

} /* namespace render */
} /* namespace mapcrafter */

#endif /* RENDERMODE_H_ */
