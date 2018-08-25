/*
 * Copyright 2012-2016 Moritz Hilscher
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

#include "mapcraftercore/config/loggingconfig.h"
#include "mapcraftercore/renderer/manager.h"
#include "mapcraftercore/util.h"
#include "mapcraftercore/version.h"

#include <iostream>
#include <string>
#include <cstring>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <locale.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// evil, I know
using namespace mapcrafter;

int main(int argc, char** argv) {

	try {
		std::string locale = std::locale("").name();
	} catch (std::runtime_error& ex) {
		std::cerr << "There seems to be an issue with your locale, please verify your environment:"
			<< ex.what() << std::endl;
		throw ex;
	}

	renderer::RenderOpts opts;
	std::string arg_color, arg_config;

	po::options_description general("General options");
	general.add_options()
		("help,h", "shows this help message")
		("version,v", "shows the version of Mapcrafter")
		("mc-version", "shows the required Minecraft version");

	po::options_description logging("Logging/output options");
	logging.add_options()
		("logging-config", po::value<fs::path>(&opts.logging_config),
			"the path to the global logging configuration file to use (automatically determined if not specified)")
		("color", po::value<std::string>(&arg_color)->default_value("auto"),
			"whether terminal output is colored (true, false or auto)")
		("batch,b", "deactivates the animated progress bar and enables the progress logger instead");

	po::options_description renderer("Renderer options");
	renderer.add_options()
		("find-resources", "shows available resource paths, for example template/texture directory and global logging configuration file")
		("config,c", po::value<std::string>(&arg_config),
			"the path to the configuration file to use (required)")
		("render-skip,s", po::value<std::vector<std::string>>(&opts.render_skip)->multitoken(),
			"skips rendering the specified map(s)")
		("render-reset,r", "skips rendering all maps")
		("render-auto,a", po::value<std::vector<std::string>>(&opts.render_auto)->multitoken(),
			"renders the specified map(s)")
		("render-force,f", po::value<std::vector<std::string>>(&opts.render_force)->multitoken(),
			"renders the specified map(s) completely")
		("render-force-all,F", "force renders all maps")
		("jobs,j", po::value<int>(&opts.jobs)->default_value(1),
			"the count of jobs to use when rendering the map");

	po::options_description all("Allowed options");
	all.add(general).add(logging).add(renderer);

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, all), vm);
	} catch (po::error& ex) {
		std::cerr << "There is a problem parsing the command line arguments: "
				<< ex.what() << std::endl;
		std::cerr << "Use '" << argv[0] << " --help' for more information." << std::endl;
		return 1;
	}

	po::notify(vm);

	if (arg_color == "true")
		util::setcolor::setEnabled(util::TerminalColorStates::ENABLED);
	else if (arg_color == "false")
		util::setcolor::setEnabled(util::TerminalColorStates::DISABLED);
	else if (arg_color == "auto")
		util::setcolor::setEnabled(util::TerminalColorStates::AUTO);
	else {
		std::cerr << "Invalid argument '" << arg_color << "' for '--color'." << std::endl;
		std::cerr << "Allowed arguments are 'true', 'false' or 'auto'." << std::endl;
		std::cerr << "Use '" << argv[0] << " --help' for more information." << std::endl;
		return 1;
	}

	if (vm.count("help")) {
		std::cout << all << std::endl;
		std::cout << "Mapcrafter online documentation: <http://docs.mapcrafter.org>" << std::endl;
		return 0;
	}

	if (vm.count("version")) {
		std::cout << "Mapcrafter version: " << MAPCRAFTER_VERSION;
		if (strlen(MAPCRAFTER_GITVERSION))
			std::cout << " (" << MAPCRAFTER_GITVERSION << ")";
		std::cout << std::endl;
		return 0;
	}

	if (vm.count("mc-version")) {
		std::cout << MINECRAFT_VERSION << std::endl;
		return 0;
	}

	if (vm.count("find-resources")) {
		fs::path mapcrafter_bin = util::findExecutablePath();
		std::cout << "Your home directory: " << util::findHomeDir().string() << std::endl;
		std::cout << "Mapcrafter binary: " << mapcrafter_bin.string() << std::endl;

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

		util::PathList blocks = util::findBlockDirs(mapcrafter_bin);
		std::cout << "Block directories:" << std::endl;
		for (size_t i = 0; i < blocks.size(); i++)
			std::cout << "  " << i+1 << ". " << BOOST_FS_ABSOLUTE1(blocks[i]).string() << std::endl;
		if (blocks.size() == 0)
			std::cout << "  Nothing found." << std::endl;

		util::PathList configs = util::findLoggingConfigFiles(mapcrafter_bin);
		std::cout << "Logging configuration file:" << std::endl;
		for (size_t i = 0; i < configs.size(); i++)
			std::cout << "  " << i+1 << ". " << BOOST_FS_ABSOLUTE1(configs[i]).string() << std::endl;
		if (configs.size() == 0)
			std::cout << "  Nothing found." << std::endl;
		return 0;
	}

	if (!vm.count("config")) {
		std::cerr << "You have to specify a configuration file!" << std::endl;
		std::cerr << "Use '" << argv[0] << " --help' for more information." << std::endl;
		return 1;
	}

	opts.config = arg_config;
	opts.skip_all = vm.count("render-reset");
	opts.force_all = vm.count("render-force-all");
	opts.batch = vm.count("batch");
	if (!vm.count("logging-config"))
		opts.logging_config = util::findLoggingConfigFile();

	if (opts.skip_all && opts.force_all) {
		std::cerr << "You may only use one of --render-reset or --render-force-all!" << std::endl;
		std::cerr << "Use '" << argv[0] << " --help' for more information." << std::endl;
		return 1;
	}

	// ###
	// ### First big step: Load/parse/validate the configuration file
	// ###

	config::MapcrafterConfig config;
	config::ValidationMap validation = config.parseFile(opts.config.string());

	// show infos/warnings/errors if configuration file has something
	if (!validation.isEmpty()) {
		if (validation.isCritical())
			LOG(FATAL) << "Unable to parse configuration file:";
		else
			LOG(WARNING) << "There is a problem parsing the configuration file:";
		validation.log();
		LOG(WARNING) << "Please have a look at the documentation.";
	}
	if (validation.isCritical())
		return 1;

	// parse global logging configuration file and configure logging
	config::LoggingConfig::configureLogging(opts.logging_config);

	// configure logging from this configuration file
	config.configureLogging();

	renderer::RenderManager manager(config);
	manager.setRenderBehaviors(renderer::RenderBehaviors::fromRenderOpts(config, opts));
	if (!manager.run(opts.jobs, opts.batch))
		return 1;
	return 0;
}
