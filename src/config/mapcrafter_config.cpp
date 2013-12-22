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

#include "mapcrafter_config.h"

#include "../util.h"
#include "validation.h"

namespace mapcrafter {
namespace config {

WorldSection::WorldSection(bool global)
		: global(global) {
}

WorldSection::~WorldSection() {
}

void WorldSection::setGlobal(bool global) {
	this->global = global;
}

bool WorldSection::parse(const ConfigSection& section, const fs::path& config_dir,
		ValidationList& validation) {
	mc::BlockPos crop_center;

	// go through all configuration options in this section
	//   - load/parse the individual options
	//   - warn the user about unknown options
	auto entries = section.getEntries();
	for (auto entry_it = entries.begin(); entry_it != entries.end(); ++entry_it) {
		std::string key = entry_it->first;
		std::string value = entry_it->second;

		if (key == "input_dir") {
			if (input_dir.load(key, value, validation)) {
				input_dir.setValue(BOOST_FS_ABSOLUTE(input_dir.getValue(), config_dir));
				if (!fs::is_directory(input_dir.getValue()))
					validation.push_back(ValidationMessage::error(
							"'input_dir' must be an existing directory! '"
							+ input_dir.getValue().string() + "' does not exist!"));
			}
		}

		else if (key == "crop_min_y") {
			if (min_y.load(key, value, validation))
				worldcrop.setMinY(min_y.getValue());
		} else if (key == "crop_max_y") {
			if (max_y.load(key, value, validation))
				worldcrop.setMaxY(max_y.getValue());
		} else if (key == "crop_min_x") {
			if (min_x.load(key, value, validation))
				worldcrop.setMinX(min_x.getValue());
		} else if (key == "crop_max_x") {
			if (max_x.load(key, value, validation))
				worldcrop.setMaxX(max_x.getValue());
		} else if (key == "crop_min_z") {
			if (min_z.load(key, value, validation))
				worldcrop.setMinZ(min_z.getValue());
		} else if (key == "crop_max_z") {
			if (max_z.load(key, value, validation))
				worldcrop.setMaxZ(max_z.getValue());

		} else if (key == "crop_center_x")
			center_x.load(key, value, validation);
		else if (key == "crop_center_z")
			center_z.load(key, value, validation);
		else if (key == "crop_radius")
			radius.load(key, value, validation);

		else {
			validation.push_back(ValidationMessage::warning(
					"Unknown configuration option '" + key + "'!"));
		}
	}

	// validate the world croppping
	bool crop_rectangular = min_x.isLoaded() || max_x.isLoaded() || min_z.isLoaded() || max_z.isLoaded();
	bool crop_circular = center_x.isLoaded() || center_z.isLoaded() || radius.isLoaded();

	if (crop_rectangular && crop_circular) {
		validation.push_back(ValidationMessage::error(
				"You can not use both world cropping types at the same time!"));
	} else if (crop_rectangular) {
		if (min_x.isLoaded() && max_x.isLoaded() && min_x.getValue() > max_x.getValue())
			validation.push_back(ValidationMessage::error("min_x must be smaller than or equal to max_x!"));
		if (min_z.isLoaded() && max_z.isLoaded() && min_z.getValue() > max_z.getValue())
			validation.push_back(ValidationMessage::error("min_z must be smaller than or equal to max_z!"));
	} else if (crop_circular) {
		std::string message = "You have to specify crop_center_x, crop_center_z "
				"and crop_radius for circular world cropping!";
		center_x.require(validation, message)
			&& center_z.require(validation, message)
			&& radius.require(validation, message);

		worldcrop.setCenter(mc::BlockPos(center_x.getValue(), center_z.getValue(), 0));
		worldcrop.setRadius(radius.getValue());
	}

	if (min_y.isLoaded() && max_y.isLoaded() && min_y.getValue() > max_y.getValue())
		validation.push_back(ValidationMessage::error("min_y must be smaller than or equal to max_y!"));

	// check if required options were specified
	if (!global) {
		input_dir.require(validation, "You have to specify an input directory ('input_dir')!");
	}

	return isValidationValid(validation);
}

fs::path WorldSection::getInputDir() const {
	return input_dir.getValue();
}

const mc::WorldCrop WorldSection::getWorldCrop() const {
	return worldcrop;
}

bool WorldSection::needsWorldCentering() const {
	// circular cropped worlds and cropped worlds with complete x- and z-bounds
	return (min_x.isLoaded() && max_x.isLoaded() && min_z.isLoaded() && max_z.isLoaded())
			|| center_x.isLoaded() || center_z.isLoaded() || radius.isLoaded();
}

MapSection::MapSection(bool global)
		: global(global), texture_size(12),  render_unknown_blocks(false),
		  render_leaves_transparent(false), render_biomes(false) {
}

MapSection::~MapSection() {
}

void MapSection::setGlobal(bool global) {
	this->global = global;
}

bool MapSection::parse(const ConfigSection& section, const fs::path& config_dir, ValidationList& validation) {
	name_short = section.getName();
	name_long = name_short;

	// set some default configuration values
	// check if we can find a default texture directory
	bool has_default_textures = !util::findTextureDir().empty();
	if (has_default_textures)
		texture_dir.setDefault(util::findTextureDir());
	rotations.setDefault("top-left");
	rendermode.setDefault("daylight");
	texture_size.setDefault(12);
	render_unknown_blocks.setDefault(false);
	render_leaves_transparent.setDefault(true);
	render_biomes.setDefault(true);
	use_image_mtimes.setDefault(true);

	// go through all configuration options in this section
	//   - load/parse the individual options
	//   - warn the user about unknown options
	auto entries = section.getEntries();
	for (auto entry_it = entries.begin(); entry_it != entries.end(); ++entry_it) {
		std::string key = entry_it->first;
		std::string value = entry_it->second;

		if (key == "name") {
			name_long = value;
		} else if (key == "world") {
			world.load(key, value, validation);
		} else if (key == "texture_dir") {
			if (texture_dir.load(key, value, validation)) {
				texture_dir.setValue(BOOST_FS_ABSOLUTE(texture_dir.getValue(), config_dir));
				if (!fs::is_directory(texture_dir.getValue()))
					validation.push_back(ValidationMessage::error(
							"'texture_dir' must be an existing directory! '"
							+ texture_dir.getValue().string() + "' does not exist!"));
			}
		} else if (key == "rotations") {
			rotations.load(key, value ,validation);
		} else if (key == "rendermode") {
			if (rendermode.load(key, value, validation)) {
				std::string r = rendermode.getValue();
				if (r != "normal" && r != "daylight" && r != "nightlight" && r != "cave")
					validation.push_back(ValidationMessage::error(
							"'rendermode' must be one of: 'normal', 'daylight', 'nightlight', 'cave'"));
			}
		} else if (key == "texture_size") {
			if (texture_size.load(key, value, validation)
					&& (texture_size.getValue() <= 0  || texture_size.getValue() > 32))
					validation.push_back(ValidationMessage::error(
							"'texture_size' must a number between 1 and 32!"));
		} else if (key == "render_unknown_blocks") {
			render_unknown_blocks.load(key, value, validation);
		} else if (key == "render_leaves_transparent") {
			render_leaves_transparent.load(key, value, validation);
		} else if (key == "render_biomes") {
			render_biomes.load(key, value, validation);
		} else if (key == "use_image_mtimes") {
			use_image_mtimes.load(key, value, validation);
		} else {
			validation.push_back(ValidationMessage::warning(
					"Unknown configuration option '" + key + "'!"));
		}

	}


	// parse rotations
	rotations_set.clear();
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

	// check if required options were specified
	if (!global) {
		world.require(validation, "You have to specify a world ('world')!");
		// a texture directory is only required
		// if mapcrafter can not find a default texture directory
		if (!has_default_textures)
			texture_dir.require(validation, "You have to specify a texture directory ('texture_dir')!");
	}

	return isValidationValid(validation);
}

std::string MapSection::getShortName() const {
	return name_short;
}

std::string MapSection::getLongName() const {
	return name_long;
}

std::string MapSection::getWorld() const {
	return world.getValue();
}

fs::path MapSection::getTextureDir() const {
	return texture_dir.getValue();
}

std::set<int> MapSection::getRotations() const {
	return rotations_set;
}

std::string MapSection::getRendermode() const {
	return rendermode.getValue();
}

int MapSection::getTextureSize() const {
	return texture_size.getValue();
}

bool MapSection::renderUnknownBlocks() const {
	return render_unknown_blocks.getValue();
}

bool MapSection::renderLeavesTransparent() const {
	return render_leaves_transparent.getValue();
}

bool MapSection::renderBiomes() const {
	return render_biomes.getValue();
}

bool MapSection::useImageModificationTimes() const {
	return use_image_mtimes.getValue();
}

MapcrafterConfigFile::MapcrafterConfigFile()
		: world_global(true), map_global(true) {
}

MapcrafterConfigFile::~MapcrafterConfigFile() {
}

bool MapcrafterConfigFile::parse(const std::string& filename, ValidationMap& validation) {
	ConfigFile config;
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
		ok = world_global.parse(config.getSection("global", "worlds"), config_dir, msgs) && ok;
		if (!msgs.empty())
			validation.push_back(std::make_pair("Global world configuration", msgs));
		if (!ok)
			return false;
	}

