/*
 * Copyright 2012, 2013 Moritz Hilscher
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

#include "marker.h"

#include "../../util.h"

namespace mapcrafter {
namespace config {

MarkerSection::MarkerSection(bool global) {
	setGlobal(global);
}

MarkerSection::~MarkerSection() {
}

std::string MarkerSection::formatSign(std::string format, const mc::SignEntity& sign) const {
	std::string textp = sign.getText();
	std::string text = textp;

	if (textp.size() > prefix.getValue().size())
		text = util::trim(textp.substr(prefix.getValue().size()));

	format = util::replaceAll(format, "%text", text);
	format = util::replaceAll(format, "%textp", textp);
	format = util::replaceAll(format, "%line0", sign.getLines()[0]);
	format = util::replaceAll(format, "%line1", sign.getLines()[1]);
	format = util::replaceAll(format, "%line2", sign.getLines()[2]);
	format = util::replaceAll(format, "%line3", sign.getLines()[3]);
	return format;
}

void MarkerSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	name.setDefault(section_name);
	title_format.setDefault("%text");
}

bool MarkerSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "name")
		name.load(key, value, validation);
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
	else
		return false;
	return true;
}

void MarkerSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	text_format.setDefault(title_format.getValue());
}

std::string MarkerSection::getName() const {
	return name.getValue();
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

bool MarkerSection::matchesSign(const mc::SignEntity& sign) const {
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
