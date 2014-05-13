/*
 * Copyright 2012-2014 Moritz Hilscher
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
	BOOST_CHECK_EQUAL(util::binary<1011011011101>::value, 5853);
	BOOST_CHECK_EQUAL(util::binary<10001001100010>::value, 8802);
	BOOST_CHECK_EQUAL(util::binary<1100100110111>::value, 6455);
	BOOST_CHECK_EQUAL(util::binary<1000010111001>::value, 4281);
	BOOST_CHECK_EQUAL(util::binary<11101011010>::value, 1882);
	BOOST_CHECK_EQUAL(util::binary<1111011011111>::value, 7903);
	BOOST_CHECK_EQUAL(util::binary<100101100>::value, 300);
	BOOST_CHECK_EQUAL(util::binary<111111011010>::value, 4058);
	BOOST_CHECK_EQUAL(util::binary<1101110010011>::value, 7059);
	BOOST_CHECK_EQUAL(util::binary<10010000001001>::value, 9225);
	BOOST_CHECK_EQUAL(util::binary<101010000>::value, 336);
	BOOST_CHECK_EQUAL(util::binary<1110000110011>::value, 7219);
	BOOST_CHECK_EQUAL(util::binary<1101110100111>::value, 7079);
	BOOST_CHECK_EQUAL(util::binary<111000110101>::value, 3637);
	BOOST_CHECK_EQUAL(util::binary<1111000010111>::value, 7703);
	BOOST_CHECK_EQUAL(util::binary<1000110010100>::value, 4500);
	BOOST_CHECK_EQUAL(util::binary<1010000101000>::value, 5160);
	BOOST_CHECK_EQUAL(util::binary<1000100100111>::value, 4391);
	BOOST_CHECK_EQUAL(util::binary<11000001100>::value, 1548);
	BOOST_CHECK_EQUAL(util::binary<111010111110>::value, 3774);
}
