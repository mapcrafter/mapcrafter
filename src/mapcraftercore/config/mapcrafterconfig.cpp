/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "mapcrafterconfig.h"

#include "configparser.h"
#include "iniconfig.h"
#include "../util.h"

#include <sstream>

namespace mapcrafter {
namespace util {

template <>
mapcrafter::config::Color as<mapcrafter::config::Color>(const std::string& from) {
	std::string error_message = "Hex color must be in the format '#rrggbb'.";
	if (from.size() != 7)
		throw std::invalid_argument(error_message);
	for (size_t i = 1; i < 7; i++)
		if (!isxdigit(from[i]))
			throw std::invalid_argument(error_message);

	mapcrafter::config::Color color;
	color.hex = from;
	color.red = util::parseHexNumber(from.substr(1, 2));
	color.green = util::parseHexNumber(from.substr(3, 2));
	color.blue = util::parseHexNumber(from.substr(5, 2));
	return color;
}

}
}

namespace mapcrafter {
namespace config {

MapcrafterConfigRootSection::MapcrafterConfigRootSection() {
}

MapcrafterConfigRootSection::~MapcrafterConfigRootSection() {
}

void MapcrafterConfigRootSection::setConfigDir(const fs::path& config_dir) {
	this->config_dir = config_dir;
}

void MapcrafterConfigRootSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	fs::path default_template_dir = util::findTemplateDir();
	if (!default_template_dir.empty())
		template_dir.setDefault(default_template_dir);
	background_color.setDefault({"#DDDDDD", 0xDD, 0xDD, 0xDD});
}

bool MapcrafterConfigRootSection::parseField(const std::string key,
		const std::string value, ValidationList& validation) {
	if (key == "output_dir") {
		if (output_dir.load(key, value, validation))
			output_dir.setValue(BOOST_FS_ABSOLUTE(output_dir.getValue(), config_dir));
	} else if (key == "template_dir") {
		if (template_dir.load(key, value, validation)) {
			template_dir.setValue(BOOST_FS_ABSOLUTE(template_dir.getValue(), config_dir));
			if (!fs::is_directory(template_dir.getValue()))
				validation.error("'template_dir' must be an existing directory! '"
						+ template_dir.getValue().string() + "' does not exist!");
		}
	} else if (key == "background_color") {
		background_color.load(key, value, validation);
	} else
		return false;
	return true;
}

void MapcrafterConfigRootSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	output_dir.require(validation, "You have to specify an output directory ('output_dir')!");
	template_dir.require(validation, "You have to specify a template directory ('template_dir')!");
}

fs::path MapcrafterConfigRootSection::getOutputDir() const {
	return output_dir.getValue();
}

fs::path MapcrafterConfigRootSection::getTemplateDir() const {
	return template_dir.getValue();
}

Color MapcrafterConfigRootSection::getBackgroundColor() const {
	return background_color.getValue();
}

MapcrafterConfig::MapcrafterConfig()
	: world_global(true), map_global(true) {
}

MapcrafterConfig::~MapcrafterConfig() {
}

ValidationMap MapcrafterConfig::parse(const std::string& filename) {
	ValidationMap validation;

	INIConfig config;
	ValidationMessage msg;
	if (!config.loadFile(filename, msg)) {
		validation.section("Configuration file").message(msg);
		return validation;
	}

	fs::path config_dir = BOOST_FS_ABSOLUTE1(fs::path(filename)).parent_path();
	root_section.setConfigDir(config_dir);

	ConfigParser parser(config);
	parser.parseRootSection(root_section);
	parser.parseSections(worlds, "world", MapcrafterConfigSectionFactory<WorldSection>(config_dir));
	parser.parseSections(maps, "map", MapcrafterConfigSectionFactory<MapSection>(config_dir));
	parser.parseSections(markers, "marker");
	parser.validate();
	validation = parser.getValidation();

	// check if all worlds specified for maps exist
	// 'map_it->getWorld() != ""' because that's already handled by map section class
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it)
		if (map_it->getWorld() != "" && !hasWorld(map_it->getWorld())) {
			validation.section(util::capitalize(map_it->getPrettyName())).error(
					"World '" + map_it->getWorld() + "' does not exist!");
		}

	return validation;
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
	out << "  output_dir = " << getOutputDir().string() << std::endl;
	out << "  template_dir = " << getTemplateDir().string() << std::endl;
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
	return root_section.getOutputDir();
}

fs::path MapcrafterConfig::getTemplateDir() const {
	return root_section.getTemplateDir();
}

Color MapcrafterConfig::getBackgroundColor() const {
	return root_section.getBackgroundColor();
}

std::string MapcrafterConfig::getOutputPath(
		const std::string& path) const {
	return (getOutputDir() / path).string();
}

std::string MapcrafterConfig::getTemplatePath(
		const std::string& path) const {
	return (getTemplateDir() / path).string();
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

bool MapcrafterConfig::hasMarker(const std::string marker) const {
	for (auto it = markers.begin(); it != markers.end(); ++it)
		if (it->getShortName() == marker)
			return true;
	return false;
}

const std::vector<MarkerSection>& MapcrafterConfig::getMarkers() const {
	return markers;
}

const MarkerSection& MapcrafterConfig::getMarker(const std::string& marker) const {
	for (auto it = markers.begin(); it != markers.end(); ++it)
		if (it->getShortName() == marker)
			return *it;
	throw std::out_of_range("Marker not found!");
}

} /* namespace config */
} /* namespace mapcrafter */
