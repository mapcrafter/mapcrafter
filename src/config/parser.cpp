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

#include "parser.h"

#include "validation.h"

namespace mapcrafter {
namespace config2 {

bool WorldSection::parse(const ConfigSection& section, ValidationList& validation) {
	if (input_dir.load(validation, section, "input_dir") && !fs::is_directory(input_dir.getValue()))
		validation.push_back(ValidationMessage::error("'input_dir' must be an existing directory! '"
				+ input_dir.getValue().string() + "' does not exist!"));

	if (!global) {
		input_dir.require(validation, "You have to specify an input directory ('input_dir')!");
	}

	return isValidationValid(validation);
}

bool MapSection::parse(const ConfigSection& section, ValidationList& validation) {
	name_short = section.getName();
	name_long = section.has("name") ? section.get("name") : name_short;

	world.load(validation, section, "world");

	if (texture_dir.load(validation, section, "texture_dir") && !fs::is_directory(texture_dir.getValue()))
		validation.push_back(ValidationMessage::error("'texture_dir' must be an existing directory! '"
				+ texture_dir.getValue().string() + "' does not exist!"));

	if (rotations.load(validation, section, "rotations", "top-left")) {
		std::string str = rotations.getValue();
		std::stringstream ss;
		ss << str;
		std::string elem;
		while (ss >> elem) {
			int r = stringToRotation(elem);
			if (r != -1)
				rotations_set.insert(r);
			else
				validation.push_back(ValidationMessage::error("Invalid rotation '" + elem + "'!"));
		}
	}

	if (rendermode.load(validation, section, "rendermode", "normal")) {
		std::string r = rendermode.getValue();
		if (r != "normal" && r != "daylight" && r != "nightlight" && r != "cave")
			validation.push_back(ValidationMessage::error("'rendermode' must be one of: normal, daylight, nightlight, cave"));
	}

	if (texture_size.load(validation, section, "texture_size", 12))
		if (texture_size.getValue() <= 0 || texture_size.getValue() > 32)
			validation.push_back(ValidationMessage::error("'texture_size' must a number between 1 and 32!"));

	render_unknown_blocks.load(validation, section, "render_unkown_blocks", false);
	render_leaves_transparent.load(validation, section, "render_leaves_transparent", true);
	render_biomes.load(validation, section, "render_biomes", true);
	use_image_timestamps.load(validation, section, "use_image_timestamps", true);

	if (!global) {
		world.require(validation, "You have to specify a world ('world')!");
	}

	return isValidationValid(validation);
}

bool MapcrafterConfigFile::parse(const std::string& filename, ValidationMap& validation) {
	ConfigFile config;
	ValidationMessage msg;
	if (!config.loadFile(filename, msg)) {
		validation.push_back(std::make_pair("Configuration file", makeValidationList(msg)));
		return false;
	}

	bool ok = true;

	ValidationList general_msgs;
	output_dir.load(general_msgs, config.getRootSection(), "output_dir");
	output_dir.require(general_msgs, "You have to specify an output directory ('output_dir')!");
	if (template_dir.load(general_msgs, config.getRootSection(), "template_dir") &&
			!fs::is_directory(template_dir.getValue()))
		general_msgs.push_back(ValidationMessage::error("'template_dir' must be an existing directory! '"
				+ template_dir.getValue().string() + "' does not exist!"));
	validation.push_back(std::make_pair("Configuration file", general_msgs));

	if (config.hasSection("global", "world")) {
		ValidationList msgs;
		ok = world_global.parse(config.getSection("global", "world"), msgs) && ok;
		if (!msgs.empty())
			validation.push_back(std::make_pair("Global world configuration", msgs));
		if (!ok)
			return false;
	}

	if (config.hasSection("global", "map")) {
		ValidationList msgs;
		ok = map_global.parse(config.getSection("global", "map"), msgs) && ok;
		if (!msgs.empty())
			validation.push_back(std::make_pair("Global map configuration", msgs));
		if (!ok)
			return false;
	}

	auto sections = config.getSections();

	for (auto it = sections.begin(); it != sections.end(); ++it)
		if (it->getType() != "world" && it->getType() != "map"
				&& it->getNameType() != "global:world"
				&& it->getNameType() != "global:map") {
			validation.push_back(std::make_pair("Section '" + it->getName() + "' with type '" + it->getType() + "'",
					makeValidationList(ValidationMessage::warning("Unknown section type!"))));
		}

	for (auto it = sections.begin(); it != sections.end(); ++it) {
		if (it->getType() != "world")
			continue;
		ValidationList msgs;
		WorldSection world = world_global;
		world.setGlobal(false);
		ok = world.parse(*it, msgs) && ok;

		if (hasWorld(it->getName())) {
			msgs.push_back(ValidationMessage::error("World name '" + it->getName() + "' already used!"));
			ok = false;
		} else
			worlds[it->getName()] = world;

		validation.push_back(std::make_pair("World section '" + it->getName() + "'", msgs));
	}

	for (auto it = sections.begin(); it != sections.end(); ++it) {
		if (it->getType() != "map")
			continue;
		ValidationList msgs;
		MapSection map = map_global;
		map.setGlobal(false);
		ok = map.parse(*it, msgs) && ok;

		if (hasMap(it->getName())) {
			msgs.push_back(ValidationMessage::error("Map name '" + it->getName() + "' already used!"));
			ok = false;
		} else
			maps.push_back(map);

		validation.push_back(std::make_pair("Map section '" + it->getName() + "'", msgs));
	}

	return ok;
}

std::string rotationsToString(std::set<int> rotations) {
	std::string str;
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		if (*it >= 0 && *it < 4)
			str += " " + ROTATION_NAMES[*it];
	util::trim(str);
	return str;
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
	out << "  use_image_timestamps = " << section.useImageTimestamps() << std::endl;
}

void MapcrafterConfigFile::dump(std::ostream& out) const {
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

bool MapcrafterConfigFile::hasMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.end(); ++it)
		if (it->getShortName() == map)
			return true;
	return false;
}

const MapSection& MapcrafterConfigFile::getMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.begin(); ++it)
		if (it->getShortName() == map)
			return *it;
	throw std::out_of_range("Map not found!");
}

