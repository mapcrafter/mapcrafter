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

#include "../mapcraftercore/renderer/renderviews/isometric/blockimages.h"
#include "../mapcraftercore/renderer/renderviews/topdown/blockimages.h"

#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace mapcrafter::renderer;

int main(int argc, char **argv) {
	std::string texture_dir, output_file, render_view;
	int texture_size, texture_blur;

	po::options_description all("Allowed options");
	all.add_options()
		("help,h", "shows a help message")

		("texture-dir,i",po::value<std::string>(&texture_dir),
			"the path to the textures (required)")
		("output-image,o", po::value<std::string>(&output_file),
			"the path to the output image (default: blocks.png)")
		("render-view,r", po::value<std::string>(&render_view),
			"the render view to generate block images for (default: 'isometric')")
		("texture-size,t", po::value<int>(&texture_size),
			"the texture size used to generate the blocks (default: 16)")
		("texture-blur,b", po::value<int>(&texture_blur),
			"the texture blur used to generate the blocks (default: 0)");

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
	if (!vm.count("render-view"))
		render_view = "isometric";
	else if (render_view != "isometric" && render_view != "topdown") {
		std::cerr << "Invalid render view '" << render_view << "'!" << std::endl;
		std::cerr << "Render view must be 'isometric' or 'topdown'!" << std::endl;
		return 1;
	}
	if (!vm.count("texture-size"))
		texture_size = 16;
	if (!vm.count("texture-blur"))
		texture_blur = 0;

	BlockImageTextureResources resources;
	resources.setTextureSize(texture_size, texture_blur);
	if (!resources.loadAll(texture_dir))
		return 0;

	BlockImages* images;
	if (render_view == "isometric") {
		IsometricBlockImages* isometric_images = new IsometricBlockImages();
		isometric_images->setBlockSideDarkening(0.75, 0.6);
		images = isometric_images;
	} else {
		images = new TopdownBlockImages();
	}
	images->setRotation(0);
	images->setRenderSpecialBlocks(true, true);
	images->loadBlocks(resources);
	images->saveBlocks(output_file);

	return 1;
}
