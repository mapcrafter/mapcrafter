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

#ifndef BIOMES_H_
#define BIOMES_H_

#include "image.h"
#include "../mc/java.h"

#include <cstdint>
#include <cstdlib>

namespace mapcrafter {
namespace mc {
class BlockPos;
}

namespace renderer {

class RGBAImage;
enum class ColorMapType;
class ColorMap;

static const uint32_t one = rgba(0xff, 0xff, 0xff, 0xff);
static const uint32_t default_water = rgba(0x30, 0x59, 0xad, 0xff);
static const uint32_t unknown_tint = rgba(0xff, 0x00, 0xff, 0xff);

/**
 * A Minecraft Biome with data to tint the biome-depend blocks.
 */
class Biome {
private:
	// id of the biome
	uint16_t id;

	// temperature and rainfall
	// used to calculate the position of the tinting color in the color image
	double temperature;
	double rainfall;

	uint32_t green_tint, water_tint;

	static const mc::JavaSimplexGenerator SWAMP_GRASS_NOISE;
public:
	Biome(uint16_t id = 0, double temperature = 0, double rainfall = 0,
			uint32_t green_tint = one, uint32_t water_tint = default_water);

	uint16_t getID() const;
	uint32_t getColor(const mc::BlockPos& pos, const ColorMapType& color_type, const ColorMap& color_map) const;
};

// different Minecraft Biomes
// first few biomes from Minecraft Overviewer
// temperature/rainfall data from Minecraft source code (via MCP)
// DO NOT directly access this array, use the getBiome function
static const Biome BIOMES[] = {
	{0, 0.5, 0.5},     // Ocean
	{1, 0.8, 0.4, one, default_water},     // Plains
	{2, 2.0, 0.0},     // Desert
	{3, 0.2, 0.3},     // Extreme Hills
	{4, 0.7, 0.8, one, default_water},     // Forest
	{5, 0.25, 0.8, one, default_water},    // Taiga
	//{6, 0.8, 0.9, 205, 128, 255}, // Swampland
	//{6, 0.8, 0.9, 0x4C, 0x76, 0x3C}, // Swampland (greenish)
	{6, 0.8, 0.9, rgba(0x6a, 0x70, 0x39, 0xff), rgba(0x6A, 0x70, 0x39, 0xff)}, // Swampland (brownish)
	{7, 0.5, 0.5, one, default_water},     // River
	{8, 2.0, 0.0},     // Nether Wastes
	{9, 0.5, 0.5},     // The End

	{10, 0.0, 0.5, one, rgba(0x2b, 0x2a, 0x98, 0xff)},    // Frozen Ocean
	{11, 0.0, 0.5, one, rgba(0x2b, 0x2a, 0x98, 0xff)},    // Frozen River
	{12, 0.0, 0.5},    // Snowy Tundra
	{13, 0.0, 0.5},    // Snowy Mountains
	{14, 0.9, 1.0},    // Mushroom Island
	{15, 0.9, 1.0},    // Mushroom Island Shore
	{16, 0.8, 0.4, one, default_water},    // Beach
	{17, 2.0, 0.0},    // Desert Hills
	{18, 0.7, 0.8, one, default_water},    // Wooden Hills
	{19, 0.25, 0.8, one, default_water},   // Taiga Hills

	{20, 0.2, 0.3},    // Mountain Edge
	{21, 0.95, 0.9},   // Jungle
	{22, 0.95, 0.9},   // Jungle Hills
	{23, 0.95, 0.8},   // Jungle Edge
	{24, 0.5, 0.5},    // Deep Ocean
	{25, 0.2, 0.3, one, default_water},    // Stone Beach
	{26, 0.05, 0.3},   // Snowy Beach
	{27, 0.6, 0.6, one, default_water},    // Birch Forest
	{28, 0.6, 0.6, one, default_water},    // Birch Forest Hills
	{29, 0.7, 0.8, one, default_water},    // Dark Forest

	{30, -0.5, 0.4},   // Snowy Taiga
	{31, -0.5, 0.4},   // Snowy Taiga Hills
	{32, 0.3, 0.8, one, default_water},    // Giant Tree Taiga
	{33, 0.3, 0.8, one, default_water},    // Giant Tree Taiga Hills
	{34, 0.2, 0.3},    // Wooden Mountains
	{35, 1.2, 0.0},    // Savanna
	{36, 1.0, 0.0},    // Savanna Plateau
	{37, 2.0, 0},      // Badlands
	{38, 2.0, 0},      // Wooden Badlands Plateau
	{39, 2.0, 0},      // Badlands Plateau

	{40, 0.5, 0.4}, // Small End Islands
	{41, 0.5, 0.4}, // End Midlands
	{42, 0.5, 0.4}, // End Highlands
	{43, 0.5, 0.4}, // End Barrens
	{44, 0.8, 0.4, one, rgba(0x33, 0xa1, 0xb4, 0xff)}, // Warm Ocean
	{45, 0.8, 0.4, one, rgba(0x33, 0xa1, 0xb4, 0xff)}, // Lukewarm Ocean
	{46, 0.8, 0.4, one, rgba(0x2e, 0x42, 0xa2, 0xff)}, // Cold Ocean
	{47, 0.8, 0.4, one, rgba(0x33, 0xa1, 0xb4, 0xff)}, // Deep Warm Ocean
	{48, 0.8, 0.4, one, rgba(0x33, 0xa1, 0xb4, 0xff)}, // Deep Lukewarm Ocean
	{49, 0.8, 0.4, one, rgba(0x2e, 0x42, 0xa2, 0xff)}, // Deep Cold Ocean
	{50, 0.8, 0.4, one, rgba(0x2b, 0x2a, 0x98, 0xff)}, // Deep Frozen Ocean

	{127, 0.5, 0.5}, // The Void

	{129, 0.8, 0.4, one, default_water},   // Sunflower Plains
	{130, 2.0, 0.0},   // Desert Lakes
	{131, 0.25, 0.3},   // Gravelly Mountains
	{132, 0.7, 0.8, one, default_water},   // Flower Forest
	{133, 0.05, 0.8, one, default_water},  // Taiga Mountains
	//{134, 0.8, 0.9, 205, 128, 255}, // Swampland M (= Swampland)
	//{134, 0.8, 0.9, 0x4C, 0x76, 0x3C}, // Swampland M (= Swampland) (greenish)
	{134, 0.8, 0.9, rgba(0x6a, 0x70, 0x39, 0xff), rgba(0x6A, 0x70, 0x39, 0xff)}, // Swamp Hills (brownish)
	
	{140, 0.0, 0.5},   // Ice Spikes
	
	{149, 0.95, 0.9},  // Modified Jungle
	{151, 0.95, 0.9},  // Modified Jungle Edge
	
	{155, 0.6, 0.6, one, default_water},   // Tall Birch Forest
	{156, 0.6, 0.6, one, default_water},   // Tall Birch Hills
	{157, 0.7, 0.8, one, default_water},   // Dark Forest Hills
	{158, 0.05, 0.8},                          // Snowy Taiga Mountains

	{160, 0.25, 0.8, one, default_water},  // Giant Spruce Taiga
	{161, 0.25, 0.8, one, default_water},  // Giant Spruce Taiga Hills
	{162, 0.2, 0.3},   // Gravelly Mountains +
	{163, 1.2, 0.0},   // Shattered Savanna
	{164, 1.0, 0.0},   // Shattered Savanna Plateau
	{165, 2.0, 0.0},   // Eroded Badlands
	{166, 2.0, 0.0},   // Modified Wooden Badlands Plateau
	{167, 2.0, 0.0},   // Modified Badlands Plateau

	{168, 0.95, 0.9},  // Bamboo Jungle
	{169, 0.95, 0.9},  // Bamboo Jungle Hills

	{170, 2.0, 0.0},   // Soul Sand Valley
	{171, 2.0, 0.0},   // Crimson Forest
	{172, 2.0, 0.0},   // Warped Forest

	{210, 0.0, 0.0, unknown_tint, unknown_tint}, // "Unknown" biome
};

static const std::size_t BIOMES_SIZE = sizeof(BIOMES) / sizeof(Biome);
static const int DEFAULT_BIOME = 21; // Jungle

Biome getBiome(uint16_t id);

} /* namespace render */
} /* namespace mapcrafter */
#endif /* BIOMES_H_ */
