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

#ifndef SECTIONS_MARKER_H
#define SECTIONS_MARKER_H

#include "base.h"
#include "../validation.h"
#include "../../mc/worldhelper.h"

namespace mapcrafter {
namespace config {

class MarkerSection : public ConfigSectionBase {
public:
	MarkerSection(bool global = false);
	~MarkerSection();

	virtual void preParse(const INIConfigSection& section,
				ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

	std::string getPrefix() const;
	std::string getTitleFormat() const;
	std::string getTextFormat() const;
	std::string getIcon() const;
	std::string getIconSize() const;

	bool matchesSign(const mc::Sign& sign) const;
	std::string formatTitle(const mc::Sign& sign) const;
	std::string formatText(const mc::Sign& sign) const;
private:
	Field<std::string> prefix;
	Field<std::string> title_format, text_format;
	Field<std::string> icon, icon_size;

	std::string formatSign(std::string format, const mc::Sign& sign) const;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_MARKER_H */
