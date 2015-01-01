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

MapcrafterConfigHelper::MapcrafterConfigHelper() {
}

MapcrafterConfigHelper::MapcrafterConfigHelper(const MapcrafterConfig& config)
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
		world_tile_offsets[world_it->first] = std::array<renderer::TilePos, 4>();
	}
}

MapcrafterConfigHelper::~MapcrafterConfigHelper() {
}

std::string MapcrafterConfigHelper::generateTemplateJavascript() const {
	std::string js = "";

	auto maps = config.getMaps();
	for (auto it = maps.begin(); it != maps.end(); ++it) {
		auto world = config.getWorld(it->getWorld());

		js += "\"" + it->getShortName() + "\" : {\n";
		js += "\tname: \"" + it->getLongName() + "\",\n";
		js += "\tworld: \"" + it->getWorld() + "\",\n";
		js += "\tworldName: \"" + world.getWorldName() + "\",\n";
		js += "\ttextureSize: " + util::str(it->getTextureSize()) + ",\n";
		js += "\ttileSize: " + util::str(32 * it->getTextureSize()) + ",\n";
		js += "\tmaxZoom: " + util::str(getMapZoomlevel(it->getShortName())) + ",\n";
		js += "\timageFormat: \"" + it->getImageFormatSuffix() + "\",\n";

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

		if (!world.getDefaultView().empty())
			js += "\tdefaultView: [" + world.getDefaultView() + "],\n";
		if (world.getDefaultZoom() != 0)
			js += "\tdefaultZoom: " + util::str(world.getDefaultZoom()) + ",\n";
		if (world.getDefaultRotation() != -1)
			js += "\tdefaultRotation: " + util::str(world.getDefaultRotation()) + ",\n";

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
		int rotation, const renderer::TilePos& tile_offset) {
	world_tile_offsets[world][rotation] = tile_offset;
}

const renderer::TilePos& MapcrafterConfigHelper::getWorldTileOffset(
		const std::string& world, int rotation) {
	return world_tile_offsets.at(world)[rotation];
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
