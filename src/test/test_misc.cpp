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

#include "../mapcraftercore/renderer/rendermodes/slimeoverlay.h"

#include <set>
#include <boost/test/unit_test.hpp>

using namespace mapcrafter::renderer;
namespace mc = mapcrafter::mc;

BOOST_AUTO_TEST_CASE(misc_testSlimeOverlay1) {
	long long world_seed = 42;
	
	std::set<mc::ChunkPos> slimes = {
		mc::ChunkPos(-9, 0), mc::ChunkPos(-9, 8), mc::ChunkPos(-8, -8),
		mc::ChunkPos(-7, -2), mc::ChunkPos(-7, 6), mc::ChunkPos(-7, 7),
		mc::ChunkPos(-6, -9), mc::ChunkPos(-6, 0), mc::ChunkPos(-6, 6),
		mc::ChunkPos(-6, 9), mc::ChunkPos(-4, -7), mc::ChunkPos(-3, -7),
		mc::ChunkPos(-3, -6), mc::ChunkPos(-3, -4), mc::ChunkPos(-3, 1),
		mc::ChunkPos(-3, 3), mc::ChunkPos(-3, 6), mc::ChunkPos(-2, -4),
		mc::ChunkPos(-2, -1), mc::ChunkPos(-2, 1), mc::ChunkPos(-2, 8),
		mc::ChunkPos(-2, 9), mc::ChunkPos(0, -8), mc::ChunkPos(0, 6),
		mc::ChunkPos(0, 8), mc::ChunkPos(0, 10), mc::ChunkPos(1, -3),
		mc::ChunkPos(1, 9), mc::ChunkPos(2, -7), mc::ChunkPos(2, 0),
		mc::ChunkPos(2, 7), mc::ChunkPos(3, -3), mc::ChunkPos(4, -9),
		mc::ChunkPos(4, -8), mc::ChunkPos(4, -3), mc::ChunkPos(5, -9),
		mc::ChunkPos(5, 9), mc::ChunkPos(6, -1), mc::ChunkPos(6, 1),
		mc::ChunkPos(6, 4), mc::ChunkPos(6, 10), mc::ChunkPos(7, -1),
		mc::ChunkPos(7, 5), mc::ChunkPos(8, -6), mc::ChunkPos(9, 1),
		mc::ChunkPos(9, 3), mc::ChunkPos(10, 1),
	};

	for (int x = -10; x < 10; x++) {
		for (int z = -10; z < 10; z++) {
			mc::ChunkPos chunk(x, z);
			bool is_slime = SlimeOverlay::isSlimeChunk(chunk, world_seed);
			if (slimes.count(chunk)) {
				BOOST_CHECK_MESSAGE(is_slime, chunk << " must be a slime chunk!");
			} else {
				BOOST_CHECK_MESSAGE(!is_slime, chunk << " must not be a slime chunk!");
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(misc_testSlimeOverlay2) {
	long long world_seed = 73;

	std::set<mc::ChunkPos> slimes = {
		mc::ChunkPos(72432, -69013), mc::ChunkPos(-36824, 1003),
		mc::ChunkPos(-46931, 83584), mc::ChunkPos(17005, 15947),
		mc::ChunkPos(46510, -38805), mc::ChunkPos(-63169, -14832),
		mc::ChunkPos(-61901, 28562), mc::ChunkPos(-49722, -12913),
		mc::ChunkPos(-77037, -62323), mc::ChunkPos(-53407, 38590),
		mc::ChunkPos(-85273, -93392), mc::ChunkPos(62558, 44909),
		mc::ChunkPos(73967, -93444), mc::ChunkPos(28936, 33531),
		mc::ChunkPos(63390, 67930), mc::ChunkPos(67324, 4700),
		mc::ChunkPos(966, -66317), mc::ChunkPos(-44515, 93892),
		mc::ChunkPos(71566, 50792), mc::ChunkPos(28476, -429),
	};

	std::set<mc::ChunkPos> not_slimes = {
		mc::ChunkPos(-97837, 27086), mc::ChunkPos(-71390, -92204),
		mc::ChunkPos(93321, 85049), mc::ChunkPos(-74488, 95823),
		mc::ChunkPos(97112, 52984), mc::ChunkPos(11580, -70890),
		mc::ChunkPos(-76848, 11243), mc::ChunkPos(61561, -62898),
		mc::ChunkPos(-8120, 38712), mc::ChunkPos(-74904, 38289),
		mc::ChunkPos(-56405, 38637), mc::ChunkPos(-71969, -94666),
		mc::ChunkPos(81663, -86270), mc::ChunkPos(12106, 54473),
		mc::ChunkPos(-79718, 14474), mc::ChunkPos(-37018, -5569),
		mc::ChunkPos(-97474, -209), mc::ChunkPos(-12994, -81218),
		mc::ChunkPos(-28509, 11721), mc::ChunkPos(-38469, -65952),
	};

	for (auto it = slimes.begin(); it != slimes.end(); ++it)
		BOOST_CHECK_MESSAGE(SlimeOverlay::isSlimeChunk(*it, world_seed), *it << " must be a slime chunk!");
	for (auto it = not_slimes.begin(); it != not_slimes.end(); ++it)
		BOOST_CHECK_MESSAGE(!SlimeOverlay::isSlimeChunk(*it, world_seed), *it << " must not be a slime chunk!");
}

