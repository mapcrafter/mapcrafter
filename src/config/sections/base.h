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

#ifndef SECTIONS_BASE_H_
#define SECTIONS_BASE_H_

#include <string>

#include "../validation.h"

namespace mapcrafter {
namespace config {

class INIConfigSection;

class ConfigSectionBase {
public:
	ConfigSectionBase();
	virtual ~ConfigSectionBase();

	/**
	 * Specifies whether this is a global section.
	 */
	void setGlobal(bool global);

	/**
	 * Parses the given configurations section and adds infos/warnings/errors to the
	 * validation list object.
	 *
	 * Returns false if there was an parsing error.
	 */
	bool parse(const INIConfigSection& section, ValidationList& validation);

	/**
	 * This method is called before parsing the section entries. The method can output
	 * infos/warnings/errors via the validation list object.
	 *
	 * Used to set default configuration options for example.
	 */
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);

	/**
	 * This method is called to parse one configuration entry.
	 *
	 * Should return false if the configuration key is unknown.
	 */
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);

	/**
	 * This method is called after parsing the section entries. The method can output
	 * infos/warnings/errors via the validation list object.
	 *
	 * Used for further validation things for example.
	 */
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

protected:
	// whether this is a global section ([global:sections])
	// might change with future versions, [section:my_glob_*] seems to be a good idea, too
	bool global;

	// name of this section
	std::string section_name;
};

}
}

#endif /* SECTIONS_BASE_H_ */
