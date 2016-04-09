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

#include "color.h"

#include <algorithm>
#include <sstream>

namespace mapcrafter {
namespace util {

Color::Color(uint8_t red, uint8_t green, uint8_t blue)
	: red(red), green(green), blue(blue), alpha(255), alpha_set(false) {
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	: red(red), green(green), blue(blue), alpha(alpha), alpha_set(true) {
}

uint8_t Color::getRed() const {
	return red;
}

uint8_t Color::getGreen() const {
	return green;
}

uint8_t Color::getBlue() const {
	return blue;
}

uint8_t Color::getAlpha() const {
	return alpha;
}

bool Color::isAlphaSet() const {
	return alpha_set;
}

uint32_t Color::getRGBA() const {
	return (alpha << 24) | (blue << 16) | (green << 8) | red;
}

std::string Color::getHex() const {
	std::stringstream ss;
	ss.width(2);
	ss.fill('0');
	ss << std::hex << (int) red;
	ss << std::hex << (int) green;
	ss << std::hex << (int) blue;
	ss << std::hex << (int) alpha;
	return "#" + ss.str();
}

Color Color::mix(Color other, double t) const {
	Color mixed = Color(
		(1 - t) * red + t * other.red,
		(1 - t) * green + t * other.green,
		(1 - t) * blue + t * other.blue
	);
	if (!isAlphaSet() && !other.isAlphaSet())
		return mixed;
	return mixed.withAlpha((1 - t) * alpha + t * other.alpha);
}

Color Color::withAlpha(uint8_t new_alpha) const {
	return Color(red, green, blue, new_alpha);
}

Color Color::withAlphaIfAbsent(uint8_t new_alpha) const {
	if (isAlphaSet())
		return *this;
	return withAlpha(new_alpha);
}

bool Color::operator==(const Color& color) const {
	return red == color.red
		&& green == color.green
		&& blue == color.blue
		&& alpha == color.alpha;
}

bool Color::operator!=(const Color& color) const {
	return !operator==(color);
}

Color Color::byString(const std::string& string) {
	try {
		return Color::byName(string);
	} catch (std::invalid_argument& err) {
	}
	return Color::byHex(string);
}

const Color Color::WHITE = Color::byHex("#FFFFFF");
const Color Color::SILVER = Color::byHex("#C0C0C0");
const Color Color::GRAY = Color::byHex("#808080");
const Color Color::BLACK = Color::byHex("#000000");
const Color Color::RED = Color::byHex("#FF0000");
const Color Color::MAROON = Color::byHex("#800000");
const Color Color::YELLOW = Color::byHex("#FFFF00");
const Color Color::OLIVE = Color::byHex("#808000");
const Color Color::LIME = Color::byHex("#00FF00");
const Color Color::GREEN = Color::byHex("#008000");
const Color Color::AQUA = Color::byHex("#00FFFF");
const Color Color::TEAL = Color::byHex("#008080");
const Color Color::BLUE = Color::byHex("#0000FF");
const Color Color::NAVY = Color::byHex("#000080");
const Color Color::FUCHSIA = Color::byHex("#FF00FF");
const Color Color::PURPLE = Color::byHex("#800080");

const std::map<std::string, Color> Color::namedColors = {
	{"white", Color::WHITE},
	{"silver", Color::SILVER},
	{"gray", Color::GRAY},
	{"black", Color::BLACK},
	{"red", Color::RED},
	{"maroon", Color::MAROON},
	{"yellow", Color::YELLOW},
	{"olive", Color::OLIVE},
	{"lime", Color::LIME},
	{"green", Color::GREEN},
	{"aqua", Color::AQUA},
	{"teal", Color::TEAL},
	{"blue", Color::BLUE},
	{"navy", Color::NAVY},
	{"fuchsia", Color::FUCHSIA},
	{"purple", Color::PURPLE},
};

Color Color::byHex(const std::string& string) {
	if (string.empty() || string[0] != '#')
		throw std::invalid_argument("Hex color must start with '#'!");
	std::string hex = string.substr(1);
	if (hex.size() != 3 && hex.size() != 4 && hex.size() != 6 && hex.size() != 8)
		throw std::invalid_argument("Hex color has invalid length!");
	if (!isHexNumber(hex))
		throw std::invalid_argument("Hex color may only contain hex characters!");

	Color color;
	if (hex.size() <= 4) {
		color.red = parseHexNumber(hex.substr(0, 1));
		color.red += color.red * 16;
		color.green = parseHexNumber(hex.substr(1, 1));
		color.green += color.green * 16;
		color.blue = parseHexNumber(hex.substr(2, 1));
		color.blue += color.blue * 16;
	}
	if (hex.size() == 4) {
		color.alpha = parseHexNumber(hex.substr(3, 1));
		color.alpha += color.alpha * 16;
		color.alpha_set = true;
	}

	if (hex.size() >= 6) {
		color.red = parseHexNumber(hex.substr(0, 2));
		color.green = parseHexNumber(hex.substr(2, 2));
		color.blue = parseHexNumber(hex.substr(4, 2));
	}
	if (hex.size() == 8) {
		color.alpha = parseHexNumber(hex.substr(6, 2));
		color.alpha_set = true;
	}
	return color;
}

Color Color::byName(const std::string& string) {
	std::string name = string;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (namedColors.count(name))
		return namedColors.at(name);
	throw std::invalid_argument("Unknown color");
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
	return out << color.getHex();
}

template <>
Color as(const std::string& from) {
	return Color::byString(from);
}

}
}

