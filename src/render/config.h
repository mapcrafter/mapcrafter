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

#ifndef CONFIG_H_
#define CONFIG_H_

#include "util.h"

#include <string>

namespace mapcrafter {
namespace render {

/**
 * A simple INI-like config file parser.
 */
class ConfigFile {
public:
	ConfigFile();
	~ConfigFile();

	bool loadFile(const std::string& filename);

	bool hasSection(const std::string& section) const;
	bool has(const std::string& section, const std::string& key);

	std::string get(const std::string& section, const std::string& key) const;

	template<typename T>
	T get(const std::string& section, const std::string& key) const {
		return as<T>(get(section, key));
	}
};

}
}

#endif /* CONFIG_H_ */
