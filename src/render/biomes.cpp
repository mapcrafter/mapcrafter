/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "biomes.h"

#include <cmath>

namespace mapcrafter {
namespace render {


Biome::Biome(uint8_t id, double temperature, double rainfall, uint8_t r, uint8_t g, uint8_t b)
	: id(id), temperature(temperature), rainfall(rainfall),
	  extra_r(r), extra_g(g), extra_b(b) {
}

Biome& Biome::operator+=(const Biome& other) {
	rainfall += other.rainfall;
	temperature += other.temperature;

	extra_r += other.extra_r;
	extra_g += other.extra_g;
	extra_b += other.extra_b;

	return *this;
}

/**
 * Used to calculate average biome data, to create smooth biome edges.
 */
Biome& Biome::operator/=(int n) {
	rainfall /= n;
	temperature /= n;

	extra_r /= n;
	extra_g /= n;
	extra_b /= n;

	return *this;
}

/**
 * Checks if two biomes are equal.
 */
bool Biome::operator==(const Biome& other) const {
	double epsilon = 0.1;
	return std::abs(other.rainfall - rainfall) <= epsilon
			&& std::abs(other.temperature - temperature) <= epsilon
			&& extra_r == other.extra_r && extra_g == other.extra_g && extra_b == other.extra_b;
}

/**
 * Returns the biome ID.
 */
uint8_t Biome::getID() const {
	return id;
}

/**
 * Calculates the color of the biome with a biome color image.
 */
uint32_t Biome::getColor(const Image& colors, bool flip_xy) const {
	// x is temperature
	double tmp_temperature = temperature;
	// y is temperature * rainfall
	double tmp_rainfall = rainfall * temperature;

	// check if temperature and rainfall are valid
	if(tmp_temperature > 1)
		tmp_temperature = 1;
	if(tmp_rainfall > 1)
		tmp_rainfall = 1;

	// calculate positions
	int x = 255 - (255 * tmp_temperature);
	int y = 255 - (255 * tmp_rainfall);

	// flip them, if needed
	if (flip_xy) {
		int tmp = x;
		x = 255 - y;
		y = 255 - tmp;
	}

	// get color at this position
	uint32_t color = colors.getPixel(x, y);
	if (extra_r != 255 || extra_g != 255 || extra_b != 255) {
		// multiply with fixed biome color values if specified
		// necessary for the swampland biome
		return rgba_multiply(color, (uint8_t) extra_r, (uint8_t) extra_g, (uint8_t) extra_b, 255);
	}
	return color;
}

bool Biome::isBiomeBlock(uint16_t id, uint16_t data) {
	return id == 2 // grass block
			|| id == 18 || id == 161 // leaves
			|| id == 31 // grass
			|| id == 106 // vines
			|| id == 111 // lily pad
			|| (id == 175 && ((data & 0b11) == 2 || (data & 0b11) == 3)); // large flowers (tallgrass, fern)
	return false;
}

Biome getBiome(uint8_t id) {
	for (size_t i = 0; i < BIOMES_SIZE; i++)
		if (BIOMES[i].getID() == id)
			return BIOMES[i];
	return BIOMES[DEFAULT_BIOME];
}

} /* namespace render */
} /* namespace mapcrafter */
