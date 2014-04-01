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

#include "biomes.h"

#include <cmath>
#include <iostream>

namespace mapcrafter {
namespace renderer {


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
uint32_t Biome::getColor(const RGBAImage& colors, bool flip_xy) const {
	// x is temperature
	double tmp_temperature = temperature;
	// y is temperature * rainfall
	double tmp_rainfall = rainfall * temperature;

	// check if temperature and rainfall are valid
	if(tmp_temperature > 1)
		tmp_temperature = 1;
	if (tmp_rainfall > 1)
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

// array with all possible biomes with IDs 0 ... 255
// empty/unknown biomes in this array have the ID 0
static Biome ALL_BIOMES[256] = {};
static bool biomes_initialized;

void initializeBiomes() {
	// put all biomes with their IDs into the array with all possible biomes
	for (size_t i = 0; i < BIOMES_SIZE; i++) {
		Biome biome = BIOMES[i];
		ALL_BIOMES[biome.getID()] = biome;
	}

	biomes_initialized = true;
}

Biome getBiome(uint8_t id) {
	// initialize biomes at the first time we access them
	if (!biomes_initialized)
		initializeBiomes();

	// check if this biome exists and return the default biome otherwise
	if (ALL_BIOMES[id].getID() == id)
		return ALL_BIOMES[id];
	return ALL_BIOMES[DEFAULT_BIOME];
}

} /* namespace render */
} /* namespace mapcrafter */
