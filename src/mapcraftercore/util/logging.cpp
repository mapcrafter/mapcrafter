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

#include "../util.h"

#include <ctime>

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
	: entry({level, logger, file, line, ""}), ss(new std::stringstream) {
}

LogStream::~LogStream() {
	/*
	std::string filename = file;
	if (filename.find_last_of('/') != std::string::npos)
		filename = filename.substr(filename.find_last_of('/') + 1);
	std::cout << "[" << logger << ":" << filename << ":" << line << "]";
	std::cout << " [" << LogLevelHelper::levelToString(level) << "] " << ss->str() << std::endl;
	*/

	entry.message = ss->str();
	LogManager::getInstance()->handleLogEntry(entry);
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

LogSink::LogSink() {
}

LogSink::~LogSink() {
}

void LogSink::sink(const LogEntry& entry) {
}

FormattedLogSink::FormattedLogSink(std::string format, std::string date_format)
	: format(format), date_format(date_format) {
}

FormattedLogSink::~FormattedLogSink() {
}

std::string FormattedLogSink::formatLogEntry(const LogEntry& entry) {
	std::string formatted = format;

	time_t t = time(nullptr);
	char buffer[256];
	strftime(buffer, sizeof(buffer), date_format.c_str(), localtime(&t));
	formatted = util::replaceAll(formatted, "%(date)", std::string(buffer));

	formatted = util::replaceAll(formatted, "%(level)", LogLevelHelper::levelToString(entry.level));
	formatted = util::replaceAll(formatted, "%(logger)", entry.logger);
	formatted = util::replaceAll(formatted, "%(file)", entry.file);
	formatted = util::replaceAll(formatted, "%(line)", util::str(entry.line));
	formatted = util::replaceAll(formatted, "%(message)", entry.message);
	return formatted;
}

void FormattedLogSink::setFormat(const std::string& format) {
	this->format = format;
}

void FormattedLogSink::sink(const LogEntry& entry) {
	sinkFormatted(entry, formatLogEntry(entry));
}

void FormattedLogSink::sinkFormatted(const LogEntry& entry,
		const std::string& formatted) {
}

LogOutputSink::LogOutputSink(std::string format, std::string date_format)
	: FormattedLogSink(format, date_format) {
}

LogOutputSink::~LogOutputSink() {
}

void LogOutputSink::sinkFormatted(const LogEntry& entry,
		const std::string& formatted) {
	if (entry.level < LogLevel::NOTICE || entry.level == LogLevel::UNKNOWN)
		std::cerr << formatted << std::endl;
	else
		std::cout << formatted << std::endl;
}

LogManager* LogManager::instance = nullptr;

LogManager::LogManager()
	: global_verbosity(LogLevel::INFO) {
	addSink("output", new LogOutputSink("%(date) [%(level)] [%(logger)] %(message)", "%F %T"));
}

LogManager::~LogManager() {
}

void LogManager::setGlobalVerbosity(LogLevel level) {
	global_verbosity = level;
}

void LogManager::setSinkVerbosity(const std::string& sink, LogLevel level) {
	sink_verbosity[sink] = level;
}

LogLevel LogManager::getSinkVerbosity(const std::string& sink) const {
	if (sink_verbosity.count(sink))
		return sink_verbosity.at(sink);
	return global_verbosity;
}

void LogManager::addSink(const std::string& name, LogSink* sink) {
	sinks[name] = std::shared_ptr<LogSink>(sink);
}

void LogManager::handleLogEntry(const LogEntry& entry) {
	for (auto it = sinks.begin(); it != sinks.end(); ++it) {
		if (entry.level <= getSinkVerbosity(it->first))
			(*it->second).sink(entry);
	}
}

LogManager* LogManager::getInstance() {
	if (instance == nullptr)
		instance = new LogManager();
	return instance;
}

} /* namespace util */
} /* namespace mapcrafter */
