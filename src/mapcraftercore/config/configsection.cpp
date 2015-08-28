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

#include "configsection.h"

#include "iniconfig.h"

namespace mapcrafter {
namespace config {

ConfigSection::ConfigSection()
	: global(false) {
}

ConfigSection::~ConfigSection() {
}

bool ConfigSection::isGlobal() const {
	return global;
}

void ConfigSection::setGlobal(bool global) {
	this->global = global;
}

std::string ConfigSection::getSectionName() const {
	return section_name;
}

std::string ConfigSection::getPrettyName() const {
	return "unknown section";
}

void ConfigSection::dump(std::ostream& out) const {
	out << getPrettyName() << std::endl;
}

ValidationList ConfigSection::parse(const INIConfigSection& section) {
	section_name = section.getName();

	preParse(section, validation);

	auto entries = section.getEntries();
	for (auto entry_it = entries.begin(); entry_it != entries.end(); ++entry_it) {
		std::string key = entry_it->first;
		std::string value = entry_it->second;

		if (!parseField(key, value, validation))
			validation.warning("Unknown configuration option '" + key + "'!");
	}

	postParse(section, validation);

	return validation;
}

void ConfigSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
}

bool ConfigSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	return false;
}

void ConfigSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
}

std::ostream& operator<<(std::ostream& out, const ConfigSection& section) {
	section.dump(out);
	return out;
}

}
}
