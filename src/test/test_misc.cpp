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

#include "../mapcraftercore/renderer/rendermode.h"
#include "../mapcraftercore/renderer/rendermodes/slimeoverlay.h"

#include <set>
#include <boost/test/unit_test.hpp>

using namespace mapcrafter::renderer;
namespace mc = mapcrafter::mc;

/**
 * Test data generated with Test.java program.
 */

BOOST_AUTO_TEST_CASE(misc_testSlimeOverlay1) {
	long long world_seed = 42;

	std::set<mc::ChunkPos> slimes = {
		mc::ChunkPos(-9, 0),
		mc::ChunkPos(-9, 8),
		mc::ChunkPos(-8, -8),
		mc::ChunkPos(-7, -2),
		mc::ChunkPos(-7, 6),
		mc::ChunkPos(-7, 7),
		mc::ChunkPos(-6, -9),
		mc::ChunkPos(-6, 0),
		mc::ChunkPos(-6, 6),
		mc::ChunkPos(-6, 9),
		mc::ChunkPos(-4, -7),
		mc::ChunkPos(-3, -7),
		mc::ChunkPos(-3, -6),
		mc::ChunkPos(-3, -4),
		mc::ChunkPos(-3, 1),
		mc::ChunkPos(-3, 3),
		mc::ChunkPos(-3, 6),
		mc::ChunkPos(-2, -4),
		mc::ChunkPos(-2, -1),
		mc::ChunkPos(-2, 1),
		mc::ChunkPos(-2, 8),
		mc::ChunkPos(-2, 9),
		mc::ChunkPos(0, -8),
		mc::ChunkPos(0, 6),
		mc::ChunkPos(0, 8),
		mc::ChunkPos(0, 10),
		mc::ChunkPos(1, -3),
		mc::ChunkPos(1, 9),
		mc::ChunkPos(2, -7),
		mc::ChunkPos(2, 0),
		mc::ChunkPos(2, 7),
		mc::ChunkPos(3, -3),
		mc::ChunkPos(4, -9),
		mc::ChunkPos(4, -8),
		mc::ChunkPos(4, -3),
		mc::ChunkPos(5, -9),
		mc::ChunkPos(5, 9),
		mc::ChunkPos(6, -1),
		mc::ChunkPos(6, 1),
		mc::ChunkPos(6, 4),
		mc::ChunkPos(6, 10),
		mc::ChunkPos(7, -1),
		mc::ChunkPos(7, 5),
		mc::ChunkPos(8, -6),
		mc::ChunkPos(9, 1),
		mc::ChunkPos(9, 3),
		mc::ChunkPos(10, 1),
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
		mc::ChunkPos(6177, 1554),
		mc::ChunkPos(3552, 6651),
		mc::ChunkPos(1868, 1935),
		mc::ChunkPos(-7254, 2306),
		mc::ChunkPos(-9966, -464),
		mc::ChunkPos(1311, -3043),
		mc::ChunkPos(1688, -3166),
		mc::ChunkPos(4061, 2919),
		mc::ChunkPos(3222, -5722),
		mc::ChunkPos(-5514, 8463),
		mc::ChunkPos(4628, 7204),
		mc::ChunkPos(-4326, 5205),
		mc::ChunkPos(-7321, 4556),
		mc::ChunkPos(715, 8785),
		mc::ChunkPos(-4275, -5173),
		mc::ChunkPos(124, -8281),
		mc::ChunkPos(-5279, 8460),
		mc::ChunkPos(3223, 9801),
		mc::ChunkPos(-752, -4699),
		mc::ChunkPos(7633, 3534),
	};

	std::set<mc::ChunkPos> not_slimes = {
		mc::ChunkPos(-5021, 8664),
		mc::ChunkPos(8193, 1041),
		mc::ChunkPos(-150, 933),
		mc::ChunkPos(8056, 2834),
		mc::ChunkPos(-5175, -2570),
		mc::ChunkPos(8099, 7748),
		mc::ChunkPos(6844, 8625),
		mc::ChunkPos(-4555, -247),
		mc::ChunkPos(-4437, -715),
		mc::ChunkPos(187, -9472),
		mc::ChunkPos(-4932, 2539),
		mc::ChunkPos(8748, -1460),
		mc::ChunkPos(4667, -2592),
		mc::ChunkPos(-2737, 426),
		mc::ChunkPos(6002, -3594),
		mc::ChunkPos(-8542, 2576),
		mc::ChunkPos(8872, 5180),
		mc::ChunkPos(7047, 1493),
		mc::ChunkPos(1588, -4488),
		mc::ChunkPos(940, -305),
	};

	for (auto it = slimes.begin(); it != slimes.end(); ++it)
		BOOST_CHECK_MESSAGE(SlimeOverlay::isSlimeChunk(*it, world_seed), *it << " must be a slime chunk!");
	for (auto it = not_slimes.begin(); it != not_slimes.end(); ++it)
		BOOST_CHECK_MESSAGE(!SlimeOverlay::isSlimeChunk(*it, world_seed), *it << " must not be a slime chunk!");
}

BOOST_AUTO_TEST_CASE(misc_renderModeRendererTypeEnum) {
	BOOST_CHECK(RenderModeRendererType::DUMMY == RenderModeRendererType::DUMMY);
	BOOST_CHECK(RenderModeRendererType::LIGHTING == RenderModeRendererType::LIGHTING);
	BOOST_CHECK(RenderModeRendererType::OVERLAY == RenderModeRendererType::OVERLAY);

	BOOST_CHECK(RenderModeRendererType::DUMMY != RenderModeRendererType::LIGHTING);
	BOOST_CHECK(RenderModeRendererType::DUMMY != RenderModeRendererType::OVERLAY);
	BOOST_CHECK(RenderModeRendererType::LIGHTING != RenderModeRendererType::DUMMY);
	BOOST_CHECK(RenderModeRendererType::LIGHTING != RenderModeRendererType::OVERLAY);
	BOOST_CHECK(RenderModeRendererType::OVERLAY != RenderModeRendererType::DUMMY);
	BOOST_CHECK(RenderModeRendererType::OVERLAY != RenderModeRendererType::LIGHTING);
}

