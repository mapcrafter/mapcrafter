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

#ifndef SECTIONS_MARKER_H
#define SECTIONS_MARKER_H

#include "../configsection.h"

#include "../validation.h"
#include "../../mc/worldentities.h"

namespace mapcrafter {
namespace config {

class MarkerSection : public ConfigSection {
public:
	MarkerSection();
	~MarkerSection();

	virtual std::string getPrettyName() const;
	virtual void dump(std::ostream& out) const;

	std::string getShortName() const;
	std::string getLongName() const;
	std::string getPrefix() const;
	std::string getPostfix() const;
	std::string getTitleFormat() const;
	std::string getTextFormat() const;
	std::string getIcon() const;
	std::string getIconSize() const;
	bool isMatchedEmpty() const;
	bool isShownByDefault() const;

	bool matchesSign(const mc::SignEntity& sign) const;
	std::string formatTitle(const mc::SignEntity& sign) const;
	std::string formatText(const mc::SignEntity& sign) const;

protected:
	virtual void preParse(const INIConfigSection& section,
				ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	Field<std::string> name_long;
	Field<std::string> prefix, postfix;
	Field<std::string> title_format, text_format;
	Field<std::string> icon, icon_size;
	Field<bool> match_empty, show_default;

	std::string formatSign(std::string format, const mc::SignEntity& sign) const;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_MARKER_H */
