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

#include "iniconfig.h"

#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

INIConfigSection::INIConfigSection(const std::string& type, const std::string& name)
	: type(type), name(name) {
}

INIConfigSection::~INIConfigSection() {
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

std::string INIConfigSection::get(const std::string& key,
		const std::string& default_value) const {
	int index = getEntryIndex(key);
	if (index == -1)
		return default_value;
	return entries[index].second;
}

const std::vector<INIConfigEntry>& INIConfigSection::getEntries() const {
	return entries;
}

INIConfigSection& INIConfigSection::set(const std::string& key, const std::string& value) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries[index].second = value;
	else
		entries.push_back(std::make_pair(key, value));
	return *this;
}

INIConfigSection& INIConfigSection::remove(const std::string& key) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries.erase(entries.begin() + index);
	return *this;
}

int INIConfigSection::getEntryIndex(const std::string& key) const {
	for (size_t i = 0; i < entries.size(); i++)
		if (entries[i].first == key)
			return i;
	return -1;
}

std::ostream& operator<<(std::ostream& out, const INIConfigSection& section) {
	std::string name = section.getName();
	std::string type = section.getType();

	if (!name.empty()) {
		if (type.empty())
			out << "[" << name << "]" << std::endl;
		else
			out << "[" << type << ":" << name << "]" << std::endl;
	}

	auto entries = section.getEntries();
	for (auto entry_it = entries.begin(); entry_it != entries.end(); ++entry_it)
		out << entry_it->first << " = " << entry_it->second << std::endl;
	return out;
}

INIConfig::INIConfig() {
}

INIConfig::~INIConfig() {
}

void INIConfig::load(std::istream& in) {
	int section = -1;
	std::string line;
	size_t line_number = 0;
	while (std::getline(in, line)) {
		line_number++;

		// trim the line
		line = util::trim(line);

		// ignore empty/comment lines
		if (line.empty() || line[0] == '#')
			continue;

		// a line with a new section
		else if (line[0] == '[') {
			if (line[line.size() - 1] != ']') {
				throw INIConfigError("Expecting ']' at end of line "
						+ util::str(line_number) + ".");
				return;
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
				throw INIConfigError("Invalid section name on line "
						+ util::str(line_number) + ".");
				return;
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
					throw INIConfigError("No '=' found on line "
							+ util::str(line_number) + ".");
					return;
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
}

void INIConfig::loadFile(const std::string& filename) {
	if (!fs::is_regular_file(filename))
		throw INIConfigError("'" + filename + "' is not a valid file!");
	std::ifstream in(filename);
	if (!in)
		throw INIConfigError("Unable to read file '" + filename + "'!");
	else
		load(in);
}

void INIConfig::loadString(const std::string& str) {
	std::stringstream ss(str);
	load(ss);
}

void INIConfig::write(std::ostream& out) const {
	if (!root.isEmpty())
		out << root << std::endl;
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].isNamed())
			out << sections[i] << std::endl;
}

void INIConfig::writeFile(const std::string& filename) const {
	std::ofstream out(filename);
	if (!out)
		throw INIConfigError("Unable to write file '" + filename + "'!");
	else
		write(out);
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

const std::vector<INIConfigSection>& INIConfig::getSections() const {
	return sections;
}

const INIConfigSection& INIConfig::getSection(const std::string& type,
		const std::string& name) const {
	int index = getSectionIndex(type, name);
	if (index == -1)
		return empty_section;
	return sections.at(index);
}

INIConfigSection& INIConfig::getSection(const std::string& type,
		const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index != -1)
		return sections[index];
	INIConfigSection section(type, name);
	sections.push_back(section);
	return sections.back();
}

void INIConfig::removeSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index == -1)
		sections.erase(sections.begin() + index);
}

int INIConfig::getSectionIndex(const std::string& type,
		const std::string& name) const {
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getType() == type && sections[i].getName() == name)
			return i;
	return -1;
}

} /* namespace config */
} /* namespace mapcrafter */
