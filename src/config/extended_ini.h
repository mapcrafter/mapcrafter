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

#ifndef EXTENDED_CONFIG_H_
#define EXTENDED_CONFIG_H_

#include "../util.h"

#include <iostream>
#include <string>

namespace mapcrafter {
namespace config2 {

class ValidationMessage {
private:
	int type;
	std::string message;
public:
	ValidationMessage(int type = -1, const std::string& message = "")
		: type(type), message(message) {}
	~ValidationMessage() {}

	int getType() const { return type; }
	const std::string& getMessage() const { return message; };

	std::ostream& operator<<(std::ostream& out) const {
		out << type << ": " << message;
		return out;
	}

	static const int INFO = 0;
	static const int WARNING = 1;
	static const int ERROR = 2;
};

typedef std::vector<ValidationMessage> ValidationMessages;

class ConfigSection {
private:
	std::string type;
	std::string name;

	std::vector<std::pair<std::string, std::string> > entries;

	int getEntryIndex(const std::string& key) const;
public:
	ConfigSection(const std::string& type = "", const std::string& name = "")
		: type(type), name(name) {}
	~ConfigSection() {}
	
	const std::string& getType() const { return type; }
	const std::string& getName() const { return name; }

	bool has(const std::string& key) const;
	
	std::string get(const std::string& key, const std::string& default_value = "") const;
	template<typename T>
	std::string get(const std::string& key, const T& default_value = T()) const {
		if (has(key))
			return util::as<T>(get(key));
		return default_value;
	}
	
	void set(const std::string& key, const std::string& value);
	void remove(const std::string& key);
};

class ConfigFile {
private:
	std::vector<ConfigSection> sections;

	ConfigSection empty_section;

	int getSectionIndex(const std::string& type, const std::string& name) const;
public:
	ConfigFile() {}
	~ConfigFile() {}

	bool load(std::istream& in, ValidationMessage& msg);
	bool load(std::istream& in) {
		ValidationMessage msg;
		return load(in, msg);
	}
	bool loadFile(const std::string& filename, ValidationMessage& msg);
	bool loadFile(const std::string& filename) {
		ValidationMessage msg;
		return loadFile(filename, msg);
	}

	bool write(std::ostream& out) const;
	bool writeFile(const std::string& filename) const;
	
	const ConfigSection& getSection(const std::string& type, const std::string& name) const;
	ConfigSection& getSection(const std::string& type, const std::string& name);

	ConfigSection& addSection(const std::string& type, const std::string& name);
	void removeSection(const std::string& type, const std::string& name);
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* EXTENDED_CONFIG_H_ */
