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

#ifndef VALIDATION_H_
#define VALIDATION_H_

#include "../util.h"

#include <iostream>
#include <map>
#include <type_traits>
#include <vector>

namespace mapcrafter {
namespace config {

/**
 * This class represents a so called 'validation message'.
 *
 * A validation message is a string message with a specified type/severity
 * (information, warning, error).
 */
class ValidationMessage {
public:
	ValidationMessage(int type = -1, const std::string& message = "");
	~ValidationMessage();

	/**
	 * Returns the type of the message. One of ValidationMessage::INFO/WARNING/ERROR.
	 */
	int getType() const;

	/**
	 * Returns the actual message.
	 */
	const std::string& getMessage() const;

	/**
	 * Creates an information validation message object from a string message.
	 */
	static ValidationMessage info(const std::string& message);

	/**
	 * Creates a warning validation message object from a string message.
	 */
	static ValidationMessage warning(const std::string& message);

	/**
	 * Creates an error validation message object from a string message.
	 */
	static ValidationMessage error(const std::string& message);

	// different possible types
	static const int INFO = 0;
	static const int WARNING = 1;
	static const int ERROR = 2;

private:
	int type;
	std::string message;
};

std::ostream& operator<<(std::ostream& out, const ValidationMessage& msg);

/**
 * This class represents a so called 'validation list'.
 *
 * A validation list is a list consisting of validation messages. Named validation lists
 * are used as sections in validation maps.
 */
class ValidationList {
public:
	ValidationList();
	~ValidationList();

	/**
	 * Adds a validation message to the list.
	 */
	void message(const ValidationMessage& message);

	/**
	 * Adds an info message to the list.
	 */
	void info(const std::string& message);

	/**
	 * Adds a warning message to the list.
	 */
	void warning(const std::string& message);

	/**
	 * Adds an error message to the list.
	 */
	void error(const std::string& message);

	/**
	 * Returns whether this validation list is empty, i.e. does not contain any messages.
	 */
	bool isEmpty() const;

	/**
	 * Returns whether this validation list is critical, i.e. contains at least one
	 * error message.
	 */
	bool isCritical() const;

	/**
	 * Returns a list with all contained validation messages.
	 */
	const std::vector<ValidationMessage> getMessages() const;

private:
	std::vector<ValidationMessage> messages;
};

/**
 * This class represents a so called 'validation map'.
 *
 * A validation map consists of named, ordered validation sections which hold lists of
 * validation messages.
 */
class ValidationMap {
public:
	ValidationMap();
	~ValidationMap();

	/**
	 * Returns a reference to the validation section with a specific name.
	 */
	ValidationList& section(const std::string& section);

	/**
	 * Returns all validation sections.
	 */
	const std::vector<std::pair<std::string, ValidationList>>& getSections() const;

	/**
	 * Returns if this validation map is empty, i.e. does not contain any validation
	 * sections or if all validation sections are also empty
	 * (= no validation messages at all).
	 */
	bool isEmpty() const;

	/**
	 * Returns if this validation map contains a critical (error-) validation message.
	 */
	bool isCritical() const;

	/**
	 * Sends this validation map with all (not empty) validation sections to the log.
	 */
	void log(std::string logger = "default") const;

private:
	// stores indices for validation sections in sections array
	// (section name -> index in array)
	std::map<std::string, int> sections_order;
	// validation sections with name -> validation section
	std::vector<std::pair<std::string, ValidationList>> sections;
};

template <typename T>
class Field {
public:
	Field(T default_value) : default_value(default_value), user_loaded(false), default_loaded(true) {}
	Field() : user_loaded(false), default_loaded(false) {}
	~Field() {}

	/**
	 * Whether the field has a default or a user value set.
	 */
	bool hasAnyValue() const;
	
	/**
	 * Whether the field has a user value set (via. load or setValue).
	 */
	bool hasUserValue() const;
	
	/**
	 * Gets the user value of the field.
	 */
	T getValue() const;

	/**
	 * Sets the user value of the field.
	 */
	void setValue(T value);

	/**
	 * Tries to load/parse the value of the field.
	 * Uses the util::as function to convert the string value to the type of this field.
	 * Returns false if this function threw an std::invalid_argument exception
	 * and adds an error message to the validation list.
	 */
	bool load(const std::string& key, const std::string& value,
			ValidationList& validation);

	/**
	 * Whether the field has a default value set (via. setValue).
	 */
	bool hasDefaultValue() const;

	/**
	 * Sets the default value of the field.
	 */
	void setDefault(T value);

	/**
	 * Checks if the field has a value set (default or user) and adds an error to the
	 * validation list if not.
	 */
	bool require(ValidationList& validation, std::string message) const;

protected:
	T user_value, default_value;
	bool user_loaded, default_loaded;
};

static std::string ROTATION_NAMES[4] = {"top-left", "top-right", "bottom-right", "bottom-left"};
static std::string ROTATION_NAMES_SHORT[4] = {"tl", "tr", "br", "bl"};

int stringToRotation(const std::string& rotation, std::string names[4] = ROTATION_NAMES);

template <typename T>
bool Field<T>::hasAnyValue() const {
	return user_loaded || default_loaded;
}

template <typename T>
bool Field<T>::hasUserValue() const {
	return user_loaded;
}

template <typename T>
T Field<T>::getValue() const {
	return user_loaded ? user_value : default_value;
}

template <typename T>
void Field<T>::setValue(T value) {
	user_value = value;
	user_loaded = true;
}

template <typename T>
bool Field<T>::load(const std::string& key, const std::string& value,
		ValidationList& validation) {
	try {
		setValue(util::as<T>(value));
		return true;
	} catch (std::invalid_argument& e) {
		validation.error("Invalid value for '" + key + "': " + e.what());
	}
	return false;
}

template <typename T>
bool Field<T>::hasDefaultValue() const {
	return default_loaded;
}

template <typename T>
void Field<T>::setDefault(T value) {
	default_value = value;
	default_loaded = true;
}

template <typename T>
bool Field<T>::require(ValidationList& validation, std::string message) const {
	if (!hasAnyValue()) {
		validation.error(message);
		return false;
	}
	return true;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, Field<T> field) {
	if (field.hasAnyValue())
		out << util::str(field.getValue());
	else
		out << "<not specified>";
	return out;
}

} /* namespace config */
} /* namespace mapcrafter */
#endif /* VALIDATION_H_ */
