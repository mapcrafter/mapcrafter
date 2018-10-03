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

#include "../configsections/map.h"
#include "../iniconfig.h"
#include "../../util.h"

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

template <>
renderer::RenderModeType as<renderer::RenderModeType>(const std::string& from) {
	if (from == "plain")
		return renderer::RenderModeType::PLAIN;
	else if (from == "daylight")
		return renderer::RenderModeType::DAYLIGHT;
	else if (from == "nightlight")
		return renderer::RenderModeType::NIGHTLIGHT;
	else if (from == "cave")
		return renderer::RenderModeType::CAVE;
	else if (from == "cavelight")
		return renderer::RenderModeType::CAVELIGHT;
	throw std::invalid_argument("Must be one of 'plain', 'daylight', 'nightlight', "
			"'cave' or 'cavelight'!");
}

template <>
renderer::RenderViewType as<renderer::RenderViewType>(const std::string& from) {
	if (from == "isometric")
		return renderer::RenderViewType::ISOMETRIC;
	else if (from == "isometricnew")
		return renderer::RenderViewType::ISOMETRICNEW;
	else if (from == "side")
		return renderer::RenderViewType::SIDE;
	else if (from == "topdown")
		return renderer::RenderViewType::TOPDOWN;
	throw std::invalid_argument("Must be 'isometric' or 'topdown'!");
}

template <>
renderer::OverlayType as<renderer::OverlayType>(const std::string& from) {
	if (from == "none")
		return renderer::OverlayType::NONE;
	else if (from == "slime")
		return renderer::OverlayType::SLIME;
	else if (from == "spawnday")
		return renderer::OverlayType::SPAWNDAY;
	else if (from == "spawnnight")
		return renderer::OverlayType::SPAWNNIGHT;
	throw std::invalid_argument("Must be 'none', 'slime', 'spawnday', or "
			"'spawnnight'!");
}

}
}

