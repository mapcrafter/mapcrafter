/*
 * Copyright 2012-2016 Moritz Hilscher
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

/**
 * Returns the home directory of the current user.
 *
 * Returns the value of the 'HOME' environment variable on every unix-like operating
 * system, returns the value of the 'APPDATA' environment variable on W***.
 */
fs::path findHomeDir();

/**
 * Returns the path to the currently running program.
 */
fs::path findExecutablePath();

/**
 * Returns the directory which contains the Mapcrafter executable.
 *
 * Usually returns the directory of the program currently running, but returns the parent
 * directory of the program directory if the program is another Mapcrafter tool such as
 * testconfig or testtextures and the program directory is called tools/.
 */
fs::path findExecutableMapcrafterDir(fs::path executable = findExecutablePath());

typedef std::vector<fs::path> PathList;

/**
 * Returns all possible Mapcrafter resource directories.
 */
PathList findResourceDirs(const fs::path& executable);

/**
 * Returns all possible Mapcrafter template directories.
 */
PathList findTemplateDirs(const fs::path& executable);

/**
 * Returns all possible Mapcrafter block directories.
 */
PathList findBlockDirs(const fs::path& executable);

/**
 * Returns all possible logging configuration files.
 */
PathList findLoggingConfigFiles(const fs::path& executable);

/**
 * Returns the first existing template directory.
 */
fs::path findTemplateDir();

/**
 * Returns the first existing block directory.
 */
fs::path findBlockDir();

/**
 * Returns the first existing logging configuration file.
 */
fs::path findLoggingConfigFile();

} /* namespace util */
} /* namespace mapcrafter */
#endif /* FILESYSTEM_H_ */
