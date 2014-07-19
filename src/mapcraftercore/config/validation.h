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

#ifndef VALIDATION_H_
#define VALIDATION_H_

#include "iniconfig.h"

#include <map>
#include <type_traits>
#include <vector>

namespace mapcrafter {
namespace config {

class ValidationMessage {
private:
	int type;
	std::string message;
public:
	ValidationMessage(int type = -1, const std::string& message = "");
	~ValidationMessage();

	int getType() const;
	const std::string& getMessage() const;

	static ValidationMessage info(const std::string& message);
	static ValidationMessage warning(const std::string& message);
	static ValidationMessage error(const std::string& message);

	static const int INFO = 0;
	static const int WARNING = 1;
	static const int ERROR = 2;
};

std::ostream& operator<<(std::ostream& out, const ValidationMessage& msg);

//typedef std::vector<ValidationMessage> ValidationList;

class ValidationList {
public:
	ValidationList();
	~ValidationList();

	void message(const ValidationMessage& message);
	void info(const std::string& message);
	void warning(const std::string& message);
	void error(const std::string& message);

	bool empty() const;
	bool isCritical() const;

	const std::vector<ValidationMessage> getMessages() const;

private:
	std::vector<ValidationMessage> messages;
};

class ValidationMap {
public:
	ValidationMap();
	~ValidationMap();

	ValidationList& section(const std::string& section);

	bool empty() const;
	bool isCritical() const;

	void log(std::string logger = "default") const;

private:
	std::vector<std::string> sections_order;
	std::map<std::string, ValidationList> sections;
};

//typedef std::vector<std::pair<std::string, ValidationList > > ValidationMap;

template <typename T>
class Field {
private:
	T value;
	bool loaded;
public:
	Field(T value = T()) : value(value), loaded(false) {}
	~Field() {}

	/**
	 * Sets the default value of a configuration option.
	 */
	void setDefault(T value) {
		// do not overwrite an already loaded value with a default value
		if (!loaded) {
			this->value = value;
			loaded = true;
		}
	}

	/**
	 * Tries to load/parse the value of a configuration option.
	 * Uses the util::as function to convert the string value to the type of this field.
	 * Returns false if this function threw an std::invalid_argument exception
	 * and adds an error message to the validation list.
	 */
	bool load(const std::string& key, const std::string& value, ValidationList& validation) {
		try {
			this->value = util::as<T>(value);
			loaded = true;
			return true;
		} catch (std::invalid_argument& e) {
			validation.error("Invalid value for '" + key + "': " + e.what());
		}
		return false;
	}

	/**
	 * Checks if the configuration option was specified and adds an error to the
	 * validation list if not.
	 */
	bool require(ValidationList& validation, std::string message) const {
		if (!loaded) {
			validation.error(message);
			return false;
		}
		return true;
	}

	T getValue() const { return value; }
	void setValue(T value) { this->value = value; }

	bool isLoaded() const { return loaded; }
};

static std::string ROTATION_NAMES[4] = {"top-left", "top-right", "bottom-right", "bottom-left"};
static std::string ROTATION_NAMES_SHORT[4] = {"tl", "tr", "br", "bl"};

int stringToRotation(const std::string& rotation, std::string names[4] = ROTATION_NAMES);

} /* namespace config */
} /* namespace mapcrafter */
#endif /* VALIDATION_H_ */
