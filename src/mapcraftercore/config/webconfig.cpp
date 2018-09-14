/*
 * Copyright 2012-2016 Moritz Hilscher
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
		// EDIT: apparently it's also sad now if the tile size is 1 or something small
		// seems like it's stuck in a loop trying to loading (many? tile size 1?) tiles
		// -> greater values seem to work
		map_tile_size[map_name] = std::make_tuple<>(420, 420);
		map_max_zoom[map_name] = 0;
		for (int i = 0; i < 4; i++)
			map_last_rendered[map_name][i] = 0;

		auto tile_sets = map_it->getTileSets();
		for (auto tile_set_it = tile_sets.begin(); tile_set_it != tile_sets.end(); ++tile_set_it)
			tile_sets_max_zoom[*tile_set_it] = 0;
	}
}

WebConfig::~WebConfig() {
}

bool WebConfig::readConfigJS() {
	// try to read config.js file or migrate old map.settings files
	if (fs::is_regular_file(config.getOutputPath("config.js"))) {
		// TODO check if map/world configuration has changed?
		static std::string try_again = "Please fix the problem in the config.js file "
			"or delete the corrupt file (Warning: You will have to render your maps again).";

		std::ifstream in(config.getOutputPath("config.js").string());
		if (!in) {
			LOG(FATAL) << "Unable to open config.js file!";
			LOG(FATAL) << try_again;
			return false;
		}
		std::stringstream ss;
		ss << in.rdbuf();
		std::string config_data = ss.str();
		if (!util::startswith(config_data, "var CONFIG = ")) {
			LOG(FATAL) << "Invalid config.js file! "
					<< "'var CONFIG = ' is expected at beginning of file!";
			LOG(FATAL) << try_again;
			return false;
		}

		config_data = config_data.substr(std::string("var CONFIG = ").size());
		picojson::value value;
		std::string json_error;
		picojson::parse(value, config_data.begin(), config_data.end(), &json_error);
		if (!json_error.empty()) {
			LOG(FATAL) << "Unable to parse json in config.js file: " << json_error;
			LOG(FATAL) << try_again;
			return false;
		}

		if (!value.is<picojson::object>()) {
			LOG(FATAL) << "Invalid config json object in config.js file!";
			LOG(FATAL) << try_again;
			return false;
		}

		try {
			parseConfigJSON(value.get<picojson::object>());
		} catch (util::JSONError& exception) {
			LOG(FATAL) << "Unable to parse config json: " << exception.what();
			LOG(FATAL) << try_again;
			return false;
		}
	} else {
		bool map_settings_found = false;
		auto maps = config.getMaps();
		for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
			std::string map_name = map_it->getShortName();
			auto tile_sets = map_it->getTileSets();

			fs::path settings_file = config.getOutputDir() / map_name / "map.settings";
			if (!fs::is_regular_file(settings_file))
				continue;

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
				int s = root.get<int>("texture_size", 12) * 32;
				map_tile_size[map_name] = std::make_tuple<>(s, s);
				map_max_zoom[map_name] = root.get<int>("max_zoom");

				std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
				for (auto tile_set_it = tile_sets.begin();
						tile_set_it != tile_sets.end(); ++tile_set_it) {
					int rotation = tile_set_it->rotation;
					auto section = config.getSection("rotation", rotation_names[rotation]);
					int offset_x = section.get<int>("tile_offset_x", 0);
					int offset_y = section.get<int>("tile_offset_y", 0);
					renderer::TilePos offset(offset_x, offset_y);
					tile_set_tile_offset[*tile_set_it] = offset;
					map_last_rendered[map_name][rotation] = section.get<int>("last_render");
				}
			} catch (config::INIConfigError& exception) {
				LOG(WARNING) << "Unable to read map.settings file: " << exception.what();
				continue;
			}

			// move old map.settings file
			fs::rename(settings_file, settings_file.string() + ".old");
		}

		// write config.js for first time with data from old map.settings files
		if (map_settings_found)
			writeConfigJS();
	}
	
	// if no config.js file was found that's just fine
	// probably rendering for the first time
	return true;
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

int WebConfig::getTileSetsMaxZoom(const TileSetGroupID& tile_set) const {
	if (!tile_sets_max_zoom.count(tile_set))
		return 0;
	return tile_sets_max_zoom.at(tile_set);
}

void WebConfig::setTileSetsMaxZoom(const TileSetGroupID& tile_set, int max_zoom) {
	tile_sets_max_zoom[tile_set] = max_zoom;
}

renderer::TilePos WebConfig::getTileSetTileOffset(const TileSetID& tile_set) const {
	if (!tile_set_tile_offset.count(tile_set))
		return renderer::TilePos(0, 0);
	return tile_set_tile_offset.at(tile_set);
}

void WebConfig::setTileSetTileOffset(const TileSetID& tile_set,
		const renderer::TilePos& tile_offset) {
	tile_set_tile_offset[tile_set] = tile_offset;
}

std::tuple<int, int> WebConfig::getMapTileSize(const std::string& map) const {
	return map_tile_size.at(map);
}

void WebConfig::setMapTileSize(const std::string& map, std::tuple<int, int> tile_size) {
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
	// we'll just call all the tile set group things tile sets here
	// it would be too long otherwise
	picojson::object config_json, maps_json, tile_sets_json;
	picojson::array maps_order_json;

	// get used tile set groups
	std::set<TileSetGroupID> tile_sets;
	auto maps = config.getMaps();
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it)
		tile_sets.insert(map_it->getTileSetGroup());

	// create the tile set group objects
	for (auto it = tile_sets.begin(); it != tile_sets.end(); ++it) {
		picojson::object tile_set_json;
		tile_set_json["maxZoom"] = picojson::value((double) getTileSetsMaxZoom(*it));
		
		picojson::array tile_offsets_json;
		for (int rotation = 0; rotation < 4; rotation++) {
			renderer::TilePos offset = getTileSetTileOffset(TileSetID(*it, rotation));
			picojson::array offset_json;
			offset_json.push_back(picojson::value((double) offset.getX()));
			offset_json.push_back(picojson::value((double) offset.getY()));
			tile_offsets_json.push_back(picojson::value(offset_json));
		}
		tile_set_json["tileOffsets"] = picojson::value(tile_offsets_json);
		tile_set_json["tileWidth"] = picojson::value((double) it->tile_width);

		tile_sets_json[it->toString()] = picojson::value(tile_set_json);
	}

	// create the map objects
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		auto world = config.getWorld(map_it->getWorld());
		auto tile_sets = map_it->getTileSets();
		maps_order_json.push_back(picojson::value(map_it->getShortName()));

		picojson::object map_json;
		map_json["name"] = picojson::value(map_it->getLongName());
		map_json["world"] = picojson::value(map_it->getWorld());
		map_json["worldName"] = picojson::value(world.getWorldName());
		map_json["worldSeaLevel"] = picojson::value((double) world.getSeaLevel());
		map_json["renderView"] = picojson::value(util::str(map_it->getRenderView()));
		map_json["textureSize"] = picojson::value((double) map_it->getTextureSize());
		map_json["imageFormat"] = picojson::value(map_it->getImageFormatSuffix());
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
		auto rotations = map_it->getRotations();
		for (auto it = rotations.begin(); it != rotations.end(); ++it)
			rotations_json.push_back(picojson::value((double) *it));
		map_json["rotations"] = picojson::value(rotations_json);

		std::tuple<int, int> tile_size = getMapTileSize(map_it->getShortName());
		picojson::array tile_size_json;
		tile_size_json.push_back(picojson::value((double) std::get<0>(tile_size)));
		tile_size_json.push_back(picojson::value((double) std::get<1>(tile_size)));
		map_json["tileSize"] = picojson::value(tile_size_json);

		map_json["maxZoom"] = picojson::value((double) getMapMaxZoom(map_it->getShortName()));

		picojson::array last_rendered_json;
		for (int rotation = 0; rotation < 4; rotation++) {
			int last_rendered = getMapLastRendered(map_it->getShortName(), rotation);
			last_rendered_json.push_back(picojson::value((double) last_rendered));
		}
		map_json["lastRendered"] = picojson::value(last_rendered_json);

		map_json["tileSetGroup"] = picojson::value(map_it->getTileSetGroup().toString());
		
		maps_json[map_it->getShortName()] = picojson::value(map_json);
	}

	config_json["tileSetGroups"] = picojson::value(tile_sets_json);
	config_json["mapsOrder"] = picojson::value(maps_order_json);
	config_json["maps"] = picojson::value(maps_json);

	return picojson::value(config_json);
}

renderer::TilePos parseTilePosJSON(const picojson::value& value) {
	static std::string error = "Invalid 'tileOffsets' array!";
	if (!value.is<picojson::array>())
		throw util::JSONError(error);
	picojson::array array = value.get<picojson::array>();
	if (array.size() != 2 || !array[0].is<double>() || !array[1].is<double>())
		throw util::JSONError(error);
	return renderer::TilePos(array[0].get<double>(), array[1].get<double>());
}

void WebConfig::parseConfigJSON(const picojson::object& object) {
	// we'll just call all the tile set group things tile sets here too
	// it would be too long otherwise
	picojson::object tile_sets_json = util::json_get<picojson::object>(object, "tileSetGroups");
	picojson::object maps_json = util::json_get<picojson::object>(object, "maps");

	// get used tile set groups
	std::set<TileSetGroupID> tile_sets;
	auto maps = config.getMaps();
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it)
		tile_sets.insert(map_it->getTileSetGroup());

	// parse the tile set group objects
	for (auto it = tile_sets.begin(); it != tile_sets.end(); ++it) {
		TileSetGroupID tile_set = *it;
		// it's okay if we can't find a specific tile set group object
		// -> probably rendering it for the first time
		if (!tile_sets_json.count(tile_set.toString()))
			continue;
		picojson::object tile_set_json = util::json_get<picojson::object>(tile_sets_json, tile_set.toString());
		tile_sets_max_zoom[tile_set] = util::json_get<double>(tile_set_json, "maxZoom");
		LOG(DEBUG) << "ts " << tile_set.toString() << " max_zoom=" << tile_sets_max_zoom[tile_set];
		
		picojson::array array = util::json_get<picojson::array>(tile_set_json, "tileOffsets");
		if (array.size() != 4)
			throw util::JSONError("Invalid 'tileOffsets' array!");
		for (int r = 0; r < 4; r++)
			tile_set_tile_offset[TileSetID(tile_set, r)] = parseTilePosJSON(array[r]);
		LOG(DEBUG) << "ts " << tile_set.toString() << " tile_offsets="
			<< tile_set_tile_offset[TileSetID(tile_set, 0)] << ","
			<< tile_set_tile_offset[TileSetID(tile_set, 1)] << ","
			<< tile_set_tile_offset[TileSetID(tile_set, 2)] << ","
			<< tile_set_tile_offset[TileSetID(tile_set, 3)];
	}

	// parse the map objects
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		std::string map_name = map_it->getShortName();
		// it's okay if we can't find a specific map object
		// -> probably rendering it for the first time
		if (!maps_json.count(map_name))
			continue;
		picojson::object map_json = util::json_get<picojson::object>(maps_json, map_name);

		picojson::value tile_size = util::json_get<picojson::value>(map_json, "tileSize");
		// tileSize is array [w, h] in newer versions
		if (tile_size.is<picojson::array>()) {
			picojson::array& array = tile_size.get<picojson::array>();
			if (array.size() != 2)  {
				throw util::JSONError("Invalid 'tileSize' array!");
			}
			int w = array[0].get<double>();
			int h = array[1].get<double>();
			map_tile_size[map_name] = std::make_tuple<>(w, h);
		} else {
			// but also just one number for both sides is possible
			int s = tile_size.get<double>();
			map_tile_size[map_name] = std::make_tuple<>(s, s);
		}
		LOG(DEBUG) << "map " << map_name << " tile_size="
			<< std::get<0>(map_tile_size[map_name]) << "x" << std::get<1>(map_tile_size[map_name]);

		map_max_zoom[map_name] = util::json_get<double>(map_json, "maxZoom");
		LOG(DEBUG) << "map " << map_name << " max_zoom=" << map_max_zoom[map_name];

		auto last_rendered_json = util::json_get<picojson::array>(map_json, "lastRendered");
		if (last_rendered_json.size() != 4
				|| !last_rendered_json[0].is<double>()
				|| !last_rendered_json[1].is<double>()
				|| !last_rendered_json[2].is<double>()
				|| !last_rendered_json[3].is<double>())
			throw util::JSONError("Invalid 'lastRendered' array!");
		for (int r = 0; r < 4; r++)
			map_last_rendered[map_name][r] = last_rendered_json[r].get<double>();
		LOG(DEBUG) << "map " << map_name << " last_rendered=["
			<< map_last_rendered[map_name][0] << ", "
			<< map_last_rendered[map_name][1] << ", "
			<< map_last_rendered[map_name][2] << ", "
			<< map_last_rendered[map_name][3] << "]";
	}
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
