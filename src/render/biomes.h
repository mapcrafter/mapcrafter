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
	Biome(uint8_t id = 0, double temperature = 0, double rainfall = 0,
			uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);

	Biome& operator+=(const Biome& other);
	Biome& operator/=(int n);
	bool operator==(const Biome& other) const;

	uint8_t getID() const;
	uint32_t getColor(const Image& colors, bool flip_xy = false) const;

	static bool isBiomeBlock(uint16_t id, uint16_t data);
};

// different Minecraft Biomes
// first few biomes from Minecraft Overviewer
// temperature/rainfall data from Minecraft source code (via MCP)
// DO NOT directly access this array, use the getBiome function
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
	{22, 2.0, 0.25}, // Jungle Hills

	{23, 0.95, 0.8}, // Jungle Edge
	{24, 0.5, 0.5}, // Deep Ocean
	{25, 0.2, 0.3}, // Stone Beach
	{26, 0.05, 0.3}, // Cold Beach
	{27, 0.6, 0.6}, // Birch Forest
	{28, 0.6, 0.6}, // Birch Forest Hills
	{29, 0.7, 0.8}, // Roofed Forest
	// Cold Taiga/Cold Taiga Hills have -0.5/0.4 as temperature/rainfall
	// but I'm not sure yet how to handle negative temperatures
	// so they just have the colors of normal Taiga
	{30, 0.05, 0.8}, // Cold Taiga
	{31, 0.05, 0.8}, // Cold Taiga Hills
	{32, 0.3, 0.8}, // Mega Taiga
	{33, 0.3, 0.8}, // Mega Taiga Hills
	{34, 0.2, 0.3}, // Extreme Hills+
	{35, 1.2, 0.0}, // Savanna
	{36, 1.0, 0.0},// id 36 // Savanna Plateau
	{37, 2.0, 0}, // Mesa
	{38, 2.0, 0}, // Mesa Plateau F
	{39, 2.0, 0}, // Mesa Pleateau

	// id 129 // Sunflower Plains
	// id 130 // Desert M
	// id 131 // Extreme Hills M
	// id 132 // Flower Forest
	// id 133 // Taiga M
	// id 134 // Swampland M
	// id 140 // Ice Plains Spikes
	// id 141 // Ice Mountains Spikes
	// id 149 // Jungle M
	// id 151 // Jungle Edge M
	// id 155 // Birch Forest M
	// id 156 // Birch Forest Hills M
	// id 157 // Roffed Forest M
	// id 158 // Cold Taiga M
	// id 160 // Mega Spruce Taiga
	// id 161 // Mega Spruce Taiga Hills
	// id 162 // Extreme Hills+ M
	// id 163 // Savanna M
	// id 164 // Savanna Plateau M
	// id 165 // Mesa (Bryce)
	// id 166 // Mesa Plateau F M
	// id 167 // Mesa Pleatau M
};

static const size_t BIOMES_SIZE = sizeof(BIOMES) / sizeof(Biome);
static const int DEFAULT_BIOME = 21; // Jungle

} /* namespace render */
} /* namespace mapcrafter */
#endif /* BIOMES_H_ */
