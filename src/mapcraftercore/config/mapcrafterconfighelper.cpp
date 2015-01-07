/*
 * Copyright 2012-2015 Moritz Hilscher
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

#include "mapcrafterconfighelper.h"

namespace mapcrafter {
namespace config {

MapSettings::MapSettings()
	: texture_size(12), image_format("png"), lighting_intensity(1.0),
	  render_unknown_blocks(0), render_leaves_transparent(0), render_biomes(false),
	  tile_size(0), max_zoom(0) {
	for (int i = 0; i < 4; i++) {
		rotations[i] = false;
		last_render[i] = 0;
		tile_offsets[i] = renderer::TilePos(0, 0);
	}
}

/**
 * This method reads the map settings from a file.
 */
bool MapSettings::read(const fs::path& filename) {
	config::INIConfig config;
	try {
		config.loadFile(filename.string());
	} catch (config::INIConfigError& exception) {
		LOG(WARNING) << "Unable to read map.settings file '" << filename.string() << "': "
				<< exception.what();
		return false;
	}

	config::INIConfigSection& root = config.getRootSection();

	// don't set default values for new options here, that's done in syncMapConfig
	if (root.has("texture_size"))
		texture_size.set(root.get<int>("texture_size"));
	if (root.has("image_format"))
		image_format.set(root.get<std::string>("image_format"));
	if (root.has("lighting_intensity"))
		lighting_intensity.set(root.get<double>("lighting_intensity"));
	// exception for default value of cave_high_contrast
	// according to config defaults to true, but for older maps it's false
	cave_high_contrast.set(root.get<bool>("cave_high_contrast", false));
	if (root.has("render_unknown_blocks"))
		render_unknown_blocks.set(root.get<bool>("render_unknown_blocks"));
	if (root.has("render_leaves_transparent"))
		render_leaves_transparent.set(root.get<bool>("render_leaves_transparent"));
	if (root.has("render_biomes"))
		render_biomes.set(root.get<bool>("render_biomes"));

	// TODO
	// try to read tile size, otherwise fall back to the default value
	// when there was only the 3d isometric render mode
	if (root.has("tile_size"))
		tile_size = root.get<int>("tile_size");
	else
		tile_size = texture_size.get() * 32;
	max_zoom = root.get<int>("max_zoom");

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		rotations[i] = config.hasSection("rotation", rotation_names[i]);
		if (rotations[i]) {
			auto section = config.getSection("rotation", rotation_names[i]);
			last_render[i] = section.get<int>("last_render");
			int offset_x = section.get<int>("tile_offset_x", 0);
			int offset_y = section.get<int>("tile_offset_y", 0);
			tile_offsets[i] = renderer::TilePos(offset_x, offset_y);
		}
	}

	return true;
}

/**
 * This method writes the map settings to a file.
 */
bool MapSettings::write(const fs::path& filename) const {
	config::INIConfig config;
	config::INIConfigSection& root = config.getRootSection();

	root.set("texture_size", util::str(texture_size.get()));
	root.set("image_format", image_format.get());
	root.set("lighting_intensity", util::str(lighting_intensity.get()));
	root.set("cave_high_contrast", util::str(cave_high_contrast.get()));
	root.set("render_unknown_blocks", util::str(render_unknown_blocks.get()));
	root.set("render_leaves_transparent", util::str(render_leaves_transparent.get()));
	root.set("render_biomes", util::str(render_biomes.get()));

	root.set("tile_size", util::str(tile_size));
	root.set("max_zoom", util::str(max_zoom));

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		if (rotations[i]) {
			auto& section = config.getSection("rotation", rotation_names[i]);
			section.set("last_render", util::str(last_render[i]));
			section.set("tile_offset_x", util::str(tile_offsets[i].getX()));
			section.set("tile_offset_y", util::str(tile_offsets[i].getY()));
		}
	}

	try {
		config.writeFile(filename.string());
	} catch (config::INIConfigError& exception) {
		LOG(WARNING) << "Unable to write map.settings file '" << filename.string() << "': "
				<< exception.what();
		return false;
	}
	return true;
}

