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

#include "render/tile.h"

#include <map>
#include <boost/test/unit_test.hpp>

namespace render = mapcrafter::render;

#define PATH(a, b, c, d) ((((render::Path() + a) + b) + c) + d)

BOOST_AUTO_TEST_CASE(test_tilepos) {
	std::map<render::TilePos, render::Path> tiles;
	tiles[render::TilePos(0, 0)] = PATH(4, 1, 1, 1);
	tiles[render::TilePos(-3, -5)] = PATH(1, 2, 3, 4);
	tiles[render::TilePos(2, 4)] = PATH(4, 3, 2, 1);
	tiles[render::TilePos(-2, -2)] = PATH(1, 4, 4, 1);
	tiles[render::TilePos(-3, 4)] = PATH(3, 4, 1, 2);
	tiles[render::TilePos(-8, 2)] = PATH(3, 1, 3, 1);
	tiles[render::TilePos(-5, 6)] = PATH(3, 3, 4, 2);

	for (std::map<render::TilePos, render::Path>::const_iterator it = tiles.begin();
	        it != tiles.end(); ++it) {
		BOOST_CHECK_EQUAL(it->first, it->second.getTilePos());
		BOOST_CHECK_EQUAL(it->second, render::Path::byTilePos(it->first, 4));
	}

	// check consistence of < operator of Path
	std::set<render::Path> paths;
	for(int x = -8; x <= 8; x++) {
		for(int y = -8; y <= 8; y++) {
			paths.insert(render::Path::byTilePos(render::TilePos(x, y), 4));
		}
	}
	BOOST_CHECK_EQUAL(paths.size(), 256);
}
