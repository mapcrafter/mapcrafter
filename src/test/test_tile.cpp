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

#include "../mapcraftercore/renderer/tileset.h"

#include <map>
#include <boost/test/unit_test.hpp>

namespace renderer = mapcrafter::renderer;

#define PATH(a, b, c, d) ((((renderer::TilePath() + a) + b) + c) + d)

BOOST_AUTO_TEST_CASE(test_tilepos) {
	std::map<renderer::TilePos, renderer::TilePath> tiles;
	tiles[renderer::TilePos(0, 0)] = PATH(4, 1, 1, 1);
	tiles[renderer::TilePos(-3, -5)] = PATH(1, 2, 3, 4);
	tiles[renderer::TilePos(2, 4)] = PATH(4, 3, 2, 1);
	tiles[renderer::TilePos(-2, -2)] = PATH(1, 4, 4, 1);
	tiles[renderer::TilePos(-3, 4)] = PATH(3, 4, 1, 2);
	tiles[renderer::TilePos(-8, 2)] = PATH(3, 1, 3, 1);
	tiles[renderer::TilePos(-5, 6)] = PATH(3, 3, 4, 2);

	for (std::map<renderer::TilePos, renderer::TilePath>::const_iterator it = tiles.begin();
	        it != tiles.end(); ++it) {
		BOOST_CHECK_EQUAL(it->first, it->second.getTilePos());
		BOOST_CHECK_EQUAL(it->second, renderer::TilePath::byTilePos(it->first, 4));
	}

	// check consistence of < operator of Path
	std::set<renderer::TilePath> paths;
	for(int x = -8; x <= 8; x++) {
		for(int y = -8; y <= 8; y++) {
			paths.insert(renderer::TilePath::byTilePos(renderer::TilePos(x, y), 4));
		}
	}
	BOOST_CHECK_EQUAL(paths.size(), 256);
}
