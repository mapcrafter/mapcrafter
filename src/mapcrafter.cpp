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
	std::string config_file;
	std::string output_dir;
	int jobs;

	po::options_description all("Allowed options");
	all.add_options()
		("help,h", "shows a help message")
		("config,c",po::value<std::string>(&config_file),
			"the path of the world to render (required)")
		("jobs,j", po::value<int>(&jobs),
			"the count of jobs to render the map");

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

	if (!vm.count("config")) {
		std::cout << "You have to specify a config file!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	mapcrafter::render::RenderOpts opts;
	opts.config_file = config_file;
	opts.jobs = jobs;
	if (!vm.count("jobs"))
		opts.jobs = 1;

	opts.batch = vm.count("batch");
	mapcrafter::render::RenderManager manager(opts);
	if (!manager.run())
		return 1;
	return 0;
}
