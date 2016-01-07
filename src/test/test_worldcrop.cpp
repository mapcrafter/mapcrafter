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

#include "../mapcraftercore/mc/worldcrop.h"
#include "../mapcraftercore/util.h"

#include <set>
#include <boost/test/unit_test.hpp>

namespace mc = mapcrafter::mc;
namespace util = mapcrafter::util;

BOOST_AUTO_TEST_CASE(world_crop_bounds) {
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

BOOST_AUTO_TEST_CASE(world_crop_crop_rectangular) {
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
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(31, 31)));
	BOOST_CHECK(!crop.isChunkContained(mc::ChunkPos(32, 32)));

	crop.setMinX(500);
	crop.setMaxX(700);
	crop.setMinZ(500);
	crop.setMaxZ(700);
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(0, 0)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(1, 0)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(0, 1)));
	BOOST_CHECK(crop.isRegionContained(mc::RegionPos(1, 1)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(0, 2)));
	BOOST_CHECK(!crop.isRegionContained(mc::RegionPos(2, 1)));
	BOOST_CHECK(!crop.isChunkContained(mc::ChunkPos(0, 0)));
	BOOST_CHECK(crop.isChunkContained(mc::ChunkPos(37, 37)));

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

BOOST_AUTO_TEST_CASE(world_crop_crop_circular) {
	mc::WorldCrop crop;
	mc::BlockPos center(14, 14, 0);
	int radius = 20;
	crop.setCenter(center);
	crop.setRadius(radius);

	std::set<mc::RegionPos> regions;
	std::set<mc::ChunkPos> chunks;

	for (int x = center.x - radius; x <= center.x + radius; x++)
		for (int z = center.z - radius; z <= center.z + radius; z++) {
			mc::BlockPos pos(x, z, 0);
			if (crop.isBlockContainedXZ(pos)) {
				mc::ChunkPos chunk(pos);
				chunks.insert(chunk);
				regions.insert(chunk.getRegion());
			}
		}

	for (auto it = regions.begin(); it != regions.end(); ++it)
		if (!crop.isRegionContained(*it)) {
			std::cout << "Region " << *it << std::endl;
		}

	for (auto it = chunks.begin(); it != chunks.end(); ++it)
		if (!crop.isChunkContained(*it))
			std::cout << "Chunk " << *it << std::endl;
}

BOOST_AUTO_TEST_CASE(world_crop_block_mask) {
	mc::BlockMask mask;
	mask.setAll(true);
	mask.setRange(0, 2, false);
	mask.set(3, 1, false);
	mask.set(4, 2, false);
	mask.set(4, 2, true);
	mask.set(5, 3, util::binary<11>::value, false);

	BOOST_CHECK_EQUAL(mask.getBlockState(0), mc::BlockMask::BlockState::COMPLETELY_HIDDEN);
	BOOST_CHECK_EQUAL(mask.getBlockState(1), mc::BlockMask::BlockState::COMPLETELY_HIDDEN);
	BOOST_CHECK_EQUAL(mask.getBlockState(2), mc::BlockMask::BlockState::COMPLETELY_HIDDEN);
	BOOST_CHECK(mask.isHidden(0, 3));
	BOOST_CHECK(mask.isHidden(1, 2));
	BOOST_CHECK(mask.isHidden(2, 5));

	BOOST_CHECK_EQUAL(mask.getBlockState(3), mc::BlockMask::BlockState::PARTIALLY_HIDDEN_SHOWN);
	BOOST_CHECK(mask.isHidden(3, 1));
	BOOST_CHECK(!mask.isHidden(3, 3));

	BOOST_CHECK_EQUAL(mask.getBlockState(4), mc::BlockMask::BlockState::COMPLETELY_SHOWN);
	BOOST_CHECK(!mask.isHidden(4, 2));

	BOOST_CHECK_EQUAL(mask.getBlockState(5), mc::BlockMask::BlockState::PARTIALLY_HIDDEN_SHOWN);
	BOOST_CHECK(mask.isHidden(5, 3));
	BOOST_CHECK(mask.isHidden(5, 3 | util::binary<100>::value));
	BOOST_CHECK(mask.isHidden(5, 3 | util::binary<1000>::value));
	BOOST_CHECK(mask.isHidden(5, 3 | util::binary<1100>::value));
	BOOST_CHECK(!mask.isHidden(5, 1));
	BOOST_CHECK(!mask.isHidden(5, 2));
	BOOST_CHECK(!mask.isHidden(5, 2 | util::binary<100>::value));
	BOOST_CHECK(!mask.isHidden(5, 2 | util::binary<1000>::value));
	BOOST_CHECK(!mask.isHidden(5, 2 | util::binary<1100>::value));

	BOOST_CHECK_EQUAL(mask.getBlockState(42), mc::BlockMask::BlockState::COMPLETELY_SHOWN);
	BOOST_CHECK(!mask.isHidden(42, 0));

	BOOST_CHECK_NO_THROW(mask.loadFromStringDefinition("!* 1 3:2 7-9"));
	BOOST_CHECK_NO_THROW(mask.loadFromStringDefinition("!17:3b3 !18:3b3"));
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("1f"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("9999999999"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("3-f42"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("8:6f"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("5::3"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("2:"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("2:16"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("2:-2"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("3:15b"), std::invalid_argument);
	BOOST_CHECK_THROW(mask.loadFromStringDefinition("3:15b18"), std::invalid_argument);
}
