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

#include "validation.h"

namespace mapcrafter {
namespace config {

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
