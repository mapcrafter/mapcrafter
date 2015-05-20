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

#include "webconfig.h"

#include "iniconfig.h"
#include "../util.h"

namespace mapcrafter {
namespace config {

WebConfig::WebConfig(const MapcrafterConfig& config)
	: config(config) {
	auto maps = config.getMaps();
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		std::string map_name = map_it->getShortName();

		// set tile size to something != 0 for now,
		// because Leaflet is sad if the tile size is 0
		map_tile_size[map_name] = 1;
		map_max_zoom[map_name] = 0;
		for (int i = 0; i < 4; i++)
			map_last_rendered[map_name][i] = 0;

		auto tile_sets = map_it->getTileSets();
		for (auto tile_set_it = tile_sets.begin(); tile_set_it != tile_sets.end(); ++tile_set_it)
			tile_set_max_zoom[*tile_set_it] = 0;
	}
}

WebConfig::~WebConfig() {
}

bool parseTilePosJSON(const picojson::value& value, renderer::TilePos& tile) {
	if (!value.is<picojson::array>())
		return false;
	picojson::array array = value.get<picojson::array>();
	if (array.size() != 2)
		return false;
	if (!array[0].is<double>() || !array[1].is<double>())
		return false;
	tile = renderer::TilePos(array[0].get<double>(), array[1].get<double>());
	return true;
}

void WebConfig::readConfigJS() {
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
			auto map_tile_sets = map_it->getTileSets();

			if (map_json.count("tileOffsets") && map_json["tileOffsets"].is<picojson::array>()) {
				picojson::array array = map_json["tileOffsets"].get<picojson::array>();
				if (array.size() == 4) {
					for (auto tile_set_it = map_tile_sets.begin();
							tile_set_it != map_tile_sets.end(); ++tile_set_it)
						parseTilePosJSON(array[tile_set_it->rotation], world_tile_offset[*tile_set_it]);
					/*
					LOG(DEBUG) << map_name << " tile_offsets="
							<< world_tile_offset[key][0] << ","
							<< world_tile_offset[key][1] << ","
							<< world_tile_offset[key][2] << ","
							<< world_tile_offset[key][3];
					*/
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
			auto map_tile_sets = map_it->getTileSets();

			fs::path settings_file = config.getOutputDir() / map_name / "map.settings";
			if (fs::is_regular_file(settings_file)) {
				map_settings_found = true;
				LOG(INFO) << "Found old map.settings file for map '" << map_name
						<< "', migrating to new config.js format.";

				// read map settings that are relevant for viewing the map
				// while we render it again
				config::INIConfig config;
				try {
					config.loadFile(settings_file.string());
					config::INIConfigSection& root = config.getRootSection();

					// we can calculate the tile size since there was only one render view
					map_tile_size[map_name] = root.get<int>("texture_size", 12) * 32;
					map_max_zoom[map_name] = root.get<int>("max_zoom");

					std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
					for (auto tile_set_it = map_tile_sets.begin();
							tile_set_it != map_tile_sets.end(); ++tile_set_it) {
						int rotation = tile_set_it->rotation;
						if (!map_it->getRotations().count(rotation))
							continue;
						auto section = config.getSection("rotation", rotation_names[rotation]);
						int offset_x = section.get<int>("tile_offset_x", 0);
						int offset_y = section.get<int>("tile_offset_y", 0);
						renderer::TilePos offset(offset_x, offset_y);
						world_tile_offset[*tile_set_it] = offset;
						map_last_rendered[map_name][rotation] = section.get<int>("last_render");
					}
				} catch (config::INIConfigError& exception) {
					LOG(WARNING) << "Unable to read map.settings file: " << exception.what();
					continue;
				}

				// move old map.settings file
				fs::rename(settings_file, settings_file.string() + ".old");
			}
		}

		// write config.js for first time with data from old map.settings files
		if (map_settings_found)
			writeConfigJS();
	}
}

void WebConfig::writeConfigJS() const {
	std::ofstream out(config.getOutputPath("config.js").string());
	if (!out) {
		LOG(ERROR) << "Unable to write config.js file!";
		return;
	}
	// TODO write world/map config to check if it was changed next time we read config.js?
	out << "var CONFIG = " << util::trim(getConfigJSON().serialize(true)) << ";" << std::endl;
	out.close();
}

int WebConfig::getTileSetMaxZoom(const TileSetGroupID& tile_set) const {
	if (!tile_set_max_zoom.count(tile_set))
		return 0;
	return tile_set_max_zoom.at(tile_set);
}

void WebConfig::setTileSetMaxZoom(const TileSetGroupID& tile_set, int max_zoom) {
	tile_set_max_zoom[tile_set] = max_zoom;
}

renderer::TilePos WebConfig::getTileSetTileOffset(const TileSetID& tile_set) const {
	if (!world_tile_offset.count(tile_set))
		return renderer::TilePos(0, 0);
	return world_tile_offset.at(tile_set);
}

void WebConfig::setTileSetTileOffset(const TileSetID& tile_set,
		const renderer::TilePos& tile_offset) {
	world_tile_offset[tile_set] = tile_offset;
}

int WebConfig::getMapTileSize(const std::string& map) const {
	return map_tile_size.at(map);
}

void WebConfig::setMapTileSize(const std::string& map, int tile_size) {
	map_tile_size[map] = tile_size;
}

int WebConfig::getMapMaxZoom(const std::string& map) const {
	if (!map_max_zoom.count(map))
		return 0;
	return map_max_zoom.at(map);
}

void WebConfig::setMapMaxZoom(const std::string& map, int zoomlevel) {
	map_max_zoom[map] = zoomlevel;
}

int WebConfig::getMapLastRendered(const std::string& map,
		int rotation) const {
	if (!map_last_rendered.count(map))
		return 0;
	return map_last_rendered.at(map).at(rotation);
}

void WebConfig::setMapLastRendered(const std::string& map,
		int rotation, int last_rendered) {
	map_last_rendered[map][rotation] = last_rendered;
}

picojson::value WebConfig::getConfigJSON() const {
	picojson::object config_json, maps_json;
	picojson::array maps_order_json;

	auto maps = config.getMaps();
	for (auto it = maps.begin(); it != maps.end(); ++it) {
		auto world = config.getWorld(it->getWorld());
		auto tile_sets = it->getTileSets();
		// TODO also this a bit cleaner maybe?

		maps_order_json.push_back(picojson::value(it->getShortName()));

		picojson::object map_json;
		map_json["name"] = picojson::value(it->getLongName());
		map_json["world"] = picojson::value(it->getWorld());
		map_json["worldName"] = picojson::value(world.getWorldName());
		map_json["renderView"] = picojson::value(util::str(it->getRenderView()));
		map_json["textureSize"] = picojson::value((double) it->getTextureSize());
		map_json["imageFormat"] = picojson::value(it->getImageFormatSuffix());
		if (world.getDefaultView() != mc::BlockPos(0, 0, 0)) {
			mc::BlockPos default_view = world.getDefaultView();
			picojson::array default_view_json;
			default_view_json.push_back(picojson::value((double) default_view.x));
			default_view_json.push_back(picojson::value((double) default_view.z));
			default_view_json.push_back(picojson::value((double) default_view.y));
			map_json["defaultView"] = picojson::value(default_view_json);
		}
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

		// TODO adapt the format of this config option to the new tile set key stuff?
		std::array<renderer::TilePos, 4> tile_offsets;
		for (auto tile_set_it = tile_sets.begin(); tile_set_it != tile_sets.end(); ++tile_set_it) {
			tile_offsets[tile_set_it->rotation] = getTileSetTileOffset(*tile_set_it);
		}

		picojson::array tile_offsets_json;
		for (int rotation = 0; rotation < 4; rotation++) {
			renderer::TilePos offset = tile_offsets[rotation];
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

/*
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
*/

} /* namespace config */
} /* namespace mapcrafter */
