/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "configparser.h"

namespace mapcrafter {
namespace config {

ConfigParser::ConfigParser(const INIConfig& config)
	: config(config), ok(true) {
}

ConfigParser::~ConfigParser() {
}

bool ConfigParser::validate() {
	auto config_sections = config.getSections();
	for (auto config_section_it = config_sections.begin();
			config_section_it != config_sections.end(); ++config_section_it) {
		std::string type = config_section_it->getType();
		std::string name = config_section_it->getName();
		if ((type == "global" && parsed_section_types.count(name))
				|| (type != "global" && parsed_section_types.count(type)))
			continue;
		validation.section("Section '" + name + "' with type '" + type + "'")
				.warning("Unknown section type!");
	}

	return ok;
}

const ValidationMap& ConfigParser::getValidation() const {
	return validation;
}

} /* namespace config */
} /* namespace mapcrafter */
