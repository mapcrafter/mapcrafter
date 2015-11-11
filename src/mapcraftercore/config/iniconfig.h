/*
 * Copyright 2012-2015 Moritz Hilscher
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

#ifndef INICONFIG_H
#define INICONFIG_H

#include "../util.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace mapcrafter {
namespace config {

class INIConfigError : public std::runtime_error {
public:
	INIConfigError(const std::string& what)
		: std::runtime_error(what) {}
};

typedef std::pair<std::string, std::string> INIConfigEntry;

class INIConfigSection {
public:
	INIConfigSection(const std::string& type = "", const std::string& name = "");
	~INIConfigSection();
	
	const std::string& getType() const;
	const std::string& getName() const;
	std::string getNameType() const;

	bool isNamed() const;
	bool isEmpty() const;

	bool has(const std::string& key) const;
	
	std::string get(const std::string& key,
			const std::string& default_value = "") const;

	template <typename T>
	T get(const std::string& key, T default_value = T()) const;
	
	const std::vector<INIConfigEntry>& getEntries() const;

	INIConfigSection& set(const std::string& key, const std::string& value);
	INIConfigSection& remove(const std::string& key);

private:
	std::string type, name;

	std::vector<INIConfigEntry> entries;

	int getEntryIndex(const std::string& key) const;
};

std::ostream& operator<<(std::ostream& out, const INIConfigSection& section);

class INIConfig {
public:
	INIConfig();
	~INIConfig();

	void load(std::istream& in);
	void loadFile(const std::string& filename);
	void loadString(const std::string& str);

	void write(std::ostream& out) const;
	void writeFile(const std::string& filename) const;

	bool hasSection(const std::string& type, const std::string& name) const;

	const INIConfigSection& getRootSection() const;
	INIConfigSection& getRootSection();

	const INIConfigSection& getSection(const std::string& type,
			const std::string& name) const;
	INIConfigSection& getSection(const std::string& type, const std::string& name);

	const std::vector<INIConfigSection>& getSections() const;

	void removeSection(const std::string& type, const std::string& name);

private:
	INIConfigSection root;
	std::vector<INIConfigSection> sections;

	INIConfigSection empty_section;

	int getSectionIndex(const std::string& type, const std::string& name) const;
};

template <typename T>
T INIConfigSection::get(const std::string& key, T default_value) const {
	if (has(key))
		return util::as<T>(get(key));
	return default_value;
}

} /* namespace config */
} /* namespace mapcrafter */

#endif /* INICONFIG_H */