namespace mapcrafter {
namespace config {

TileSetGroupID::TileSetGroupID()
	: render_view(renderer::RenderViewType::ISOMETRIC), tile_width(1) {
}

TileSetGroupID::TileSetGroupID(const std::string& world_name,
		renderer::RenderViewType render_view, int tile_width)
	: world_name(world_name), render_view(render_view), tile_width(tile_width) {
}

std::string TileSetGroupID::toString() const {
	std::string repr = "";
	repr += world_name + "_";
	repr += util::str(render_view) + "_";
	repr += "t" + util::str(tile_width);
	return repr;
}

bool TileSetGroupID::operator<(const TileSetGroupID& other) const {
	return toString() < other.toString();
}

TileSetID::TileSetID()
	: TileSetGroupID(), rotation(0) {
}

TileSetID::TileSetID(const std::string& world_name,
		renderer::RenderViewType render_view, int tile_width, int rotation)
	: TileSetGroupID(world_name, render_view, tile_width), rotation(rotation) {
}

TileSetID::TileSetID(const TileSetGroupID& group, int rotation)
	: TileSetGroupID(group.world_name, group.render_view, group.tile_width), rotation(rotation) {
}

std::string TileSetID::toString() const {
	return TileSetGroupID::toString() + "_r" + util::str(rotation);
}

bool TileSetID::operator<(const TileSetID& other) const {
	return toString() < other.toString();
}

std::ostream& operator<<(std::ostream& out, ImageFormat image_format) {
	if (image_format == ImageFormat::PNG)
		out << "png";
	else if (image_format == ImageFormat::JPEG)
		out << "jpeg";
	return out;
}

MapSection::MapSection()
	: texture_size(12), render_biomes(false) {
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
	out << "  render_view" << render_view << std::endl;
	out << "  render_mode = " << render_mode << std::endl;
	out << "  overlay = " << overlay << std::endl;
	out << "  rotations = " << rotations << std::endl;
	out << "  block_dir = " << block_dir << std::endl;
	out << "  texture_size = " << texture_size << std::endl;
	out << "  image_format = " << image_format << std::endl;
	out << "  png_indexed = " << png_indexed << std::endl;
	out << "  jpeg_quality = " << jpeg_quality << std::endl;
	out << "  lighting_intensity = " << lighting_intensity << std::endl;
	out << "  lighting_water_intensity = " << lighting_water_intensity << std::endl;
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

renderer::RenderViewType MapSection::getRenderView() const {
	return render_view.getValue();
}

renderer::RenderModeType MapSection::getRenderMode() const {
	return render_mode.getValue();
}

renderer::OverlayType MapSection::getOverlay() const {
	return overlay.getValue();
}

std::set<int> MapSection::getRotations() const {
	return rotations_set;
}

fs::path MapSection::getBlockDir() const {
	return block_dir.getValue();
}

int MapSection::getTextureSize() const {
	return texture_size.getValue();
}

int MapSection::getTileWidth() const {
	return tile_width.getValue();
}

ImageFormat MapSection::getImageFormat() const {
	return image_format.getValue();
}

std::string MapSection::getImageFormatSuffix() const {
	if (getImageFormat() == ImageFormat::PNG)
		return "png";
	return "jpg";
}

bool MapSection::isPNGIndexed() const {
	return png_indexed.getValue();
}

int MapSection::getJPEGQuality() const {
	return jpeg_quality.getValue();
}

double MapSection::getLightingIntensity() const {
	return lighting_intensity.getValue();
}

double MapSection::getLightingWaterIntensity() const {
	return lighting_water_intensity.getValue();
}

bool MapSection::renderBiomes() const {
	return render_biomes.getValue();
}

bool MapSection::useImageModificationTimes() const {
	return use_image_mtimes.getValue();
}

TileSetGroupID MapSection::getTileSetGroup() const {
	return TileSetGroupID(getWorld(), getRenderView(), getTileWidth());
}

TileSetID MapSection::getTileSet(int rotation) const {
	return TileSetID(getWorld(), getRenderView(), getTileWidth(), rotation);
}

const std::set<TileSetID>& MapSection::getTileSets() const {
	return tile_sets;
}

void MapSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	name_short = getSectionName();
	name_long = name_short;

	// set some default configuration values
	render_view.setDefault(renderer::RenderViewType::ISOMETRIC);
	render_mode.setDefault(renderer::RenderModeType::DAYLIGHT);
	overlay.setDefault(renderer::OverlayType::NONE);
	rotations.setDefault("top-left");
	
	fs::path block_dir_found = util::findBlockDir();
	if (!block_dir_found.empty()) {
		block_dir.setDefault(block_dir_found);
	}

	texture_size.setDefault(12);
	tile_width.setDefault(1);

	image_format.setDefault(ImageFormat::PNG);
	png_indexed.setDefault(false);
	jpeg_quality.setDefault(85);

	lighting_intensity.setDefault(1.0);
	lighting_water_intensity.setDefault(0.85);
	render_biomes.setDefault(true);
	use_image_mtimes.setDefault(true);
}

bool MapSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "name") {
		name_long = value;
	} else if (key == "world") {
		world.load(key, value, validation);
	} else if (key == "render_view") {
		if (render_view.load(key, value, validation)) {
			if (render_view.getValue() == renderer::RenderViewType::ISOMETRICNEW) {
				validation.error("Using 'isometricnew' for 'render_view' is not necessary anymore! "
						"Just use 'isometric' or 'topdown'.");
			}
		}
	} else if (key == "render_mode" || key == "rendermode") {
		render_mode.load(key, value, validation);
		if (key == "rendermode")
			validation.warning("Using the option 'rendermode' is deprecated. "
					"It's called 'render_mode' now.");
	} else if (key == "overlay") {
		overlay.load(key, value, validation);
	} else if (key == "rotations") {
		rotations.load(key, value ,validation);
	} else if (key == "block_dir") {
		if (block_dir.load(key, value, validation)) {
			block_dir.setValue(BOOST_FS_ABSOLUTE(block_dir.getValue(), config_dir));
			if (!fs::is_directory(block_dir.getValue())) {
				validation.error("'block_dir' must be an existing directory! '"
						+ block_dir.getValue().string() + "' does not exist!");
			}
		}
	} else if (key == "texture_blur") {
		texture_blur.load(key, value, validation);
	} else if (key == "texture_size") {
		if (texture_size.load(key, value, validation)
				&& (texture_size.getValue() <= 0  || texture_size.getValue() > 128))
			validation.error("'texture_size' must be a number between 1 and 32!");
	} else if (key == "tile_width") {
		tile_width.load(key, value, validation);
		if (tile_width.getValue() < 1)
			validation.error("'tile_width' must be a positive number!");
	} else if (key == "image_format") {
		image_format.load(key, value, validation);
	} else if (key == "png_indexed") {
		png_indexed.load(key, value, validation);
	} else if (key == "jpeg_quality") {
		if (jpeg_quality.load(key, value, validation)
				&& (jpeg_quality.getValue() < 0 || jpeg_quality.getValue() > 100))
			validation.error("'jpeg_quality' must be a number between 0 and 100!");
	} else if (key == "lighting_intensity") {
		lighting_intensity.load(key, value, validation);
	} else if (key == "lighting_water_intensity") {
		lighting_water_intensity.load(key, value, validation);
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
	tile_sets.clear();
	std::string str = rotations.getValue();
	std::stringstream ss;
	ss << str;
	std::string elem;
	while (ss >> elem) {
		int r = stringToRotation(elem);
		if (r != -1) {
			rotations_set.insert(r);
			tile_sets.insert(getTileSet(r));
		} else {
			validation.error("Invalid rotation '" + elem + "'!");
		}
	}

	// check if required options were specified
	if (!isGlobal()) {
		world.require(validation, "You have to specify a world ('world')!");
		block_dir.require(validation, "You have to specify a block directory ('block_dir')!");
	}
}

} /* namespace config */
} /* namespace mapcrafter */
