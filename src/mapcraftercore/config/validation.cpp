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

ValidationList::ValidationList() {

}

ValidationList::~ValidationList() {

}

void ValidationList::message(const ValidationMessage& message) {
	messages.push_back(message);
}

void ValidationList::info(const std::string& message) {
	messages.push_back(ValidationMessage(ValidationMessage::INFO, message));
}

void ValidationList::warning(const std::string& message) {
	messages.push_back(ValidationMessage(ValidationMessage::WARNING, message));
}

void ValidationList::error(const std::string& message) {
	messages.push_back(ValidationMessage(ValidationMessage::ERROR, message));
}

bool ValidationList::empty() const {
	return messages.size() == 0;
}

bool ValidationList::isCritical() const {
	for (auto message_it = messages.begin(); message_it != messages.end(); ++message_it)
		if (message_it->getType() == ValidationMessage::ERROR)
			return true;
	return false;
}

const std::vector<ValidationMessage> ValidationList::getMessages() const {
	return messages;
}

ValidationMap::ValidationMap() {

}

ValidationMap::~ValidationMap() {

}

ValidationList& ValidationMap::section(const std::string& section) {
	if (!sections.count(section))
		sections_order.push_back(section);
	return sections[section];
}

bool ValidationMap::empty() const {
	for (auto section_it = sections.begin(); section_it != sections.end(); ++section_it)
		if (!section_it->second.empty())
			return false;
	return true;
}

bool ValidationMap::isCritical() const {
	for (auto section_it = sections.begin(); section_it != sections.end(); ++section_it)
		if (section_it->second.isCritical())
			return true;
	return false;
}

void ValidationMap::log(std::string logger) const {
	for (auto section_it = sections_order.begin(); section_it != sections_order.end(); ++section_it) {
		auto messages = sections.at(*section_it).getMessages();
		if (messages.empty())
			continue;
		LOGN(WARNING, logger) << *section_it << ":";
		for (auto message_it = messages.begin(); message_it != messages.end(); ++message_it)
			LOGN(WARNING, logger) << " - " << *message_it;
	}
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
	validation.message(msg);
	return validation;
}

bool isValidationValid(const ValidationList& validation) {
	auto messages = validation.getMessages();
	for (auto message_it = messages.begin(); message_it != messages.end(); ++message_it)
		if (message_it->getType() == ValidationMessage::ERROR)
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
