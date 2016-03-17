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

#ifndef COLOR_H_
#define COLOR_H_

#include "other.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

namespace mapcrafter {
namespace util {

class Color {
public:
	Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255);

	uint8_t getRed() const;
	uint8_t getGreen() const;
	uint8_t getBlue() const;
	uint8_t getAlpha() const;

	std::string getHex() const;
	uint32_t getRGBA() const;

	Color mix(Color other, double alpha) const;

	bool operator==(const Color& color) const;
	bool operator!=(const Color& color) const;

	static Color byString(const std::string& string);
	static const Color WHITE, SILVER, GRAY, BLACK, RED, MAROON, YELLOW, OLIVE, LIME,
					   GREEN, AQUA, TEAL, BLUE, NAVY, FUCHSIA, PURPLE;

protected:
	uint8_t red, green, blue, alpha;

	static const std::map<std::string, Color> namedColors;
	static Color byHex(const std::string& string);
	static Color byName(const std::string& string);
};

std::ostream& operator<<(std::ostream& out, const Color& color);

template <>
Color as(const std::string& from);

}
}

#endif /* COLOR_H_ */
