/*
 * Copyright 2012 Moritz Hilscher
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

#include "render/textures.h"

#include <iostream>

#include "util.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main(int argc, char **argv) {
	mapcrafter::render::BlockTextures textures;
	if (!textures.loadOther("data/fire.png", "data/endportal.png")) {
		std::cerr << "Unable to load fire.png or endportal.png!" << std::endl;
		return 1;
	}

	textures.setSettings(16, true, true);
	if (!textures.loadBlocks("data/terrain.png")) {
		std::cerr << "Unable to load terrain.png!" << std::endl;
		return 1;
	}

	textures.saveBlocks("blocks.png");
}
