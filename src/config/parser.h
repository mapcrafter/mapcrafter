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

namespace mapcrafter {
namespace config2 {

class WorldSection {
private:
public:
	WorldSection() {}
	~WorldSection() {}

	bool parse(const ConfigSection& section, ValidationMessages& validation);
};

class MapSection {
private:
public:
	MapSection() {}
	~MapSection() {}

	bool parse(const ConfigSection& section, ValidationMessages& validation);
};

class ConfigParser {
private:
public:
	ConfigParser() {}
	~ConfigParser() {}

	bool parse(const std::string& filename, ValidationMessages& validation);

	bool hasWorld(const std::string& world) const;
	const std::map<std::string, WorldSection>& getWorlds() const;
	const WorldSection& getWorld(const std::string& world) const;

	const std::vector<MapSection>& getMaps() const;
	const MapSection& getMap(const std::string& map);
};

} /* namespace config */
} /* namespace mapcrafter */
#endif /* PARSER_H_ */
