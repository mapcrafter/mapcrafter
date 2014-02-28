/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "worldentities.h"

namespace mapcrafter {
namespace mc {

SignEntity::SignEntity() {
}

SignEntity::SignEntity(const mc::BlockPos& pos, const Lines& lines)
	: pos(pos), lines(lines), text() {
	// join the lines as sign text
	for (int i = 0; i < 4; i++) {
		std::string line = util::trim(lines[i]);
		if (line.empty())
			continue;
		text += line + " ";
	}
	text = util::trim(text);
}

SignEntity::~SignEntity() {
}

const mc::BlockPos& SignEntity::getPos() const {
	return pos;
}

const SignEntity::Lines& SignEntity::getLines() const {
	return lines;
}

const std::string& SignEntity::getText() const {
	return text;
}

WorldEntitiesCache::WorldEntitiesCache(const World& world)
	: world(world), cache_file(world.getRegionDir() / "entities.nbt.gz") {
}

WorldEntitiesCache::~WorldEntitiesCache() {
}

int WorldEntitiesCache::readCacheFile() {
	if (!fs::exists(cache_file))
		return 0;

	nbt::NBTFile nbt_file;
	nbt_file.readNBT(cache_file.string().c_str(), nbt::Compression::GZIP);

	nbt::TagList nbt_regions = nbt_file.findTag<nbt::TagList>("regions");
	for (auto region_it = nbt_regions.payload.begin();
			region_it != nbt_regions.payload.end(); ++region_it) {
		nbt::TagCompound region = (*region_it)->cast<nbt::TagCompound>();
		nbt::TagList chunks = region.findTag<nbt::TagList>("chunks");
		mc::RegionPos region_pos;
		region_pos.x = region.findTag<nbt::TagInt>("x").payload;
		region_pos.z = region.findTag<nbt::TagInt>("z").payload;

		for (auto chunk_it = chunks.payload.begin(); chunk_it != chunks.payload.end();
				++chunk_it) {
			nbt::TagCompound chunk = (*chunk_it)->cast<nbt::TagCompound>();
			nbt::TagList entities = chunk.findTag<nbt::TagList>("entities");
			mc::ChunkPos chunk_pos;
			chunk_pos.x = chunk.findTag<nbt::TagInt>("x").payload;
			chunk_pos.z = chunk.findTag<nbt::TagInt>("z").payload;

			for (auto entity_it = entities.payload.begin();
					entity_it != entities.payload.end(); ++entity_it) {
				nbt::TagCompound entity = (*entity_it)->cast<nbt::TagCompound>();
				this->entities[region_pos][chunk_pos].push_back(entity);
			}
		}
	}

	return fs::last_write_time(cache_file);
}

void WorldEntitiesCache::writeCacheFile() const {
	nbt::NBTFile nbt_file;
	nbt::TagList nbt_regions(nbt::TagCompound::TAG_TYPE);

	for (auto region_it = entities.begin(); region_it != entities.end(); ++region_it) {
		nbt::TagCompound nbt_region;
		nbt_region.addTag("x", nbt::TagInt(region_it->first.x));
		nbt_region.addTag("z", nbt::TagInt(region_it->first.z));
		nbt::TagList nbt_chunks(nbt::TagCompound::TAG_TYPE);
		for (auto chunk_it = region_it->second.begin();
				chunk_it != region_it->second.end(); ++chunk_it) {
			nbt::TagCompound nbt_chunk;
			nbt_chunk.addTag("x", nbt::TagInt(chunk_it->first.x));
			nbt_chunk.addTag("z", nbt::TagInt(chunk_it->first.z));
			nbt::TagList nbt_entities(nbt::TagCompound::TAG_TYPE);
			for (auto entity_it = chunk_it->second.begin();
					entity_it != chunk_it->second.end(); ++entity_it) {
				nbt_entities.payload.push_back(nbt::TagPtr(entity_it->clone()));
			}
			nbt_chunk.addTag("entities", nbt_entities);
			nbt_chunks.payload.push_back(nbt::TagPtr(nbt_chunk.clone()));
		}
		nbt_region.addTag("chunks", nbt_chunks);
		nbt_regions.payload.push_back(nbt::TagPtr(nbt_region.clone()));
	}

	nbt_file.addTag("regions", nbt_regions);
	nbt_file.writeNBT(cache_file.string().c_str(), nbt::Compression::GZIP);
}

void WorldEntitiesCache::update() {
	int timestamp = readCacheFile();

	auto regions = world.getAvailableRegions();
	for (auto region_it = regions.begin(); region_it != regions.end(); ++region_it) {
		if (fs::last_write_time(world.getRegionPath(*region_it)) < timestamp)
			continue;
		RegionFile region;
		world.getRegion(*region_it, region);
		region.read();

		auto chunks = region.getContainingChunks();
		for (auto chunk_it = chunks.begin(); chunk_it != chunks.end(); ++chunk_it) {
			if (region.getChunkTimestamp(*chunk_it) < timestamp)
				continue;

			mc::nbt::NBTFile nbt;
			const std::vector<uint8_t>& data = region.getChunkData(*chunk_it);
			nbt.readNBT(reinterpret_cast<const char*>(&data[0]), data.size(),
					mc::nbt::Compression::ZLIB);

			nbt::TagCompound& level = nbt.findTag<nbt::TagCompound>("Level");
			nbt::TagList& entities = level.findTag<nbt::TagList>("TileEntities");
			for (auto entity_it = entities.payload.begin();
					entity_it != entities.payload.end(); ++entity_it) {
				nbt::TagCompound entity = (*entity_it)->cast<nbt::TagCompound>();
				this->entities[*region_it][*chunk_it].push_back(entity);
			}
		}
	}

	writeCacheFile();
}

std::vector<SignEntity> WorldEntitiesCache::getSigns(WorldCrop worldcrop) const {
	std::vector<SignEntity> signs;

	for (auto region_it = entities.begin(); region_it != entities.end(); ++region_it) {
		if (!worldcrop.isRegionContained(region_it->first))
			continue;
		for (auto chunk_it = region_it->second.begin();
				chunk_it != region_it->second.end(); ++chunk_it) {
			if (!worldcrop.isChunkContained(chunk_it->first))
				continue;
			for (auto entity_it = chunk_it->second.begin();
					entity_it != chunk_it->second.end(); ++entity_it) {
				const nbt::TagCompound& entity = *entity_it;

				if (entity.findTag<nbt::TagString>("id").payload != "Sign")
					continue;

				mc::BlockPos pos(
					entity.findTag<nbt::TagInt>("x").payload,
					entity.findTag<nbt::TagInt>("z").payload,
					entity.findTag<nbt::TagInt>("y").payload
				);

				if (!worldcrop.isBlockContainedXZ(pos)
						|| !worldcrop.isBlockContainedY(pos))
					continue;

				mc::SignEntity::Lines lines = {{
					entity.findTag<nbt::TagString>("Text1").payload,
					entity.findTag<nbt::TagString>("Text2").payload,
					entity.findTag<nbt::TagString>("Text3").payload,
					entity.findTag<nbt::TagString>("Text4").payload
				}};

				signs.push_back(mc::SignEntity(pos, lines));
			}
		}
	}

	return signs;
}

} /* namespace mc */
} /* namespace mapcrafter */
