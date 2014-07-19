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

#include "marker.h"

#include "../../util.h"

namespace mapcrafter {
namespace config {

MarkerSection::MarkerSection(bool global) {
	setGlobal(global);
}

MarkerSection::~MarkerSection() {
}

void MarkerSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	name_long.setDefault(getSectionName());
	title_format.setDefault("%text");
	match_empty.setDefault(false);
	show_default.setDefault(true);
}

bool MarkerSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "name")
		name_long.load(key, value, validation);
	else if (key == "prefix")
		prefix.load(key, value, validation);
	else if (key == "title_format")
		title_format.load(key, value, validation);
	else if (key == "text_format")
		text_format.load(key, value, validation);
	else if (key == "icon")
		icon.load(key, value, validation);
	else if (key == "icon_size")
		icon_size.load(key, value, validation);
	else if (key == "match_empty")
		match_empty.load(key, value, validation);
	else if (key == "show_default")
		show_default.load(key, value, validation);
	else
		return false;
	return true;
}

void MarkerSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	text_format.setDefault(title_format.getValue());
}


std::string MarkerSection::formatSign(std::string format, const mc::SignEntity& sign) const {
	std::string textp = sign.getText();
	std::string text = textp;

	if (textp.size() > prefix.getValue().size())
		text = util::trim(textp.substr(prefix.getValue().size()));

	format = util::replaceAll(format, "%textp", textp);
	format = util::replaceAll(format, "%text", text);
	format = util::replaceAll(format, "%prefix", prefix.getValue());
	format = util::replaceAll(format, "%line1", sign.getLines()[0]);
	format = util::replaceAll(format, "%line2", sign.getLines()[1]);
	format = util::replaceAll(format, "%line3", sign.getLines()[2]);
	format = util::replaceAll(format, "%line4", sign.getLines()[3]);
	format = util::replaceAll(format, "%x", util::str(sign.getPos().x));
	format = util::replaceAll(format, "%z", util::str(sign.getPos().z));
	format = util::replaceAll(format, "%y", util::str(sign.getPos().y));
	return format;
}

std::string MarkerSection::getShortName() const {
	return getSectionName();
}

std::string MarkerSection::getLongName() const {
	return name_long.getValue();
}

std::string MarkerSection::getPrefix() const {
	return prefix.getValue();
}

std::string MarkerSection::getTitleFormat() const {
	return title_format.getValue();
}

std::string MarkerSection::getTextFormat() const {
	return title_format.getValue();
}

std::string MarkerSection::getIcon() const {
	return icon.getValue();
}

std::string MarkerSection::getIconSize() const {
	return icon_size.getValue();
}

bool MarkerSection::isShownByDefault() const {
	return show_default.getValue();
}

bool MarkerSection::matchesSign(const mc::SignEntity& sign) const {
	if (sign.getText().empty() && !match_empty.getValue())
		return false;
	return util::startswith(sign.getText(), prefix.getValue());
}

std::string MarkerSection::formatTitle(const mc::SignEntity& sign) const {
	return formatSign(title_format.getValue(), sign);
}

std::string MarkerSection::formatText(const mc::SignEntity& sign) const {
	return formatSign(text_format.getValue(), sign);
}

} /* namespace config */
} /* namespace mapcrafter */
