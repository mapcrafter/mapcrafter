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

#include "../mapcraftercore/mc/blockstate.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/test/unit_test.hpp>

namespace mc = mapcrafter::mc;

BOOST_AUTO_TEST_CASE(blockstate_test) {
	mc::BlockState block("mapcrafter:test");
	
	// basic initialization checks
	BOOST_CHECK_EQUAL(block.getName(), "mapcrafter:test");
	BOOST_CHECK_EQUAL(block.getProperty("foo"), "");
	BOOST_CHECK_EQUAL(block.getVariantDescription(), "");

	// check properties - overwriting / getting with default / property representation
	block.setProperty("foo", "bar");
	BOOST_CHECK_EQUAL(block.getProperty("foo"), "bar");
	BOOST_CHECK_EQUAL(block.getProperty("foo2", "test"), "test");
	BOOST_CHECK_EQUAL(block.getVariantDescription(), "foo=bar,");

	block.setProperty("foo", "newvalue");
	block.setProperty("abc", "test");
	BOOST_CHECK_EQUAL(block.getProperty("foo", "bar"), "newvalue");
	BOOST_CHECK_EQUAL(block.getProperty("abc"), "test");
	BOOST_CHECK_EQUAL(block.getVariantDescription(), "abc=test,foo=newvalue,");

	// make sure different order of setting properties results in same property repr
	block = mc::BlockState();
	block.setProperty("foo", "bar");
	block.setProperty("test", "blah");
	BOOST_CHECK_EQUAL(block.getVariantDescription(), "foo=bar,test=blah,");

	block = mc::BlockState();
	block.setProperty("test", "blah");
	block.setProperty("foo", "bar");
	BOOST_CHECK_EQUAL(block.getVariantDescription(), "foo=bar,test=blah,");

	// aaand overwrite something
	block.setProperty("foo", "baaah");
	BOOST_CHECK_EQUAL(block.getVariantDescription(), "foo=baaah,test=blah,");
}

BOOST_AUTO_TEST_CASE(blockstate_testRegistry) {
	mc::BlockStateRegistry registry;

	mc::BlockState block("mapcrafter:test");
	block.setProperty("abc", "test");
	block.setProperty("hello", "world");

	uint16_t id = registry.getBlockID(block);
	BOOST_CHECK_EQUAL(id, 0);

	mc::BlockState block_compare = registry.getBlockState(id);
	BOOST_CHECK_EQUAL(block_compare.getName(), block.getName());
	BOOST_CHECK_EQUAL(block_compare.getVariantDescription(), block.getVariantDescription());
}

