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

#include "../mapcraftercore/mc/pos.h"

#include <iostream>
#include <set>
#include <boost/test/unit_test.hpp>

namespace mc = mapcrafter::mc;

bool is_critical(const std::exception& ex) {
	return true;
}

// just some very simple tests

BOOST_AUTO_TEST_CASE(pos_test_region) {
	BOOST_CHECK(mc::RegionPos(42, 42) == mc::RegionPos(42, 42));
	BOOST_CHECK(mc::RegionPos(73, 42) != mc::RegionPos(42, 73));

	mc::RegionPos r1(1, 1), r2(1, 4), r3(2, 3);
	BOOST_CHECK(r1 < r2);
	BOOST_CHECK(r2 < r3);
	BOOST_CHECK(r1 < r3);

	BOOST_CHECK_EQUAL(mc::RegionPos::byFilename("r.42.-12.mca"), mc::RegionPos(42, -12));

	BOOST_CHECK_EXCEPTION(mc::RegionPos::byFilename("r.42..-12.mca"), std::runtime_error,
	        is_critical);
	BOOST_CHECK_EXCEPTION(mc::RegionPos::byFilename("r.f42.-12.mca"), std::runtime_error,
	        is_critical);
}

BOOST_AUTO_TEST_CASE(pos_test_chunk) {
	BOOST_CHECK_EQUAL(mc::ChunkPos(31, 31).getLocalX(), 31);
	BOOST_CHECK_EQUAL(mc::ChunkPos(31, 31).getLocalZ(), 31);
	BOOST_CHECK_EQUAL(mc::ChunkPos(32, -1).getLocalX(), 0);
	BOOST_CHECK_EQUAL(mc::ChunkPos(32, -1).getLocalZ(), 31);
	BOOST_CHECK_EQUAL(mc::ChunkPos(4+320, 26+320).getLocalX(), 4);
	BOOST_CHECK_EQUAL(mc::ChunkPos(4+320, 26+320).getLocalZ(), 26);

	BOOST_CHECK_EQUAL(mc::ChunkPos::byRowCol(0, 0).getRow(), 0);
	BOOST_CHECK_EQUAL(mc::ChunkPos::byRowCol(0, 0).getCol(), 0);
	BOOST_CHECK_EQUAL(mc::ChunkPos::byRowCol(3, 1).getRow(), 3);
	BOOST_CHECK_EQUAL(mc::ChunkPos::byRowCol(3, 1).getCol(), 1);
	BOOST_CHECK_EQUAL(mc::ChunkPos::byRowCol(5, -3).getRow(), 5);
	BOOST_CHECK_EQUAL(mc::ChunkPos::byRowCol(5, -3).getCol(), -3);
}

BOOST_AUTO_TEST_CASE(pos_test_block) {
	BOOST_CHECK_EQUAL(
	        mc::ChunkPos(mc::LocalBlockPos(3, 4, 64).toGlobalPos(mc::ChunkPos(42, 73))),
	        mc::ChunkPos(42, 73));
	BOOST_CHECK_EQUAL(
	        mc::ChunkPos(mc::LocalBlockPos(1, 2, 3).toGlobalPos(mc::ChunkPos(-4, 10))),
	        mc::ChunkPos(-4, 10));

	std::set<mc::BlockPos> blocks;
	for (int y = 0; y < 256; y++)
		for (int x = 0; x < 16; x++)
			for (int z = 0; z < 16; z++)
				blocks.insert(mc::BlockPos(x, z, y));

	// test the block ordering
	std::set<mc::BlockPos>::const_iterator it = blocks.begin();
	BOOST_CHECK_EQUAL(blocks.size(), 16*16*256);
	for (int y = 0; y < 256; y++)
		for (int x = 15; x >= 0; x--)
			for (int z = 0; z < 16; z++) {
				if (it == blocks.end())
					BOOST_ERROR("Block list too small!");
				if (it->y != y || it->x != x || it->z != z) {
					BOOST_ERROR("Block order invalid!");
					return;
				}

				++it;
			}

}
