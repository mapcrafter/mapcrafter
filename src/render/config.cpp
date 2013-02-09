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

#include "config.h"

#include <fstream>

namespace mapcrafter {
namespace render {

int ConfigSection::getEntryIndex(const std::string& key) const {
	for (int i = 0; i < entries.size(); i++)
		if (entries[i].first == key)
			return i;
	return -1;
}

bool ConfigSection::has(const std::string& key) const {
	return getEntryIndex(key) != -1;
}

void ConfigSection::set(const std::string& key, const std::string& value) {
	int index = getEntryIndex(key);
	if (index == -1)
		entries.push_back(std::make_pair(key, value));
	else
		entries[index].second = value;
}

std::string ConfigSection::get(const std::string& key) const {
	int index = getEntryIndex(key);
	if (index == -1)
		return "";
	return entries[index].second;
}

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
}

int ConfigFile::getSectionIndex(const std::string& section) const {
	for (int i = 0; i < sections.size(); i++)
		if (sections[i].name == section)
			return i;
	return -1;
}

bool ConfigFile::load(std::istream& stream) {
	int section = -1;
	std::string line;
	while (std::getline(stream, line)) {
		if (line.empty())
			continue;

		// a line with a new section
		if (line[0] == '[') {
			if (line[line.size() - 1] != ']')
				return false;

			std::string name = line.substr(1, line.size() - 2);
			//std::cout << "Section: " << name << std::endl;

			section++;
			sections.push_back(ConfigSection());
			sections[section].name = name;
			section_names.push_back(name);
		} else {
			// just a line with key = value
			std::string key, value;
			for (int i = 0; i < line.size(); i++) {
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
}

bool ConfigFile::loadFile(const std::string& filename) {
	std::ifstream in(filename);
	if (!in)
		return false;
	return load(in);
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

}
}
