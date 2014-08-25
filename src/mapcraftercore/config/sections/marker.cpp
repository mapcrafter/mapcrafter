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

#include <vector>

namespace mapcrafter {
namespace config {

MarkerSection::MarkerSection() {
}

MarkerSection::~MarkerSection() {
}

std::string MarkerSection::getPrettyName() const {
	if (isGlobal())
		return "Global marker section";
	return "Marker section '" + getSectionName() + "'";
}

void MarkerSection::dump(std::ostream& out) const {
	out << getPrettyName() << ":" << std::endl;
	out << "  name = " << getLongName() << std::endl;
	out << "  prefix = " << prefix << std::endl;
	out << "  title_format = " << title_format << std::endl;
	out << "  text_format = " << text_format << std::endl;
	out << "  icon = " << icon << std::endl;
	out << "  icon_size = " << icon_size << std::endl;
	out << "  match_empty = " << match_empty << std::endl;
	out << "  show_default = " << show_default << std::endl;
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

bool MarkerSection::isMatchedEmpty() const {
	return match_empty.getValue();
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

void MarkerSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	name_long.setDefault(getSectionName());
	title_format.setDefault("%(text)");
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

	// check if the old placeholders are used, just search for %placeholder
	// they are still supported, but show a warning
	std::vector<std::string> placeholders = {
		"text", "textp", "prefix", "line1", "line2", "line3", "line4", "x", "y", "z"
	};
	for (auto it = placeholders.begin(); it != placeholders.end(); ++it) {
		std::string placeholder = "%" + *it;
		if (title_format.getValue().find(placeholder) != std::string::npos
				|| text_format.getValue().find(placeholder) != std::string::npos) {
			validation.warning("It seems you are using the old placeholder format "
					"for 'title_format' or 'text_format'. Please use '%(placeholder)' "
					"instead of '%placeholder'.");
			return;
		}
	}
}

/**
 * Replaces the placeholder in the supplied format string. Specifically replaces %(key)
 * (and also the older, but deprecated version %key) with value.
 */
template <typename T>
void replacePlaceholder(std::string& str, const std::string& key, T value) {
	str = util::replaceAll(str, "%" + key, util::str(value));
	str = util::replaceAll(str, "%(" + key + ")", util::str(value));
}

std::string MarkerSection::formatSign(std::string format, const mc::SignEntity& sign) const {
	// sign text with prefix
	std::string textp = sign.getText();
	// sign text without prefix
	std::string text = textp;
	// remove prefix from sign text
	// but make sure there is also other text except the prefix, otherwise don't remove prefix
	if (textp.size() > prefix.getValue().size())
		text = util::trim(textp.substr(prefix.getValue().size()));

	// replace the placeholders with the sign data
	replacePlaceholder(format, "textp", textp);
	replacePlaceholder(format, "text", text);
	replacePlaceholder(format, "prefix", prefix.getValue());
	replacePlaceholder(format, "line1", sign.getLines()[0]);
	replacePlaceholder(format, "line2", sign.getLines()[1]);
	replacePlaceholder(format, "line3", sign.getLines()[2]);
	replacePlaceholder(format, "line4", sign.getLines()[3]);
	replacePlaceholder(format, "x", sign.getPos().x);
	replacePlaceholder(format, "y", sign.getPos().y);
	replacePlaceholder(format, "z", sign.getPos().z);
	return format;
}

} /* namespace config */
} /* namespace mapcrafter */
