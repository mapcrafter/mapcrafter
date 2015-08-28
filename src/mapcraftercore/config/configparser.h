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

#ifndef CONFIGPARSER_H_
#define CONFIGPARSER_H_

#include "iniconfig.h"
#include "validation.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace mapcrafter {
namespace config {

/**
 * Generic section object factory. Just creates an instance of the specified type
 * without any special options.
 *
 * All section factories must have a constant operator() that returns a new instance of
 * the section type.
 */
template <typename T>
class GenericSectionFactory {
public:
	T operator()() const;
};

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
	template <typename T>
	void parseRootSection(T& section);

	/**
	 * Parses all sections with a specific type and puts the parsed section type objects
	 * into the supplied std::vector<Section>.
	 *
	 * It also parses the global section ([global:<type>], if it exists) and uses it as
	 * default for the sections. The global section object is created with the supplied
	 * section factory and all sections are copied from this global section object. The
	 * section factory is an object that has an operator() which returns a new instance
	 * of the section type. The default GenericSectionFactory just creates a new instance,
	 * but you can overwrite this to pass additional options (for example the config
	 * directory for relative paths) to the section objects.
	 */
	template <typename Section, typename SectionFactory = GenericSectionFactory<Section>>
	void parseSections(std::vector<Section>& sections, const std::string& type,
			SectionFactory section_factory = GenericSectionFactory<Section>());

	/**
	 * Same as parseSections(std::vector<Section>& sections... but puts the parsed
	 * sections into a map with section name -> section object.
	 */
	template <typename Section, typename SectionFactory = GenericSectionFactory<Section>>
	void parseSections(std::map<std::string, Section>& sections,
			const std::string& type,
			SectionFactory section_factory = GenericSectionFactory<Section>());

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
	// the configuration to be parsed/validated
	INIConfig config;

	// validation messages resulting of validation
	ValidationMap validation;

	// set of parsed section types
	std::set<std::string> parsed_section_types;
};

template <typename T>
T GenericSectionFactory<T>::operator()() const {
	return T();
}

template <typename T>
void ConfigParser::parseRootSection(T& section) {
	ValidationList root_validation = section.parse(config.getRootSection());
	if (!root_validation.isEmpty())
		validation.section("Configuration root section") = root_validation;
}

template <typename Section, typename SectionFactory>
void ConfigParser::parseSections(std::vector<Section>& sections,
		const std::string& type, SectionFactory section_factory) {
	parsed_section_types.insert(type);

	// create global section object and parse it if a global section exists in config
	Section section_global = section_factory();
	section_global.setGlobal(true);
	if (config.hasSection("global", type)) {
		ValidationList global_validation = section_global.parse(config.getSection("global", type));
		if (!global_validation.isEmpty())
			validation.section(section_global.getPrettyName()) = global_validation;
		// stop parsing here if global section contains critical errors
		// parsing also the other sections would lead to redundant error messages
		if (global_validation.isCritical())
			return;
	}

	// set of used section names, to make sure section names aren't used multiple types
	std::set<std::string> parsed_sections_names;

	// go through all config sections with the specified type and try to parse it
	auto config_sections = config.getSections();
	for (auto config_section_it = config_sections.begin();
			config_section_it != config_sections.end(); ++config_section_it) {
		if (config_section_it->getType() != type)
			continue;

		// create section object as copy from global section object
		Section section = section_global;
		section.setGlobal(false);
		// and parse section
		ValidationList section_validation = section.parse(*config_section_it);

		// make sure section name is not in use already, otherwise add to parsed sections
		if (parsed_sections_names.count(config_section_it->getName())) {
			section_validation.error(util::capitalize(type) + " name '"
					+ config_section_it->getName() + "' already used!");
		} else {
			parsed_sections_names.insert(config_section_it->getName());
			sections.push_back(section);
		}

		// add validation messages (if any) to global validation object
		if (!section_validation.isEmpty())
			validation.section(section.getPrettyName()) = section_validation;
	}
}

template <typename Section, typename SectionFactory>
void ConfigParser::parseSections(std::map<std::string, Section>& sections,
		const std::string& type, SectionFactory section_factory) {
	std::vector<Section> sections_list;
	parseSections(sections_list, type, section_factory);
	for (auto it = sections_list.begin(); it != sections_list.end(); ++it)
		sections[it->getSectionName()] = *it;
}

} /* namespace config */
} /* namespace mapcrafter */

#endif /* CONFIGPARSER_H_ */
