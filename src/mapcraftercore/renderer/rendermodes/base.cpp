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

#include "base.h"

#include "../../mc/world.h"

namespace mapcrafter {
namespace renderer {

Rendermode::Rendermode()
	: current_chunk(nullptr) {
}

void Rendermode::initialize(std::shared_ptr<BlockImages> images,
		std::shared_ptr<mc::WorldCache> world, mc::Chunk** current_chunk) {
	this->images = images;
	this->world = world;
	this->current_chunk = current_chunk;
}

Rendermode::~Rendermode() {
}

void Rendermode::start() {
}

void Rendermode::end() {
}

bool Rendermode::isHidden(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	return false;
}

void Rendermode::draw(RGBAImage& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
}

mc::Block Rendermode::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, *current_chunk, get);
}

bool createRendermode(const config::WorldSection& world_config,
		const config::MapSection& map_config,
		std::vector<std::shared_ptr<Rendermode>>& modes) {
	std::string name = map_config.getRendermode();
	if (name.empty() || name == "plain")
		return true;

	if (name == "cave")
		modes.push_back(std::shared_ptr<Rendermode>(new CaveRendermode(
				map_config.hasCaveHighContrast())));
	else if (name == "daylight")
		modes.push_back(std::shared_ptr<Rendermode>(new LightingRendermode(
				true, map_config.getLightingIntensity(),
				world_config.getDimension() == mc::Dimension::END)));
	else if (name == "nightlight")
		modes.push_back(std::shared_ptr<Rendermode>(new LightingRendermode(
				false, map_config.getLightingIntensity(),
				world_config.getDimension() == mc::Dimension::END)));
	else
		return false;
	return true;
}

} /* namespace render */
} /* namespace mapcrafter */
