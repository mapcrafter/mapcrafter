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
	if (input_dir.load(section, "input_dir") && !fs::is_directory(input_dir.getValue()))
		validation.push_back(ValidationMessage::error("'input_dir' must be an existing directory! '"
				+ input_dir.getValue().string() + "' does not exist!"));

	if (!global) {
		input_dir.require(validation, "You have to specify an input directory ('input_dir')!");
	}

	return validation_valid(validation);
}

bool MapSection::parse(const ConfigSection& section, ValidationList& validation) {
	name_short = section.getName();
	name_long = section.has("name") ? section.get("name") : name_short;

	world.load(section, "world");

	if (texture_dir.load(section, "texture_dir") && !fs::is_directory(texture_dir.getValue()))
		validation.push_back(ValidationMessage::error("'texture_dir' must be an existing directory! '"
				+ texture_dir.getValue().string() + "' does not exist!"));

	if (rotations.load(section, "rotations", "top-left")) {
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

	if (rendermode.load(section, "rendermode", "normal")) {
		std::string r = rendermode.getValue();
		if (r != "normal" && r != "daylight" && r != "nightlight" && r != "cave")
			validation.push_back(ValidationMessage::error("'rendermode' must be one of: normal, daylight, nightlight, cave"));
	}

	if (texture_size.load(section, "texture_size", 12))
		if (texture_size.getValue() <= 0 || texture_size.getValue() > 32)
			validation.push_back(ValidationMessage::error("'texture_size' must a number between 1 and 32!"));

	render_unknown_blocks.load(section, "render_unkown_blocks", false);
	render_leaves_transparent.load(section, "render_leaves_transparent", true);
	render_biomes.load(section, "render_biomes", true);
	use_image_timestamps.load(section, "use_image_timestamps", true);

	if (!global) {
		world.require(validation, "You have to specify a world ('world')!");
	}

	return validation_valid(validation);
}

bool ConfigParser::parse(const std::string& filename, ValidationMap& validation) {
	ConfigFile config;
	ValidationMessage msg;
	if (!config.loadFile(filename, msg)) {
		validation.push_back(std::make_pair("Configuration file", make_validation_list(msg)));
		return false;
	}

	bool ok = true;

	ValidationList general_msgs;
	output_dir.load(config.getRootSection(), "output_dir");
	output_dir.require(general_msgs, "You have to specify an output directory ('output_dir')!");
	if (template_dir.load(config.getRootSection(), "template_dir") &&
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
					make_validation_list(ValidationMessage::warning("Unknown section type!"))));
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

bool ConfigParser::hasMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.end(); ++it)
		if (it->getShortName() == map)
			return true;
	return false;
}

const MapSection& ConfigParser::getMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.begin(); ++it)
		if (it->getShortName() == map)
			return *it;
	throw std::out_of_range("Map not found!");
}

} /* namespace config */
} /* namespace mapcrafter */
