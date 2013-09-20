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

#include <string>
#include <vector>
#include <set>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config2 {

class WorldSection {
private:
	fs::path input_dir;
public:
	WorldSection() {}
	~WorldSection() {}

	bool parse(const ConfigSection& section, ValidationMessages& validation);

	fs::path getInputDir() const { return input_dir; }
};

class MapSection {
private:
	std::string name_short, name_long;
	std::string world;

	fs::path texture_dir;
	std::set<int> rotations;
	std::string rendermode;
	int texture_size;

	bool render_unknown_blocks, render_leaves_transparent, render_biomes;
public:
	MapSection()
		: texture_size(12), render_unknown_blocks(false), render_leaves_transparent(false),
		  render_biomes(false) {}
	~MapSection() {}

	bool parse(const ConfigSection& section, ValidationMessages& validation);

	std::string getShortName() const { return name_short; }
	std::string getLongName() const { return name_long; }
	std::string getWorld() const { return world; }

	fs::path getTextureDir() const { return texture_dir; }
	std::set<int> getRotations() const { return rotations; }
	std::string getRendermode() const { return rendermode; }
	int getTextureSize() const { return texture_size; }

	bool renderUnknownBlocks() const { return render_unknown_blocks; }
	bool renderLeavesTransparent() const { return render_leaves_transparent; }
	bool renderBiomes() const { return render_biomes; }
};

class ConfigParser {
private:
	fs::path output_dir;
public:
	ConfigParser() {}
	~ConfigParser() {}

	bool parse(const std::string& filename, ValidationMessages& validation);

	fs::path getOutputDir() const { return output_dir; }

	bool hasWorld(const std::string& world) const;
	const std::map<std::string, WorldSection>& getWorlds() const;
	const WorldSection& getWorld(const std::string& world) const;

	const std::vector<MapSection>& getMaps() const;
	const MapSection& getMap(const std::string& map);
};

} /* namespace config */
} /* namespace mapcrafter */
#endif /* PARSER_H_ */
