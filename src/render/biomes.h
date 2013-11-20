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

#ifndef BIOMES_H_
#define BIOMES_H_

#include "image.h"

namespace mapcrafter {
namespace render {

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
	Biome(uint8_t id, double temperature, double rainfall,
			uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);

	Biome& operator+=(const Biome& other);
	Biome& operator/=(int n);
	bool operator==(const Biome& other) const;

	uint8_t getID() const;
	uint32_t getColor(const Image& colors, bool flip_xy = false) const;

	static bool isBiomeBlock(uint16_t id, uint16_t data);
};

// different Minecraft Biomes
// from Minecraft Overviewer (from Minecraft MCP source code)
static const Biome BIOMES[] = {
	{0, 0.5, 0.5}, // Ocean
	{1, 0.8, 0.4}, // Plains
	{2, 2.0, 0.0}, // Desert
	{3, 0.2, 0.3}, // Extreme Hills
	{4, 0.7, 0.8}, // Forest

	{5, 0.05, 0.8}, // Taiga
	{6, 0.8, 0.9, 205, 128, 255}, // Swampland
	{7, 0.5, 0.5}, // River
	{8, 2.0, 0.0}, // Hell
	{9, 0.5, 0.5}, // Sky

	{10, 0.0, 0.5}, // Frozen Ocean
	{11, 0.0, 0.5}, // Frozen River
	{12, 0.0, 0.5}, // Ice Plains
	{13, 0.0, 0.5}, // Ice Mountains
	{14, 0.9, 1.0}, // Mushroom Island

	{15, 0.9, 1.0}, // Mushroom Island Shore
	{16, 0.8, 0.4}, // Beach
	{17, 2.0, 0.0}, // Desert Hills
	{18, 0.7, 0.8}, // Forest Hills
	{19, 0.05, 0.8}, // Taiga Hills

	{20, 0.2, 0.3}, // Extreme Hills Edge
	{21, 2.0, 0.45}, // Jungle
	{22, 2.0, 0.25}, // Jungle Mountains
};

static const size_t BIOMES_SIZE = sizeof(BIOMES) / sizeof(Biome);
static const int DEFAULT_BIOME = 21; // Jungle

Biome getBiome(uint8_t id);

} /* namespace render */
} /* namespace mapcrafter */
#endif /* BIOMES_H_ */
