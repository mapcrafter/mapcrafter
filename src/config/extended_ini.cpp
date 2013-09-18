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

#include "extended_ini.h"

namespace mapcrafter {
namespace config2 {

int ConfigSection::getEntryIndex(const std::string& key) const {
	for (size_t i = 0; i < entries.size(); i++)
		if (entries[i].first == key)
			return i;
	return -1;
}

bool ConfigSection::has(const std::string& key) const {
	return getEntryIndex(key) != -1;
}

std::string ConfigSection::get(const std::string& key, const std::string& default_value) const {
	int index = getEntryIndex(key);
	if (index == -1)
		return default_value;
	return entries[index].second;
}

void ConfigSection::set(const std::string& key, const std::string& value) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries[index].second = value;
	else
		entries.push_back(std::make_pair(key, value));
}

void ConfigSection::remove(const std::string& key) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries.erase(entries.begin() + index);
}

std::ostream& operator<<(std::ostream& out, const ConfigSection& section) {
	if (section.getName() != "") {
		if (section.getType() == "")
			out << "[" << section.getName() << "]" << std::endl;
		else
			out << "[" << section.getType() << ":" << section.getName() << "]" << std::endl;
	}

	const std::vector<ConfigEntry>& entries = section.getEntries();
	for (std::vector<ConfigEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		out << it->first << " = " << it->second << std::endl;
	return out;
}

bool ConfigFile::load(std::istream& in, ValidationMessage& msg) {
	return true;
}

bool ConfigFile::loadFile(const std::string& filename, ValidationMessage& msg) {
	return true;
}

bool ConfigFile::write(std::ostream& out) const {
	return true;
}

bool ConfigFile::writeFile(const std::string& filename) const {
	return true;
}

int ConfigFile::getSectionIndex(const std::string& type, const std::string& name) const {
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getType() == type && sections[i].getName() == name)
			return i;
	return -1;
}

bool ConfigFile::hasSection(const std::string& type, const std::string& name) const {
	return getSectionIndex(type, name) != -1;
}

const ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) const {
	int index = getSectionIndex(type, name);
	if (index == -1)
		return empty_section;
	return sections.at(index);
}

ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index != -1)
		return sections[index];
	ConfigSection section(type, name);
	sections.push_back(section);
	return sections.back();
}

void ConfigFile::removeSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index == -1)
		sections.erase(sections.begin() + index);
}

} /* namespace config */
} /* namespace mapcrafter */
