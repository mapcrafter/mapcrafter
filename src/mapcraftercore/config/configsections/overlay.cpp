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

#include "overlay.h"

#include "../iniconfig.h"
#include "../../util.h"

#include <iostream>
#include <sstream>

namespace mapcrafter {
namespace util {

template <>
config::OverlayType as<config::OverlayType>(const std::string& from) {
	if (from == "area")
		return config::OverlayType::AREA;
	else if (from == "height")
		return config::OverlayType::HEIGHT;
	else if (from == "lighting")
		return config::OverlayType::LIGHTING;
	else if (from == "lightlevel")
		return config::OverlayType::LIGHTLEVEL;
	else if (from == "slime")
		return config::OverlayType::SLIME;
	else if (from == "spawn")
		return config::OverlayType::SPAWN;
	throw std::invalid_argument("Must be 'none', 'area', 'lighting', 'lightlevel', 'spawn' or 'slime'.");
}

template <>
config::HeightColor as<config::HeightColor>(const std::string& from) {
	if (std::count(from.begin(), from.end(), ':') != 1)
		throw std::invalid_argument("Must be of format y:color.");
	int index = from.find(':');
	config::HeightColor color;
	color.y = as<int>(from.substr(0, index));
	color.color = as<Color>(from.substr(index + 1));
	return color;
}

}
}

namespace mapcrafter {
namespace config {

std::ostream& operator<<(std::ostream& out, OverlayType overlay) {
	switch (overlay) {
	case OverlayType::AREA: return out << "area";
	case OverlayType::HEIGHT: return out << "height";
	case OverlayType::LIGHTING: return out << "lighting";
	case OverlayType::LIGHTLEVEL: return out << "lightlevel";
	case OverlayType::SLIME: return out << "slime";
	case OverlayType::SPAWN: return out << "spawn";
	default: return out << "unknown";
	}
}

OverlaySection::OverlaySection() {
}

OverlaySection::~OverlaySection() {
}

std::string OverlaySection::getPrettyName() const {
	if (isGlobal())
		return "Global " + getSectionType() + " section";
	return util::capitalize(getSectionType()) + " section '" + getSectionName() + "'";
}

void OverlaySection::dump(std::ostream& out) const {
	out << getPrettyName() << ":" << std::endl;
	out << "  name = " << name << std::endl;
}

std::string OverlaySection::getID() const {
	return getSectionName();
}

std::string OverlaySection::getName() const {
	return name.getValue();
}

OverlayType OverlaySection::getType() const {
	std::string section_name = getSectionType();
	section_name = section_name.substr(std::string("section-").size());
	return util::as<OverlayType>(section_name);
}

bool OverlaySection::isBase() const {
	return base.getValue();
}

void OverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	name.setDefault(getSectionName());
}

bool OverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "name") {
		name.load(key, value, validation);
	} else if (key == "base") {
		base.load(key, value, validation);
	} else {
		return false;
	}
	return true;
}

void OverlaySection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	base.setDefault(getType() == OverlayType::LIGHTING);
}

std::ostream& operator<<(std::ostream& out, const HeightColor& color) {
	return out << color.y << ":" << color.color;
}

AreaOverlaySection::AreaOverlaySection()
	: area(mc::Area::dummy()) {
}

void AreaOverlaySection::dump(std::ostream& out) const {
	OverlaySection::dump(out);
	out << "  color = " << color << std::endl;
	out << "  min_y = " << min_y << std::endl;
	out << "  max_y = " << max_y << std::endl;
	out << "  min_x = " << min_x << std::endl;
	out << "  max_x = " << max_x << std::endl;
	out << "  min_z = " << min_z << std::endl;
	out << "  max_z = " << max_z << std::endl;
	out << "  center_x = " << center_x << std::endl;
	out << "  center_z = " << center_z << std::endl;
	out << "  radius = " << radius << std::endl;
}

const util::Color& AreaOverlaySection::getColor() const {
	return color.getValue();
}

const mc::Area& AreaOverlaySection::getArea() const {
	return area;
}

void AreaOverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::preParse(section, validation);

	color.setDefault(util::Color(255, 228, 0, 85));
}

bool AreaOverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (OverlaySection::parseField(key, value, validation))
		return true;
	if (key == "color")
		color.load(key, value, validation);
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

void AreaOverlaySection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::postParse(section, validation);
	
	bool rectangular = min_x.hasAnyValue() || max_x.hasAnyValue() || min_z.hasAnyValue() || max_z.hasAnyValue();
	bool circular = center_x.hasAnyValue() || center_z.hasAnyValue() || radius.hasAnyValue();

	if (rectangular && circular) {
		validation.error("You can not use both area types at the same time!");
		return;
	} else if (rectangular) {
		if (min_x.hasAnyValue() && max_x.hasAnyValue() && min_x.getValue() > max_x.getValue())
			validation.error("min_x must be smaller than or equal to max_x!");
		if (min_z.hasAnyValue() && max_z.hasAnyValue() && min_z.getValue() > max_z.getValue())
			validation.error("min_z must be smaller than or equal to max_z!");
		util::Interval1D<int> x = intervalFromFields(min_x, max_x);
		util::Interval1D<int> z = intervalFromFields(min_z, max_z);
		area = mc::Area::rectangular(x, z);
	} else if (circular) {
		std::string message = "You have to specify center_x, center_z "
				"and radius for a circular area!";
		center_x.require(validation, message)
			&& center_z.require(validation, message)
			&& radius.require(validation, message);

		area = mc::Area::circular(mc::BlockPos(center_x.getValue(), center_z.getValue(), 0), radius.getValue());
	}

	area = area.withYBounding(intervalFromFields(min_y, max_y));
}

