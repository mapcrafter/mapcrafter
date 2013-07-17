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

#include "ini.h"

#include <fstream>

namespace mapcrafter {
namespace config {

ConfigSection::ConfigSection(const std::string& name)
		: name(name) {
}

ConfigSection::~ConfigSection() {
}

const std::string& ConfigSection::getName() const {
	return name;
}

int ConfigSection::getEntryIndex(const std::string& key) const {
	for (size_t i = 0; i < entries.size(); i++)
		if (entries[i].first == key)
			return i;
	return -1;
}

bool ConfigSection::has(const std::string& key) const {
	return getEntryIndex(key) != -1;
}

std::string ConfigSection::get(const std::string& key) const {
	int index = getEntryIndex(key);
	if (index == -1)
		return "";
	return entries[index].second;
}

const std::vector<std::pair<std::string, std::string> >& ConfigSection::getEntries() const {
	return entries;
}

void ConfigSection::set(const std::string& key, const std::string& value) {
	int index = getEntryIndex(key);
	if (index == -1)
		entries.push_back(std::make_pair(key, value));
	else
		entries[index].second = value;
}

std::ostream& operator<<(std::ostream& stream, const ConfigSection& section) {
	if (!section.getName().empty())
		stream << "[" << section.getName() << "]" << std::endl;

	auto entries = section.getEntries();
	for (auto it = entries.begin(); it != entries.end(); ++it)
		stream << it->first << " = " << it->second << std::endl;

	return stream;
}

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
}

int ConfigFile::getSectionIndex(const std::string& section) const {
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getName() == section)
			return i;
	return -1;
}

bool ConfigFile::load(std::istream& stream) {
	int section = -1;
	std::string line;
	while (std::getline(stream, line)) {
		if (line.empty())
			continue;

		// a line starting with a # is a comment
		if (line[0] == '#')
			continue;

		// a line with a new section
		else if (line[0] == '[') {
			if (line[line.size() - 1] != ']')
				return false;

			std::string name = line.substr(1, line.size() - 2);
			//std::cout << "Section: " << name << std::endl;

			section++;
			sections.push_back(ConfigSection(name));
			section_names.push_back(name);
		} else {
			// just a line with key = value
			std::string key, value;
			for (size_t i = 0; i < line.size(); i++) {
				if (line[i] == '=') {
					key = line.substr(0, i);
					value = line.substr(i + 1, line.size() - i - 1);
					break;
				}
				if (i == line.size() - 1)
					return false;
			}

			trim(key);
			trim(value);
			//std::cout << "   " << key << "=\"" << value << "\"" << std::endl;

			if (section == -1)
				root.set(key, value);
			else
				sections[section].set(key, value);
		}
	}
	return true;
}

void ConfigFile::write(std::ostream& stream) const {
	stream << root << std::endl;

	for (std::vector<ConfigSection>::const_iterator it = sections.begin();
			it != sections.end(); ++it) {
		stream << *it << std::endl;
	}
}

bool ConfigFile::loadFile(const std::string& filename) {
	std::ifstream in(filename);
	if (!in)
		return false;
	return load(in);
}

bool ConfigFile::writeFile(const std::string& filename) {
	std::ofstream out(filename);
	if (!out)
		return false;
	write(out);
	return true;
}

bool ConfigFile::hasSection(const std::string& section) const {
	return getSectionIndex(section) != -1;
}

const std::vector<std::string>& ConfigFile::getSections() const {
	return section_names;
}

bool ConfigFile::has(const std::string& section, const std::string& key) const {
	if (section == "")
		return root.has(key);
	int index = getSectionIndex(section);
	if (index == -1)
		return false;
	return sections.at(index).has(key);
}

std::string ConfigFile::get(const std::string& section, const std::string& key) const {
	if (section == "")
		return root.get(key);
	int index = getSectionIndex(section);
	if (index == -1)
		return "";
	return sections.at(index).get(key);
}

void ConfigFile::set(const std::string& section, const std::string& key,
		const std::string& value) {
	if (section == "")
		root.set(key, value);
	else {
		int index = getSectionIndex(section);
		if (index == -1) {
			sections.push_back(ConfigSection(section));
			index = sections.size() - 1;
		}
		sections[index].set(key, value);
	}
}

} /* namespace config */
} /* namespace mapcrafter */
