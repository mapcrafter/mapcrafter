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

#ifndef SECTIONS_MAP_H_
#define SECTIONS_MAP_H_

#include "base.h"
#include "../validation.h"
#include "../../util.h"

#include <set>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

enum class ImageFormat {
	PNG,
	JPEG
};

class INIConfigSection;

class MapSection : public ConfigSectionBase {
public:
	MapSection(bool global = false);
	~MapSection();

	virtual std::string getPrettyName() const;

	void setConfigDir(const fs::path& config_dir);

	std::string getShortName() const;
	std::string getLongName() const;
	std::string getWorld() const;

	std::string getRendermode() const;
	std::set<int> getRotations() const;
	fs::path getTextureDir() const;
	int getTextureSize() const;

	ImageFormat getImageFormat() const;
	std::string getImageFormatSuffix() const;
	int getJPEGQuality() const;

	double getLightingIntensity() const;
	bool renderUnknownBlocks() const;
	bool renderLeavesTransparent() const;
	bool renderBiomes() const;
	bool useImageModificationTimes() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	fs::path config_dir;

	std::string name_short, name_long;
	Field<std::string> world;

	Field<std::string> rendermode;
	Field<std::string> rotations;
	std::set<int> rotations_set;

	Field<fs::path> texture_dir;
	Field<int> texture_size;

	Field<ImageFormat> image_format;
	Field<int> jpeg_quality;

	Field<double> lighting_intensity;
	Field<bool> render_unknown_blocks, render_leaves_transparent, render_biomes, use_image_mtimes;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_MAP_H_ */
