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

#include "../mapcraftercore/util.h"

#include <boost/test/unit_test.hpp>

namespace util = mapcrafter::util;

BOOST_AUTO_TEST_CASE(util_testMath) {
	// some random numbers
	BOOST_CHECK_EQUAL(util::binary<110110100>::value, 436);
	BOOST_CHECK_EQUAL(util::binary<100101011>::value, 299);
	BOOST_CHECK_EQUAL(util::binary<10111111>::value, 191);
	BOOST_CHECK_EQUAL(util::binary<1101010101>::value, 853);
	BOOST_CHECK_EQUAL(util::binary<1001100010>::value, 610);
	BOOST_CHECK_EQUAL(util::binary<1110111001>::value, 953);
	BOOST_CHECK_EQUAL(util::binary<111001101>::value, 461);
	BOOST_CHECK_EQUAL(util::binary<1110101001>::value, 937);
	BOOST_CHECK_EQUAL(util::binary<11000100>::value, 196);
	BOOST_CHECK_EQUAL(util::binary<1000001110>::value, 526);
	BOOST_CHECK_EQUAL(util::binary<1000111101>::value, 573);
	BOOST_CHECK_EQUAL(util::binary<111111>::value, 63);
	BOOST_CHECK_EQUAL(util::binary<1100111101>::value, 829);
	BOOST_CHECK_EQUAL(util::binary<1111001010>::value, 970);
	BOOST_CHECK_EQUAL(util::binary<101110001>::value, 369);
	BOOST_CHECK_EQUAL(util::binary<111000110>::value, 454);
	BOOST_CHECK_EQUAL(util::binary<1100011111>::value, 799);
	BOOST_CHECK_EQUAL(util::binary<1000001100>::value, 524);
	BOOST_CHECK_EQUAL(util::binary<101000101>::value, 325);
	BOOST_CHECK_EQUAL(util::binary<11011101>::value, 221);
}

