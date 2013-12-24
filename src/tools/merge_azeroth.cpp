#include "../util.h"
#include "../mc/nbt.h"
#include "../mc/world.h"

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

namespace util = mapcrafter::util;
namespace mc = mapcrafter::mc;
namespace nbt = mapcrafter::mc::nbt;

void mergeRegions(std::string outname, const std::vector<std::string>& regions) {
	if (!boost::filesystem::exists(regions[0])) {
		std::cout << "Can skip " << outname << std::endl;
		return;
	}

	// cp regions[0] outname
	// to start with the lowest layers as region
	system(std::string("cp ").append(regions[0]).append(" ").append(outname).c_str());
	mc::RegionFile region0(outname);
	if (!region0.read()) {
		std::cerr << "Unable to read " << outname << " (" << regions[0] << ")!" << std::endl;
		return;
	}

	bool has_chunk[1024];
	nbt::NBTFile chunks[1024];
	for (int x = 0; x < 32; x++)
		for (int z = 0; z < 32; z++) {
			mc::ChunkPos chunkpos(x, z);
			has_chunk[z*32+x] = false;
			if (!region0.hasChunk(chunkpos))
				continue;
			has_chunk[z*32+x] = true;
			const std::vector<uint8_t>& data = region0.getChunkData(chunkpos);
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
	for (size_t i = 1; i < regions.size(); i++) {
		std::string filename = regions[i];
		mc::RegionFile region2(filename);
		if (!region2.read()) {
			std::cerr << "Warning: Unable to read region " << filename << std::endl;
			continue;
		}
		sections_diff += 12;

		for (int x = 0; x < 32; x++)
			for (int z = 0; z < 32; z++) {
				mc::ChunkPos chunkpos(x, z);
				if (!region0.hasChunk(chunkpos) || !region2.hasChunk(chunkpos))
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
					if (i != regions.size()-1 && y.payload == 15)
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
			region0.setChunkData(chunkpos, data, 2);

		}

	if (!region0.write()) {
		std::cerr << "Unable to write back region " << outname << std::endl;
	}
}

std::string getRegionName(int x, int z, int layer) {
	z += layer*100;
	return std::string("r.") + util::str(x) + "." + util::str(z) + ".mca";
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./merge_azeroth [input-regions] [output-regions]" << std::endl;
		return 1;
	}

	std::string input(argv[1]), output(argv[2]);
	if (input[input.size()-1] != '/')
		input += '/';
	if (output[output.size()-1] != '/')
		output += '/';

	std::vector<int> layers = {-1, 0, 1, 2, 3, 4, 5};

	util::ProgressBar progress;
	progress.setMax(100*100);
	progress.setValue(0);
	for (int x = -49; x <= 49; x++) {
		for (int z = -49; z <= 49; z++) {
			std::string outname(output + getRegionName(x, z, layers[0]));
			std::vector<std::string> regions;
			for (size_t i = 0; i < layers.size(); i++) {
				int layer = layers[i];
				regions.push_back(input + getRegionName(x, z, layer));
			}

			mergeRegions(outname, regions);

			progress.setValue(progress.getValue()+1);
		}
	}
}
