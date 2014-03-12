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

#include "world.h"

#include "../iniconfig.h"

namespace mapcrafter {
namespace config {

WorldSection::WorldSection(bool global)
	: dimension(mc::Dimension::OVERWORLD) {
	setGlobal(global);
}

WorldSection::~WorldSection() {
}

void WorldSection::setConfigDir(const fs::path& config_dir) {
	this->config_dir = config_dir;
}

void WorldSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	dimension_name.setDefault("overworld");
	world_name.setDefault(section.getName());

	default_view.setDefault("");
	default_zoom.setDefault(0);
	default_rotation.setDefault(-1);
}

bool WorldSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "input_dir") {
		if (input_dir.load(key, value, validation)) {
			input_dir.setValue(BOOST_FS_ABSOLUTE(input_dir.getValue(), config_dir));
			if (!fs::is_directory(input_dir.getValue()))
				validation.push_back(ValidationMessage::error(
						"'input_dir' must be an existing directory! '"
						+ input_dir.getValue().string() + "' does not exist!"));
		}
	} else if (key == "dimension")
		dimension_name.load(key, value, validation);
	else if (key == "world_name")
		world_name.load(key, value, validation);

	else if (key == "default_view")
		default_view.load(key, value, validation);
	else if (key == "default_zoom")
		default_zoom.load(key, value, validation);
	else if (key == "default_rotation") {
		int rotation = stringToRotation(value, ROTATION_NAMES);
		if (rotation == -1)
			validation.push_back(ValidationMessage::error(
					"Invalid rotation '" + value + "'!"));
		default_rotation.setValue(rotation);
	}

	else if (key == "crop_min_y") {
		if (min_y.load(key, value, validation))
			worldcrop.setMinY(min_y.getValue());
	} else if (key == "crop_max_y") {
		if (max_y.load(key, value, validation))
			worldcrop.setMaxY(max_y.getValue());
	} else if (key == "crop_min_x") {
		if (min_x.load(key, value, validation))
			worldcrop.setMinX(min_x.getValue());
	} else if (key == "crop_max_x") {
		if (max_x.load(key, value, validation))
			worldcrop.setMaxX(max_x.getValue());
	} else if (key == "crop_min_z") {
		if (min_z.load(key, value, validation))
			worldcrop.setMinZ(min_z.getValue());
	} else if (key == "crop_max_z") {
		if (max_z.load(key, value, validation))
			worldcrop.setMaxZ(max_z.getValue());
	}

	else if (key == "crop_center_x")
		center_x.load(key, value, validation);
	else if (key == "crop_center_z")
		center_z.load(key, value, validation);
	else if (key == "crop_radius")
		radius.load(key, value, validation);
	else
		return false;
	return true;
}

void WorldSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	if (dimension_name.getValue() == "nether")
		dimension = mc::Dimension::NETHER;
	else if (dimension_name.getValue() == "overworld")
		dimension = mc::Dimension::OVERWORLD;
	else if (dimension_name.getValue() == "end")
		dimension = mc::Dimension::END;
	else
		validation.push_back(ValidationMessage::error(
				"Unknown dimension '" + dimension_name.getValue() + "'!"));

	if (default_zoom.isLoaded() && default_zoom.getValue() < 0)
		validation.push_back(ValidationMessage::error(
				"The default zoom level must be bigger or equal to 0 ('default_zoom')."));

	// validate the world croppping
	bool crop_rectangular = min_x.isLoaded() || max_x.isLoaded() || min_z.isLoaded() || max_z.isLoaded();
	bool crop_circular = center_x.isLoaded() || center_z.isLoaded() || radius.isLoaded();

	if (crop_rectangular && crop_circular) {
		validation.push_back(ValidationMessage::error(
				"You can not use both world cropping types at the same time!"));
	} else if (crop_rectangular) {
		if (min_x.isLoaded() && max_x.isLoaded() && min_x.getValue() > max_x.getValue())
			validation.push_back(ValidationMessage::error("min_x must be smaller than or equal to max_x!"));
		if (min_z.isLoaded() && max_z.isLoaded() && min_z.getValue() > max_z.getValue())
			validation.push_back(ValidationMessage::error("min_z must be smaller than or equal to max_z!"));
	} else if (crop_circular) {
		std::string message = "You have to specify crop_center_x, crop_center_z "
				"and crop_radius for circular world cropping!";
		center_x.require(validation, message)
			&& center_z.require(validation, message)
			&& radius.require(validation, message);

		worldcrop.setCenter(mc::BlockPos(center_x.getValue(), center_z.getValue(), 0));
		worldcrop.setRadius(radius.getValue());
	}

	if (min_y.isLoaded() && max_y.isLoaded() && min_y.getValue() > max_y.getValue())
		validation.push_back(ValidationMessage::error("min_y must be smaller than or equal to max_y!"));

	// check if required options were specified
	if (!global) {
		input_dir.require(validation, "You have to specify an input directory ('input_dir')!");
	}
}

std::string WorldSection::getShortName() {
	return section_name;
}

fs::path WorldSection::getInputDir() const {
	return input_dir.getValue();
}

mc::Dimension WorldSection::getDimension() const {
	return dimension;
}

std::string WorldSection::getWorldName() const {
	return world_name.getValue();
}

std::string WorldSection::getDefaultView() const {
	return default_view.getValue();
}

int WorldSection::getDefaultZoom() const {
	return default_zoom.getValue();
}

int WorldSection::getDefaultRotation() const {
	return default_rotation.getValue();
}

const mc::WorldCrop WorldSection::getWorldCrop() const {
	return worldcrop;
}

bool WorldSection::needsWorldCentering() const {
	// circular cropped worlds and cropped worlds with complete x- and z-bounds
	return (min_x.isLoaded() && max_x.isLoaded() && min_z.isLoaded() && max_z.isLoaded())
			|| center_x.isLoaded() || center_z.isLoaded() || radius.isLoaded();
}

} /* namespace config */
} /* namespace mapcrafter */
