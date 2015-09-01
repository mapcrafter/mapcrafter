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

SlimeOverlay::SlimeOverlay(fs::path world_dir, int rotation)
	: OverlayRenderMode(OverlayMode::PER_BLOCK), world_dir(world_dir),
	  rotation(rotation), world_seed(0) {
	try {
		nbt::NBTFile level_dat;
		level_dat.readNBT((world_dir / "level.dat").string().c_str());

		nbt::TagCompound data = level_dat.findTag<nbt::TagCompound>("Data");
		nbt::TagLong random_seed = data.findTag<nbt::TagLong>("RandomSeed");
		world_seed = random_seed.payload;
	} catch (nbt::NBTError& e) {
		LOG(ERROR) << "Unable to read world seed from level.dat file for slime overlay: " << e.what();
	}
}

SlimeOverlay::~SlimeOverlay() {
}

bool SlimeOverlay::isSlimeChunk(const mc::ChunkPos& chunk, long long world_seed) {
	int32_t chunkx = chunk.x, chunkz = chunk.z;
	long long seed = world_seed
		+ (long long) (chunkx * chunkx * 0x4c1906)
		+ (long long) (chunkx * 0x5ac0db)
		+ (long long) (chunkz * chunkz * 0x4307a7LL)
		+ (long long)((chunkz * 0x5f24f) ^ 0x3ad8025f);

	JavaRandom random;
	random.setSeed(seed);
	return random.nextInt(10) == 0;
}

RGBAPixel SlimeOverlay::getBlockColor(const mc::BlockPos& pos, uint16_t id, uint16_t data) {
	// get original (not rotated) chunk position
	mc::ChunkPos chunk(pos);
	if (rotation) {
		// -rotation = -rotation + 4 (mod 4), rotate accepts only positive numbers
		chunk.rotate(-rotation + 4);
	}

	if (isSlimeChunk(chunk, world_seed))
		return rgba(60, 200, 20, 255);
	return rgba(0, 0, 0, 0);
}

}
}

