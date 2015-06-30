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

#include "../../mc/nbt.h"
#include "../../util.h"

namespace nbt = mapcrafter::mc::nbt;

namespace mapcrafter {
namespace renderer {

JavaRandom::JavaRandom()
	: seed(0) {
}

JavaRandom::~JavaRandom() {
}

void JavaRandom::setSeed(long long seed) {
	this->seed = (seed ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
}

int JavaRandom::next(int bits) {
	seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
	return (int) (seed >> (48 - bits));
}

int JavaRandom::nextInt(int max) {
	// invalid max
	if (max <= 0)
		return -1;
	if ((max & -max) == max) // i.e., n is a power of 2
		return (int) ((max * (long long) next(31)) >> 31);
	int bits, val;
	do {
		bits = next(31);
		val = bits % max;
	} while (bits - val + (max - 1) < 0);
	return val;
}

SlimeOverlay::SlimeOverlay(fs::path world_dir)
	: OverlayRenderMode(OverlayMode::PER_BLOCK), world_dir(world_dir), world_seed(0) {
	// TODO error handling!
	nbt::NBTFile level_dat;
	level_dat.readNBT((world_dir / "level.dat").string().c_str());

	nbt::TagCompound data = level_dat.findTag<nbt::TagCompound>("Data");
	nbt::TagLong random_seed = data.findTag<nbt::TagLong>("RandomSeed");
	world_seed = random_seed.payload;
}

SlimeOverlay::~SlimeOverlay() {
}

RGBAPixel SlimeOverlay::getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	mc::ChunkPos chunk(pos);
	// TODO not sure yet if that's really right
	long long seed = world_seed
		+ chunk.x * chunk.x * 0x4c1906LL
		+ chunk.x * 0x5ac0dbLL
		+ chunk.z * chunk.z * 0x4307a7LL
		+ (chunk.z * 0x5f24fLL) ^ 0x3ad8025fLL;

	JavaRandom random;
	random.setSeed(seed);
	if (random.nextInt(10) == 0)
	//if (is_slime(world_seed, chunk.x, chunk.z))
		return rgba(60, 200, 20, 255);
	return rgba(0, 0, 0, 0);
}

}
}

