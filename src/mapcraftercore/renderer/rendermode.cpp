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

#include "rendermode.h"

#include "blockimages.h"
#include "image.h"
#include "rendermodes/cave.h"
#include "rendermodes/lighting.h"
#include "rendermodes/heighttinting.h"
#include "../config/configsections/map.h"
#include "../config/configsections/world.h"
#include "../mc/chunk.h"
#include "../mc/pos.h"
#include "../util.h"

namespace mapcrafter {
namespace renderer {

AbstractRenderMode::AbstractRenderMode()
	: images(nullptr), world(nullptr), current_chunk(nullptr) {
}

AbstractRenderMode::~AbstractRenderMode() {
}

void AbstractRenderMode::initialize(BlockImages* images, mc::WorldCache* world,
		mc::Chunk** current_chunk) {
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;
}

void AbstractRenderMode::start() {
}

void AbstractRenderMode::end() {
}

bool AbstractRenderMode::isHidden(const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	return false;
}

void AbstractRenderMode::draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
}

mc::Block AbstractRenderMode::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

MultiplexingRenderMode::~MultiplexingRenderMode() {
	for (auto it = render_modes.begin(); it != render_modes.end(); ++it)
		delete *it;
}

void MultiplexingRenderMode::addRenderMode(RenderMode* render_mode) {
	render_modes.push_back(render_mode);
}

void MultiplexingRenderMode::initialize(BlockImages* images, mc::WorldCache* world,
		mc::Chunk** current_chunk) {
	for (auto it = render_modes.begin(); it != render_modes.end(); ++it)
		(*it)->initialize(images, world, current_chunk);
}

void MultiplexingRenderMode::start() {
	for (auto it = render_modes.begin(); it != render_modes.end(); ++it)
		(*it)->start();
}

void MultiplexingRenderMode::end() {
	for (auto it = render_modes.begin(); it != render_modes.end(); ++it)
		(*it)->end();
}

bool MultiplexingRenderMode::isHidden(const mc::BlockPos& pos, uint16_t id,
		uint16_t data) {
	for (auto it = render_modes.begin(); it != render_modes.end(); ++it)
		if ((*it)->isHidden(pos, id, data))
			return true;
	return false;
}

void MultiplexingRenderMode::draw(RGBAImage& image, const mc::BlockPos& pos,
		uint16_t id, uint16_t data) {
	for (auto it = render_modes.begin(); it != render_modes.end(); ++it)
		(*it)->draw(image, pos, id, data);
}

RenderMode* createRenderMode(const config::WorldSection& world_config,
		const config::MapSection& map_config) {
	std::string name = map_config.getRenderMode();
	// TODO maybe use a dummy render mode here instead of the empty abstract one?
	if (name.empty() || name == "plain")
		return new AbstractRenderMode();

	if (name == "cave" || name == "cavelight") {
		MultiplexingRenderMode* render_mode = new MultiplexingRenderMode();
		// hide some walls of caves which would cover the view into the caves
		if (map_config.getRenderView() == RenderViewType::ISOMETRIC)
			render_mode->addRenderMode(new CaveRenderMode({mc::DIR_SOUTH, mc::DIR_WEST, mc::DIR_TOP}));
		else
			render_mode->addRenderMode(new CaveRenderMode({mc::DIR_TOP}));
		// if we want some shadows, then simulate the sun light because it's dark in caves
		if (name == "cavelight")
			render_mode->addRenderMode(new LightingRenderMode(true, map_config.getLightingIntensity(), true));
		render_mode->addRenderMode(new HeightTintingRenderMode(map_config.hasCaveHighContrast()));
		return render_mode;
	}
	else if (name == "daylight")
		return new LightingRenderMode(true, map_config.getLightingIntensity(),
				world_config.getDimension() == mc::Dimension::END);
	else if (name == "nightlight")
		return new LightingRenderMode(false, map_config.getLightingIntensity(),
				world_config.getDimension() == mc::Dimension::END);
	// TODO assertion/validation is needed if this returns null!
	return nullptr;
}

} /* namespace render */
} /* namespace mapcrafter */
