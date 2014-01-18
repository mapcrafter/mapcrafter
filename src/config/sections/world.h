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

#ifndef SECTIONS_WORLD_H_
#define SECTIONS_WORLD_H_

#include "base.h"
#include "../validation.h"
#include "../../mc/worldcrop.h"

#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

class INIConfigSection;

class WorldSection : public ConfigSectionBase {
public:
	WorldSection(bool global = false);
	~WorldSection();

	void setConfigDir(const fs::path& config_dir);

	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

	fs::path getInputDir() const;
	std::string getWorldName() const;
	const mc::WorldCrop getWorldCrop() const;
	bool needsWorldCentering() const;

private:
	fs::path config_dir;

	Field<fs::path> input_dir;
	Field<std::string> world_name;

	Field<int> min_y, max_y;
	Field<int> min_x, max_x, min_z, max_z;
	Field<int> center_x, center_z, radius;
	mc::WorldCrop worldcrop;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_WORLD_H_ */
