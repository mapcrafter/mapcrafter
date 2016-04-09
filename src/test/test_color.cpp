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

#include "../mapcraftercore/util/color.h"

#include <boost/test/unit_test.hpp>

using namespace mapcrafter::util;

BOOST_AUTO_TEST_CASE(util_test_color) {
	BOOST_CHECK_EQUAL(Color::byString("red"), Color::RED);
	BOOST_CHECK_EQUAL(Color::byString("teal"), Color::TEAL);
	BOOST_CHECK_EQUAL(Color::byString("#f00"), Color::RED);
	BOOST_CHECK_EQUAL(Color::byString("#fF0000"), Color::RED);
	BOOST_CHECK_EQUAL(Color::byString("#f00"), Color::RED);
	BOOST_CHECK_EQUAL(Color::byString("#0000FF"), Color::BLUE);

	BOOST_CHECK_EQUAL(Color::byString("#00f"), Color::BLUE);
	BOOST_CHECK_EQUAL(Color::byString("#123456"), Color(0x12, 0x34, 0x56));
	BOOST_CHECK_EQUAL(Color::byString("#123456AB"), Color(0x12, 0x34, 0x56, 0xAB));
	BOOST_CHECK_EQUAL(Color::byString("#123"), Color(0x11, 0x22, 0x33));
	BOOST_CHECK_EQUAL(Color::byString("#1234"), Color(0x11, 0x22, 0x33, 0x44));

	BOOST_CHECK_THROW(Color::byString("gelb"), std::invalid_argument);
	BOOST_CHECK_THROW(Color::byString("#"), std::invalid_argument);
	BOOST_CHECK_THROW(Color::byString("#1"), std::invalid_argument);
	BOOST_CHECK_THROW(Color::byString("#12"), std::invalid_argument);
	BOOST_CHECK_THROW(Color::byString("#12z"), std::invalid_argument);
	BOOST_CHECK_THROW(Color::byString("#12345"), std::invalid_argument);
	BOOST_CHECK_THROW(Color::byString("#12345y"), std::invalid_argument);

	BOOST_CHECK(!Color::byString("#123").isAlphaSet());
	BOOST_CHECK(!Color::byString("#123456").isAlphaSet());
	BOOST_CHECK(Color::byString("#1234").isAlphaSet());
	BOOST_CHECK(Color::byString("#12345678").isAlphaSet());
	BOOST_CHECK(!Color::byString("red").isAlphaSet());
	BOOST_CHECK(!Color(12, 34, 56).isAlphaSet());
	BOOST_CHECK(Color(12, 34, 56, 78).isAlphaSet());

	Color with_alpha(12, 34, 56, 78);
	Color without_alpha(56, 78, 12);
	BOOST_CHECK(with_alpha.mix(without_alpha, 0.5).isAlphaSet());
	BOOST_CHECK(with_alpha.mix(with_alpha, 0.5).isAlphaSet());
	BOOST_CHECK(without_alpha.mix(with_alpha, 0.5).isAlphaSet());
	BOOST_CHECK(!without_alpha.mix(without_alpha, 0.5).isAlphaSet());
}

