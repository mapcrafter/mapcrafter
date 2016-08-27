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

#include "../configsections/world.h"
#include "../iniconfig.h"
#include "../../util.h"

#include <sstream>

namespace mapcrafter {
namespace util {

template <>
mc::Dimension as<mc::Dimension>(const std::string& from) {
	if (from == "nether")
		return mc::Dimension::NETHER;
	else if (from == "overworld")
		return mc::Dimension::OVERWORLD;
	else if (from == "end")
		return mc::Dimension::END;
	else
		throw std::invalid_argument("Dimension must be one of 'nether', 'overworld' or 'end'!");
}

template <>
mc::BlockPos as<mc::BlockPos>(const std::string& from) {
	std::stringstream ss(util::replaceAll(from, ",", " "));
	mc::BlockPos pos;
	ss >> pos.x >> pos.z >> pos.y;
	if (!ss || !ss.eof())
		throw std::invalid_argument("Invalid block coordinates '" + from + "', "
				"must be of the format '<x>,<z>,<y>'!");
	return pos;
}

}
}

namespace mapcrafter {
namespace config {

WorldSection::WorldSection() {
}

WorldSection::~WorldSection() {
}

std::string WorldSection::getPrettyName() const {
	if (isGlobal())
		return "Global world section";
	return "World section '" + getSectionName() + "'";
}

void WorldSection::dump(std::ostream& out) const {
	out << getPrettyName() << ":" << std::endl;
	out << "  input_dir = " << input_dir << std::endl;
	out << "  dimension = " << dimension << std::endl;
	out << "  world_name = " << world_name << std::endl;
	out << "  default_view = " << default_view << std::endl;
	out << "  default_zoom = " << default_zoom << std::endl;
	out << "  default_rotation = " << default_rotation << std::endl;
	out << "  sea_level = " << sea_level << std::endl;
	out << "  min_y = " << min_y << std::endl;
	out << "  max_y = " << max_y << std::endl;
	out << "  min_x = " << min_x << std::endl;
	out << "  max_x = " << max_x << std::endl;
	out << "  min_z = " << min_z << std::endl;
	out << "  max_z = " << max_z << std::endl;
	out << "  center_x = " << center_x << std::endl;
	out << "  center_z = " << center_z << std::endl;
	out << "  radius = " << radius << std::endl;
	out << "  crop_unpopulated_chunks = " << crop_unpopulated_chunks << std::endl;
	out << "  block_mask = " << block_mask << std::endl;
}

void WorldSection::setConfigDir(const fs::path& config_dir) {
	this->config_dir = config_dir;
}

std::string WorldSection::getShortName() {
	return getSectionName();
}

fs::path WorldSection::getInputDir() const {
	return input_dir.getValue();
}

mc::Dimension WorldSection::getDimension() const {
	return dimension.getValue();
}

std::string WorldSection::getWorldName() const {
	return world_name.getValue();
}

mc::BlockPos WorldSection::getDefaultView() const {
	return default_view.getValue();
}

int WorldSection::getDefaultZoom() const {
	return default_zoom.getValue();
}

int WorldSection::getDefaultRotation() const {
	return default_rotation.getValue();
}

int WorldSection::getSeaLevel() const {
	return sea_level.getValue();
}

bool WorldSection::hasCropUnpopulatedChunks() const {
	return crop_unpopulated_chunks.getValue();
}

std::string WorldSection::getBlockMask() const {
	return block_mask.getValue();
}

const mc::WorldCrop WorldSection::getWorldCrop() const {
	return world_crop;
}

bool WorldSection::needsWorldCentering() const {
	// circular cropped worlds and cropped worlds with complete x- and z-bounds
	return (min_x.hasAnyValue() && max_x.hasAnyValue() && min_z.hasAnyValue() && max_z.hasAnyValue())
			|| center_x.hasAnyValue() || center_z.hasAnyValue() || radius.hasAnyValue();
}

void WorldSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	dimension.setDefault(mc::Dimension::OVERWORLD);
	world_name.setDefault(section.getName());

	default_view.setDefault(mc::BlockPos(0, 0, 0));
	default_zoom.setDefault(0);
	default_rotation.setDefault(-1);
	sea_level.setDefault(64);

