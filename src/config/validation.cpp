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

#include "validation.h"

namespace mapcrafter {
namespace config {

ValidationMessage::ValidationMessage(int type, const std::string& message)
		: type(type), message(message) {
}

ValidationMessage::~ValidationMessage() {
}

int ValidationMessage::getType() const {
	return type;
}

const std::string& ValidationMessage::getMessage() const {
	return message;
}

ValidationMessage ValidationMessage::info(const std::string& message) {
	return ValidationMessage(INFO, message);
}

ValidationMessage ValidationMessage::warning(const std::string& message) {
	return ValidationMessage(WARNING, message);
}

ValidationMessage ValidationMessage::error(const std::string& message) {
	return ValidationMessage(ERROR, message);
}

std::ostream& operator<<(std::ostream& out, const ValidationMessage& msg) {
	switch (msg.getType()) {
		case (ValidationMessage::INFO): out << "Info: "; break;
		case (ValidationMessage::WARNING): out << "Warning: "; break;
		case (ValidationMessage::ERROR): out << "Error: "; break;
		default: out << msg.getType(); break;
	}
	out << msg.getMessage();
	return out;
}

ValidationList makeValidationList(const ValidationMessage& msg) {
	ValidationList validation;
	validation.push_back(msg);
	return validation;
}

bool isValidationValid(const ValidationList& validation) {
	for (auto it = validation.begin(); it != validation.end(); ++it)
		if (it->getType() == ValidationMessage::ERROR)
			return false;
	return true;
}

int stringToRotation(const std::string& rotation, std::string names[4]) {
	for (int i = 0; i < 4; i++)
		if (rotation == names[i])
			return i;
	return -1;
}

} /* namespace config2 */
} /* namespace mapcrafter */
