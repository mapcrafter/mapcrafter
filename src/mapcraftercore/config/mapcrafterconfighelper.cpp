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

TileSetKey::TileSetKey(const std::string& map_name, const std::string render_view,
		int tile_width, int rotation)
	: map_name(map_name), render_view(render_view), tile_width(tile_width),
	  rotation(rotation) {
}

bool TileSetKey::operator<(const TileSetKey& other) const {
	if (map_name != other.map_name)
		return map_name < other.map_name;
	if (render_view != other.render_view)
		return render_view < other.render_view;
	if (tile_width != other.tile_width)
		return tile_width < other.tile_width;
	if (rotation != other.rotation)
		return rotation < other.rotation;
	return false;
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
			WorldRenderView key(map_it->getWorld(), map_it->getRenderView());
			world_rotations[key].insert(*rotation_it);
			world_max_max_zoom[key] = 0;
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
	if (fs::is_regular_file(config.getOutputPath("config.js"))) {
		// TODO try to read config.js file
		// TODO better validation? this is only a first try...

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
					WorldRenderView key(map_it->getWorld(), map_it->getRenderView());
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
	}

	// TODO try to migrate old map.settings files
	bool map_settings_found = false;
	auto maps = config.getMaps();
	for (auto map_it = maps.begin(); map_it != maps.end(); ++map_it) {
		if (fs::is_regular_file(config.getOutputPath(map_it->getShortName()
				+ "/map.settings"))) {
			if (!map_settings_found) {
				LOG(INFO) << "Found old map.settings file(s), trying to migrate to config.js";
				map_settings_found = true;
			}

			// TODO read map.settings file
		}
	}
}

void MapcrafterConfigHelper::writeMapSettings() const {
	std::ofstream out(config.getOutputPath("config.js").string());
	if (!out) {
		LOG(ERROR) << "Unable to write config.js file!";
		return;
	}
	out << "var CONFIG = " << util::trim(getConfigJSON().serialize(true)) << ";";
	out.close();
}

std::set<int> MapcrafterConfigHelper::getUsedRotations(
		const std::string& world, const std::string& render_view) const {
	WorldRenderView key(world, render_view);
	if (!world_rotations.count(key))
		return std::set<int>();
	return world_rotations.at(key);
}

void MapcrafterConfigHelper::addUsedRotations(const std::string& world,
		const std::string& render_view, const std::set<int>& rotations) {
	for (auto rotation_it = rotations.begin(); rotation_it != rotations.end(); ++rotation_it)
		world_rotations[WorldRenderView(world, render_view)].insert(*rotation_it);
}

int MapcrafterConfigHelper::getTileSetMaxZoom(const std::string& world,
		const std::string& render_view) const {
	WorldRenderView key(world, render_view);
	if (!world_max_max_zoom.count(key))
		return 0;
	return world_max_max_zoom.at(key);
}

void MapcrafterConfigHelper::setTileSetMaxZoom(const std::string& world,
		const std::string& render_view, int max_zoom) {
	world_max_max_zoom[WorldRenderView(world, render_view)] = max_zoom;
}

renderer::TilePos MapcrafterConfigHelper::getWorldTileOffset(
		const std::string& world, const std::string& render_view, int rotation) const {
	WorldRenderView key(world, render_view);
	if (!world_tile_offset.count(key))
		return renderer::TilePos(0, 0);
	return world_tile_offset.at(key)[rotation];
}

void MapcrafterConfigHelper::setWorldTileOffset(const std::string& world,
		const std::string& render_view, int rotation,
		const renderer::TilePos& tile_offset) {
	world_tile_offset[WorldRenderView(world, render_view)][rotation] = tile_offset;
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
			renderer::TilePos offset = getWorldTileOffset(it->getWorld(), it->getRenderView(), rotation);
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
