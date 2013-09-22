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

#ifndef VALIDATION_H_
#define VALIDATION_H_

#include "extended_ini.h"

#include <map>
#include <type_traits>

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

	static ValidationMessage info(const std::string& message) {
		return ValidationMessage(INFO, message);
	}
	static ValidationMessage warning(const std::string& message) {
		return ValidationMessage(WARNING, message);
	}
	static ValidationMessage error(const std::string& message) {
		return ValidationMessage(ERROR, message);
	}

	static const int INFO = 0;
	static const int WARNING = 1;
	static const int ERROR = 2;
};

std::ostream& operator<<(std::ostream& out, const ValidationMessage& msg);

typedef std::vector<ValidationMessage> ValidationList;
typedef std::vector<std::pair<std::string, ValidationList > > ValidationMap;

ValidationList makeValidationList(const ValidationMessage& msg);
bool isValidationValid(const ValidationList& validation);

template <typename T>
class Field {
private:
	T value;
	bool loaded;
	bool valid;
public:
	Field(T value = T()) : value(value), loaded(false), valid(false) {}
	~Field() {}

	bool load(const ConfigSection& section, const std::string& key) {
		if (section.has(key)) {
			value = section.get<T>(key);
			loaded = true;
			valid = true;
			return true;
		}
		return false;
	}

	bool load(const ConfigSection& section, const std::string& key, T default_value) {
		if (loaded && !section.has(key))
			return false;
		value = section.get<T>(key, default_value);
		loaded = true;
		valid = true;
		return loaded;
	}

	bool require(ValidationList& validation, std::string message) const {
		if (!loaded) {
			validation.push_back(ValidationMessage::error(message));
			return false;
		}
		return true;
	}

	T getValue() const { return value; }
	bool isLoaded() const { return loaded; }
	bool isValid() const { return valid; }
};

static std::string ROTATION_NAMES[4] = {"top-left", "top-right", "bottom-right", "bottom-left"};
static std::string ROTATION_NAMES_SHORT[4] = {"tl", "tr", "br", "bl"};

int stringToRotation(const std::string& rotation, std::string names[4] = ROTATION_NAMES);

} /* namespace config2 */
} /* namespace mapcrafter */
#endif /* VALIDATION_H_ */
