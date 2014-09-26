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

namespace mapcrafter {
namespace util {

bool isHexString(const std::string& str) {
	for (size_t i = 0; i < str.size(); i++)
		if (!isxdigit(str[i]))
			return false;
	return true;
}

std::string convertUnicodeEscapeSequence(const std::string& escape_sequence) {
	if (!startswith(escape_sequence, "\\u") && !startswith(escape_sequence, "\\U"))
		return "";
	// todo check length of escape sequence
	std::string hex_numbers = escape_sequence.substr(2);
	std::stringstream ss;
	ss << std::hex << hex_numbers;
	unsigned int ordinal;
	ss >> ordinal;

	std::string converted;
	if (ordinal <= 0x7f)
		converted += (char) ordinal;
	else if (ordinal <= 0x7ff) {
		converted += (char) (0b11000000 | (ordinal >> 6));
		converted += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0xffff) {
		converted += (char) (0b11100000 | (ordinal >> 12));
		converted += (char) (0b10000000 | ((ordinal >> 6) & 0b00111111));
		converted += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0x1fffff) {
		converted += (char) (0b11110000 | (ordinal >> 18));
		converted += (char) (0b10000000 | ((ordinal >> 12) & 0b00111111));
		converted += (char) (0b10000000 | ((ordinal >> 6)  & 0b00111111));
		converted += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0x3ffffff) {
		converted += (char) (0b11111000 | (ordinal >> 24));
		converted += (char) (0b10000000 | ((ordinal >> 18) & 0b00111111));
		converted += (char) (0b10000000 | ((ordinal >> 12) & 0b00111111));
		converted += (char) (0b10000000 | ((ordinal >> 6)  & 0b00111111));
		converted += (char) (0b10000000 | (ordinal & 0b00111111));
	} else if (ordinal <= 0x7fffffff) {
		converted += (char) (0b11111100 | (ordinal >> 30));
		converted += (char) (0b10000000 | ((ordinal >> 24) & 0b00111111));
		converted += (char) (0b10000000 | ((ordinal >> 18) & 0b00111111));
		converted += (char) (0b10000000 | ((ordinal >> 12) & 0b00111111));
		converted += (char) (0b10000000 | ((ordinal >> 6)  & 0b00111111));
		converted += (char) (0b10000000 | (ordinal & 0b00111111));
	}
	return converted;
}

std::string replaceUnicodeEscapeSequences(const std::string& string) {
	// todo this is a mess... where are your manners, m0r13?!

	std::string converted = string;
	size_t pos = 0;
	while ((pos = converted.find("\\u", pos)) != std::string::npos) {
		//std::cout << "found \\u at " << pos << std::endl;
		std::string before_substr = converted.substr(0, pos);
		std::string before(before_substr.rbegin(), before_substr.rend());
		size_t before_backslash = before.find_first_not_of("\\");
		bool escaped = false;
		if ((before_backslash == std::string::npos && (before.size() % 2) == 1)
				|| (before_backslash != std::string::npos && (before_backslash % 2) == 1)) {
			//std::cout << "escaped!" << std::endl;
			escaped = true;
		}
		//std::cout << "before: " << before << std::endl;

		if (!escaped) {
			std::string after = converted.substr(pos+2);
			//std::cout << "after: " << after << std::endl;
			std::string escape = "";
			if (after.size() >= 8 && isHexString(after.substr(0, 8)))
				escape = converted.substr(pos, 10);
			if (after.size() >= 4 && isHexString(after.substr(0, 4)))
				escape = converted.substr(pos, 6);
			if (after.size() >= 2 && isHexString(after.substr(0, 2)))
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
