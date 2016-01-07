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

#include "../mapcraftercore/mc/chunk.h"
#include "../mapcraftercore/mc/region.h"
#include "../mapcraftercore/util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/test/unit_test.hpp>

namespace mc = mapcrafter::mc;

BOOST_AUTO_TEST_CASE(region_testReadWrite) {
	mc::RegionFile in1("data/region/r.-1.0.mca");
	BOOST_CHECK(in1.read());
	BOOST_CHECK_EQUAL(in1.getContainingChunksCount(), 120);
	BOOST_CHECK(in1.write("data/r.-1.0.mca"));

	mc::RegionFile in2("data/r.-1.0.mca");
	BOOST_CHECK(in2.read());
	BOOST_CHECK_EQUAL(in2.getContainingChunksCount(), 120);

	auto chunks1 = in1.getContainingChunks();
	auto chunks2 = in2.getContainingChunks();
	auto it1 = chunks1.begin();
	auto it2 = chunks2.begin();
	for ( ; it1 != chunks1.end() && it2 != chunks2.end(); ++it1, ++it2) {
		BOOST_CHECK_EQUAL(*it1, *it2);

		mc::Chunk chunk1, chunk2;
		BOOST_CHECK(in1.loadChunk(*it1, chunk1));
		BOOST_CHECK(in2.loadChunk(*it2, chunk2));
	}

}