	if (config.hasSection("global", "maps")) {
		ValidationList msgs;
		ok = map_global.parse(config.getSection("global", "maps"), config_dir, msgs) && ok;
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
		ok = world.parse(*it, config_dir, msgs) && ok;

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
		ok = map.parse(*it, config_dir, msgs) && ok;

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
	out << "  use_image_timestamps = " << section.useImageModificationTimes() << std::endl;
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

fs::path MapcrafterConfigFile::getOutputDir() const {
	return output_dir.getValue();
}

fs::path MapcrafterConfigFile::getTemplateDir() const {
	return template_dir.getValue();
}

std::string MapcrafterConfigFile::getOutputPath(
		const std::string& path) const {
	return (output_dir.getValue() / path).string();
}

std::string MapcrafterConfigFile::getTemplatePath(
		const std::string& path) const {
	return (template_dir.getValue() / path).string();
}

bool MapcrafterConfigFile::hasWorld(const std::string& world) const {
	return worlds.count(world);
}

const std::map<std::string, WorldSection>& MapcrafterConfigFile::getWorlds() const {
	return worlds;
}

const WorldSection& MapcrafterConfigFile::getWorld(
		const std::string& world) const {
	return worlds.at(world);
}

bool MapcrafterConfigFile::hasMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.end(); ++it)
		if (it->getShortName() == map)
			return true;
	return false;
}

const std::vector<MapSection>& MapcrafterConfigFile::getMaps() const {
	return maps;
}

const MapSection& MapcrafterConfigFile::getMap(const std::string& map) const {
	for (auto it = maps.begin(); it != maps.end(); ++it)
		if (it->getShortName() == map)
			return *it;
	throw std::out_of_range("Map not found!");
}

MapcrafterConfigHelper::MapcrafterConfigHelper() {
}

MapcrafterConfigHelper::MapcrafterConfigHelper(const MapcrafterConfigFile& config)
	: config(config) {
	auto maps = config.getMaps();
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		map_zoomlevels[map_it->getShortName()] = 0;
		for (int i = 0; i < 4; i++)
			render_behaviors[map_it->getShortName()][i] = RENDER_AUTO;
	}

