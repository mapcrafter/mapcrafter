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
		map_tile_sizes[map_it->getShortName()] = 0;
		map_zoomlevels[map_it->getShortName()] = 0;
		for (int i = 0; i < 4; i++)
			render_behaviors[map_it->getShortName()][i] = RENDER_AUTO;
	}

	/*
	auto worlds = config.getWorlds();
	for (auto world_it = worlds.begin(); world_it != worlds.end(); ++world_it) {
		world_rotations[world_it->first] = std::set<int>();
		world_zoomlevels[world_it->first] = 0;
		world_tile_offsets[world_it->first] = std::array<renderer::TilePos, 4>();
	}
	*/
}

MapcrafterConfigHelper::~MapcrafterConfigHelper() {
}

void MapcrafterConfigHelper::readMapSettings() {
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

int MapcrafterConfigHelper::getWorldZoomlevel(const std::string& world,
		const std::string& render_view) const {
	WorldRenderView key(world, render_view);
	if (!world_zoomlevels.count(key))
		return 0;
	return world_zoomlevels.at(key);
}

void MapcrafterConfigHelper::setWorldZoomlevel(const std::string& world,
		const std::string& render_view, int zoomlevel) {
	world_zoomlevels[WorldRenderView(world, render_view)] = zoomlevel;
}

renderer::TilePos MapcrafterConfigHelper::getWorldTileOffset(
		const std::string& world, const std::string& render_view, int rotation) const {
	WorldRenderView key(world, render_view);
	if (!world_tile_offsets.count(key))
		return renderer::TilePos(0, 0);
	return world_tile_offsets.at(key)[rotation];
}

void MapcrafterConfigHelper::setWorldTileOffset(const std::string& world,
		const std::string& render_view, int rotation,
		const renderer::TilePos& tile_offset) {
	world_tile_offsets[WorldRenderView(world, render_view)][rotation] = tile_offset;
}

int MapcrafterConfigHelper::getMapTileSize(const std::string& map) const {
	return map_tile_sizes.at(map);
}

void MapcrafterConfigHelper::setMapTileSize(const std::string& map, int tile_size) {
	map_tile_sizes[map] = tile_size;
}

int MapcrafterConfigHelper::getMapZoomlevel(const std::string& map) const {
	if (!map_zoomlevels.count(map))
		return 0;
	return map_zoomlevels.at(map);
}

void MapcrafterConfigHelper::setMapZoomlevel(const std::string& map, int zoomlevel) {
	map_zoomlevels[map] = zoomlevel;
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

picojson::value MapcrafterConfigHelper::getConfigJSON() const {
	picojson::object config_json;

	auto maps = config.getMaps();
	for (auto it = maps.begin(); it != maps.end(); ++it) {
		auto world = config.getWorld(it->getWorld());

		picojson::object map_json;
		map_json["name"] = picojson::value(it->getLongName());
		map_json["world"] = picojson::value(it->getWorld());
		map_json["worldName"] = picojson::value(world.getWorldName());
		map_json["renderView"] = picojson::value(it->getRenderView());
		map_json["textureSize"] = picojson::value((double) it->getTextureSize());
		map_json["tileSize"] = picojson::value((double) getMapTileSize(it->getShortName()));
		map_json["maxZoom"] = picojson::value((double) getMapZoomlevel(it->getShortName()));
		map_json["imageFormat"] = picojson::value(it->getImageFormatSuffix());

		picojson::array rotations_json;
		auto rotations = it->getRotations();
		for (auto it2 = rotations.begin(); it2 != rotations.end(); ++it2)
			rotations_json.push_back(picojson::value((double) *it2));
		map_json["rotations"] = picojson::value(rotations_json);

		picojson::array tile_offsets_json;
		for (int rotation = 0; rotation < 4; rotation++) {
			renderer::TilePos offset = getWorldTileOffset(it->getWorld(), it->getRenderView(), rotation);
			picojson::array offset_json;
			offset_json.push_back(picojson::value((double) offset.getX()));
			offset_json.push_back(picojson::value((double) offset.getY()));
			tile_offsets_json.push_back(picojson::value(offset_json));
		}
		map_json["tileOffsets"] = picojson::value(tile_offsets_json);

		if (!world.getDefaultView().empty())
			map_json["defaultView"] = picojson::value(world.getDefaultView());
		if (world.getDefaultZoom() != 0)
			map_json["defaultZoom"] = picojson::value((double) world.getDefaultZoom());
		if (world.getDefaultRotation() != -1)
			map_json["defaultRotation"] = picojson::value((double) world.getDefaultRotation());

		config_json[it->getShortName()] = picojson::value(map_json);
	}

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
			render_behaviors[map][r] = behavior;
		else
			std::fill(&render_behaviors[map][0], &render_behaviors[map][4], behavior);
	}
}

} /* namespace config */
} /* namespace mapcrafter */