bool MapSettings::syncMapConfig(const config::MapSection& map) {
	if (texture_size.isNull())
		texture_size.set(map.getTextureSize());
	if (image_format.isNull())
		image_format.set(map.getImageFormatSuffix());
	if (lighting_intensity.isNull())
		lighting_intensity.set(map.getLightingIntensity());
	if (cave_high_contrast.isNull())
		cave_high_contrast.set(map.hasCaveHighContrast());
	if (render_unknown_blocks.isNull())
		render_unknown_blocks.set(map.renderUnknownBlocks());
	if (render_leaves_transparent.isNull())
		render_leaves_transparent.set(map.renderLeavesTransparent());
	if (render_biomes.isNull())
		render_biomes.set(map.renderBiomes());

	bool changed = true;
	bool force_required = false;
	if (texture_size.get() != map.getTextureSize()) {
		LOG(ERROR) << "You changed the texture size from " << texture_size.get()
				<< " to " << map.getTextureSize() << ".";
		force_required = true;
	} else if (image_format.get() != map.getImageFormatSuffix()) {
		LOG(ERROR) << "You changed the image format from " << image_format.get()
				<< " to " << map.getImageFormatSuffix() << ".";
		LOG(ERROR) << "Force-render the whole map in order for the new "
				<< "configuration to come into effect and delete the images "
				<< "generated with the other image format.";
		force_required = true;
		return false;
	} else if (!util::floatingPointEquals(lighting_intensity.get(), map.getLightingIntensity())) {
		LOG(WARNING) << "You changed the lighting intensity from "
				<< lighting_intensity.get() << " to " << map.getLightingIntensity() << ".";
	} else if (cave_high_contrast.get() != map.hasCaveHighContrast()) {
		LOG(WARNING) << "You have changed the cave high contrast mode from "
				<< util::str(cave_high_contrast.get()) << " to "
				<< util::str(map.hasCaveHighContrast()) << ".";
	} else if (render_unknown_blocks.get() != map.renderUnknownBlocks()) {
		LOG(WARNING) << "You changed the rendering of unknown blocks from "
				<< util::str(render_unknown_blocks.get()) << " to "
				<< util::str(map.renderUnknownBlocks()) << ".";
	} else if (render_leaves_transparent.get() != map.renderLeavesTransparent()) {
		LOG(WARNING) << "You changed the rendering of transparent leaves from "
				<< util::str(render_leaves_transparent.get()) << " to "
				<< util::str(map.renderLeavesTransparent()) << ".";
	} else if (render_biomes.get() != map.renderBiomes()) {
		LOG(WARNING) << "You changed the rendering of biomes from "
				<< util::str(render_biomes.get()) << " to "
				<< util::str(map.renderBiomes()) << ".";
	} else {
		changed = false;
	}

	if (changed) {
		(force_required ? LOG(ERROR) : LOG(WARNING))
				<< "Force-render the whole map in order for the new "
				<< "configuration to come into effect.";
	}

	return !(changed && force_required);
}

MapSettings MapSettings::byMapConfig(const config::MapSection& map) {
	MapSettings settings;

	settings.texture_size.set(map.getTextureSize());
	settings.image_format.set(map.getImageFormatSuffix());
	settings.lighting_intensity.set(map.getLightingIntensity());
	settings.cave_high_contrast.set(map.hasCaveHighContrast());
	settings.render_unknown_blocks.set(map.renderUnknownBlocks());
	settings.render_leaves_transparent.set(map.renderLeavesTransparent());
	settings.render_biomes.set(map.renderBiomes());

	auto rotations = map.getRotations();
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		settings.rotations[*it] = true;

	return settings;
}

MapcrafterConfigHelper::MapcrafterConfigHelper() {
}

MapcrafterConfigHelper::MapcrafterConfigHelper(const MapcrafterConfig& config)
	: config(config) {
	auto maps = config.getMaps();
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		std::string map_name = map_it->getShortName();

		// set tile size to something != 0 for now,
		// because Leaflet is sad if the tile size is 0
		map_tile_size[map_name] = 1;
		map_max_zoom[map_name] = 0;
		for (int i = 0; i < 4; i++) {
			map_last_rendered[map_name][i] = 0;
			map_render_behavior[map_name][i] = RENDER_AUTO;
		}

		auto rotations = map_it->getRotations();
		for (auto rotation_it = rotations.begin(); rotation_it != rotations.end(); ++rotation_it) {
			world_rotations[map_it->getTileSetKey()].insert(*rotation_it);
			world_max_max_zoom[map_it->getTileSetKey()] = 0;
		}
	}
}

