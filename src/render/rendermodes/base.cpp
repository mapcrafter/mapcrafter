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

#include "base.h"

namespace mapcrafter {
namespace render {

Rendermode::Rendermode(RenderState& state)
		: state(state) {
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

void Rendermode::draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint16_t data) {
}

bool createRendermode(const std::string& name, const RenderWorldConfig& config,
		RenderState& state, std::vector<std::unique_ptr<Rendermode>>& modes) {
	if (name.empty() || name == "normal")
		return true;

	if (name == "cave")
		modes.push_back(std::unique_ptr<Rendermode>(new CaveRendermode(state)));
	else if (name == "daylight")
		modes.push_back(std::unique_ptr<Rendermode>(new LightingRendermode(state, true)));
	else if (name == "nightlight")
		modes.push_back(std::unique_ptr<Rendermode>(new LightingRendermode(state, false)));
	else
		return false;
	return true;
}

} /* namespace render */
} /* namespace mapcrafter */
