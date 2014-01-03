/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "../config/mapcrafter_config.h"

#include <iostream>
#include <string>

namespace config = mapcrafter::config;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: ./testconfig [configfile]" << std::endl;
		return 1;
	}

	std::string configfile = argv[1];

	config::MapcrafterConfigFile parser;
	config::ValidationMap validation;
	bool ok = parser.parse(configfile, validation);

	if (validation.size() > 0) {
		std::cout << (ok ? "Some notes on your configuration file:" : "Your configuration file is invalid!") << std::endl;
		for (auto it = validation.begin(); it != validation.end(); ++it) {
			std::cout << it->first << ":" << std::endl;
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				std::cout << " - " << *it2 << std::endl;
			}
		}
	} else {
		std::cout << "Everything ok." << std::endl;
	}

	std::cout << std::endl << "The parsed configuration file:" << std::endl;
	parser.dump(std::cout);

	return 0;
}
