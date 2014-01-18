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

#include "iniconfig.h"

#include "validation.h"

#include <fstream>

namespace mapcrafter {
namespace config {

INIConfigSection::INIConfigSection(const std::string& type, const std::string& name)
		: type(type), name(name) {
}

INIConfigSection::~INIConfigSection() {
}

int INIConfigSection::getEntryIndex(const std::string& key) const {
	for (size_t i = 0; i < entries.size(); i++)
		if (entries[i].first == key)
			return i;
	return -1;
}

const std::string& INIConfigSection::getType() const {
	return type;
}

const std::string& INIConfigSection::getName() const {
	return name;
}

std::string INIConfigSection::getNameType() const {
	return type + ":" + name;
}

bool INIConfigSection::isNamed() const {
	return !name.empty();
}

bool INIConfigSection::isEmpty() const {
	return entries.size() == 0;
}

bool INIConfigSection::has(const std::string& key) const {
	return getEntryIndex(key) != -1;
}

std::string INIConfigSection::get(const std::string& key, const std::string& default_value) const {
	int index = getEntryIndex(key);
	if (index == -1)
		return default_value;
	return entries[index].second;
}

const std::vector<INIConfigEntry> INIConfigSection::getEntries() const {
	return entries;
}

void INIConfigSection::set(const std::string& key, const std::string& value) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries[index].second = value;
	else
		entries.push_back(std::make_pair(key, value));
}

void INIConfigSection::remove(const std::string& key) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries.erase(entries.begin() + index);
}

std::ostream& operator<<(std::ostream& out, const INIConfigSection& section) {
	if (section.getName() != "") {
		if (section.getType() == "")
			out << "[" << section.getName() << "]" << std::endl;
		else
			out << "[" << section.getType() << ":" << section.getName() << "]" << std::endl;
	}

	const std::vector<INIConfigEntry>& entries = section.getEntries();
	for (std::vector<INIConfigEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		out << it->first << " = " << it->second << std::endl;
	return out;
}

INIConfig::INIConfig() {
}

INIConfig::~INIConfig() {
}

int INIConfig::getSectionIndex(const std::string& type, const std::string& name) const {
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getType() == type && sections[i].getName() == name)
			return i;
	return -1;
}

bool INIConfig::load(std::istream& in, ValidationMessage& msg) {
	int section = -1;
	std::string line;
	int linenumber = 0;
	while (std::getline(in, line)) {
		linenumber++;
		if (line.empty())
			continue;

		// trim the line
		line = util::trim(line);

		// a line starting with a # is a comment
		if (line[0] == '#')
			continue;

		// a line with a new section
		else if (line[0] == '[') {
			if (line[line.size() - 1] != ']') {
				msg = ValidationMessage::error("Expecting ']' at end of line " + util::str(linenumber) + ".");
				return false;
			}

			std::string type, name;
			std::string section_name = line.substr(1, line.size() - 2);
			std::string::size_type colon = section_name.find(':');
			if (colon == std::string::npos)
				name = section_name;
			else {
				type = section_name.substr(0, colon);
				name = section_name.substr(colon+1, std::string::npos);
			}

			if (name.empty()) {
				msg = ValidationMessage::error("Invalid section name on line " + util::str(linenumber) + ".");
				return false;
			}

			section++;
			sections.push_back(INIConfigSection(type, name));
		} else {
			// just a line with key = value
			std::string key, value;
			for (size_t i = 0; i < line.size(); i++) {
				if (line[i] == '=') {
					key = line.substr(0, i);
					value = line.substr(i + 1, line.size() - i - 1);
					break;
				}
				if (i == line.size() - 1) {
					msg = ValidationMessage::error("No '=' found on line " + util::str(linenumber) + ".");
					return false;
				}
			}

			key = util::trim(key);
			value = util::trim(value);

			if (section == -1)
				root.set(key, value);
			else
				sections[section].set(key, value);
		}
	}

	return true;
}

bool INIConfig::load(std::istream& in) {
	ValidationMessage msg;
	return load(in, msg);
}

bool INIConfig::loadFile(const std::string& filename, ValidationMessage& msg) {
	std::ifstream in(filename);
	if (!in) {
		msg = ValidationMessage::error("Unable to read file '" + filename + "'!");
		return false;
	}
	return load(in, msg);
}

bool INIConfig::loadFile(const std::string& filename) {
	ValidationMessage msg;
	return loadFile(filename, msg);
}

bool INIConfig::write(std::ostream& out) const {
	if (!root.isEmpty())
		out << root << std::endl;
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].isNamed())
			out << sections[i] << std::endl;
	return true;
}

bool INIConfig::writeFile(const std::string& filename) const {
	std::ofstream out(filename);
	if (!out)
		return false;
	return write(out);
}

bool INIConfig::hasSection(const std::string& type, const std::string& name) const {
	return getSectionIndex(type, name) != -1;
}

const INIConfigSection& INIConfig::getRootSection() const {
	return root;
}

INIConfigSection& INIConfig::getRootSection() {
	return root;
}

const std::vector<INIConfigSection> INIConfig::getSections() const {
	return sections;
}

INIConfigSection& INIConfig::addSection(const std::string& type,
		const std::string& name) {
	return getSection(type, name);
}

const INIConfigSection& INIConfig::getSection(const std::string& type, const std::string& name) const {
	int index = getSectionIndex(type, name);
	if (index == -1)
		return empty_section;
	return sections.at(index);
}

INIConfigSection& INIConfig::getSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index != -1)
		return sections[index];
	INIConfigSection section(type, name);
	sections.push_back(section);
	return sections.back();
}

INIConfigSection& INIConfig::addSection(const INIConfigSection& section) {
	int index = getSectionIndex(section.getType(), section.getName());
	if (index == -1) {
		sections.push_back(section);
		return sections.back();
	}
	sections[index] = section;
	return sections[index];
}

void INIConfig::removeSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index == -1)
		sections.erase(sections.begin() + index);
}

} /* namespace config */
} /* namespace mapcrafter */
