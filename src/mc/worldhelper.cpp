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

#include "worldhelper.h"

#include "../util.h"

namespace mapcrafter {
namespace mc {

Sign::Sign() {
}

Sign::Sign(const mc::BlockPos& pos, const Lines& lines)
	: pos(pos), lines(lines), text() {
	// join the lines as sign text
	for (int i = 0; i < 4; i++) {
		std::string line = lines[i];
		util::trim(line);
		if (line.empty())
			continue;
		text += line + " ";
	}
	util::trim(text);
}

Sign::~Sign() {
}

const mc::BlockPos& Sign::getPos() const {
	return pos;
}

const Sign::Lines& Sign::getLines() const {
	return lines;
}

const std::string& Sign::getText() const {
	return text;
}

std::vector<Sign> findSignsInWorld(const mc::World& world) {
	std::vector<Sign> signs;

	// TODO some more validation with unknown regions, chunks, nbt data...

	auto regions = world.getAvailableRegions();
	for (auto region_it = regions.begin(); region_it != regions.end(); ++region_it) {
		RegionFile region;
		world.getRegion(*region_it, region);
		region.read();

		auto chunks = region.getContainingChunks();
		for (auto chunk_it = chunks.begin(); chunk_it != chunks.end(); ++chunk_it) {
			mc::nbt::NBTFile nbt;
			const std::vector<uint8_t>& data = region.getChunkData(*chunk_it);
			nbt.readNBT(reinterpret_cast<const char*>(&data[0]), data.size(), mc::nbt::Compression::ZLIB);

			nbt::TagCompound& level = nbt.findTag<nbt::TagCompound>("Level");
			nbt::TagList& entities = level.findTag<nbt::TagList>("TileEntities");
			for (auto entity_it = entities.payload.begin();
					entity_it != entities.payload.end(); ++entity_it) {
				nbt::TagCompound entity = (*entity_it)->cast<nbt::TagCompound>();
				if (entity.findTag<nbt::TagString>("id").payload != "Sign")
					continue;

				mc::BlockPos pos(
					entity.findTag<nbt::TagInt>("x").payload,
					entity.findTag<nbt::TagInt>("z").payload,
					entity.findTag<nbt::TagInt>("y").payload
				);

				mc::Sign::Lines lines = {{
					entity.findTag<nbt::TagString>("Text1").payload,
					entity.findTag<nbt::TagString>("Text2").payload,
					entity.findTag<nbt::TagString>("Text3").payload,
					entity.findTag<nbt::TagString>("Text4").payload
				}};

				mc::Sign sign(pos, lines);
				signs.push_back(sign);
			}
		}
	}

	return signs;
}

} /* namespace mc */
} /* namespace mapcrafter */
