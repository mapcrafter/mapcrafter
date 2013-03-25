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

#include "render/manager.h"

#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv) {
	mapcrafter::render::RenderOpts opts;
	std::string input_dir;
	std::string output_dir;
	std::string template_dir, textures_dir;

	std::string north_dir;

	po::options_description all("Allowed options");
	all.add_options()
		("help,h", "shows a help message")
		("input-dir,i",po::value<std::string>(&input_dir),
			"the path of the world to render (required)")
		("output-dir,o", po::value<std::string>(&output_dir),
			"the path to save the rendered map (required)")
		("template", po::value<std::string>(&template_dir),
			"the path with the template files (default data/template)")
		("textures", po::value<std::string>(&textures_dir),
			"the path with the texture images (default data/textures)")

		("jobs,j", po::value<int>(&opts.jobs),
			"the number of threads to render the map (default 1)")
		("incremental,u", "renders only the changed tiles")
		("batch,b", "deactivates the animated progress bar")

		("texture-size", po::value<int>(&opts.texture_size),
			"size of the minecraft textures (default 12 (px))")
		("north-dir", po::value<std::string>(&north_dir),
			"rotation of the map: top-left, top-right, bottom-right, bottom-left"
			"\ntop-left means, that north is on the top left, this is default")
		("render-unknown-blocks", "renders unknown blocks as red blocks")
		("render-leaves-transparent", "uses the transparent texture for leaves");

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, all), vm);
	} catch (po::unknown_option& ex) {
		std::cout << "I have no idea what you mean with '" << ex.get_option_name()
		        << "'..." << std::endl;
		std::cout << all << std::endl;
		return 1;
	} catch (po::invalid_option_value& ex) {
		std::cout << "I can't convert the value of '" << ex.get_option_name() << "'..."
		        << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	po::notify(vm);

	if (vm.count("help")) {
		std::cout << all << std::endl;
		return 1;
	}

	if (!vm.count("input-dir")) {
		std::cout << "You have to specify an input directory!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	} else if (!vm.count("output-dir")) {
		std::cout << "You have to specify an output directory!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	opts.incremental = vm.count("incremental");
	opts.batch = vm.count("batch");
	if (opts.incremental && vm.count("texture-size")) {
		std::cout << "You can't set a texture size when incremental rendering activated!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	opts.input_dir = fs::path(input_dir);
	opts.output_dir = fs::path(output_dir);
	opts.template_dir = fs::path(template_dir);
	opts.textures_dir = fs::path(textures_dir);
	if (!vm.count("template"))
		opts.template_dir = fs::path("data/template");
	if (!vm.count("textures"))
		opts.textures_dir = fs::path("data/textures");
	if (!vm.count("jobs"))
		opts.jobs = 1;
	if (!vm.count("texture-size"))
		opts.texture_size = 12;

	if (!vm.count("north-dir"))
		north_dir = "top-left";

	std::string directions[] = {"top-left", "top-right", "bottom-right", "bottom-left"};
	for (int i = 0; i < 4; i++) {
		if (north_dir.compare(directions[i]) == 0) {
			opts.rotation = i;
			break;
		}

		if (i == 3) {
			std::cout << "Invalid north direction: '" << north_dir << "'" << std::endl;
			std::cout << "Possible directions are: top-left, top-right, bottom-right, bottom-left" << std::endl;
			std::cout << all << std::endl;
			return 1;
		}
	}

	opts.render_unknown_blocks = vm.count("render-unknown-blocks");
	opts.render_leaves_transparent = vm.count("render-leaves-transparent");

	mapcrafter::render::RenderManager manager(opts);
	if (!manager.run())
		return 1;
	return 0;
}