MapcrafterConfigHelper::~MapcrafterConfigHelper() {
}

bool parseTilePosJSON(const picojson::value& value, renderer::TilePos& tile) {
	if (!value.is<picojson::array>())
		return false;
	picojson::array array = value.get<picojson::array>();
	if (!array.size() == 2)
		return false;
	if (!array[0].is<double>() || !array[1].is<double>())
		return false;
	tile = renderer::TilePos(array[0].get<double>(), array[1].get<double>());
	return true;
}

void MapcrafterConfigHelper::readMapSettings() {
	// try to read config.js file or migrate old map.settings files
	if (fs::is_regular_file(config.getOutputPath("config.js"))) {
		// TODO try to read config.js file
		// TODO better validation? this is only a first try...
		// TODO check if map/world configuration has changed?

		std::ifstream in(config.getOutputPath("config.js").string());
		if (!in) {
			LOG(WARNING) << "Unable to open config.js file!";
		}
		std::stringstream ss;
		ss << in.rdbuf();
		std::string config_data = ss.str();
		if (!util::startswith(config_data, "var CONFIG = ")) {
			LOG(WARNING) << "Invalid config.js file! "
					<< "'var CONFIG = ' is expected at beginning of file!";
			return;
		}

		config_data = config_data.substr(std::string("var CONFIG = ").size());
		picojson::value value;
		std::string json_error;
		picojson::parse(value, config_data.begin(), config_data.end(), &json_error);
		if (!json_error.empty()) {
			LOG(WARNING) << "Unable to parse config.js file: " << json_error;
			return;
		}

		if (!value.is<picojson::object>() || !value.contains("maps")
				|| !value.get("maps").is<picojson::object>()) {
			LOG(WARNING) << "Invalid config json object in config.js file!";
			return;
		}

		picojson::object maps_json = value.get<picojson::object>()["maps"].get<picojson::object>();

		auto maps = config.getMaps();
		for (auto map_it = maps.begin(); map_it < maps.end(); ++map_it) {
			std::string map_name = map_it->getShortName();
			if (!maps_json.count(map_name)
					|| !maps_json[map_name].is<picojson::object>())
				continue;
			picojson::object map_json = maps_json[map_name].get<picojson::object>();

			if (map_json.count("tileOffsets") && map_json["tileOffsets"].is<picojson::array>()) {
				picojson::array array = map_json["tileOffsets"].get<picojson::array>();
				if (array.size() == 4) {
					TileSetKey key = map_it->getTileSetKey();
					for (int rotation = 0; rotation < 4; rotation++)
						parseTilePosJSON(array[rotation], world_tile_offset[key][rotation]);
					LOG(DEBUG) << map_name << " tile_offsets="
							<< world_tile_offset[key][0] << ","
							<< world_tile_offset[key][1] << ","
							<< world_tile_offset[key][2] << ","
							<< world_tile_offset[key][3];
				}
			}

			if (map_json.count("tileSize") && map_json["tileSize"].is<double>()) {
				map_tile_size[map_name] = map_json["tileSize"].get<double>();
				LOG(DEBUG) << map_name << " tile_size=" << map_tile_size[map_name];
			}

			if (map_json.count("maxZoom") && map_json["maxZoom"].is<double>()) {
				map_max_zoom[map_name] = map_json["maxZoom"].get<double>();
				LOG(DEBUG) << map_name << " max_zoom=" << map_max_zoom[map_name];
			}

			if (map_json.count("lastRendered") && map_json["lastRendered"].is<picojson::array>()) {
				picojson::array last_rendered_json = map_json["lastRendered"].get<picojson::array>();
				if (last_rendered_json.size() == 4
						&& last_rendered_json[0].is<double>()
						&& last_rendered_json[1].is<double>()
						&& last_rendered_json[2].is<double>()
						&& last_rendered_json[3].is<double>()) {
					for (size_t i = 0; i < 4; i++)
						map_last_rendered[map_name][i] = last_rendered_json[i].get<double>();
					LOG(DEBUG) << map_name << " last_rendered=["
							<< map_last_rendered[map_name][0] << ", "
							<< map_last_rendered[map_name][1] << ", "
							<< map_last_rendered[map_name][2] << ", "
							<< map_last_rendered[map_name][3] << "]";
				}
			}
		}
		return;
	} else {
		bool map_settings_found = false;
		auto maps = config.getMaps();
		for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
			std::string map_name = map_it->getShortName();
			fs::path settings_file = config.getOutputDir() / map_name / "map.settings";
			if (fs::is_regular_file(settings_file)) {
				map_settings_found = true;
				LOG(INFO) << "Found old map.settings file for map '" << map_name
						<< "', migrating to new config.js format.";

				// read map settings that are relevant for viewing the map
				// while we render it again
				MapSettings settings;
				if (settings.read(settings_file.string())) {
					world_tile_offset[map_it->getTileSetKey()] = settings.tile_offsets;
					map_tile_size[map_name] = settings.tile_size;
					map_max_zoom[map_name] = settings.max_zoom;
					map_last_rendered[map_name] = settings.last_render;
				}
				// move old map.settings file
				fs::rename(settings_file, settings_file.string() + ".old");
			}
		}

		// write config.js for first time with data from old map.settings files
		if (map_settings_found)
			writeMapSettings();
	}
}

