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

#include "logging.h"

namespace mapcrafter {
namespace util {

LogLevel LogLevelHelper::levelFromString(const std::string& str) {
	if (str == "EMERGENCY")
		return LogLevel::EMERGENCY;
	if (str == "ALERT")
		return LogLevel::ALERT;
	if (str == "ERROR")
		return LogLevel::ERROR;
	if (str == "WARNING")
		return LogLevel::WARNING;
	if (str == "NOTICE")
		return LogLevel::NOTICE;
	if (str == "INFO")
		return LogLevel::INFO;
	if (str == "DEBUG")
		return LogLevel::DEBUG;
	return LogLevel::UNKNOWN;
}

std::string LogLevelHelper::levelToString(LogLevel level) {
	if (level == LogLevel::EMERGENCY)
		return "EMERGENCY";
	if (level == LogLevel::ALERT)
		return "ALERT";
	if (level == LogLevel::ERROR)
		return "ERROR";
	if (level == LogLevel::WARNING)
		return "WARNING";
	if (level == LogLevel::NOTICE)
		return "NOTICE";
	if (level == LogLevel::INFO)
		return "INFO";
	if (level == LogLevel::DEBUG)
		return "DEBUG";
	return "UNKNOWN";
}

LogStream::LogStream(LogLevel level, const std::string& logger,
		const std::string& file, int line)
	: level(level), logger(logger), file(file), line(line), ss(new std::stringstream) {
}

LogStream::~LogStream() {
	std::string filename = file;
	if (filename.find_last_of('/') != std::string::npos)
		filename = filename.substr(filename.find_last_of('/') + 1);
	std::cout << "[" << logger << ":" << filename << ":" << line << "]";
	std::cout << " [" << LogLevelHelper::levelToString(level) << "] " << ss->str() << std::endl;
}

std::map<std::string, Logger*> Logger::loggers;

Logger::Logger(const std::string& name)
	: name(name) {
}

Logger::~Logger() {
}

LogStream Logger::log(LogLevel level, const std::string& file, int line) {
	return LogStream(level, name, file, line);
}

Logger* Logger::getLogger(const std::string& name) {
	if (!loggers.count(name))
		loggers[name] = new Logger(name);
	return loggers.at(name);
}

} /* namespace util */
} /* namespace mapcrafter */
