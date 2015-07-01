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

#include "renderview.h"
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

	/**
	 * This method is called by the tile renderer so you can modify block images that
	 * are about to be rendered.
	 */
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id,
			uint16_t data) = 0;
};

/**
 * Types of render mode renderers that are available for render modes.
 */
enum class RenderModeRendererType {
	DUMMY,
	LIGHTING,
    OVERLAY,
};

/**
 * Empty class just to have a base class for the rendering part of each render mode.
 */
class RenderModeRenderer {
public:
	virtual ~RenderModeRenderer();

	// every render mode renderer needs a ...
	// static const RenderModeRendererType TYPE;
};

/**
 * An empty dummy render mode renderer for render modes that don't have a rendering part.
 */
class DummyRenderer : public RenderModeRenderer {
public:
	virtual ~DummyRenderer();

	static const RenderModeRendererType TYPE;
};

/**
 * The base render mode class already implements handling of the initialize-method and
 * some other stuff (a comfortable getBlock-method that takes the current_chunk into
 * account).
 *
 * Also there is a per render view specific renderer for each render mode that wants to
 * modify the block images that are about to be rendered. The base render mode class is
 * a template class with the abstract renderer class as template argument. Each render
 * view has to provide an implementation of this renderer class. The base render mode
 * class calls the createRenderModeRenderer-method of the render view, casts the
 * obtained renderer to the specified template class and stores it in the 'renderer'
 * variable. If a base render mode doesn't need a renderer, just specify the
 * DummyRenderer class as renderer in the template.
 */
template <typename Renderer = DummyRenderer>
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
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);

	RenderModeRenderer* renderer_ptr;
	Renderer* renderer;

	BlockImages* images;
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

	/**
	 * Calls this method of each render mode.
	 */
	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

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
	SPAWN
};

std::ostream& operator<<(std::ostream& out, RenderModeType render_mode);
std::ostream& operator<<(std::ostream& out, OverlayType overlay);

/**
 * Creates the render mode for a map config section.
 */
RenderMode* createRenderMode(const config::WorldSection& world_config,
		const config::MapSection& map_config, int rotation);

template <typename Renderer>
BaseRenderMode<Renderer>::BaseRenderMode()
	: renderer_ptr(nullptr), images(nullptr), world(nullptr), current_chunk(nullptr) {
}

template <typename Renderer>
BaseRenderMode<Renderer>::~BaseRenderMode() {
	if (renderer_ptr != nullptr)
		delete renderer_ptr;
}

template <typename Renderer>
void BaseRenderMode<Renderer>::initialize(const RenderView* render_view, 
		BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk) {
	// create the render mode renderer by calling the render view factory method
	// for this renderer type
	this->renderer_ptr = render_view->createRenderModeRenderer(Renderer::TYPE);
	// try to cast it to the right subclass, make sure that works if there is a renderer
	this->renderer = dynamic_cast<Renderer*>(renderer_ptr);
	if (Renderer::TYPE != RenderModeRendererType::DUMMY)
		assert(renderer);
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;
}

template <typename Renderer>
bool BaseRenderMode<Renderer>::isHidden(const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	return false;
}

template <typename Renderer>
void BaseRenderMode<Renderer>::draw(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
}

template <typename Renderer>
mc::Block BaseRenderMode<Renderer>::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

} /* namespace render */
} /* namespace mapcrafter */

#endif /* RENDERMODE_H_ */