void MapcrafterConfigHelper::writeMapSettings() const {
	std::ofstream out(config.getOutputPath("config.js").string());
	if (!out) {
		LOG(ERROR) << "Unable to write config.js file!";
		return;
	}
	// TODO write world/map config to check if it was changed next time we read config.js?
	out << "var CONFIG = " << util::trim(getConfigJSON().serialize(true)) << ";" << std::endl;
	out.close();
}

std::set<int> MapcrafterConfigHelper::getUsedRotations(
		const TileSetKey& tile_set) const {
	if (!world_rotations.count(tile_set))
		return std::set<int>();
	return world_rotations.at(tile_set);
}

int MapcrafterConfigHelper::getTileSetMaxZoom(const TileSetKey& tile_set) const {
	if (!world_max_max_zoom.count(tile_set))
		return 0;
	return world_max_max_zoom.at(tile_set);
}

void MapcrafterConfigHelper::setTileSetMaxZoom(const TileSetKey& tile_set,
		int max_zoom) {
	world_max_max_zoom[tile_set] = max_zoom;
}

renderer::TilePos MapcrafterConfigHelper::getWorldTileOffset(
		const TileSetKey& tile_set, int rotation) const {
	if (!world_tile_offset.count(tile_set))
		return renderer::TilePos(0, 0);
	return world_tile_offset.at(tile_set)[rotation];
}

void MapcrafterConfigHelper::setWorldTileOffset(const TileSetKey& tile_set,
		int rotation, const renderer::TilePos& tile_offset) {
	world_tile_offset[tile_set][rotation] = tile_offset;
}

int MapcrafterConfigHelper::getMapTileSize(const std::string& map) const {
	return map_tile_size.at(map);
}

void MapcrafterConfigHelper::setMapTileSize(const std::string& map, int tile_size) {
	map_tile_size[map] = tile_size;
}

int MapcrafterConfigHelper::getMapMaxZoom(const std::string& map) const {
	if (!map_max_zoom.count(map))
		return 0;
	return map_max_zoom.at(map);
}

void MapcrafterConfigHelper::setMapMaxZoom(const std::string& map, int zoomlevel) {
	map_max_zoom[map] = zoomlevel;
}

int MapcrafterConfigHelper::getRenderBehavior(const std::string& map, int rotation) const {
	return map_render_behavior.at(map).at(rotation);
}

int MapcrafterConfigHelper::getMapLastRendered(const std::string& map,
		int rotation) const {
	if (!map_last_rendered.count(map))
		return 0;
	return map_last_rendered.at(map).at(rotation);
}

void MapcrafterConfigHelper::setMapLastRendered(const std::string& map,
		int rotation, int last_rendered) {
	map_last_rendered[map][rotation] = last_rendered;
}

