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


#ifndef OVERLAY_H_
#define OVERLAY_H_

#include "blockimages.h"
#include "rendermode.h"
#include "image.h"
#include "../config/configsections/overlay.h"

#include <iosfwd>
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

template <typename Config>
class HasConfig {
public:
	void initializeConfig(std::shared_ptr<config::ConfigSection> config_section);

protected:
	std::shared_ptr<config::ConfigSection> config_ptr;
	const Config* config;
};

/**
 * Interface for an overlay that is used as additional map layer on top of the rendered
 * terrain. You get some information about the world and the current block and draw
 * on an image how the overlay on top of the block would look like.
 */
class Overlay {
public:
	virtual ~Overlay() {}

	/**
	 * Passes the overlay some required data about the map and the world to render.
	 */
	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk) = 0;

	/**
	 * This method lets the overlay draw its overlay for a given block. You just have to
	 * draw the transparent part (or not even transparent) that should be displayed on
	 * top of that specific block on the overlay image.
	 *
	 * The renderer makes sure that overlay pixels are only visible if there is a
	 * corresponding pixel on the block image, so you could fill the overlay image with
	 * just one color for example.
	 */
	virtual void drawOverlay(const RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos,
			uint16_t id, uint16_t data) = 0;

	virtual std::string getID() const = 0;
	virtual std::string getName() const = 0;
	virtual bool isBase() const = 0;
};

/**
 * An overlay that's already implementing the storing of the given world and accessing it.
 * You just have to implement the drawOverlay-method.
 */
template <typename Renderer, typename Config>
class AbstractOverlay : public Overlay, public HasRenderModeRenderer<Renderer>, public HasConfig<Config> {
public:
	AbstractOverlay(std::shared_ptr<config::ConfigSection> overlay_config);
	virtual ~AbstractOverlay();

	virtual void initialize(const RenderView* render_view, BlockImages* images,
			mc::WorldCache* world, mc::Chunk** current_chunk);
	
	virtual void drawOverlay(const RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data) = 0;

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

/**
 * Different working modes for the tinting overlay:
 * - per block: Whole blocks are overlayed with the same color
 * - per face: Each face of each block can get another color
 */
enum class TintingOverlayMode {
	PER_BLOCK,
	PER_FACE,
};

/**
 * An overlay that's generating the overlay image for you. You just have to specify a
 * color for each block / face.
 */
template <typename Config>
class TintingOverlay : public AbstractOverlay<OverlayRenderer, Config> {
public:
	/**
	 * Constructor. You have to specify the working mode of the tinting overlay.
	 */
	TintingOverlay(TintingOverlayMode overlay_mode, std::shared_ptr<config::ConfigSection> overlay_config);

	virtual void drawOverlay(const RGBAImage& block, RGBAImage& overlay, const mc::BlockPos& pos, uint16_t id, uint16_t data);

protected:
	/**
	 * Depending on the mode of the tinting overlay, should return:
	 * - If per-block-tinting: Color for block xyz (getBlockColor(xyz, xyz, ...))
	 * - If per-face-tinting: Color for face of block xyz seen from block abc (getBlockColor(abc, xyz, ...))
	 */
	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, const mc::BlockPos& pos_for, uint16_t id, uint16_t data) = 0;

private:
	TintingOverlayMode overlay_mode;
};

Overlay* createOverlay(const config::WorldSection& world_config,
		const config::MapSection& map_config,
		std::shared_ptr<config::OverlaySection> overlay_config, int rotation);
	
std::vector<std::shared_ptr<Overlay>> createOverlays(
		const config::WorldSection& world_config, const config::MapSection& map_config,
		const std::map<std::string, std::shared_ptr<config::OverlaySection>>& overlays_config,
		int rotation);

template <typename Config>
void HasConfig<Config>::initializeConfig(std::shared_ptr<config::ConfigSection> config_section) {
	config_ptr = config_section;
	config = dynamic_cast<Config*>(config_ptr.get());
	assert(this->config);
}

template <typename Renderer, typename Config>
AbstractOverlay<Renderer, Config>::AbstractOverlay(std::shared_ptr<config::ConfigSection> overlay_config)
	: images(nullptr), world(nullptr), current_chunk(nullptr) {
	HasConfig<Config>::initializeConfig(overlay_config);
}

template <typename Renderer, typename Config>
AbstractOverlay<Renderer, Config>::~AbstractOverlay() {
}

template <typename Renderer, typename Config>
void AbstractOverlay<Renderer, Config>::initialize(const RenderView* render_view, 
		BlockImages* images, mc::WorldCache* world, mc::Chunk** current_chunk) {
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;

	HasRenderModeRenderer<Renderer>::initializeRenderer(render_view);
}

template <typename Renderer, typename Config>
std::string AbstractOverlay<Renderer, Config>::getID() const {
	return this->config->getID();
}

template <typename Renderer, typename Config>
std::string AbstractOverlay<Renderer, Config>::getName() const {
	return this->config->getName();
}

template <typename Renderer, typename Config>
bool AbstractOverlay<Renderer, Config>::isBase() const {
	return this->config->isBase();
}

template <typename Renderer, typename Config>
mc::Block AbstractOverlay<Renderer, Config>::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

template <typename Config>
TintingOverlay<Config>::TintingOverlay(TintingOverlayMode overlay_mode,
		std::shared_ptr<config::ConfigSection> overlay_config)
	: AbstractOverlay<OverlayRenderer, Config>(overlay_config), overlay_mode(overlay_mode) {
}

template <typename Config>
void TintingOverlay<Config>::drawOverlay(const RGBAImage& block, RGBAImage& overlay,
		const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	if (overlay_mode == TintingOverlayMode::PER_BLOCK) {
		// simple mode where we just tint whole blocks
		RGBAPixel color = this->getBlockColor(pos, pos, id, data);
		if (rgba_alpha(color) == 0)
			return;
		this->renderer->tintBlock(overlay, color);
	} else {
		// "advanced" mode where each face gets a color
		// (and transparent blocks a single color)
		if (this->images->isBlockTransparent(id, data)) {
			RGBAPixel color = getBlockColor(pos, pos, id, data);
			if (rgba_alpha(color) == 0)
				return;
			this->renderer->tintBlock(overlay, color);
		} else {
			mc::Block top, left, right;
			RGBAPixel color_top, color_left, color_right;
			top = this->getBlock(pos + mc::DIR_TOP, mc::GET_ID | mc::GET_DATA);
			left = this->getBlock(pos + mc::DIR_WEST, mc::GET_ID | mc::GET_DATA);
			right = this->getBlock(pos + mc::DIR_SOUTH, mc::GET_ID | mc::GET_DATA);
			color_top = getBlockColor(pos + mc::DIR_TOP, pos, top.id, top.data);
			color_left = getBlockColor(pos + mc::DIR_WEST, pos, left.id, left.data);
			color_right = getBlockColor(pos + mc::DIR_SOUTH, pos, right.id, right.data);
			
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

#endif /* OVERLAY_H_ */

