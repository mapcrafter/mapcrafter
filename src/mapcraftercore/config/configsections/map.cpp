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

#include "map.h"

#include "../iniconfig.h"
#include "../../util.h"

namespace mapcrafter {
namespace util {

template <>
renderer::ImageFormatType as<renderer::ImageFormatType>(const std::string& from) {
	if (from == "png")
		return renderer::ImageFormatType::PNG;
	else if (from == "jpeg")
		return renderer::ImageFormatType::JPEG;
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
	else if (from == "topdown")
		return renderer::RenderViewType::TOPDOWN;
	throw std::invalid_argument("Must be 'isometric' or 'topdown'!");
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
	out << "  render_view" << render_view << std::endl;
	out << "  render_mode = " << render_mode << std::endl;
	out << "  overlays = " << overlays << std::endl;
	out << "  default_overlays = " << default_overlays << std::endl;
	out << "  rotations = " << rotations << std::endl;
	out << "  texture_dir = " << texture_dir << std::endl;
	out << "  texture_size = " << texture_size << std::endl;
	out << "  water_opacity = " << water_opacity << std::endl;
	out << "  image_format = " << getImageFormat() << std::endl;
	out << "  png_indexed = " << png_indexed << std::endl;
	out << "  jpeg_quality = " << jpeg_quality << std::endl;
	out << "  lighting_intensity = " << lighting_intensity << std::endl;
	out << "  lighting_water_intensity = " << water_opacity << std::endl;
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

renderer::RenderViewType MapSection::getRenderView() const {
	return render_view.getValue();
}

renderer::RenderModeType MapSection::getRenderMode() const {
	return render_mode.getValue();
}

std::vector<std::string> MapSection::getOverlays() const {
	return overlays_vector;
}

std::vector<std::string> MapSection::getDefaultOverlays() const {
	return default_overlays_vector;
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

int MapSection::getTextureBlur() const {
	return texture_blur.getValue();
}

double MapSection::getWaterOpacity() const {
	return water_opacity.getValue();
}

int MapSection::getTileWidth() const {
	return tile_width.getValue();
}

renderer::ImageFormat MapSection::getImageFormat() const {
	if (image_format_type.getValue() == renderer::ImageFormatType::PNG)
		return renderer::ImageFormat::png(png_indexed.getValue());
	return renderer::ImageFormat::jpeg(jpeg_quality.getValue(), renderer::rgba(0, 0, 0, 0));
}

double MapSection::getLightingIntensity() const {
	return lighting_intensity.getValue();
}

double MapSection::getLightingWaterIntensity() const {
	return lighting_water_intensity.getValue();
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
	overlays.setValue("");
	rotations.setDefault("top-left");

	// check if we can find a default texture directory
	fs::path texture_dir_found = util::findTextureDir();
	if (!texture_dir_found.empty())
		texture_dir.setDefault(texture_dir_found);
	texture_size.setDefault(12);
	texture_blur.setDefault(0);
	water_opacity.setDefault(1.0);
	tile_width.setDefault(1);

	image_format_type.setDefault(renderer::ImageFormatType::PNG);
	png_indexed.setDefault(false);
	jpeg_quality.setDefault(85);

	lighting_intensity.setDefault(1.0);
	lighting_water_intensity.setDefault(1.0);
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
	} else if (key == "render_view") {
		render_view.load(key, value, validation);
	} else if (key == "render_mode" || key == "rendermode") {
		render_mode.load(key, value, validation);
		if (key == "rendermode")
			validation.warning("Using the option 'rendermode' is deprecated. "
					"It's called 'render_mode' now.");
	} else if (key == "overlays") {
		overlays.load(key, value, validation);
		overlays_vector.clear();
		std::stringstream ss;
		ss << overlays.getValue();
		std::string overlay;
		while (ss >> overlay) {
			// TODO make Field::load able to load sets/vectors of types?
			overlays_vector.push_back(overlay);
		}
	} else if (key == "default_overlays") {
		default_overlays.load(key, value, validation);
		default_overlays_vector.clear();
		std::stringstream ss;
		ss << default_overlays.getValue();
		std::string overlay;
		while (ss >> overlay) {
			// TODO make Field::load able to load sets/vectors of types?
			default_overlays_vector.push_back(overlay);
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
	} else if (key == "texture_blur") {
		texture_blur.load(key, value, validation);
	} else if (key == "texture_size") {
		if (texture_size.load(key, value, validation)
				&& (texture_size.getValue() <= 0  || texture_size.getValue() > 32))
			validation.error("'texture_size' must be a number between 1 and 32!");
	} else if (key == "water_opacity") {
		if (water_opacity.load(key, value, validation)
				&& (water_opacity.getValue() < 0 || water_opacity.getValue() > 1.0))
			validation.error("'water_opacity' must be a number between 0.0 and 1.0!");
	} else if (key == "tile_width") {
		tile_width.load(key, value, validation);
		if (tile_width.getValue() < 1)
			validation.error("'tile_width' must be a positive number!");
	} else if (key == "image_format") {
		image_format_type.load(key, value, validation);
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
		texture_dir.require(validation, "You have to specify a texture directory ('texture_dir')!");
	}
}

} /* namespace config */
} /* namespace mapcrafter */
