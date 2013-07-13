#include <iostream>
#include <string>

#include "mc/nbt.h"

namespace nbt = mapcrafter::mc::nbt;

int main(int argc, char** argv) {	
	if (argc < 2) {
		std::cerr << "Usage: ./nbtdump [--gzip|--zlib|--nocompression] [nbtfile]" << std::endl;
		return 1;
	}
	
	nbt::Compression cmpr = nbt::Compression::GZIP;
	std::string filename = argv[1];
	if (argc > 2) {
		std::string cmpr_arg = argv[1];
		filename = argv[2];
		
		if (cmpr_arg == "--gzip")
			cmpr = nbt::Compression::GZIP;
		else if (cmpr_arg == "--zlib")
			cmpr = nbt::Compression::ZLIB;
		else if (cmpr_arg == "--nocompression")
			cmpr = nbt::Compression::NO_COMPRESSION;
		else {
			std::cerr << "Invalid compression type: " << cmpr_arg << std::endl;
			return 1;
		}
	}
	
	nbt::NBTFile f;
	f.readNBT(filename.c_str(), cmpr);
	f.dump(std::cout);
	return 0;
}
