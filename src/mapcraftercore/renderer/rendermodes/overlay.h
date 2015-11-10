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


#ifndef RENDERMODES_OVERLAY_H_
#define RENDERMODES_OVERLAY_H_

#include "../rendermode.h"

#include "../image.h"

#include <string>
#include <tuple>
#include <vector>

namespace mapcrafter {

namespace config {
class MapSection;
class OverlaySection;
class WorldSection;
}

namespace renderer {

class RGBAImage;

class OverlayRenderer : public RenderModeRenderer {
public:
	OverlayRenderer();
	virtual ~OverlayRenderer();

	// TODO
	// DEPRECATED
	void setHighContrast(bool high_contrast);

	virtual void tintBlock(RGBAImage& image, RGBAPixel color) const;

	virtual void tintLeft(RGBAImage& image, RGBAPixel color) const = 0;
	virtual void tintRight(RGBAImage& image, RGBAPixel color) const = 0;
	virtual void tintTop(RGBAImage& image, RGBAPixel color, int offset) const = 0;

	static const RenderModeRendererType TYPE;

protected:
	std::tuple<int, int, int> getRecolor(RGBAPixel color) const;

	bool high_contrast;
};

/**
 * A render mode that renders an overlay on top of the blocks. You just have to implement
 * the function that returns the color for each block. Return something with alpha == 0
 * if there should be no color.
 */
class OverlayRenderMode : public RenderMode {
public:
	virtual ~OverlayRenderMode() {}

	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

	virtual void drawOverlay(RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

	virtual std::string getID() const = 0;
	virtual std::string getName() const = 0;
	virtual bool isBase() const = 0;
};

template <typename Renderer = OverlayRenderer>
class BaseOverlayRenderMode : public OverlayRenderMode, public HasRenderModeRenderer<Renderer> {
public:
	BaseOverlayRenderMode(const std::string& id, const std::string& name, bool base);
	virtual ~BaseOverlayRenderMode();

	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk);
	
	/**
	 * Dummy implementation of interface method. Returns false as default.
	 */
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual void draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual void drawOverlay(RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data);

	virtual std::string getID() const;
	virtual std::string getName() const;
	virtual bool isBase() const;

protected:
	mc::Block getBlock(const mc::BlockPos& pos, int get = mc::GET_ID | mc::GET_DATA);
	
	std::string id, name;
	bool base;

	BlockImages* images;
	mc::WorldCache* world;
	mc::Chunk** current_chunk;
};

enum class OverlayMode {
	PER_BLOCK,
	PER_FACE,
};

class TintingOverlay : public BaseOverlayRenderMode<> {
public:
	TintingOverlay(OverlayMode overlay_mode, const std::string& id, const std::string& name);
	virtual ~TintingOverlay();

	virtual void drawOverlay(RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

private:
	OverlayMode overlay_mode;
};

std::vector<std::shared_ptr<OverlayRenderMode>> createOverlays(
		const config::WorldSection& world_config, const config::MapSection& map_config,
		const std::map<std::string, config::OverlaySection>& overlays_config,
		int rotation);

template <typename Renderer>
BaseOverlayRenderMode<Renderer>::BaseOverlayRenderMode(const std::string& id, const std::string& name, bool base)
	: id(id), name(name), base(base), images(nullptr), world(nullptr), current_chunk(nullptr) {
}

template <typename Renderer>
BaseOverlayRenderMode<Renderer>::~BaseOverlayRenderMode() {
}

template <typename Renderer>
void BaseOverlayRenderMode<Renderer>::draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	RGBAImage overlay = image.emptyCopy();
	drawOverlay(image, overlay, pos, id, data);
	overlay.applyMask(image);
	image.alphaBlit(overlay, 0, 0);
}

template <typename Renderer>
void BaseOverlayRenderMode<Renderer>::drawOverlay(RGBAImage& block, RGBAImage& overlay,
		const mc::BlockPos& pos, uint16_t id, uint16_t data) {
}

template <typename Renderer>
void BaseOverlayRenderMode<Renderer>::initialize(const RenderView* render_view, 
		BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk) {
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;

	HasRenderModeRenderer<Renderer>::initializeRenderer(render_view);
}

template <typename Renderer>
bool BaseOverlayRenderMode<Renderer>::isHidden(const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	return false;
}

template <typename Renderer>
std::string BaseOverlayRenderMode<Renderer>::getID() const {
	return id;
}

template <typename Renderer>
std::string BaseOverlayRenderMode<Renderer>::getName() const {
	return name;
}

template <typename Renderer>
bool BaseOverlayRenderMode<Renderer>::isBase() const {
	return base;
}

template <typename Renderer>
mc::Block BaseOverlayRenderMode<Renderer>::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

}
}

#endif /* RENDERMODES_OVERLAY_H_ */

