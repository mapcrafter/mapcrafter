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

#include "slimeoverlay.h"

namespace mapcrafter {
namespace renderer {

JavaRandom::JavaRandom()
	: seed(0) {
}

JavaRandom::~JavaRandom() {
}

void JavaRandom::setSeed(long long seed) {
	this->seed = (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
}

int JavaRandom::next(int bits) {
	seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	return (int) (seed >> (48 - bits));
}

int JavaRandom::nextInt(int max) {
	// invalid max
	if (max <= 0)
		return -1;
	if ((max & -max) == max) // i.e., n is a power of 2
		return (int) ((max * (long) next(31)) >> 31);
	int bits, val;
	do {
		bits = next(31);
		val = bits % max;
	} while (bits - val + (max - 1) < 0);
	return val;
}

SlimeOverlay::SlimeOverlay()
	: OverlayRenderMode(OverlayMode::PER_BLOCK) {
}

SlimeOverlay::~SlimeOverlay() {
}

RGBAPixel SlimeOverlay::getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	mc::ChunkPos chunk(pos);
	// TODO we need the actual world seed!
	// TODO make sure that overlay is correct
	long long world_seed = 0;
	long long seed = world_seed;
	seed += (long) chunk.x * chunk.x * 0x4c1906;
	seed += (long) chunk.x * 0x5ac0db;
	seed += (long) chunk.z * chunk.z * 0x4307a7L;
	seed += (long) (chunk.z * 0x5f24f) ^ 0x3ad8025f;

	JavaRandom random;
	random.setSeed(seed);
	if (random.nextInt(10) == 0)
		return rgba(60, 200, 20, 255);
	return rgba(0, 0, 0, 0);
}

}
}