void MapcrafterConfigHelper::setRenderBehavior(const std::string& map, int rotation, int behavior) {
	if (rotation == -1)
		for (size_t i = 0; i < 4; i++)
			map_render_behavior[map][i] = behavior;
	else
		map_render_behavior[map][rotation] = behavior;
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

void MapcrafterConfigHelper::parseRenderBehaviors(bool skip_all,
		std::vector<std::string> render_skip,
		std::vector<std::string> render_auto,
		std::vector<std::string> render_force) {
	if (!skip_all)
		setRenderBehaviors(render_skip, RENDER_SKIP);
	else
		for (size_t i = 0; i < config.getMaps().size(); i++)
			for (int j = 0; j < 4; j++)
				map_render_behavior[config.getMaps()[i].getShortName()][j] = RENDER_SKIP;
	setRenderBehaviors(render_auto, RENDER_AUTO);
	setRenderBehaviors(render_force, RENDER_FORCE);
}

picojson::value MapcrafterConfigHelper::getConfigJSON() const {
	picojson::object config_json, maps_json;
	picojson::array maps_order_json;

	auto maps = config.getMaps();
	for (auto it = maps.begin(); it != maps.end(); ++it) {
		auto world = config.getWorld(it->getWorld());
		// TODO also this a bit cleaner maybe?

		maps_order_json.push_back(picojson::value(it->getShortName()));

		picojson::object map_json;
		map_json["name"] = picojson::value(it->getLongName());
		map_json["world"] = picojson::value(it->getWorld());
		map_json["worldName"] = picojson::value(world.getWorldName());
		map_json["renderView"] = picojson::value(it->getRenderView());
		map_json["textureSize"] = picojson::value((double) it->getTextureSize());
		map_json["imageFormat"] = picojson::value(it->getImageFormatSuffix());
		if (!world.getDefaultView().empty())
			map_json["defaultView"] = picojson::value(world.getDefaultView());
		if (world.getDefaultZoom() != 0)
			map_json["defaultZoom"] = picojson::value((double) world.getDefaultZoom());
		if (world.getDefaultRotation() != -1)
			map_json["defaultRotation"] = picojson::value((double) world.getDefaultRotation());

		picojson::array rotations_json;
		auto rotations = it->getRotations();
		for (auto it2 = rotations.begin(); it2 != rotations.end(); ++it2)
			rotations_json.push_back(picojson::value((double) *it2));
		map_json["rotations"] = picojson::value(rotations_json);

		map_json["tileSize"] = picojson::value((double) getMapTileSize(it->getShortName()));
		map_json["maxZoom"] = picojson::value((double) getMapMaxZoom(it->getShortName()));

		picojson::array last_rendered_json;
		for (int rotation = 0; rotation < 4; rotation++)
			last_rendered_json.push_back(picojson::value((double) getMapLastRendered(it->getShortName(), rotation)));
		map_json["lastRendered"] = picojson::value(last_rendered_json);

		picojson::array tile_offsets_json;
		for (int rotation = 0; rotation < 4; rotation++) {
			renderer::TilePos offset = getWorldTileOffset(it->getTileSetKey(), rotation);
			picojson::array offset_json;
			offset_json.push_back(picojson::value((double) offset.getX()));
			offset_json.push_back(picojson::value((double) offset.getY()));
			tile_offsets_json.push_back(picojson::value(offset_json));
		}
		map_json["tileOffsets"] = picojson::value(tile_offsets_json);

		maps_json[it->getShortName()] = picojson::value(map_json);
	}

	config_json["maps_order"] = picojson::value(maps_order_json);
	config_json["maps"] = picojson::value(maps_json);

	return picojson::value(config_json);
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
			LOG(WARNING) << "Unknown map '" << map << "'.";
			continue;
		}

		if (!rotation.empty()) {
			if (r == -1) {
				LOG(WARNING) << "Unknown rotation '" << rotation << "'.";
				continue;
			}
			if (!config.getMap(map).getRotations().count(r)) {
				LOG(WARNING) << "Map '" << map << "' does not have rotation '" << rotation << "'.";
				continue;
			}
		}

		if (r != -1)
			map_render_behavior[map][r] = behavior;
		else
			std::fill(&map_render_behavior[map][0], &map_render_behavior[map][4], behavior);
	}
}

} /* namespace config */
} /* namespace mapcrafter */