MapcrafterConfigHelper::MapcrafterConfigHelper() {
}

MapcrafterConfigHelper::MapcrafterConfigHelper(const MapcrafterConfigFile& config)
	: config(config) {
	auto worlds = config.getWorlds();
	for (auto it = worlds.begin(); it != worlds.end(); ++it)
		world_zoomlevels[it->first] = 0;
}

MapcrafterConfigHelper::~MapcrafterConfigHelper() {
}

std::string MapcrafterConfigHelper::generateTemplateJavascript() const {
	std::string js = "";

	auto maps = config.getMaps();
	for (auto it = maps.begin(); it != maps.end(); ++it) {
		std::string world_name = BOOST_FS_FILENAME(config.getWorld(it->getWorld()).getInputDir());

		js += "\"" + it->getShortName() + "\" : {\n";
		js += "\tname: \"" + it->getLongName() + "\",\n";
		js += "\tworldName: \"" + world_name + "\",\n";
		js += "\ttextureSize: " + util::str(it->getTextureSize()) + ",\n";
		js += "\ttileSize: " + util::str(32 * it->getTextureSize()) + ",\n";
		//js += "\tmaxZoom: " + util::str(worlds_max_zoom[i]) + ",\n";
		js += "\trotations: [";
		for (auto it2 = it->getRotations().begin(); it2 != it->getRotations().end(); ++it2)
			js += util::str(*it2) + ",";
		js += "],\n";
		js += "},";
	}

	return js;
}


const std::set<int>& MapcrafterConfigHelper::getUsedRotations(const std::string& world) const {
	return world_rotations.at(world);
}

void MapcrafterConfigHelper::setUsedRotations(const std::string& world, const std::set<int>& rotations) {
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		world_rotations[world].insert(*it);
}

int MapcrafterConfigHelper::getWorldZoomlevel(const std::string& world) const {
	return world_zoomlevels.at(world);
}

void MapcrafterConfigHelper::setWorldZoomlevel(const std::string& world, int zoomlevel) {
	world_zoomlevels[world] = zoomlevel;
}

int MapcrafterConfigHelper::getRenderBehavior(const std::string& map, int rotation) const {
	return render_behaviors.at(map).at(rotation);
}

void MapcrafterConfigHelper::setRenderBehavior(const std::string& map, int rotation, int behavior) {
	if (rotation == -1)
		for (size_t i = 0; i < 4; i++)
			render_behaviors[map][i] = behavior;
	else
		render_behaviors[map][rotation] = behavior;
}

bool MapcrafterConfigHelper::isCompleteRenderSkip(const std::string& map) const {
	const std::set<int>& rotations = config.getMap(map).getRotations();
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		if (getRenderBehavior(map, *it) != RENDER_SKIP)
			return false;
	return true;
}

bool MapcrafterConfigHelper::isCompleteRenderForce(const std::string& map) const {
	const std::set<int>& rotations = config.getMap(map).getRotations();
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		if (getRenderBehavior(map, *it) != RENDER_FORCE)
			return false;
	return true;
}

} /* namespace config */
} /* namespace mapcrafter */
