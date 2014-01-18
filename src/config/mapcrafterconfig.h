/*
 * Copyright 2012-2014 Moritz Hilscher
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

#ifndef MAPCRAFTERCONFIG_H_
#define MAPCRAFTERCONFIG_H_

#include "validation.h"
#include "sections/base.h"

#include <map>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

class MapcrafterConfig {
private:
	WorldSection world_global;
	MapSection map_global;

	Field<fs::path> output_dir, template_dir;

	std::map<std::string, WorldSection> worlds;
	std::vector<MapSection> maps;
public:
	MapcrafterConfig();
	~MapcrafterConfig();

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

} /* namespace config */
} /* namespace mapcrafter */
#endif /* MAPCRAFTERCONFIG_H_ */
