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

#include "filesystem.h"

#include "../util.h"

#include <iostream>
#include <fstream>

#if defined(__APPLE__)
  #include <mach-o/dyld.h>
#elif defined(__WIN32__) || defined(__WIN64__)
  #include <windows.h>
#endif

namespace mapcrafter {
namespace util {

bool copyFile(const fs::path& from, const fs::path& to) {
	std::ifstream in(from.string().c_str());
	if (!in)
		return false;
	std::ofstream out(to.string().c_str());
	if (!out)
		return false;

	out << in.rdbuf();
	if (!out)
		return false;
	in.close();
	out.close();
	return true;
}

bool copyDirectory(const fs::path& from, const fs::path& to) {
	if (!fs::exists(from) || !fs::is_directory(from))
		return false;
	if (!fs::exists(to) && !fs::create_directories(to))
		return false;
	fs::directory_iterator end;
	for (fs::directory_iterator it(from); it != end; ++it) {
		if (fs::is_regular_file(*it)) {
			if (!copyFile(*it, to / it->path().filename()))
				return false;
		} else if (fs::is_directory(*it)) {
			if (!copyDirectory(*it, to / it->path().filename()))
				return false;
		}
	}
	return true;
}

bool moveFile(const fs::path& from, const fs::path& to) {
	if (!fs::exists(from) || (fs::exists(to) && !fs::remove(to)))
		return false;
	fs::rename(from, to);
	return true;
}

// TODO make sure this works on different OSes
fs::path findHomeDir() {
	char* path;
#if defined(__WIN32__) || defined(__WIN64__)
	path = getenv("APPDATA");
#else
	path = getenv("HOME");
#endif
	if (path != nullptr)
		return fs::path(path);
	return fs::path("");
}

// TODO check different OSes
// see also http://stackoverflow.com/questions/12468104/multi-os-get-executable-path
fs::path findExecutablePath() {
	char buf[1024];
#if defined(__APPLE__)
	uint32_t size = sizeof(buf);
	if (_NSGetExecutablePath(buf, &size) == 0) {
		char real_path[1024];
		if (realpath(buf, real_path)) {
			size_t len = strlen(real_path);
			return fs::path(std::string(real_path, len));
		}
	}
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__linux__)
	int len;
	if ((len = readlink("/proc/self/exe", buf, sizeof(buf))) != -1)
		return fs::path(std::string(buf, len));
#elif defined(__FreeBSD__)
	int mib[4]
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	sysctl(mib, 4, buf, sizeof(buf), NULL, 0);
	return fs::path(std::string(buf));
#elif defined(__WIN32__) || defined(__WIN64__)
	GetModuleFileName(NULL, buf, 1024);
	return fs::path(std::string(buf));
#else
	static_assert(0, "Unable to find the executable's path!");
#endif
	return fs::path("");
}

fs::path findExecutableMapcrafterDir(fs::path executable) {
	std::string filename = executable.filename().string();
	fs::path directory = executable.parent_path();
	if ((filename == "testconfig" || filename == "mapcrafter_markers") &&
			executable.parent_path().filename().string() == "tools")
		return directory.parent_path().parent_path();
	return directory.parent_path();
}

PathList findResourceDirs(const fs::path& executable) {
	fs::path mapcrafter_dir = findExecutableMapcrafterDir(executable);
	PathList resources = {
		mapcrafter_dir / "share" / "mapcrafter",
		mapcrafter_dir / "src" / "data",
	};
	fs::path home = findHomeDir();
	if (!home.empty())
		resources.insert(resources.begin(), home / ".mapcrafter");

	for (PathList::iterator it = resources.begin(); it != resources.end(); ) {
		if (!fs::is_directory(*it))
			resources.erase(it);
		else
			++it;
	}
	return resources;
}

PathList findTemplateDirs(const fs::path& executable) {
	PathList templates, resources = findResourceDirs(executable);
	for (PathList::iterator it = resources.begin(); it != resources.end(); ++it)
		if (fs::is_directory(*it / "template"))
			templates.push_back(*it / "template");
	return templates;
}

PathList findTextureDirs(const fs::path& executable) {
	PathList textures, resources = findResourceDirs(executable);
	for (PathList::iterator it = resources.begin(); it != resources.end(); ++it)
		if (fs::is_directory(*it / "textures"))
			textures.push_back(*it / "textures");
	return textures;
}

fs::path findTemplateDir() {
	PathList templates = findTemplateDirs(findExecutablePath());
	if (templates.size())
		return *templates.begin();
	return fs::path();
}

fs::path findTextureDir() {
	PathList textures = findTextureDirs(findExecutablePath());
	if (textures.size())
		return *textures.begin();
	return fs::path();
}

} /* namespace util */
} /* namespace mapcrafter */
