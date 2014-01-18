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

#include "mapcrafterconfig.h"

#include "iniconfig.h"
#include "../util.h"

namespace mapcrafter {
namespace config {

MapcrafterConfig::MapcrafterConfig()
	: world_global(true), map_global(true) {
}

MapcrafterConfig::~MapcrafterConfig() {
}

bool MapcrafterConfig::parse(const std::string& filename, ValidationMap& validation) {
	INIConfig config;
	ValidationMessage msg;
	if (!config.loadFile(filename, msg)) {
		validation.push_back(std::make_pair("Configuration file", makeValidationList(msg)));
		return false;
	}

	fs::path config_dir = fs::path(filename).parent_path();

	bool ok = true;

	ValidationList general_msgs;

	bool has_default_template = !util::findTemplateDir().empty();
	if (has_default_template)
		template_dir.setDefault(util::findTemplateDir());

	auto entries = config.getRootSection().getEntries();
	for (auto entry_it = entries.begin(); entry_it != entries.end(); ++entry_it) {
		std::string key = entry_it->first;
		std::string value = entry_it->second;

		if (key == "output_dir") {
			if (output_dir.load(key, value, general_msgs))
				output_dir.setValue(BOOST_FS_ABSOLUTE(output_dir.getValue(), config_dir));
		} else if (key == "template_dir") {
			if (template_dir.load(key, value, general_msgs)) {
				template_dir.setValue(BOOST_FS_ABSOLUTE(template_dir.getValue(), config_dir));
				if (!fs::is_directory(template_dir.getValue()))
					general_msgs.push_back(ValidationMessage::error(
							"'template_dir' must be an existing directory! '"
							+ template_dir.getValue().string() + "' does not exist!"));
			}
		} else {
			general_msgs.push_back(ValidationMessage::warning(
					"Unknown configuration option '" + key + "'!"));
		}
	}

	if (!output_dir.require(general_msgs, "You have to specify an output directory ('output_dir')!"))
		ok = false;
	if (!has_default_template)
		template_dir.require(general_msgs, "You have to specify a template directory ('template_dir')!");

	if (!general_msgs.empty())
		validation.push_back(std::make_pair("Configuration file", general_msgs));

	if (config.hasSection("global", "worlds")) {
		ValidationList msgs;
		world_global.setConfigDir(config_dir);
		ok = world_global.parse(config.getSection("global", "worlds"), msgs) && ok;
		if (!msgs.empty())
			validation.push_back(std::make_pair("Global world configuration", msgs));
		if (!ok)
			return false;
	}

	if (config.hasSection("global", "maps")) {
		ValidationList msgs;
		map_global.setConfigDir(config_dir);
		ok = map_global.parse(config.getSection("global", "maps"), msgs) && ok;
		if (!msgs.empty())
			validation.push_back(std::make_pair("Global map configuration", msgs));
		if (!ok)
			return false;
	}

	auto sections = config.getSections();

	for (auto it = sections.begin(); it != sections.end(); ++it)
		if (it->getType() != "world" && it->getType() != "map"
				&& it->getNameType() != "global:worlds"
				&& it->getNameType() != "global:maps") {
			validation.push_back(std::make_pair("Section '" + it->getName() + "' with type '" + it->getType() + "'",
					makeValidationList(ValidationMessage::warning("Unknown section type!"))));
		}

	for (auto it = sections.begin(); it != sections.end(); ++it) {
		if (it->getType() != "world")
			continue;
		ValidationList msgs;
		WorldSection world = world_global;
		world.setGlobal(false);
		world.setConfigDir(config_dir);
		ok = world.parse(*it, msgs) && ok;

		if (hasWorld(it->getName())) {
			msgs.push_back(ValidationMessage::error("World name '" + it->getName() + "' already used!"));
			ok = false;
		} else
			worlds[it->getName()] = world;

		if (!msgs.empty())
			validation.push_back(std::make_pair("World section '" + it->getName() + "'", msgs));
	}

	for (auto it = sections.begin(); it != sections.end(); ++it) {
		if (it->getType() != "map")
			continue;
		ValidationList msgs;
		MapSection map = map_global;
		map.setGlobal(false);
		map.setConfigDir(config_dir);
		ok = map.parse(*it, msgs) && ok;

		if (hasMap(it->getName())) {
			msgs.push_back(ValidationMessage::error("Map name '" + it->getName() + "' already used!"));
			ok = false;
		} else if (map.getWorld() != "" && !hasWorld(map.getWorld())) {
			msgs.push_back(ValidationMessage::error("World '" + map.getWorld() + "' does not exist!"));
			ok = false;
		} else
			maps.push_back(map);

		if (!msgs.empty())
			validation.push_back(std::make_pair("Map section '" + it->getName() + "'", msgs));
	}

	return ok;
}

std::string rotationsToString(std::set<int> rotations) {
	std::string str;
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		if (*it >= 0 && *it < 4)
			str += " " + ROTATION_NAMES[*it];
	return util::trim(str);
}

void dumpWorldSection(std::ostream& out, const WorldSection& section) {
	out << "  input_dir = " << section.getInputDir().string() << std::endl;
}

void dumpMapSection(std::ostream& out, const MapSection& section) {
	out << "  name = " << section.getLongName() << std::endl;
	out << "  world = " << section.getWorld() << std::endl;
	out << "  texture_dir = " << section.getTextureDir() << std::endl;
	out << "  rotations = " << rotationsToString(section.getRotations()) << std::endl;
	out << "  rendermode = " << section.getRendermode() << std::endl;
	out << "  texture_size = " << section.getTextureSize() << std::endl;
	out << "  render_unknown_blocks = " << section.renderUnknownBlocks() << std::endl;
	out << "  render_leaves_transparent = " << section.renderLeavesTransparent() << std::endl;
	out << "  render_biomes = " << section.renderBiomes() << std::endl;
	out << "  use_image_timestamps = " << section.useImageModificationTimes() << std::endl;
}

void MapcrafterConfig::dump(std::ostream& out) const {
	out << "General:" << std::endl;
	out << "  output_dir = " << output_dir.getValue().string() << std::endl;
	out << "  template_dir = " << template_dir.getValue().string() << std::endl;
	out << std::endl;

	out << "Global world configuration:" << std::endl;
	dumpWorldSection(out, world_global);
	out << std::endl;

	out << "Global map configuration:" << std::endl;
	dumpMapSection(out, map_global);
	out << std::endl;

	for (auto it = worlds.begin(); it != worlds.end(); ++it) {
		out << "World '" << it->first << "':" << std::endl;
		dumpWorldSection(out, it->second);
		out << std::endl;
	}

	for (auto it = maps.begin(); it != maps.end(); ++it) {
		out << "Map '" << it->getShortName() << "':" << std::endl;
		dumpMapSection(out, *it);
		out << std::endl;
	}
}

fs::path MapcrafterConfig::getOutputDir() const {
	return output_dir.getValue();
}

fs::path MapcrafterConfig::getTemplateDir() const {
	return template_dir.getValue();
}

std::string MapcrafterConfig::getOutputPath(
		const std::string& path) const {
	return (output_dir.getValue() / path).string();
}

std::string MapcrafterConfig::getTemplatePath(
		const std::string& path) const {
	return (template_dir.getValue() / path).string();
}

bool MapcrafterConfig::hasWorld(const std::string& world) const {
	return worlds.count(world);
}

const std::map<std::string, WorldSection>& MapcrafterConfig::getWorlds() const {
	return worlds;
}

const WorldSection& MapcrafterConfig::getWorld(
		const std::string& world) const {
	return worlds.at(world);
}

bool MapcrafterConfig::hasMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.end(); ++it)
		if (it->getShortName() == map)
			return true;
	return false;
}

const std::vector<MapSection>& MapcrafterConfig::getMaps() const {
	return maps;
}

const MapSection& MapcrafterConfig::getMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.end(); ++it)
		if (it->getShortName() == map)
			return *it;
	throw std::out_of_range("Map not found!");
}

} /* namespace config */
} /* namespace mapcrafter */
