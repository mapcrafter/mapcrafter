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

BOOST_AUTO_TEST_CASE(util_utf8) {
	// basic tests if the conversion from unicode character escape sequences to UTF-8 works
	BOOST_CHECK_EQUAL("", util::convertUnicodeEscapeSequence(""));
	BOOST_CHECK_EQUAL("", util::convertUnicodeEscapeSequence("\\a23"));
	BOOST_CHECK_EQUAL("", util::convertUnicodeEscapeSequence("\\u"));
	BOOST_CHECK_EQUAL("", util::convertUnicodeEscapeSequence("\\uabcg"));
	BOOST_CHECK_EQUAL("", util::convertUnicodeEscapeSequence("\\uffffffff"));
	BOOST_CHECK_EQUAL("", util::convertUnicodeEscapeSequence("\\u1234ffffffff"));
	BOOST_CHECK_EQUAL(u8"a", util::convertUnicodeEscapeSequence("\\u0061"));
	BOOST_CHECK_EQUAL(u8"N", util::convertUnicodeEscapeSequence("\\u004e"));
	BOOST_CHECK_EQUAL(u8"$", util::convertUnicodeEscapeSequence("\\u0024"));
	BOOST_CHECK_EQUAL(u8"€", util::convertUnicodeEscapeSequence("\\u20ac"));
	BOOST_CHECK_EQUAL(u8"<", util::convertUnicodeEscapeSequence("\\u003c"));
	BOOST_CHECK_EQUAL(u8"☭", util::convertUnicodeEscapeSequence("\\u262d"));

	// test the first/last unicode code points possible per count of bytes
	// (1/2/3/4/5/6 bytes, see http://en.wikipedia.org/wiki/UTF-8#Description)
	BOOST_CHECK_EQUAL(u8"\u007f", util::convertUnicodeEscapeSequence("\\u007f"));
	BOOST_CHECK_EQUAL(u8"\u0080", util::convertUnicodeEscapeSequence("\\u0080"));
	BOOST_CHECK_EQUAL(u8"\u07ff", util::convertUnicodeEscapeSequence("\\u07ff"));
	BOOST_CHECK_EQUAL(u8"\u0800", util::convertUnicodeEscapeSequence("\\u0800"));
	BOOST_CHECK_EQUAL(u8"\uffff", util::convertUnicodeEscapeSequence("\\uffff"));
	BOOST_CHECK_EQUAL(u8"\U00010000", util::convertUnicodeEscapeSequence("\\U00010000"));
	BOOST_CHECK_EQUAL(u8"\U001fffff", util::convertUnicodeEscapeSequence("\\U001fffff"));
	BOOST_CHECK_EQUAL(u8"\U00200000", util::convertUnicodeEscapeSequence("\\U00200000"));
	BOOST_CHECK_EQUAL(u8"\U03ffffff", util::convertUnicodeEscapeSequence("\\U03ffffff"));
	BOOST_CHECK_EQUAL(u8"\U04000000", util::convertUnicodeEscapeSequence("\\U04000000"));
	BOOST_CHECK_EQUAL(u8"\U7fffffff", util::convertUnicodeEscapeSequence("\\U7fffffff"));

	// check if escaping unicode character escape sequences works
	// todo maybe more
	BOOST_CHECK_EQUAL(u8"\\\\u262d", util::replaceUnicodeEscapeSequences("\\\\u262d"));
	BOOST_CHECK_EQUAL(u8"\\\\\u262d", util::replaceUnicodeEscapeSequences("\\\\\\u262d"));
	BOOST_CHECK_EQUAL(u8"test \\\\u262d", util::replaceUnicodeEscapeSequences("test \\\\u262d"));
	BOOST_CHECK_EQUAL(u8"test \\\\\u262d", util::replaceUnicodeEscapeSequences("test \\\\\\u262d"));

	// check if we always use the correct length of hex numbers after the \u and \U
	BOOST_CHECK_EQUAL(u8"\\u12", util::replaceUnicodeEscapeSequences("\\u12"));
	BOOST_CHECK_EQUAL(u8"\u1234", util::replaceUnicodeEscapeSequences("\\1234"));
	BOOST_CHECK_EQUAL(u8"\u1234" "567", util::replaceUnicodeEscapeSequences("\\u1234567"));
	BOOST_CHECK_EQUAL(u8"\\U123", util::replaceUnicodeEscapeSequences("\\U123"));
	BOOST_CHECK_EQUAL(u8"\U12345678", util::replaceUnicodeEscapeSequences("\\U12345678"));
	BOOST_CHECK_EQUAL(u8"\U12345678" "9", util::replaceUnicodeEscapeSequences("\\U123456789"));

	// general tests of unicode character escape sequence replacer
	// todo more
	BOOST_CHECK_EQUAL(u8"42€ \u0026 $73", util::replaceUnicodeEscapeSequences("42\\u20ac & \\u002473"));
	BOOST_CHECK_EQUAL(u8">> Test", util::replaceUnicodeEscapeSequences("\\u003e\\u003e Test"));
}
