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

#include "render/textures.h"

#include <iostream>

using namespace mapcrafter::render;

int main(int argc, char **argv) {
	mapcrafter::render::BlockImages images;

	images.setSettings(16, 0, true, true);
	if(!images.loadChests("data/textures/chest.png", "data/textures/largechest.png",
			"data/textures/enderchest.png"))
		std::cerr << "Unable to load chest.png, largechest.png or enderchest.png" << std::endl;
	else if (!images.loadOther("data/textures/fire.png", "data/textures/endportal.png"))
		std::cerr << "Unable to load fire.png or endportal.png!" << std::endl;
	else if (!images.loadColors("data/textures/foliagecolor.png", "data/textures/grasscolor.png"))
		std::cerr << "Unable to load foliagecolor.png or grasscolor.png!" << std::endl;
	else if (!images.loadBlocks("data/textures/blocks"))
		std::cerr << "Unable to load blocks!" << std::endl;
	else {
		images.saveBlocks("blocks.png");
		return 0;
	}

	return 1;
}