	auto worlds = config.getWorlds();
	for (auto world_it = worlds.begin(); world_it != worlds.end(); ++world_it) {
		world_rotations[world_it->first] = std::set<int>();
		world_zoomlevels[world_it->first] = 0;
		world_tile_offsets[world_it->first] = std::array<render::TilePos, 4>();
	}
}

MapcrafterConfigHelper::~MapcrafterConfigHelper() {
}

std::string MapcrafterConfigHelper::generateTemplateJavascript() const {
	std::string js = "";

	auto maps = config.getMaps();
	for (auto it = maps.begin(); it != maps.end(); ++it) {
		js += "\"" + it->getShortName() + "\" : {\n";
		js += "\tname: \"" + it->getLongName() + "\",\n";
		js += "\tworldName: \"" + it->getWorld() + "\",\n";
		js += "\ttextureSize: " + util::str(it->getTextureSize()) + ",\n";
		js += "\ttileSize: " + util::str(32 * it->getTextureSize()) + ",\n";
		js += "\tmaxZoom: " + util::str(getMapZoomlevel(it->getShortName())) + ",\n";
		js += "\trotations: [";
		auto rotations = it->getRotations();
		for (auto it2 = rotations.begin(); it2 != rotations.end(); ++it2)
			js += util::str(*it2) + ",";
		js += "],\n";

		std::string tile_offsets = "[";
		auto offsets = world_tile_offsets.at(it->getWorld());
		for (auto it2 = offsets.begin(); it2 != offsets.end(); ++it2)
			tile_offsets += "[" + util::str(it2->getX()) + ", " + util::str(it2->getY()) + "], ";
		tile_offsets += "]";

		js += "\ttileOffsets: " + tile_offsets + ",\n";

		js += "},";
	}

	return js;
}


