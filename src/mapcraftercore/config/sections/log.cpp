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

#include "log.h"

namespace mapcrafter {
namespace util {

template<>
config::LogSinkType as<config::LogSinkType>(const std::string& from) {
	if (from == "output")
		return config::LogSinkType::OUTPUT;
	else if (from == "file")
		return config::LogSinkType::FILE;
	else if (from == "syslog")
		return config::LogSinkType::SYSLOG;
	throw std::invalid_argument("Must be 'output', 'file' or 'syslog'!");
}

template<>
util::LogLevel as<util::LogLevel>(const std::string& from) {
	util::LogLevel level = util::LogLevelHelper::levelFromString(from);
	if (level != util::LogLevel::UNKNOWN)
		return level;
	throw std::invalid_argument("Must be 'EMERGENCY', 'ALERT', 'FATAL', 'ERROR', 'WARNING', 'NOTICE', 'INFO' or 'DEBUG'!");
}

}
}

namespace mapcrafter {
namespace config {

LogSection::LogSection(bool global) {
	setGlobal(global);
}

LogSection::~LogSection() {
}

std::string LogSection::getPrettyName() const {
	if (isGlobal())
		return "global log section " + getSectionName();
	return "log section '" + getSectionName() + "'";
}

LogSinkType LogSection::getType() const {
	return type.getValue();
}

util::LogLevel LogSection::getVerbosity() const {
	return verbosity.getValue();
}

bool LogSection::getLogProgress() const {
	return log_progress.getValue();
}

std::string LogSection::getFormat() const {
	return format.getValue();
}

std::string LogSection::getDateFormat() const {
	return date_format.getValue();
}

fs::path LogSection::getFile() const {
	return file.getValue();
}

void LogSection::preParse(const INIConfigSection& section,
		ValidationList& validation) {
	verbosity.setDefault(util::LogLevel::INFO);
	format.setDefault("%(date) [%(level)] [%(logger)] %(message)");
	date_format.setDefault("%F %T");
}

bool LogSection::parseField(const std::string key, const std::string value,
		ValidationList& validation) {
	if (key == "type")
		type.load(key, value, validation);
	else if (key == "verbosity")
		verbosity.load(key, value, validation);
	else if (key == "log_progress")
		log_progress.load(key, value, validation);
	else if (key == "format")
		format.load(key, value, validation);
	else if (key == "date_format")
		date_format.load(key, value, validation);
	else if (key == "file")
		file.load(key, value, validation);
	else
		return false;
	return true;
}

void LogSection::postParse(const INIConfigSection& section,
		ValidationList& validation) {
	log_progress.setDefault(type.getValue() == LogSinkType::FILE
			|| type.getValue() == LogSinkType::SYSLOG);
}

} /* namespace config */
} /* namespace mapcrafter */