void HeightOverlaySection::dump(std::ostream& out) const {
	OverlaySection::dump(out);
	out << "  default_opacity = " << default_opacity << std::endl;
	out << "  colors = " << colors << std::endl;
}

int HeightOverlaySection::getDefaultOpacity() const {
	return default_opacity.getValue();
}

const std::vector<HeightColor> HeightOverlaySection::getColors() const {
	return colors_vector;
}

void HeightOverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::preParse(section, validation);

	default_opacity.setDefault(85);
}

bool HeightOverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (OverlaySection::parseField(key, value, validation))
		return true;
	if (key == "default_opacity") {
		default_opacity.load(key, value, validation);
	} else if (key == "colors") {
		colors.load(key, value, validation);
	} else {
		return false;
	}
	return true;
}

void HeightOverlaySection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::postParse(section, validation);

	// TODO error handling
	colors_vector.clear();
	std::stringstream ss(colors.getValue());
	std::string str;
	while (ss >> str) {
		if (str.empty())
			continue;
		colors_vector.push_back(util::as<HeightColor>(str));
	}
}

void LightingOverlaySection::dump(std::ostream& out) const {
	OverlaySection::dump(out);
	out << "  day = " << day << std::endl;
	out << "  intensity = " << intensity << std::endl;
	out << "  water_intensity = " << water_intensity << std::endl;
}

bool LightingOverlaySection::isDay() const {
	return day.getValue();
}

double LightingOverlaySection::getIntensity() const {
	return intensity.getValue();
}

double LightingOverlaySection::getWaterIntensity() const {
	return water_intensity.getValue();
}

void LightingOverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::preParse(section, validation);

	day.setDefault(true);
	intensity.setDefault(1.0);
	water_intensity.setDefault(1.0);
}

bool LightingOverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (OverlaySection::parseField(key, value, validation))
		return true;

	if (key == "day") {
		day.load(key, value, validation);
	} else if (key == "intensity") {
		intensity.load(key, value, validation);
	} else if (key == "water_intensity") {
		water_intensity.load(key, value, validation);
	} else {
		return false;
	}
	return true;
}

void LightLevelOverlaySection::dump(std::ostream& out) const {
	OverlaySection::dump(out);
	out << "  day = " << day << std::endl;
}

bool LightLevelOverlaySection::isDay() const {
	return day.getValue();
}

void LightLevelOverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::preParse(section, validation);
	day.setDefault(true);
}

bool LightLevelOverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (OverlaySection::parseField(key, value, validation))
		return true;

	if (key == "day") {
		day.load(key, value, validation);
	} else {
		return false;
	}
	return true;
}

void SlimeOverlaySection::dump(std::ostream& out) const {
	OverlaySection::dump(out);
	out << "  color = " << color << std::endl;
	out << "  opacity = " << opacity << std::endl;
}

void SlimeOverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::preParse(section, validation);

	color.setDefault(util::Color(60, 200, 20));
	opacity.setDefault(85);
}

bool SlimeOverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (OverlaySection::parseField(key, value, validation))
		return true;

	if (key == "color") {
		color.load(key, value, validation);
	} else if (key == "opacity") {
		if (opacity.load(key, value, validation) && (getOpacity() <= 0 || getOpacity() > 255)) {
			validation.error("'opacity' must be an integer between 0 and 255!");
		}
	} else {
		return false;
	}
	return true;
}

util::Color SlimeOverlaySection::getColor() const {
	return color.getValue();
}

int SlimeOverlaySection::getOpacity() const {
	return opacity.getValue();
}

void SpawnOverlaySection::dump(std::ostream& out) const {
	OverlaySection::dump(out);
	out << "  day = " << day << std::endl;
}

bool SpawnOverlaySection::isDay() const {
	return day.getValue();
}

util::Color SpawnOverlaySection::getColor() const {
	return color.getValue();
}

void SpawnOverlaySection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	OverlaySection::preParse(section, validation);
	day.setDefault(true);
	color.setDefault(util::Color(255, 0, 0, 85));
}

bool SpawnOverlaySection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (OverlaySection::parseField(key, value, validation))
		return true;

	if (key == "day") {
		day.load(key, value, validation);
	} else if (key == "color") {
		color.load(key, value, validation);
	} else {
		return false;
	}
	return true;
}

} /* namespace config */
} /* namespace mapcrafter */
