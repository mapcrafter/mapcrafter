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

#include "extended_ini.h"

namespace mapcrafter {
namespace config2 {

bool ConfigSection::has(const std::string& key) const {
	return false;
}

std::string ConfigSection::get(const std::string& key, const std::string& default_value) const {
	return "";
}

void ConfigSection::set(const std::string& key, const std::string& value) {
}

void ConfigSection::remove(const std::string& key) {
}

bool ConfigFile::load(std::istream& in, ValidationMessage& msg) {
	return true;
}

bool ConfigFile::loadFile(const std::string& filename, ValidationMessage& msg) {
	return true;
}

bool ConfigFile::write(std::ostream& out) const {
	return true;
}

bool ConfigFile::writeFile(const std::string& filename) const {
	return true;
}

const ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) const {
	return empty_section;
}

ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) {
	return empty_section;
}

void ConfigFile::addSection(const std::string& type, const std::string& name) {
}

void ConfigFile::removeSection(const std::string& type, const std::string& name) {
}

} /* namespace config */
} /* namespace mapcrafter */
