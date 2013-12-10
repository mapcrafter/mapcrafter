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

#include "../mc/worldcrop.h"

#include <boost/test/unit_test.hpp>

namespace mc = mapcrafter::mc;

BOOST_AUTO_TEST_CASE(worldcrop_bounds) {
	mc::Bounds<int> bounds;

	// case 1: no borders
	BOOST_CHECK(bounds.contains(42));
	BOOST_CHECK(bounds.contains(-73));

	// case 2: only a minimum border
	bounds.resetMax();
	bounds.setMin(42);
	BOOST_CHECK(bounds.contains(42));
	BOOST_CHECK(!bounds.contains(21));

	// case 3: only a maximum border
	bounds.setMax(42);
	BOOST_CHECK(bounds.contains(42));
	BOOST_CHECK(!bounds.contains(43));

	// case 3: two borders
	bounds.setMax(73);
	BOOST_CHECK(bounds.contains(42));
	BOOST_CHECK(bounds.contains(73));
	BOOST_CHECK(bounds.contains(54));
	BOOST_CHECK(!bounds.contains(21));
	BOOST_CHECK(!bounds.contains(89));
}

BOOST_AUTO_TEST_CASE(worlcrop_crop_rectangular) {
	mc::WorldCrop crop;

	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(0, 0)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(-1000, 2000)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(42, -73)));

	crop.setMinX(0);
	crop.setMaxX(511);
	crop.setMinZ(0);
	crop.setMaxZ(511);
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(0, 0)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(-1, 0)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(1, 1)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(3, 4)));
	BOOST_CHECK(!crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(31, 31)));
	BOOST_CHECK(!crop.isChunkContained(mc::ChunkPos(32, 32)));

	crop.setMinX(1000);
	crop.setMaxX(1200);
	crop.setMinZ(1000);
	crop.setMaxZ(1200);
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(0, 0)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(1, 0)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(0, 1)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(1, 1)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(0, 2)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(2, 1)));
	BOOST_CHECK(!crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(68, 68)));

	crop = mc::WorldCrop();
	crop.setMaxX(-1);
	crop.setMaxZ(-1);
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(-1, -1)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(-42, -73)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(0, 0)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(-1, 2)));
	BOOST_CHECK(!crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(-1, -4)));
}

BOOST_AUTO_TEST_CASE(worlcrop_crop_circular) {
	// TODO
}
