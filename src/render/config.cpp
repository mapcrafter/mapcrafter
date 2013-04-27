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

#include "render/config.h"

#include "util.h"

#include <fstream>

namespace mapcrafter {
namespace render {


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

		// a line with a new section
		if (line[0] == '[') {
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

RenderWorldConfig::RenderWorldConfig()
		: textures_dir("data/images"), texture_size(12) {
	rotations.insert(0);
}

int stringToRotation(const std::string& str) {
	std::string directions[] = {"top-left", "top-right", "bottom-right", "bottom-left"};
	for (int i = 0; i < 4; i++)
		if (str == directions[i])
			return i;
	return -1;
}

void RenderWorldConfig::readFromConfig(const ConfigFile& config, const std::string& section) {
	if (config.has(section, "name"))
		name_long = config.get(section, "name");
	if (name_long.empty())
		name_long = name_short;

	if (config.has(section, "world"))
		input_dir = config.get(section, "world");
	if (config.has(section, "textures_dir"))
		textures_dir = config.get(section, "textures_dir");

	if (config.has(section, "rotations")) {
		rotations.clear();
		std::string str = config.get(section, "rotations");
		std::stringstream ss;
		ss << str;
		std::string elem;
		while (ss >> elem) {
			int r = stringToRotation(elem);
			if (r != -1)
				rotations.insert(r);
		}
		std::cout << std::endl;
	}
	if (config.has(section, "texture_size"))
		texture_size = config.get<int>(section, "texture_size");
}

void RenderWorldConfig::print(std::ostream& stream) const {
		std::cout << name_short << " '" << name_long << "'" << std::endl;
		std::cout << "  input_dir " << input_dir << std::endl;
		std::cout << "  textures_dir " << textures_dir << std::endl;
		std::cout << "  texture_size " << texture_size << std::endl;
		std::cout << "  rotations ";
		for (auto it = rotations.begin(); it != rotations.end(); ++it)
			std::cout << *it << " ";
		std::cout << std::endl;
}

RenderConfigParser::RenderConfigParser() {
}

RenderConfigParser::~RenderConfigParser() {
}

bool RenderConfigParser::loadFile(const std::string& filename) {
	if (!config.loadFile(filename))
		return false;

	default_config.readFromConfig(config, "");

	std::vector<std::string> sections = config.getSections();
	for (size_t i = 0; i < sections.size(); i++) {
		RenderWorldConfig world = default_config;
		world.name_short = sections[i];
		world.readFromConfig(config, sections[i]);
		worlds.push_back(world);
	}

	output_dir = config.get("", "output_dir");
	template_dir = config.get("", "template_dir");

	/*
	std::cout << "Loaded " << worlds.size() << " worlds." << std::endl;
	for (int i = 0; i < worlds.size(); i++) {
		worlds[i].print(std::cout);
	}
	*/

	return true;
}

const std::vector<RenderWorldConfig>& RenderConfigParser::getWorlds() const {
	return worlds;
}

const fs::path& RenderConfigParser::getOutputDir() const {
	return output_dir;
}

const fs::path& RenderConfigParser::getTemplateDir() const {
	return template_dir;
}

std::string RenderConfigParser::outputPath(std::string file) const {
	return (output_dir / file).string();
}

std::string RenderConfigParser::templatePath(std::string file) const {
	return (template_dir / file).string();
}

}
}
