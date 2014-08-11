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

#ifndef SECTIONS_WORLD_H_
#define SECTIONS_WORLD_H_

#include "base.h"
#include "../validation.h"
#include "../../mc/world.h"
#include "../../mc/worldcrop.h"

#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

class INIConfigSection;

class WorldSection : public ConfigSectionBase {
public:
	WorldSection();
	~WorldSection();

	virtual std::string getPrettyName() const;
	virtual void dump(std::ostream& out) const;

	void setConfigDir(const fs::path& config_dir);

	std::string getShortName();

	fs::path getInputDir() const;
	mc::Dimension getDimension() const;
	std::string getWorldName() const;

	std::string getDefaultView() const;
	int getDefaultZoom() const;
	int getDefaultRotation() const;

	bool hasCropUnpopulatedChunks() const;
	std::string getBlockMask() const;

	const mc::WorldCrop getWorldCrop() const;
	bool needsWorldCentering() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	fs::path config_dir;

	Field<fs::path> input_dir;
	Field<mc::Dimension> dimension;
	Field<std::string> world_name;

	Field<std::string> default_view;
	Field<int> default_zoom, default_rotation;

	Field<int> min_y, max_y;
	Field<int> min_x, max_x, min_z, max_z;
	Field<int> center_x, center_z, radius;

	Field<bool> crop_unpopulated_chunks;
	Field<std::string> block_mask;

	mc::WorldCrop world_crop;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_WORLD_H_ */
