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

int stringToRotation(const std::string& rotation, std::string names[4]) {
	for (int i = 0; i < 4; i++)
		if (rotation == names[i])
			return i;
	return -1;
}

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

RenderWorldConfig::RenderWorldConfig()
		: texture_size(12), render_unknown_blocks(false),
		  render_leaves_transparent(true), render_biomes(true) {
	rotations.insert(0);
	render_behaviors.resize(4, 1);
}

void RenderWorldConfig::readFromConfig(const fs::path& dir, const ConfigFile& config,
		const std::string& section) {
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
			else
				unknown_rotations.push_back(elem);
		}
	}

	if (config.has(section, "rendermode"))
		rendermode = config.get(section, "rendermode");

	if (config.has(section, "texture_size"))
		texture_size = config.get<int>(section, "texture_size");
	if (config.has(section, "render_unknown_blocks"))
		render_unknown_blocks = config.get<bool>(section, "render_unknown_blocks");
	if (config.has(section, "render_leaves_transparent"))
		render_leaves_transparent = config.get<bool>(section, "render_leaves_transparent");
	if (config.has(section, "render_biomes"))
		render_biomes = config.get<bool>(section, "render_biomes");

	if (config.has(section, "incremental_detection"))
		incremental_detection = config.get(section, "incremental_detection");

	if (!input_dir.empty())
		input_dir = fs::absolute(input_dir, dir).string();
	if (!textures_dir.empty())
		textures_dir = fs::absolute(textures_dir, dir).string();
}

bool RenderWorldConfig::checkValid(std::vector<std::string>& errors) const {
	bool count = errors.size();

	std::string prefix = "[" + name_short + "] ";
	if (name_long.empty())
		errors.push_back(prefix + "You have to specify a world name (name)!");

	if (input_dir.empty())
		errors.push_back(prefix + "You have to specify a world directory (input_dir)!");
	else if (!fs::is_directory(input_dir))
		errors.push_back(prefix + "The world directory " + input_dir + " does not exist!");

	if (textures_dir.empty())
		errors.push_back(prefix + "You have to specify a textures directory (textures_dir)!");
	else if (!fs::is_directory(textures_dir))
		errors.push_back(prefix + "The textures directory " + textures_dir + " does not exist!");

	if (rotations.size() == 0)
		errors.push_back(prefix + "You have to specify a world rotation (rotations)!");
	else if (unknown_rotations.size() != 0) {
		for (size_t i = 0; i < unknown_rotations.size(); i++)
			errors.push_back(prefix + "Unknown rotation '" + unknown_rotations[i] + "'!");
	}

	if (texture_size <= 0 || texture_size > 32)
		errors.push_back(prefix + "You have to specify a sane texture size (0 < texture_size <= 32)!");

	if (incremental_detection != ""
			&& incremental_detection != "timestamp"
			&& incremental_detection != "filetimes")
		errors.push_back(prefix + "The value of incremental_detection must be 'timestamp' or 'filetimes'!");

	return errors.size() == count;
}

bool RenderWorldConfig::canSkip() const {
	for (int i = 0; i < 4; i++)
		if (render_behaviors[i] != RENDER_SKIP)
			return false;
	return true;
}

bool RenderWorldConfig::isCompleteRenderForce() const {
	for (std::set<int>::const_iterator it = rotations.begin(); it != rotations.end(); ++it)
		if (render_behaviors[*it] != RENDER_FORCE)
			return false;
	return true;
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

	fs::path dir = fs::path(filename).parent_path();

	output_dir = config.get("", "output_dir");
	template_dir = config.get("", "template_dir");

	if (!output_dir.empty())
		output_dir = fs::absolute(output_dir, dir).string();
	if (!template_dir.empty())
		template_dir = fs::absolute(template_dir, dir).string();

	default_config.readFromConfig(dir, config, "");

	std::vector<std::string> sections = config.getSections();
	for (size_t i = 0; i < sections.size(); i++) {
		RenderWorldConfig world = default_config;
		world.name_short = sections[i];
		world.readFromConfig(dir, config, sections[i]);
		worlds.push_back(world);
	}

	worlds_max_zoom.resize(worlds.size(), 0);

	/*
	std::cout << "Loaded " << worlds.size() << " worlds." << std::endl;
	for (int i = 0; i < worlds.size(); i++) {
		worlds[i].print(std::cout);
	}
	*/

	return true;
}

