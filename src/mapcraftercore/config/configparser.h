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

#ifndef CONFIGPARSER_H_
#define CONFIGPARSER_H_

#include "validation.h"

#include <string>
#include <vector>

namespace mapcrafter {
namespace config {

/**
 * A class to parse and validate arbitrary configuration files.
 */
class ConfigParser {
public:
	ConfigParser();
	~ConfigParser();

	/**
	 * Parses the root section of the configuration with the supplied section type object.
	 */
	template<typename T>
	void parseRootSection(T& section, const INIConfigSection& config_section) {
		section.parse(config, validation);
	}

	/**
	 * Parses all sections with a specific type and puts the parsed section type objects
	 * into the supplied vector<T>. It also parses the global section ([global:<name>s],
	 * if it exists) and uses it as default for the sections.
	 */
	template<typename T>
	void parseSections(std::vector<T>& sections, const std::string& name,
			const INIConfig& config) {
	}

	/**
	 * Does the remaining validation work after parsing the sections, for example add
	 * warnings for unknown section types.
	 */
	void validate();

	/**
	 * Returns the validation of the parsed sections. Also adds warnings for unknown
	 * section types.
	 */
	const ValidationMap& getValidation() const;

private:
	ValidationMap validation;

	std::vector<std::string> parsed_section_types;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* CONFIGPARSER_H_ */
