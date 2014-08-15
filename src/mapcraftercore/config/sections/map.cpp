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

#include "map.h"

#include "../iniconfig.h"

namespace mapcrafter {
namespace util {

template <>
config::ImageFormat as<config::ImageFormat>(const std::string& from) {
	if (from == "png")
		return config::ImageFormat::PNG;
	else if (from == "jpeg")
		return config::ImageFormat::JPEG;
	throw std::invalid_argument("Must be 'png' or 'jpeg'!");
}

}
}

namespace mapcrafter {
namespace config {

std::ostream& operator<<(std::ostream& out, ImageFormat image_format) {
	if (image_format == ImageFormat::PNG)
		out << "png";
	else if (image_format == ImageFormat::JPEG)
		out << "jpeg";
	return out;
}

MapSection::MapSection()
	: texture_size(12), render_unknown_blocks(false),
	  render_leaves_transparent(false), render_biomes(false) {
}

MapSection::~MapSection() {
}

std::string MapSection::getPrettyName() const {
	if (isGlobal())
		return "Global map section";
	return "Map section '" + getSectionName() + "'";
}

void MapSection::dump(std::ostream& out) const {
	out << getPrettyName() << ":" << std::endl;
	out << "  name = " << getLongName() << std::endl;
	out << "  world = " << world << std::endl;
	out << "  rendermode = " << rendermode << std::endl;
	out << "  rotations = " << rotations << std::endl;
	out << "  texture_dir = " << texture_dir << std::endl;
	out << "  texture_size = " << texture_size << std::endl;
	out << "  image_format = " << image_format << std::endl;
	out << "  jpeg_quality = " << jpeg_quality << std::endl;
	out << "  lighting_intensity = " << lighting_intensity << std::endl;
	out << "  render_unknown_blocks = " << render_unknown_blocks << std::endl;
	out << "  render_leaves_transparent = " << render_leaves_transparent << std::endl;
	out << "  render_biomes = " << render_biomes << std::endl;
	out << "  use_image_timestamps = " << use_image_mtimes << std::endl;
}

void MapSection::setConfigDir(const fs::path& config_dir) {
	this->config_dir = config_dir;
}

std::string MapSection::getShortName() const {
	return name_short;
}

std::string MapSection::getLongName() const {
	return name_long;
}

std::string MapSection::getWorld() const {
	return world.getValue();
}

std::string MapSection::getRendermode() const {
	return rendermode.getValue();
}

std::set<int> MapSection::getRotations() const {
	return rotations_set;
}

fs::path MapSection::getTextureDir() const {
	return texture_dir.getValue();
}

int MapSection::getTextureSize() const {
	return texture_size.getValue();
}

ImageFormat MapSection::getImageFormat() const {
	return image_format.getValue();
}

std::string MapSection::getImageFormatSuffix() const {
	if (getImageFormat() == ImageFormat::PNG)
		return "png";
	return "jpg";
}

int MapSection::getJPEGQuality() const {
	return jpeg_quality.getValue();
}

double MapSection::getLightingIntensity() const {
	return lighting_intensity.getValue();
}

bool MapSection::renderUnknownBlocks() const {
	return render_unknown_blocks.getValue();
}

bool MapSection::renderLeavesTransparent() const {
	return render_leaves_transparent.getValue();
}

bool MapSection::renderBiomes() const {
	return render_biomes.getValue();
}

bool MapSection::useImageModificationTimes() const {
	return use_image_mtimes.getValue();
}

void MapSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	name_short = getSectionName();
	name_long = name_short;

	// set some default configuration values
	rendermode.setDefault("daylight");
	rotations.setDefault("top-left");

	// check if we can find a default texture directory
	fs::path texture_dir_found = util::findTextureDir();
	if (!texture_dir_found.empty())
		texture_dir.setDefault(texture_dir_found);
	texture_size.setDefault(12);

	image_format.setDefault(ImageFormat::PNG);
	jpeg_quality.setDefault(85);

	lighting_intensity.setDefault(1.0);
	render_unknown_blocks.setDefault(false);
	render_leaves_transparent.setDefault(true);
	render_biomes.setDefault(true);
	use_image_mtimes.setDefault(true);
}

bool MapSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "name") {
		name_long = value;
	} else if (key == "world") {
		world.load(key, value, validation);
	} else if (key == "rendermode") {
		if (rendermode.load(key, value, validation)) {
			std::string r = rendermode.getValue();
			if (r != "plain" && r != "daylight" && r != "nightlight" && r != "cave")
				validation.error("'rendermode' must be one of: 'plain', 'daylight', 'nightlight', 'cave'");
		}
	} else if (key == "rotations") {
		rotations.load(key, value ,validation);
	} else if (key == "texture_dir") {
		if (texture_dir.load(key, value, validation)) {
			texture_dir.setValue(BOOST_FS_ABSOLUTE(texture_dir.getValue(), config_dir));
			if (!fs::is_directory(texture_dir.getValue()))
				validation.error("'texture_dir' must be an existing directory! '"
						+ texture_dir.getValue().string() + "' does not exist!");
		}
	} else if (key == "texture_size") {
		if (texture_size.load(key, value, validation)
				&& (texture_size.getValue() <= 0  || texture_size.getValue() > 32))
				validation.error("'texture_size' must a number between 1 and 32!");
	} else if (key == "image_format") {
		image_format.load(key, value, validation);
	} else if (key == "jpeg_quality") {
		if (jpeg_quality.load(key, value, validation)
				&& (jpeg_quality.getValue() < 0 || jpeg_quality.getValue() > 100))
			validation.error("'jpeg_quality' must be a number between 0 and 100!");
	} else if (key == "lighting_intensity") {
		lighting_intensity.load(key, value, validation);
	} else if (key == "render_unknown_blocks") {
		render_unknown_blocks.load(key, value, validation);
	} else if (key == "render_leaves_transparent") {
		render_leaves_transparent.load(key, value, validation);
	} else if (key == "render_biomes") {
		render_biomes.load(key, value, validation);
	} else if (key == "use_image_mtimes") {
		use_image_mtimes.load(key, value, validation);
	} else
		return false;
	return true;
}

void MapSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	// parse rotations
	rotations_set.clear();
	std::string str = rotations.getValue();
	std::stringstream ss;
	ss << str;
	std::string elem;
	while (ss >> elem) {
		int r = stringToRotation(elem);
		if (r != -1)
			rotations_set.insert(r);
		else
			validation.error("Invalid rotation '" + elem + "'!");
	}

	// check if required options were specified
	if (!isGlobal()) {
		world.require(validation, "You have to specify a world ('world')!");
		texture_dir.require(validation, "You have to specify a texture directory ('texture_dir')!");
	}
}

} /* namespace config */
} /* namespace mapcrafter */