	crop_unpopulated_chunks.setDefault(false);
}

bool WorldSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "input_dir") {
		if (input_dir.load(key, value, validation)) {
			input_dir.setValue(BOOST_FS_ABSOLUTE(input_dir.getValue(), config_dir));
			if (!fs::is_directory(input_dir.getValue()))
				validation.error("'input_dir' must be an existing directory! '"
						+ input_dir.getValue().string() + "' does not exist!");
		}
	} else if (key == "dimension")
		dimension.load(key, value, validation);
	else if (key == "world_name")
		world_name.load(key, value, validation);

	else if (key == "default_view")
		default_view.load(key, value, validation);
	else if (key == "default_zoom")
		default_zoom.load(key, value, validation);
	else if (key == "default_rotation") {
		int rotation = stringToRotation(value, ROTATION_NAMES);
		if (rotation == -1)
			validation.error("Invalid rotation '" + value + "'!");
		default_rotation.setValue(rotation);
	} else if (key == "sea_level") 
		sea_level.load(key,value, validation);
	else if (key == "crop_min_y") 
		min_y.load(key, value, validation);
	else if (key == "crop_max_y") 
		max_y.load(key, value, validation);
	else if (key == "crop_min_x") 
		min_x.load(key, value, validation);
	else if (key == "crop_max_x") 
		max_x.load(key, value, validation);
	else if (key == "crop_min_z") 
		min_z.load(key, value, validation);
	else if (key == "crop_max_z") 
		max_z.load(key, value, validation);
	else if (key == "crop_center_x")
		center_x.load(key, value, validation);
	else if (key == "crop_center_z")
		center_z.load(key, value, validation);
	else if (key == "crop_radius")
		radius.load(key, value, validation);
	else if (key == "crop_unpopulated_chunks")
		crop_unpopulated_chunks.load(key, value, validation);
	else if (key == "block_mask")
		block_mask.load(key, value, validation);
	else
		return false;
	return true;
}

namespace {

template <typename T>
util::Interval1D<T> intervalFromFields(Field<T> f1, Field<T> f2) {
	util::Interval1D<T> interval;
	if (f1.hasAnyValue())
		interval.setMin(f1.getValue());
	if (f2.hasAnyValue())
		interval.setMax(f2.getValue());
	return interval;
}

}

void WorldSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	if (default_zoom.hasAnyValue() && default_zoom.getValue() < 0)
		validation.error("The default zoom level must be bigger or equal to 0 ('default_zoom').");

	// validate the world croppping
	bool crop_rectangular = min_x.hasAnyValue() || max_x.hasAnyValue() || min_z.hasAnyValue() || max_z.hasAnyValue();
	bool crop_circular = center_x.hasAnyValue() || center_z.hasAnyValue() || radius.hasAnyValue();

	mc::Area area = mc::Area::dummy();
	if (crop_rectangular && crop_circular) {
		validation.error("You can not use both world cropping types at the same time!");
	} else if (crop_rectangular) {
		if (min_x.hasAnyValue() && max_x.hasAnyValue() && min_x.getValue() > max_x.getValue())
			validation.error("min_x must be smaller than or equal to max_x!");
		if (min_z.hasAnyValue() && max_z.hasAnyValue() && min_z.getValue() > max_z.getValue())
			validation.error("min_z must be smaller than or equal to max_z!");
		util::Interval1D<int> x = intervalFromFields(min_x, max_x);
		util::Interval1D<int> z = intervalFromFields(min_z, max_z);
		area = mc::Area::rectangular(x, z);
	} else if (crop_circular) {
		std::string message = "You have to specify crop_center_x, crop_center_z "
				"and crop_radius for circular world cropping!";
		center_x.require(validation, message)
			&& center_z.require(validation, message)
			&& radius.require(validation, message);

		area = mc::Area::circular(mc::BlockPos(center_x.getValue(), center_z.getValue(), 0), radius.getValue());
	}

	if (crop_rectangular ^ crop_circular) {
		world_crop.setArea(area.withYBounding(intervalFromFields(min_y, max_y)));
	}

	if (min_y.hasAnyValue() && max_y.hasAnyValue() && min_y.getValue() > max_y.getValue())
		validation.error("min_y must be smaller than or equal to max_y!");

	world_crop.setCropUnpopulatedChunks(crop_unpopulated_chunks.getValue());
	if (block_mask.hasAnyValue()) {
		try {
			world_crop.loadBlockMask(block_mask.getValue());
		} catch (std::invalid_argument& exception) {
			validation.error(std::string("There is a problem parsing the block mask: ")
				+ exception.what());
		}
	}

	// check if required options were specified
	if (!isGlobal()) {
		input_dir.require(validation, "You have to specify an input directory ('input_dir')!");
	}
}

} /* namespace config */
} /* namespace mapcrafter */
