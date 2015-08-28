#!/usr/bin/env python2

# developer stuff:
# lists the block textures in the 'blocks' dir and generates the c++ texture code

import sys
import os
import re

HEADER_TEMPLATE = """#ifndef BLOCKTEXTURES_H_
#define BLOCKTEXTURES_H_

#include "textureimage.h"

#include <string>
#include <vector>

namespace mapcrafter {
namespace renderer {

/**
 * Collection of Minecraft block textures.
 */
class BlockTextures {
public:
	BlockTextures();
	~BlockTextures();

	bool load(const std::string& block_dir, int size, int blur, double water_opacity);

	TextureImage
		%(texture_objects);
	std::vector<TextureImage*> textures;
};

}
}

#endif /* BLOCKTEXTURES_H_ */"""

SOURCE_TEMPLATE = """#include "blocktextures.h"

#include "../util.h"

#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace renderer {

BlockTextures::BlockTextures()
	: %(texture_objects)
	  textures({ %(texture_object_references) }) {
}

BlockTextures::~BlockTextures() {
}

/**
 * Loads all block textures from the 'blocks' directory.
 */
bool BlockTextures::load(const std::string& block_dir, int size, int blur, double water_opacity) {
	if (!fs::exists(block_dir) || !fs::is_directory(block_dir)) {
		LOG(ERROR) << "Directory '" << block_dir << "' with block textures does not exist.";
		return false;
	}

	// go through all textures and load them
	bool loaded_all = true;
	for (size_t i = 0; i < textures.size(); i++) {
		if (!textures[i]->load(block_dir, size, blur, water_opacity)) {
			LOG(WARNING) << "Unable to load block texture '" << textures[i]->getName() << ".png'.";
			loaded_all = false;
		}
	}
	if (!loaded_all)
		LOG(WARNING) << "Unable to load some block textures.";
	return true;
}

}
}"""

def lowercase(name):
	return re.sub("[A-Z]", lambda m: "_" + m.group(0).lower(), name)

if __name__ == "__main__":
	if len(sys.argv) < 3 or sys.argv[1] not in ("--header", "--source"):
		print("Usage: %s [--header|--source] [directory]" % sys.argv[0])
		sys.exit(1)
	
	files = []
	for filename in os.listdir(sys.argv[2]):
		if not filename.endswith(".png"):
			continue
		name = filename.replace(".png", "")
		const_name = lowercase(name).upper()
		files.append((name, const_name))
	
	files.sort()
	
	if sys.argv[1] == "--header":
		texture_objects = ",\n\t\t".join(map(lambda name: "%s" % name[1], files))
		print(HEADER_TEMPLATE.replace("%(texture_objects)", texture_objects))
	if sys.argv[1] == "--source":
		texture_objects = "\n\t  ".join(map(lambda name: "%s(\"%s\")," % (name[1], name[0]), files))
		texture_object_references = "\n\t             ".join(map(lambda name: "&%s," % name[1], files))
		print(SOURCE_TEMPLATE.replace("%(texture_objects)", texture_objects).replace("%(texture_object_references)", texture_object_references))

	# code for textures.h
	
	#print "\tTextureImage %s;" % ", ".join(map(lambda f: f[1], files))
	#print "\tstd::vector<TextureImage*> textures;"
	
	#print ""
	
	# code for textures.cpp
	
	#print ""
	#print ": " + ", ".join(map(lambda f: "%s(\"%s\")" % (f[1], f[0]), files)) \
	#		 + ", textures({" + ", ".join(map(lambda f: "&%s" % f[1], files)) + "}) {"
