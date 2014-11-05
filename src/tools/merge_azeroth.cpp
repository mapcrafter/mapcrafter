#include "../mapcraftercore/mc/nbt.h"
#include "../mapcraftercore/mc/world.h"
#include "../mapcraftercore/util.h"

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <thread>
#include <boost/filesystem.hpp>

namespace util = mapcrafter::util;
namespace mc = mapcrafter::mc;
namespace nbt = mapcrafter::mc::nbt;

bool mergeRegions(std::string outname, const std::vector<std::string>& regions) {
	if (boost::filesystem::exists(outname)) {
		std::cout << "Region " << outname << " is already merged; Skipping" << std::endl;
		return false;
	}
	if (!boost::filesystem::exists(regions[0])) {
		std::cout << "Can skip " << outname << std::endl;
		return false;
	}

	// cp regions[0] outname
	// to start with the lowest layers as region
	system(std::string("cp ").append(regions[0]).append(" ").append(outname).c_str());
	mc::RegionFile region0(outname);
	if (!region0.read()) {
		std::cerr << "Unable to read " << outname << " (" << regions[0] << ")!" << std::endl;
		return false;
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
		if (!boost::filesystem::exists(filename)) {
			//std::cerr << "Warning: Region " << filename << " does not exist." << std::endl;
			continue;
		}
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
			region0.setChunkTimestamp(chunkpos, time(nullptr));

		}

	if (!region0.write()) {
		std::cerr << "Unable to write back region " << outname << std::endl;
	}
	return true;
}

std::string getRegionName(int x, int z, int layer) {
	z += layer*100;
	return std::string("r.") + util::str(x) + "." + util::str(z) + ".mca";
}

typedef std::pair<std::string, std::vector<std::string>> Job;
typedef std::vector<Job> Work;

void runWorker(const Work& work) {
	for(auto job_it = work.begin(); job_it != work.end(); ++job_it) {
		mergeRegions(job_it->first, job_it->second);
	}
}

int main(int argc, char** argv) {
	if (argc < 4) {
		std::cerr << "Usage: ./merge_azeroth [worker-count] [input-regions] [output-regions]" << std::endl;
		return 1;
	}

	int worker_count = util::as<int>(argv[1]);
	std::string input(argv[2]), output(argv[3]);
	if (input[input.size()-1] != '/')
		input += '/';
	if (output[output.size()-1] != '/')
		output += '/';

	std::vector<int> layers = {-1, 0, 1, 2, 3, 4, 5};
	int min_x = -49, max_x = 49;
	int min_z = -49, max_z = 49;
	int region_count = (max_x - min_x + 1) * (max_z - min_z + 1);
	std::cout << "Merging " << region_count << " regions with " << worker_count << " workers..." << std::endl;

	std::vector<Work> workers(worker_count);
	int cur_worker = worker_count-1;

	//util::ProgressBar progress;
	//progress.setMax(region_count);
	//progress.setValue(0);
	for (int x = min_x; x <= max_x; x++) {
		for (int z = min_z; z <= max_z; z++) {
			std::string outname(output + getRegionName(x, z, layers[0]));
			std::vector<std::string> regions;
			for (size_t i = 0; i < layers.size(); i++) {
				int layer = layers[i];
				regions.push_back(input + getRegionName(x, z, layer));
			}

			cur_worker = (cur_worker+1) % worker_count;
			workers[cur_worker].push_back(Job(outname, regions));

			//if (mergeRegions(outname, regions))
			//	progress.setValue(progress.getValue()+1);
			//else
			//	progress.setMax(progress.getMax()-1);
		}
	}

	std::vector<std::thread> threads;
	for (int i = 0; i < worker_count; i++)
		threads.push_back(std::thread(runWorker, workers[i]));
	for (int i = 0; i < worker_count; i++)
		threads[i].join();
}
