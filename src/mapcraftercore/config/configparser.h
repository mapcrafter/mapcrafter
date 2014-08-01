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
#include "../util.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace mapcrafter {
namespace config {

/**
 * A class to parse and validate arbitrary configuration files.
 */
class ConfigParser {
public:
	ConfigParser(const INIConfig& config);
	~ConfigParser();

	/**
	 * Parses the root section of the configuration with the supplied section type object.
	 */
	template<typename T>
	void parseRootSection(T& section) {
		ValidationList root_validation = section.parse(config.getRootSection());
		if (!root_validation.isEmpty())
			validation.section("Configuration root section") = root_validation;
	}

	/**
	 * Parses all sections with a specific type and puts the parsed section type objects
	 * into the supplied std::vector<T>.
	 *
	 * It also parses the global section ([global:<type>], if it exists) and uses it as
	 * default for the sections.
	 */
	template<typename T>
	void parseSections(std::vector<T>& sections, const std::string& type) {
		parsed_section_types.insert(type);
		T section_global;
		section_global.setGlobal(true);
		if (config.hasSection("global", type)) {
			//section_global.setConfigDir(config_dir);
			ValidationList global_validation = section_global.parse(config.getSection("global", type));
			if (!global_validation.isEmpty()) {
				std::string pretty_name = util::capitalize(section_global.getPrettyName());
				validation.section(pretty_name) = global_validation;
			}
			if (global_validation.isCritical())
				return;
		}

		std::set<std::string> parsed_sections_names;

		auto config_sections = config.getSections();
		for (auto config_section_it = config_sections.begin();
				config_section_it != config_sections.end(); ++config_section_it) {
			if (config_section_it->getType() != type)
				continue;

			T section = section_global;
			section.setGlobal(false);
			//section.setConfigDir(config_dir);
			ValidationList section_validation = section.parse(*config_section_it);

			if (parsed_sections_names.count(config_section_it->getName())) {
				section_validation.error(util::capitalize(type) + " name '"
						+ config_section_it->getName() + "' already used!");
			} else
				sections.push_back(section);

			if (!section_validation.isEmpty()) {
				std::string pretty_name = util::capitalize(section.getPrettyName());
				validation.section(pretty_name) = section_validation;
			}

			parsed_sections_names.insert(config_section_it->getName());
		}
	}

	/**
	 * Same as parseSections(std::vector<T>& sections... but puts the parsed sections
	 * into a map with section name -> section object.
	 */
	template<typename T>
	void parseSections(std::map<std::string, T>& sections, const std::string& type) {
		std::vector<T> sections_list;
		parseSections(sections_list, type);
		for (auto it = sections_list.begin(); it != sections_list.end(); ++it)
			sections[it->getSectionName()] = *it;
	}

	/**
	 * Does the remaining validation work after parsing the sections, for example add
	 * warnings for unknown section types.
	 */
	bool validate();

	/**
	 * Returns the validation of the parsed sections.
	 */
	const ValidationMap& getValidation() const;

private:
	INIConfig config;

	ValidationMap validation;

	std::set<std::string> parsed_section_types;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* CONFIGPARSER_H_ */
