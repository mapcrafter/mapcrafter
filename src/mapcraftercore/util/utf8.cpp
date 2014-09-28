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
	std::string converted = str;

	// first replace all \uNNNN
	size_t pos = 0;
	// find all occurences of \u
	while ((pos = converted.find("\\u", pos)) != std::string::npos) {
		// check if this escape sequence is escaped by counting the preceding backslashes
		// it is escaped if there is an odd number if backslashes preceding
		// (an even number of backslashes would mean that there are normal
		// (also escaped) backslashes preceding)
		if ((getPrecedingBackslashes(converted, pos) % 2) == 0) {
			// so, if this escape sequence is not escaped,
			// check if a valid 4-digit hexadecimal number is following
			if (converted.size() >= pos + 6 && isHexNumber(converted.substr(pos + 2, 4))) {
				// if yes, replace it with the UTF-8 encoded equivalent
				std::string escape_sequence = converted.substr(pos, 6);
				converted = converted.substr(0, pos)
						+ convertUnicodeEscapeSequence(escape_sequence)
						+ converted.substr(pos + escape_sequence.size());
			}
		}

		pos++;
		if (pos >= converted.size())
			break;
	}

	// now do the same for the \UNNNNNNNN sequences
	pos = 0;
	while ((pos = converted.find("\\U", pos)) != std::string::npos) {
		if ((getPrecedingBackslashes(converted, pos) % 2) == 0) {
			if (converted.size() >= pos + 10 && isHexNumber(converted.substr(pos + 2, 8))) {
				std::string escape_sequence = converted.substr(pos, 10);
				converted = converted.substr(0, pos)
						+ convertUnicodeEscapeSequence(escape_sequence)
						+ converted.substr(pos + escape_sequence.size());
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