const std::set<int>& MapcrafterConfigHelper::getUsedRotations(const std::string& world) const {
	return world_rotations.at(world);
}

void MapcrafterConfigHelper::setUsedRotations(const std::string& world, const std::set<int>& rotations) {
	for (auto rotation_it = rotations.begin(); rotation_it != rotations.end(); ++rotation_it)
		world_rotations[world].insert(*rotation_it);
}

int MapcrafterConfigHelper::getWorldZoomlevel(const std::string& world) const {
	return world_zoomlevels.at(world);
}

int MapcrafterConfigHelper::getMapZoomlevel(const std::string& map) const {
	if (!map_zoomlevels.count(map))
		return 0;
	return map_zoomlevels.at(map);
}

void MapcrafterConfigHelper::setWorldZoomlevel(const std::string& world, int zoomlevel) {
	world_zoomlevels[world] = zoomlevel;
}

void MapcrafterConfigHelper::setMapZoomlevel(const std::string& map, int zoomlevel) {
	map_zoomlevels[map] = zoomlevel;
}

void MapcrafterConfigHelper::setWorldTileOffset(const std::string& world,
		int rotation, const render::TilePos& tile_offset) {
	world_tile_offsets[world][rotation] = tile_offset;
}

const render::TilePos& MapcrafterConfigHelper::getWorldTileOffset(
		const std::string& world, int rotation) {
	return world_tile_offsets.at(world).at(rotation);
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

void MapcrafterConfigHelper::setRenderBehaviors(std::vector<std::string> maps, int behavior) {
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		std::string map = *map_it;
		std::string rotation;

		size_t pos = map_it->find(":");
		if (pos != std::string::npos) {
			rotation = map.substr(pos+1);
			map = map.substr(0, pos);
		} else
			rotation = "";

		int r = stringToRotation(rotation, ROTATION_NAMES_SHORT);
		if (!config.hasMap(map)) {
			std::cout << "Warning: Unknown map '" << map << "'." << std::endl;
			continue;
		}

		if (!rotation.empty()) {
			if (r == -1) {
				std::cout << "Warning: Unknown rotation '" << rotation << "'." << std::endl;
				continue;
			}
			if (!config.getMap(map).getRotations().count(r)) {
				std::cout << "Warning: Map '" << map << "' does not have rotation '" << rotation << "'." << std::endl;
				continue;
			}
		}

		if (r != -1)
			render_behaviors[map][r] = behavior;
		else
			std::fill(&render_behaviors[map][0], &render_behaviors[map][4], behavior);
	}
}

void MapcrafterConfigHelper::parseRenderBehaviors(bool skip_all,
		std::vector<std::string> render_skip,
		std::vector<std::string> render_auto,
		std::vector<std::string> render_force) {
	if (!skip_all)
		setRenderBehaviors(render_skip, RENDER_SKIP);
	else
		for (size_t i = 0; i < config.getMaps().size(); i++)
			for (int j = 0; j < 4; j++)
				render_behaviors[config.getMaps()[i].getShortName()][j] = RENDER_SKIP;
	setRenderBehaviors(render_auto, RENDER_AUTO);
	setRenderBehaviors(render_force, RENDER_FORCE);
}

} /* namespace config */
} /* namespace mapcrafter */
