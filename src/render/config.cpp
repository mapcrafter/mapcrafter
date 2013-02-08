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

#include "config.h"

namespace mapcrafter {
namespace render {

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
}

bool ConfigFile::loadFile(const std::string& filename) {
}

bool ConfigFile::hasSection(const std::string& section) const {
}

bool ConfigFile::has(const std::string& section, const std::string& key) {
}

std::string ConfigFile::get(const std::string& section, const std::string& key) const {
}

}
}
