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

#ifndef PARSER_H_
#define PARSER_H_

#include "extended_ini.h"
#include "validation.h"

#include "../mc/worldcrop.h"
#include "../renderer/tileset.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

class WorldSection {
private:
	bool global;

	Field<fs::path> input_dir;
	Field<std::string> world_name;

	Field<int> min_y, max_y;
	Field<int> min_x, max_x, min_z, max_z;
	Field<int> center_x, center_z, radius;
	mc::WorldCrop worldcrop;
public:
	WorldSection(bool global = false);
	~WorldSection();

	void setGlobal(bool global);
	bool parse(const ConfigSection& section, const fs::path& config_dir, ValidationList& validation);

	fs::path getInputDir() const;
	std::string getWorldName() const;
	const mc::WorldCrop getWorldCrop() const;
	bool needsWorldCentering() const;
};

class MapSection {
private:
	bool global;

	std::string name_short, name_long;
	Field<std::string> world;

	Field<fs::path> texture_dir;
	Field<std::string> rotations;
	std::set<int> rotations_set;
	Field<std::string> rendermode;
	Field<int> texture_size;

	Field<bool> render_unknown_blocks, render_leaves_transparent, render_biomes, use_image_mtimes;
public:
	MapSection(bool global = false);
	~MapSection();

	void setGlobal(bool global);
	bool parse(const ConfigSection& section, const fs::path& config_dir, ValidationList& validation);

	std::string getShortName() const;
	std::string getLongName() const;
	std::string getWorld() const;

	fs::path getTextureDir() const;
	std::set<int> getRotations() const;
	std::string getRendermode() const;
	int getTextureSize() const;

	bool renderUnknownBlocks() const;
	bool renderLeavesTransparent() const;
	bool renderBiomes() const;
	bool useImageModificationTimes() const;
};

class MapcrafterConfigFile {
private:
	WorldSection world_global;
	MapSection map_global;

	Field<fs::path> output_dir, template_dir;

	std::map<std::string, WorldSection> worlds;
	std::vector<MapSection> maps;
public:
	MapcrafterConfigFile();
	~MapcrafterConfigFile();

	bool parse(const std::string& filename, ValidationMap& validation);
	void dump(std::ostream& out) const;

	fs::path getOutputDir() const;
	fs::path getTemplateDir() const;

	std::string getOutputPath(const std::string& path) const;
	std::string getTemplatePath(const std::string& path) const;

	bool hasWorld(const std::string& world) const;
	const std::map<std::string, WorldSection>& getWorlds() const;
	const WorldSection& getWorld(const std::string& world) const;

	bool hasMap(const std::string& map) const;
	const std::vector<MapSection>& getMaps() const;
	const MapSection& getMap(const std::string& map) const;
};

class MapcrafterConfigHelper {
private:
	MapcrafterConfigFile config;

	std::map<std::string, std::set<int> > world_rotations;
	std::map<std::string, int> world_zoomlevels;
	std::map<std::string, int> map_zoomlevels;
	std::map<std::string, std::array<renderer::TilePos, 4> > world_tile_offsets;

	std::map<std::string, std::array<int, 4> > render_behaviors;

	void setRenderBehaviors(std::vector<std::string> maps, int behavior);
public:
	MapcrafterConfigHelper();
	MapcrafterConfigHelper(const MapcrafterConfigFile& config);
	~MapcrafterConfigHelper();

	std::string generateTemplateJavascript() const;

	const std::set<int>& getUsedRotations(const std::string& world) const;
	void setUsedRotations(const std::string& world, const std::set<int>& rotations);

	int getWorldZoomlevel(const std::string& world) const;
	int getMapZoomlevel(const std::string& map) const;
	void setWorldZoomlevel(const std::string& world, int zoomlevel);
	void setMapZoomlevel(const std::string& map, int zoomlevel);

	void setWorldTileOffset(const std::string& world, int rotation,
			const renderer::TilePos& tile_offset);
	const renderer::TilePos& getWorldTileOffset(const std::string& world, int rotation);

	int getRenderBehavior(const std::string& map, int rotation) const;
	void setRenderBehavior(const std::string& map, int rotation, int behavior);

	bool isCompleteRenderSkip(const std::string& map) const;
	bool isCompleteRenderForce(const std::string& map) const;

	void parseRenderBehaviors(bool skip_all,
			std::vector<std::string>,
			std::vector<std::string> render_auto,
			std::vector<std::string> render_force);

	static const int RENDER_SKIP = 0;
	static const int RENDER_AUTO = 1;
	static const int RENDER_FORCE = 2;
};

} /* namespace config */
} /* namespace mapcrafter */
#endif /* PARSER_H_ */
