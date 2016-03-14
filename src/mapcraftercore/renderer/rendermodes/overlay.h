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


#ifndef RENDERMODES_OVERLAY_H_
#define RENDERMODES_OVERLAY_H_

#include "../blockimages.h"
#include "../rendermode.h"
#include "../image.h"
#include "../../config/configsections/overlay.h"

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

template <typename Renderer, typename Config>
class BaseOverlayRenderMode : public OverlayRenderMode,
	public HasRenderModeRenderer<Renderer>, public HasConfigSection<Config> {
public:
	BaseOverlayRenderMode(std::shared_ptr<config::ConfigSection> config);
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

template <typename Config>
class TintingOverlay : public BaseOverlayRenderMode<OverlayRenderer, Config> {
public:
	TintingOverlay(OverlayMode overlay_mode, std::shared_ptr<config::ConfigSection> config);

	virtual void drawOverlay(RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

private:
	OverlayMode overlay_mode;
};

std::vector<std::shared_ptr<OverlayRenderMode>> createOverlays(
		const config::WorldSection& world_config, const config::MapSection& map_config,
		const std::map<std::string, std::shared_ptr<config::OverlaySection>>& overlays_config,
		int rotation);

template <typename Renderer, typename Config>
BaseOverlayRenderMode<Renderer, Config>::BaseOverlayRenderMode(std::shared_ptr<config::ConfigSection> config)
	: images(nullptr), world(nullptr), current_chunk(nullptr) {
	HasConfigSection<Config>::initializeConfig(config);
}

template <typename Renderer, typename Config>
BaseOverlayRenderMode<Renderer, Config>::~BaseOverlayRenderMode() {
}

template <typename Renderer, typename Config>
void BaseOverlayRenderMode<Renderer, Config>::draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	RGBAImage overlay = image.emptyCopy();
	drawOverlay(image, overlay, pos, id, data);
	overlay.applyMask(image);
	image.alphaBlit(overlay, 0, 0);
}

template <typename Renderer, typename Config>
void BaseOverlayRenderMode<Renderer, Config>::drawOverlay(RGBAImage& block, RGBAImage& overlay,
		const mc::BlockPos& pos, uint16_t id, uint16_t data) {
}

template <typename Renderer, typename Config>
void BaseOverlayRenderMode<Renderer, Config>::initialize(const RenderView* render_view, 
		BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk) {
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;

	HasRenderModeRenderer<Renderer>::initializeRenderer(render_view);
}

template <typename Renderer, typename Config>
bool BaseOverlayRenderMode<Renderer, Config>::isHidden(const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	return false;
}

template <typename Renderer, typename Config>
std::string BaseOverlayRenderMode<Renderer, Config>::getID() const {
	return this->config->getID();
}

template <typename Renderer, typename Config>
std::string BaseOverlayRenderMode<Renderer, Config>::getName() const {
	return this->config->getName();
}

template <typename Renderer, typename Config>
bool BaseOverlayRenderMode<Renderer, Config>::isBase() const {
	return this->config->isBase();
}

template <typename Renderer, typename Config>
mc::Block BaseOverlayRenderMode<Renderer, Config>::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

template <typename Config>
TintingOverlay<Config>::TintingOverlay(OverlayMode overlay_mode,
		std::shared_ptr<config::ConfigSection> config)
	: BaseOverlayRenderMode<OverlayRenderer, Config>(config), overlay_mode(overlay_mode) {
}

template <typename Config>
void TintingOverlay<Config>::drawOverlay(RGBAImage& block, RGBAImage& overlay,
		const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	if (overlay_mode == OverlayMode::PER_BLOCK) {
		// simple mode where we just tint whole blocks
		RGBAPixel color = this->getBlockColor(pos, id, data);
		if (rgba_alpha(color) == 0)
			return;
		this->renderer->tintBlock(overlay, color);
	} else {
		// "advanced" mode where each block/position has a color,
		// and adjacent faces are tinted / or the transparent blocks themselves
		// TODO potential for optimization, maybe cache colors of blocks?
		if (this->images->isBlockTransparent(id, data)) {
			RGBAPixel color = getBlockColor(pos, id, data);
			if (rgba_alpha(color) == 0)
				return;
			this->renderer->tintBlock(overlay, color);
		} else {
			mc::Block top, left, right;
			RGBAPixel color_top, color_left, color_right;
			top = this->getBlock(pos + mc::DIR_TOP, mc::GET_ID | mc::GET_DATA);
			left = this->getBlock(pos + mc::DIR_WEST, mc::GET_ID | mc::GET_DATA);
			right = this->getBlock(pos + mc::DIR_SOUTH, mc::GET_ID | mc::GET_DATA);
			color_top = getBlockColor(pos + mc::DIR_TOP, top.id, top.data);
			color_left = getBlockColor(pos + mc::DIR_WEST, left.id, left.data);
			color_right = getBlockColor(pos + mc::DIR_SOUTH, right.id, right.data);
			
			if (rgba_alpha(color_top) != 0)
				this->renderer->tintTop(overlay, color_top, 0);
			if (rgba_alpha(color_left) != 0)
				this->renderer->tintLeft(overlay, color_left);
			if (rgba_alpha(color_right) != 0)
				this->renderer->tintRight(overlay, color_right);
		}
	}
}

}
}

#endif /* RENDERMODES_OVERLAY_H_ */

