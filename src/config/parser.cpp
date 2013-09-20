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

namespace mapcrafter {
namespace config2 {

bool WorldSection::parse(const ConfigSection& section, ValidationList& validation) {
	if (!input_dir.isLoaded())
		input_dir.load(section, "input_dir");

	if (!global) {
		input_dir.require(validation, "You have to specify an input directory ('input_dir')!");
	}

	return validation_valid(validation);
}

bool MapSection::parse(const ConfigSection& section, ValidationList& validation) {
	name_short = section.getName();
	name_long = section.has("name") ? section.get("name") : name_short;

	if (!world.isLoaded())
		world.load(section, "world");

	if (!texture_dir.isLoaded())
		texture_dir.load(section, "texture_dir");
	if (!rotations.isLoaded() && rotations.load(section, "rotations", "top-left")) {

	}
	if (!rendermode.isLoaded() && rendermode.load(section, "rendermode", "normal"))
		rendermode.validateOneOf(validation, "Your specified rendermode is invalid!", {
			"normal", "daylight", "nightlight", "cave",
		});
	if (!texture_size.isLoaded())
		texture_size.load(section, "texture_size", 12);

	if (!render_unknown_blocks.isLoaded())
		render_unknown_blocks.load(section, "render_unkown_blocks", false);
	if (!render_leaves_transparent.isLoaded())
		render_leaves_transparent.load(section, "render_leaves_transparent", true);
	if (!render_biomes.isLoaded())
		render_biomes.load(section, "render_biomes", true);

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

	if (config.hasSection("global", "world")) {
		ValidationList msgs;
		ok = ok && world_global.parse(config.getSection("global", "world"), msgs);
		if (!msgs.empty())
			validation.push_back(std::make_pair("Global world configuration", msgs));
		if (!ok)
			return false;
	}

	if (config.hasSection("global", "map")) {
		ValidationList msgs;
		ok = ok && map_global.parse(config.getSection("global", "map"), msgs);
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

		ok = ok && world.parse(*it, msgs);
		validation.push_back(std::make_pair("World section '" + it->getName() + "'", msgs));

		worlds[it->getName()] = world;
	}

	for (auto it = sections.begin(); it != sections.end(); ++it) {
		if (it->getType() != "map")
			continue;
		ValidationList msgs;
		MapSection map = map_global;
		map.setGlobal(false);

		ok = ok && map.parse(*it, msgs);
		validation.push_back(std::make_pair("Map section '" + it->getName() + "'", msgs));

		maps.push_back(map);
	}

	return ok;
}

} /* namespace config */
} /* namespace mapcrafter */
