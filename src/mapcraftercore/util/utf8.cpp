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

#include "utf8.h"

#include "other.h"

#include <cctype>
#include <iostream>

namespace mapcrafter {
namespace util {

/**
 * Encodes a single unicode code point (numerical value of a unicode character) in UTF-8.
 * Have a look at http://en.wikipedia.org/wiki/UTF-8#Description to understand how this
 * works.
 */
std::string encodeUTF8(unsigned int ordinal) {
	std::string utf8;
	if (ordinal <= 0x7f)
		utf8 += (char) ordinal;
	else if (ordinal <= 0x7ff) {
		utf8 += (char) (0b11000000 | (ordinal >> 6));
		utf8 += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0xffff) {
		utf8 += (char) (0b11100000 | (ordinal >> 12));
		utf8 += (char) (0b10000000 | ((ordinal >> 6) & 0b00111111));
		utf8 += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0x1fffff) {
		utf8 += (char) (0b11110000 | (ordinal >> 18));
		utf8 += (char) (0b10000000 | ((ordinal >> 12) & 0b00111111));
		utf8 += (char) (0b10000000 | ((ordinal >> 6)  & 0b00111111));
		utf8 += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0x3ffffff) {
		utf8 += (char) (0b11111000 | (ordinal >> 24));
		utf8 += (char) (0b10000000 | ((ordinal >> 18) & 0b00111111));
		utf8 += (char) (0b10000000 | ((ordinal >> 12) & 0b00111111));
		utf8 += (char) (0b10000000 | ((ordinal >> 6)  & 0b00111111));
		utf8 += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0x7fffffff) {
		utf8 += (char) (0b11111100 | (ordinal >> 30));
		utf8 += (char) (0b10000000 | ((ordinal >> 24) & 0b00111111));
		utf8 += (char) (0b10000000 | ((ordinal >> 18) & 0b00111111));
		utf8 += (char) (0b10000000 | ((ordinal >> 12) & 0b00111111));
		utf8 += (char) (0b10000000 | ((ordinal >> 6)  & 0b00111111));
		utf8 += (char) (0b10000000 | (ordinal & 0b00111111));
	} else {
		// invalid code point, value is too high for UTF-8
		return "";
	}
	return utf8;
}

/**
 * Converts a unicode character escape sequence (\uNNNN or \UNNNNNNNN, with N's as hex digits)
 * to an UTF-8 encoded string.
 */
std::string convertUnicodeEscapeSequence(const std::string& escape_sequence) {
	// make sure we really have a unicode character escape sequence
	if (!startswith(escape_sequence, "\\u") && !startswith(escape_sequence, "\\U"))
		return "";
	// escape sequence must not be too short or too long
	if (escape_sequence.size() < 3 || escape_sequence.size() > 10)
		return "";
	// escape sequence must have the correct length (2+4 for \u, 2+8 for \U)
	size_t hex_length = escape_sequence[1] == 'u' ? 4 : 8;
	if (escape_sequence.size() != 2 + hex_length)
		return "";

	// get actual hexadecimal number part of the escape sequence
	std::string hex = escape_sequence.substr(2);
	// escape sequence number must be a hex number
	if (!isHexNumber(hex))
		return "";

	// parse the hex number from the escape sequence and encode this unicode code point
	return encodeUTF8(parseHexNumber(hex));
}

/**
 * Returns how many backslashes are preceding a specific position in a string.
 */
int getPrecedingBackslashes(const std::string& str, size_t pos) {
	int preceding = 0;
	for (int i = pos - 1; i >= 0; i--) {
		if (str[i] == '\\')
			preceding++;
	}
	return preceding;
}

std::string replaceUnicodeEscapeSequences(const std::string& str) {
	// todo this is a mess... where are your manners, m0r13?!

	std::string converted = str;
	size_t pos = 0;
	while ((pos = converted.find("\\u", pos)) != std::string::npos) {
		// check if this escape sequence is escaped by counting the preceding backslashes
		// it is escaped if there is an odd number if backslashes preceding
		// (an even number of backslashes would mean that there are normal
		// (also escaped) backslashes preceding)
		bool escaped = false;
		int preceding_backslashes = getPrecedingBackslashes(converted, pos);
		if ((preceding_backslashes % 2) == 1)
			escaped = true;

		/*
		std::string before_substr = converted.substr(0, pos);
		std::string before(before_substr.rbegin(), before_substr.rend());
		size_t before_backslash = before.find_first_not_of("\\");
		if ((before_backslash == std::string::npos && (before.size() % 2) == 1)
				|| (before_backslash != std::string::npos && (before_backslash % 2) == 1)) {
			//std::cout << "escaped!" << std::endl;
			escaped = true;
		}
		//std::cout << "before: " << before << std::endl;
		 */

		if (!escaped) {
			std::string after = converted.substr(pos+2);
			//std::cout << "after: " << after << std::endl;
			std::string escape = "";
			if (after.size() >= 8 && isHexNumber(after.substr(0, 8)))
				escape = converted.substr(pos, 10);
			else if (after.size() >= 4 && isHexNumber(after.substr(0, 4)))
				escape = converted.substr(pos, 6);
			else if (after.size() >= 2 && isHexNumber(after.substr(0, 2)))
				escape = converted.substr(pos, 4);
			if (!escape.empty()) {
				converted = converted.substr(0, pos)
						+ convertUnicodeEscapeSequence(escape)
						+ converted.substr(pos + escape.size());
			}
		}

		pos++;
		if (pos >= converted.size())
			break;
	}
	return converted;
}

} /* namespace util */
} /* namespace mapcrafter */
