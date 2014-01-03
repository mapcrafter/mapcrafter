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

#include "../renderer/blockimages.h"

#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace mapcrafter::renderer;

int main(int argc, char **argv) {
	std::string texture_dir;
	std::string output_file;
	int texture_size;

	po::options_description all("Allowed options");
	all.add_options()
		("help,h", "shows a help message")

		("texture-dir,i",po::value<std::string>(&texture_dir),
			"the path to the textures (required)")
		("output-image,o", po::value<std::string>(&output_file),
			"the path to the output image (default: blocks.png)")
		("texture-size,t", po::value<int>(&texture_size),
			"the texture size used to generate the blocks (default: 16)");

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, all), vm);
	} catch (po::error& ex) {
		std::cout << "There is a problem parsing the command line arguments: "
				<< ex.what() << std::endl << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	po::notify(vm);

	if (vm.count("help")) {
		std::cout << all << std::endl;
		return 1;
	}

	if (!vm.count("texture-dir")) {
		std::cout << "You have to specify a texture directory!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	if (!vm.count("output-image"))
		output_file = "blocks.png";
	if (!vm.count("texture-size"))
		texture_size = 16;

	BlockImages images;
	images.setSettings(texture_size, 0, true, true, "");
	if(images.loadAll(texture_dir)) {
		images.saveBlocks(output_file);
		return 0;
	}

	return 1;
}
