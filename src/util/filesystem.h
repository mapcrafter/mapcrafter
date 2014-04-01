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

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace util {

bool copyFile(const fs::path& from, const fs::path& to);
bool copyDirectory(const fs::path& from, const fs::path& to);
bool moveFile(const fs::path& from, const fs::path& to);

fs::path findHomeDir();
fs::path findExecutablePath();
fs::path findExecutableMapcrafterDir(fs::path executable = findExecutablePath());

typedef std::vector<fs::path> PathList;
PathList findResourceDirs(const fs::path& executable);
PathList findTemplateDirs(const fs::path& executable);
PathList findTextureDirs(const fs::path& executable);

fs::path findTemplateDir();
fs::path findTextureDir();

} /* namespace util */
} /* namespace mapcrafter */
#endif /* FILESYSTEM_H_ */
