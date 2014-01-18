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

#ifndef SECTIONS_MAP_H_
#define SECTIONS_MAP_H_

#include "base.h"
#include "../validation.h"

#include <set>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

class INIConfigSection;

class MapSection : public ConfigSectionBase {
public:
	MapSection(bool global = false);
	~MapSection();

	void setConfigDir(const fs::path& config_dir);

	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

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

private:
	fs::path config_dir;

	std::string name_short, name_long;
	Field<std::string> world;

	Field<fs::path> texture_dir;
	Field<std::string> rotations;
	std::set<int> rotations_set;
	Field<std::string> rendermode;
	Field<int> texture_size;

	Field<bool> render_unknown_blocks, render_leaves_transparent, render_biomes, use_image_mtimes;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_MAP_H_ */
