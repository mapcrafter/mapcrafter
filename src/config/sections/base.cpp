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

#include "base.h"

#include "../iniconfig.h"

namespace mapcrafter {
namespace config {

ConfigSectionBase::ConfigSectionBase()
	: global(false) {
}

ConfigSectionBase::~ConfigSectionBase() {
}

void ConfigSectionBase::setGlobal(bool global) {
	this->global = global;
}

bool ConfigSectionBase::parse(const INIConfigSection& section,
		ValidationList& validation) {
	section_name = section.getName();

	preParse(section, validation);

	auto entries = section.getEntries();
	for (auto entry_it = entries.begin(); entry_it != entries.end(); ++entry_it) {
		std::string key = entry_it->first;
		std::string value = entry_it->second;

		if (!parseField(key, value, validation))
			validation.push_back(ValidationMessage::warning(
					"Unknown configuration option '" + key + "'!"));
	}

	postParse(section, validation);

	return isValidationValid(validation);
}

void ConfigSectionBase::preParse(const INIConfigSection& section,
		ValidationList& validation) {
}

bool ConfigSectionBase::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	return false;
}

void ConfigSectionBase::postParse(const INIConfigSection& section,
		ValidationList& validation) {
}

}
}
