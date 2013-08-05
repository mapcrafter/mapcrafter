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

#include "version.h"

#include <iostream>
#include <string>
#include <cstring>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv) {
	std::string config_file;
	std::string output_dir;
	std::string render_skip, render_auto, render_force;
	int jobs;

	po::options_description all("Allowed options");
	all.add_options()
		("help,h", "shows a help message")
		("version,v", "shows the version of mapcrafter")

		("config,c",po::value<std::string>(&config_file),
			"the path of the world to render (required)")

		("render-skip,s", po::value<std::string>(&render_skip),
			"skips rendering the specified map(s)")
		("render-reset,r", "skips rendering all maps")
		("render-auto,a", po::value<std::string>(&render_auto),
			"renders the specified map(s)")
		("render-force,f", po::value<std::string>(&render_force),
			"renders the specified map(s) completely")

		("jobs,j", po::value<int>(&jobs),
			"the count of jobs to render the map")
		("batch,b", "deactivates the animated progress bar");

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

	if (vm.count("version")) {
		std::cout << "mapcrafter version: " << mapcrafter::MAPCRAFTER_VERSION;
		if (strlen(mapcrafter::MAPCRAFTER_GITVERSION))
			std::cout << " (" << mapcrafter::MAPCRAFTER_GITVERSION << ")";
		std::cout << std::endl;
		return 0;
	}

	if (!vm.count("config")) {
		std::cout << "You have to specify a config file!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	mapcrafter::render::RenderOpts opts;
	opts.config_file = config_file;
	opts.render_skip = render_skip;
	opts.skip_all = vm.count("render-reset");
	opts.render_auto = render_auto;
	opts.render_force = render_force;
	opts.jobs = jobs;
	if (!vm.count("jobs"))
		opts.jobs = 1;

	opts.batch = vm.count("batch");
	mapcrafter::render::RenderManager manager(opts);
	if (!manager.run())
		return 1;
	return 0;
}
