#include "../mapcraftercore/mc/nbt.h"
#include "../mapcraftercore/mc/region.h"

#include <iostream>
#include <sstream>
#include <fstream>

namespace mc = mapcrafter::mc;
namespace nbt = mc::nbt;

int main(int argc, char** argv) {
	if (argc < 4) {
		std::cerr << "Usage: ./merge_regions out.mca in.1.mca in.2.mca ... in.N.mca" << std::endl;
		return 1;
	}

	// cp argv[2] argv[1]
	system(std::string("cp ").append(argv[2]).append(" ").append(argv[1]).c_str());

	mc::RegionFile region1(argv[1]);
	if (!region1.read()) {
		std::cerr << "Unable to read " << argv[1] << " (" << argv[2] << ")!" << std::endl;
		return 1;
	}

	bool has_chunk[1024];
	nbt::NBTFile chunks[1024];
	for (int x = 0; x < 32; x++)
		for (int z = 0; z < 32; z++) {
			mc::ChunkPos chunkpos(x, z);
			has_chunk[z*32+x] = false;
			if (!region1.hasChunk(chunkpos))
				continue;
			has_chunk[z*32+x] = true;
			const std::vector<uint8_t>& data = region1.getChunkData(chunkpos);
			chunks[z*32+x].readNBT(reinterpret_cast<const char*>(&data[0]),
					data.size(), nbt::Compression::ZLIB);
			//chunks[z*32+x].dump(std::cout);

			nbt::TagCompound& level = chunks[z*32+x].findTag<nbt::TagCompound>("Level");
			nbt::TagList sections = level.findTag<nbt::TagList>("Sections");
			nbt::TagList sections2(nbt::TagCompound::TAG_TYPE);

			for (auto it = sections.payload.begin(); it != sections.payload.end(); ++it) {
				nbt::TagCompound& section = (*it)->cast<nbt::TagCompound>();
				int y = section.findTag<nbt::TagByte>("Y").payload;
				if (y == 15)
					continue;

				sections2.payload.push_back(nbt::TagPtr(section.clone()));
			}
			level.addTag("Sections", sections2);
		}

	int sections_diff = 0;
	for (int i = 3; i < argc; i++) {
		std::string filename = argv[i];
		mc::RegionFile region2(filename);
		if (!region2.read()) {
			std::cerr << "Unable to read region " << filename << std::endl;
			return 1;
		}
		sections_diff += 12;

		for (int x = 0; x < 32; x++)
			for (int z = 0; z < 32; z++) {
				mc::ChunkPos chunkpos(x, z);
				if (!region1.hasChunk(chunkpos) || !region2.hasChunk(chunkpos))
					continue;

				const std::vector<uint8_t>& data = region2.getChunkData(chunkpos);
				nbt::NBTFile chunk2;
				chunk2.readNBT(reinterpret_cast<const char*>(&data[0]),
						data.size(), nbt::Compression::ZLIB);

				nbt::TagCompound& level2 = chunk2.findTag<nbt::TagCompound>("Level");
				nbt::TagList sections2 = level2.findTag<nbt::TagList>("Sections");

				nbt::TagCompound& level1 = chunks[z*32+x].findTag<nbt::TagCompound>("Level");
				nbt::TagList& sections1 = level1.findTag<nbt::TagList>("Sections");

				for (auto it = sections2.payload.begin(); it != sections2.payload.end(); ++it) {
					nbt::TagCompound section = (*it)->cast<nbt::TagCompound>();
					nbt::TagByte& y = section.findTag<nbt::TagByte>("Y");

					// remove the highest section, except of the last layer
					if (i != argc-1 && y.payload == 15)
						continue;
					// remove the first three sections
					if (y.payload < 3)
						continue;
					y.payload += sections_diff;
					sections1.payload.push_back(nbt::TagPtr(section.clone()));
				}
			}
	}

	for (int x = 0; x < 32; x++)
		for (int z = 0; z < 32; z++) {
			if (!has_chunk[z*32+x])
				continue;
			mc::ChunkPos chunkpos(x, z);

			std::stringstream out;
			chunks[z*32+x].writeNBT(out, nbt::Compression::ZLIB);
			std::vector<uint8_t> data(out.str().size());
			out.seekg(0, std::ios::beg);
			out.read(reinterpret_cast<char*>(&data[0]), out.str().size());
			region1.setChunkData(chunkpos, data, 2);

		}

	if (!region1.write()) {
		std::cout << "Unable to write back region " << argv[1] << std::endl;
		return 1;
	}
}