bool RenderConfigParser::checkValid() const {
	std::vector<std::string> errors;

	if (output_dir.empty())
		errors.push_back("You have to specify an output directory!");
	if (template_dir.empty())
		errors.push_back("You have to specify a template directory!");
	else if (!fs::is_directory(template_dir))
		errors.push_back("The template directory " + template_dir + " does not exist!");

	for (size_t i = 0; i < worlds.size(); i++) {
		worlds[i].checkValid(errors);
	}

	if (errors.size() != 0) {
		std::cerr << "The config file contains some errors:" << std::endl;
		for (size_t i = 0; i < errors.size(); i++)
			std::cerr << errors[i] << std::endl;
		return false;
	}
	return true;
}

bool nextSplit(std::string& string, std::string& world, std::string& rotation) {
	if (string.empty()) {
		world = "";
		rotation = "";
		return false;
	}

	size_t pos = string.find(",");
	std::string sub = string;
	if (pos != std::string::npos) {
		sub = string.substr(0, pos);
		string = string.substr(pos+1);
	} else {
		string = "";
	}

	world = sub;
	pos = world.find(":");
	if (pos != std::string::npos) {
		rotation = world.substr(pos+1);
		world = world.substr(0, pos);
	} else {
		rotation = "";
	}

	return true;
}

int getWorldIndex(const std::string& name, const std::vector<RenderWorldConfig>& worlds) {
	for (size_t i = 0; i < worlds.size(); i++)
		if (worlds[i].name_short == name)
			return i;
	return -1;
}

void setBehaviors(std::vector<RenderWorldConfig>& worlds, std::string string,
		int behavior) {
	std::string world, rotation;
	int w, r;

	while (nextSplit(string, world, rotation)) {
		w = getWorldIndex(world, worlds);
		r = stringToRotation(rotation, ROTATION_NAMES_SHORT);
		if (w == -1) {
			std::cout << "Warning: Unknown map '" << world << "'." << std::endl;
			continue;
		}

		if (!rotation.empty()) {
			if (r == -1) {
				std::cout << "Warning: Unknown rotation '" << rotation << "'." << std::endl;
				continue;
			}
			if (!worlds[w].rotations.count(r)) {
				std::cout << "Warning: Map '" << world << "' does not have rotation '"
					<< rotation << "'." << std::endl;
				continue;
			}
		}

		if (r != -1)
			worlds[w].render_behaviors[r] = behavior;
		else
			std::fill(&worlds[w].render_behaviors[0], &worlds[w].render_behaviors[4],
					behavior);
	}
}

void RenderConfigParser::setRenderBehaviors(bool skip_all, const std::string& render_skip,
		const std::string& render_auto, const std::string& render_force) {
	if (!skip_all)
		setBehaviors(worlds, render_skip, RenderWorldConfig::RENDER_SKIP);
	else
		for (size_t i = 0; i < worlds.size(); i++)
			for (int j = 0; j < 4; j++)
				worlds[i].render_behaviors[j] = RenderWorldConfig::RENDER_SKIP;
	setBehaviors(worlds, render_auto, RenderWorldConfig::RENDER_AUTO);
	setBehaviors(worlds, render_force, RenderWorldConfig::RENDER_FORCE);
}

const std::vector<RenderWorldConfig>& RenderConfigParser::getWorlds() const {
	return worlds;
}

fs::path RenderConfigParser::getOutputDir() const {
	return output_dir;
}

fs::path RenderConfigParser::getTemplateDir() const {
	return template_dir;
}

std::string RenderConfigParser::getOutputPath(std::string file) const {
	return (fs::path(output_dir) / file).string();
}

std::string RenderConfigParser::getTemplatePath(std::string file) const {
	return (fs::path(template_dir) / file).string();
}

void RenderConfigParser::setWorldMaxZoom(size_t world, int max_zoom) {
	if (world >= worlds.size())
		return;
	worlds_max_zoom[world] = max_zoom;
}

std::string RenderConfigParser::generateJavascript() const {
	std::string js = "";

	for (size_t i = 0; i < worlds.size(); i++) {
		RenderWorldConfig world = worlds[i];

#ifdef OLD_BOOST
		std::string world_name = fs::path(world.input_dir).filename();
#else
		std::string world_name = fs::path(world.input_dir).filename().string();
#endif

		js += "\"" + world.name_short + "\" : {\n";
		js += "\tname: \"" + world.name_long + "\",\n";
		js += "\tworldName: \"" + world_name + "\",\n";
		js += "\ttextureSize: " + str(world.texture_size) + ",\n";
		js += "\ttileSize: " + str(32*world.texture_size) + ",\n";
		js += "\tmaxZoom: " + str(worlds_max_zoom[i]) + ",\n";
		js += "\trotations: [";
		for (std::set<int>::iterator it = world.rotations.begin();
				it != world.rotations.end(); ++it)
			js += str(*it) + ",";
		js += "],\n";
		js += "},";
	}

	return js;
}

}
}
