#include "../mapcraftercore/mc/region.h"

#include <iostream>
#include <ctime>

namespace mc = mapcrafter::mc;

int main(int argc, char** argv) {
	if (argc < 1) {
		std::cerr << "./touch_region [region1.mca region2.mca region3.mca ... regionN.mca]" << std::endl;
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		std::string filename = argv[i];
		mc::RegionFile region(filename);
		if (!region.read()) {
			std::cerr << "Unable to read region " << filename << std::endl;
			continue;
		}
		for (int x = 0; x < 32; x++)
			for (int z = 0; z < 32; z++) {
				mc::ChunkPos chunkpos(x, z);
				region.setChunkTimestamp(chunkpos, time(nullptr));
			}
		if (!region.write()) {
			std::cerr << "Unable to write region " << filename << std::endl;
		}
	}
}
