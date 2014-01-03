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

#include "renderer/manager.h"

#include "util.h"
#include "version.h"

#include <iostream>
#include <string>
#include <cstring>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// evil, I know
using namespace mapcrafter;

int main(int argc, char** argv) {
	std::string config_file;
	std::string output_dir;
	std::vector<std::string> render_skip, render_auto, render_force;
	int jobs;

	po::options_description all("Allowed options");
	all.add_options()
		("help,h", "shows a help message")
		("version,v", "shows the version of mapcrafter")
		("find-resources", "shows available resource directories")

		("config,c",po::value<std::string>(&config_file),
			"the path of the world to render (required)")

		("render-skip,s", po::value<std::vector<std::string>>(&render_skip)->multitoken(),
			"skips rendering the specified map(s)")
		("render-reset,r", "skips rendering all maps")
		("render-auto,a", po::value<std::vector<std::string>>(&render_auto)->multitoken(),
			"renders the specified map(s)")
		("render-force,f", po::value<std::vector<std::string>>(&render_force)->multitoken(),
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
		std::cout << "mapcrafter version: " << MAPCRAFTER_VERSION;
		if (strlen(MAPCRAFTER_GITVERSION))
			std::cout << " (" << MAPCRAFTER_GITVERSION << ")";
		std::cout << std::endl;
		return 0;
	}

	if (vm.count("find-resources")) {
		fs::path mapcrafter_bin = util::findExecutablePath();
		std::cout << "Your home directory: " << util::findHomeDir().string() << std::endl;
		std::cout << "mapcrafter binary: " << mapcrafter_bin.string() << std::endl;
		
		util::PathList resources = util::findResourceDirs(mapcrafter_bin);
		std::cout << "Resource directories:" << std::endl;
		for (size_t i = 0; i < resources.size(); i++)
			std::cout << "  " << i+1 << ". " << BOOST_FS_ABSOLUTE1(resources[i]).string() << std::endl;
		if (resources.size() == 0)
			std::cout << "  Nothing found." << std::endl;

		util::PathList templates = util::findTemplateDirs(mapcrafter_bin);
		std::cout << "Template directories:" << std::endl;
		for (size_t i = 0; i < templates.size(); i++)
			std::cout << "  " << i+1 << ". " << BOOST_FS_ABSOLUTE1(templates[i]).string() << std::endl;
		if (templates.size() == 0)
			std::cout << "  Nothing found." << std::endl;

		util::PathList textures = util::findTextureDirs(mapcrafter_bin);
		std::cout << "Texture directories:" << std::endl;
		for (size_t i = 0; i < textures.size(); i++)
			std::cout << "  " << i+1 << ". " << BOOST_FS_ABSOLUTE1(textures[i]).string() << std::endl;
		if (textures.size() == 0)
			std::cout << "  Nothing found." << std::endl;
		return 0;
	}

	if (!vm.count("config")) {
		std::cout << "You have to specify a config file!" << std::endl;
		std::cout << all << std::endl;
		return 1;
	}

	render::RenderOpts opts;
	opts.config_file = config_file;
	opts.render_skip = render_skip;
	opts.skip_all = vm.count("render-reset");
	opts.render_auto = render_auto;
	opts.render_force = render_force;
	opts.jobs = jobs;
	if (!vm.count("jobs"))
		opts.jobs = 1;

	opts.batch = vm.count("batch");
	render::RenderManager manager(opts);
	if (!manager.run())
		return 1;
	return 0;
}
