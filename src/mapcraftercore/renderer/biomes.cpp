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

#include "biomes.h"

#include "blockimages.h" // ColorMapType
#include "image.h"
#include "../util.h"

#include <cmath>
#include <iostream>


namespace mapcrafter {
namespace renderer {


Biome::Biome(uint16_t id, double temperature, double rainfall, uint32_t green_tint, uint32_t water_tint)
	: id(id), temperature(temperature), rainfall(rainfall),
	  green_tint(green_tint), water_tint(water_tint) {
}

Biome& Biome::operator+=(const Biome& other) {
	rainfall += other.rainfall;
	temperature += other.temperature;

	/*
	extra_r += other.extra_r;
	extra_g += other.extra_g;
	extra_b += other.extra_b;
	*/

	return *this;
}

/**
 * Used to calculate average biome data, to create smooth biome edges.
 */
Biome& Biome::operator/=(int n) {
	rainfall /= n;
	temperature /= n;

	/*
	extra_r /= n;
	extra_g /= n;
	extra_b /= n;
	*/

	return *this;
}

/**
 * Checks if two biomes are equal.
 */
bool Biome::operator==(const Biome& other) const {
	double epsilon = 0.1;
	return std::abs(other.rainfall - rainfall) <= epsilon
			&& std::abs(other.temperature - temperature) <= epsilon;
//			&& extra_r == other.extra_r && extra_g == other.extra_g && extra_b == other.extra_b;
}

/**
 * Returns the biome ID.
 */
uint16_t Biome::getID() const {
	return id;
}

/**
 * Calculates the color of the biome with a biome color image.
 */
uint32_t Biome::getColor(int block_y, const ColorMapType& color_type, const RGBAImage& colors, bool flip_xy) const {
	if (color_type == ColorMapType::WATER) {
		return water_tint;
	}

	float elevation = std::max(block_y - 64, 0);
	// x is temperature
	double tmp_temperature = std::min(1.0, std::max(0.0, temperature - elevation*0.00166667f));
	// y is temperature * rainfall
	double tmp_rainfall = std::min(1.0, std::max(0.0, rainfall * temperature));

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
	if (id >= 165 && id <= 167) {
		if (color_type == ColorMapType::GRASS) {
			return rgba(0x90, 0x91, 0x4d, 0xff);
		} else if (color_type == ColorMapType::FOLIAGE) {
			return rgba(0x9e, 0x81, 0x4d, 0xff);
		}
	}
	color = rgba_multiply(color, rgba_red(green_tint), rgba_green(green_tint), rgba_blue(green_tint), rgba_alpha(green_tint));
	return color;
}

bool Biome::isBiomeBlock(uint16_t id, uint16_t data) {
	return id == 2 // grass block
		|| id == 18 || id == 161 // leaves
		|| id == 31 // grass
		|| id == 106 // vines
		|| id == 111 // lily pad
		|| (id == 175 && ((data & util::binary<11>::value) == 2 || (data & util::binary<11>::value) == 3));

	return false;
}

// array with all possible biomes with IDs 0 ... 255
// empty/unknown biomes in this array have the ID 0
static Biome ALL_BIOMES[256] = {};
static bool biomes_initialized;
static std::set<uint16_t> unknown_biomes;

void initializeBiomes() {
	// put all biomes with their IDs into the array with all possible biomes
	for (size_t i = 0; i < BIOMES_SIZE; i++) {
		Biome biome = BIOMES[i];
		ALL_BIOMES[biome.getID()] = biome;
	}

	biomes_initialized = true;
}

Biome getBiome(uint16_t id) {
	// initialize biomes at the first time we access them
	if (!biomes_initialized)
		initializeBiomes();

	// check if this biome exists and return the default biome otherwise
	if (ALL_BIOMES[id].getID() == id)
		return ALL_BIOMES[id];
	if (!unknown_biomes.count(id)) {
		LOG(WARNING) << "Unknown biome with id " << (int) id;
		unknown_biomes.insert(id);
	}
	return ALL_BIOMES[DEFAULT_BIOME];
}

} /* namespace render */
} /* namespace mapcrafter */
