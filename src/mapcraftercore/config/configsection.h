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

#ifndef CONFIGSECTION_H_
#define CONFIGSECTION_H_

#include <string>

#include "validation.h"

namespace mapcrafter {
namespace config {

/**
 * Customized section factory that passes the config directory to the section objects.
 */
template <typename T>
class ConfigDirSectionFactory {
public:
	ConfigDirSectionFactory(fs::path config_dir = "");
	~ConfigDirSectionFactory();

	T operator()() const;

private:
	fs::path config_dir;
};

class INIConfigSection;

class ConfigSection {
public:
	ConfigSection();
	virtual ~ConfigSection();

	/**
	 * Gets/sets whether this is a global section.
	 */
	bool isGlobal() const;
	void setGlobal(bool global);

	/**
	 * Returns the name of the section.
	 */
	std::string getSectionName() const;

	/**
	 * Parses the given configurations section and returns false if there was a critical
	 * parsing/validation error.
	 */
	ValidationList parse(const INIConfigSection& section);

	/**
	 * Returns the pretty name of the section.
	 * For example "Map section my_map" or "Global world section".
	 */
	virtual std::string getPrettyName() const;

	/**
	 * Prints information about the parsed section to a
	 */
	virtual void dump(std::ostream& out) const;

protected:
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

private:
	// whether this is a global section ([global:sections])
	// might change with future versions, [section:my_glob_*] seems to be a good idea, too
	bool global;

	// name of this section
	std::string section_name;

	ValidationList validation;
};

std::ostream& operator<<(std::ostream& out, const ConfigSection& section);

template <typename T>
ConfigDirSectionFactory<T>::ConfigDirSectionFactory(fs::path config_dir)
	: config_dir(config_dir) {
}

template <typename T>
ConfigDirSectionFactory<T>::~ConfigDirSectionFactory() {
}

template <typename T>
T ConfigDirSectionFactory<T>::operator()() const {
	T section;
	section.setConfigDir(config_dir);
	return section;
}

}
}

#endif /* CONFIGSECTION_H_ */
