/*
 * Copyright 2012-2015 Moritz Hilscher
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

#ifndef BIOMES_H_
#define BIOMES_H_

#include <cstdint>
#include <cstdlib>

namespace mapcrafter {
namespace renderer {

class RGBAImage;

/**
 * A Minecraft Biome with data to tint the biome-depend blocks.
 */
class Biome {
private:
	// id of the biome
	uint8_t id;

	// temperature and rainfall
	// used to calculate the position of the tinting color in the color image
	double temperature;
	double rainfall;

	// extra color values, for example for the swampland biome
	int extra_r, extra_g, extra_b;
public:
	Biome(uint8_t id = 0, double temperature = 0, double rainfall = 0,
			uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);

	Biome& operator+=(const Biome& other);
	Biome& operator/=(int n);
	bool operator==(const Biome& other) const;

	uint8_t getID() const;
	uint32_t getColor(const RGBAImage& colors, bool flip_xy = false) const;

	static bool isBiomeBlock(uint16_t id, uint16_t data);
};

// different Minecraft Biomes
// first few biomes from Minecraft Overviewer
// temperature/rainfall data from Minecraft source code (via MCP)
// DO NOT directly access this array, use the getBiome function
static const Biome BIOMES[] = {
	{0, 0.5, 0.5},     // Ocean
	{1, 0.8, 0.4},     // Plains
	{2, 2.0, 0.0},     // Desert
	{3, 0.2, 0.3},     // Extreme Hills
	{4, 0.7, 0.8},     // Forest
	{5, 0.05, 0.8},    // Taiga
	{6, 0.8, 0.9, 205, 128, 255}, // Swampland
	{7, 0.5, 0.5},     // River
	{8, 2.0, 0.0},     // Hell (Nether)
	{9, 0.5, 0.5},     // Sky (End)

	{10, 0.0, 0.5},    // Frozen Ocean
	{11, 0.0, 0.5},    // Frozen River
	{12, 0.0, 0.5},    // Ice Plains
	{13, 0.0, 0.5},    // Ice Mountains
	{14, 0.9, 1.0},    // Mushroom Island
	{15, 0.9, 1.0},    // Mushroom Island Shore
	{16, 0.8, 0.4},    // Beach
	{17, 2.0, 0.0},    // Desert Hills
	{18, 0.7, 0.8},    // Forest Hills
	{19, 0.05, 0.8},   // Taiga Hills

	{20, 0.2, 0.3},    // Extreme Hills Edge
	{21, 0.95, 0.9},   // Jungle
	{22, 0.95, 0.9},   // Jungle Hills
	{23, 0.95, 0.8},   // Jungle Edge
	{24, 0.5, 0.5},    // Deep Ocean
	{25, 0.2, 0.3},    // Stone Beach
	{26, 0.05, 0.3},   // Cold Beach
	{27, 0.6, 0.6},    // Birch Forest
	{28, 0.6, 0.6},    // Birch Forest Hills
	{29, 0.7, 0.8},    // Roofed Forest

	// Cold Taiga/Cold Taiga Hills have -0.5/0.4 as temperature/rainfall
	// but I'm not sure yet how to handle negative temperatures
	// so they just have the colors of normal Taiga
	{30, 0.05, 0.8},   // Cold Taiga
	{31, 0.05, 0.8},   // Cold Taiga Hills
	{32, 0.3, 0.8},    // Mega Taiga
	{33, 0.3, 0.8},    // Mega Taiga Hills
	{34, 0.2, 0.3},    // Extreme Hills+
	{35, 1.2, 0.0},    // Savanna
	{36, 1.0, 0.0},    // Savanna Plateau
	{37, 2.0, 0},      // Mesa
	{38, 2.0, 0},      // Mesa Plateau F
	{39, 2.0, 0},      // Mesa Plateau

	{129, 0.8, 0.4},   // Sunflower Plains (= Plains)
	{130, 2.0, 0.0},   // Desert M (= Desert)
	{131, 0.2, 0.3},   // Extreme Hills M (= Extreme Hills)
	{132, 0.7, 0.8},   // Flower Forest (= Forest)
	{133, 0.05, 0.8},  // Taiga M (= Taiga)
	{134, 0.8, 0.9, 205, 128, 255}, // Swampland M (= Swampland)
	{140, 0.0, 0.5},   // Ice Plains Spikes
	{141, 0.0, 0.5},   // Ice Mountains Spikes
	{149, 0.95, 0.9},  // Jungle M (= Jungle)
	{151, 0.95, 0.9},  // Jungle Edge M (= Jungle Edge)
	{155, 0.6, 0.6},   // Birch Forest M (= Birch Forest)
	{156, 0.6, 0.6},   // Birch Forest Hills M (= Birch Forest)
	{157, 0.7, 0.8},   // Roofed Forest M (= Forest)
	{158, 0.05, 0.8},  // Cold Taiga M (= Cold Taiga)
	{160, 0.25, 0.8},  // Mega Spruce Taiga
	{161, 0.25, 0.8},  // Mega Spruce Taiga Hills
	{162, 0.2, 0.3},   // Extreme Hills+ M (= Extreme Hills)
	{163, 1.2, 0.0},   // Savanna M (= Savanna)
	{164, 1.0, 0.0},   // Savanna Plateau M (= Savanna Plateau)
	{165, 2.0, 0.0},   // Mesa (Bryce) (= Mesa)
	{166, 2.0, 0.0},   // Mesa Plateau F M (= Mesa Plateau)
	{167, 2.0, 0.0},   // Mesa Plateau M (= Mesa Plateau)
};

static const std::size_t BIOMES_SIZE = sizeof(BIOMES) / sizeof(Biome);
static const int DEFAULT_BIOME = 21;		// Jungle

Biome getBiome(uint8_t id);

} /* namespace render */
} /* namespace mapcrafter */
#endif /* BIOMES_H_ */
