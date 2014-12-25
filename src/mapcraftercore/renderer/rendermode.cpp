/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "../mc/world.h"

namespace mapcrafter {
namespace renderer {

RenderMode::RenderMode()
	: current_chunk(nullptr) {
}

void RenderMode::initialize(std::shared_ptr<BlockImages> images,
		std::shared_ptr<mc::WorldCache> world, mc::Chunk** current_chunk) {
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;
}

RenderMode::~RenderMode() {
}

void RenderMode::start() {
}

void RenderMode::end() {
}

bool RenderMode::isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	return false;
}

void RenderMode::draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
}

mc::Block RenderMode::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

bool createRenderModes(const config::WorldSection& world_config,
		const config::MapSection& map_config, RenderModes& modes) {
	std::string name = map_config.getRendermode();
	if (name.empty() || name == "plain")
		return true;

	if (name == "cave")
		modes.push_back(std::shared_ptr<RenderMode>(new CaveRenderMode(
				map_config.hasCaveHighContrast())));
	else if (name == "daylight")
		modes.push_back(std::shared_ptr<RenderMode>(new LightingRenderMode(
				true, map_config.getLightingIntensity(),
				world_config.getDimension() == mc::Dimension::END)));
	else if (name == "nightlight")
		modes.push_back(std::shared_ptr<RenderMode>(new LightingRenderMode(
				false, map_config.getLightingIntensity(),
				world_config.getDimension() == mc::Dimension::END)));
	else
		return false;
	return true;
}

} /* namespace render */
} /* namespace mapcrafter */
