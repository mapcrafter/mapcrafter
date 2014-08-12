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
#ifdef HAVE_SYSLOG_H
#  include <syslog.h>
#endif

namespace mapcrafter {
namespace util {

#ifndef HAVE_ENUM_CLASS_COMPARISON

bool operator==(LogLevel level1, LogLevel level2) {
	return (int) level1 == (int) level2;
}

bool operator!=(LogLevel level1, LogLevel level2) {
	return !(level1 == level2);
}

bool operator<(LogLevel level1, LogLevel level2) {
	return (int) level1 < (int) level2;
}

bool operator<=(LogLevel level1, LogLevel level2) {
	return level1 < level2 || level1 == level2;
}

bool operator>(LogLevel level1, LogLevel level2) {
	return (int) level1 > (int) level2;
}

bool operator>=(LogLevel level1, LogLevel level2) {
	return level1 > level2 || level1 == level2;
}

#endif

LogLevel LogLevelHelper::levelFromString(const std::string& str) {
	if (str == "EMERGENCY")
		return LogLevel::EMERGENCY;
	if (str == "ALERT")
		return LogLevel::ALERT;
	if (str == "FATAL")
		return LogLevel::FATAL;
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
	if (level == LogLevel::FATAL)
		return "FATAL";
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

#ifdef HAVE_SYSLOG_H

int LogLevelHelper::levelToSyslog(LogLevel level) {
	if (level == LogLevel::EMERGENCY)
		return LOG_EMERG;
	if (level == LogLevel::ALERT)
		return LOG_ALERT;
	if (level == LogLevel::FATAL)
		return LOG_CRIT;
	if (level == LogLevel::ERROR)
		return LOG_ERR;
	if (level == LogLevel::WARNING)
		return LOG_WARNING;
	if (level == LogLevel::NOTICE)
		return LOG_NOTICE;
	if (level == LogLevel::INFO)
		return LOG_INFO;
	if (level == LogLevel::DEBUG)
		return LOG_DEBUG;
	return LOG_INFO;
}

#endif

std::ostream& operator<<(std::ostream& out, LogLevel level) {
	out << LogLevelHelper::levelToString(level);
	return out;
}

LogStream::LogStream(LogLevel level, const std::string& logger,
		const std::string& file, int line)
	: message({level, logger, file, line, ""}), ss(new std::stringstream) {
	if (message.file.find('/') != std::string::npos)
		message.file = message.file.substr(message.file.find_last_of('/') + 1);
}

LogStream::~LogStream() {
	message.message = ss->str();
	Logging::getInstance().handleLogMessage(message);
}

Logger::Logger(const std::string& name)
	: name(name) {
}

Logger::~Logger() {
}

LogStream Logger::log(LogLevel level, const std::string& file, int line) {
	return LogStream(level, name, file, line);
}

LogSink::LogSink() {
}

LogSink::~LogSink() {
}

void LogSink::sink(const LogMessage& message) {
}

FormattedLogSink::FormattedLogSink(std::string format, std::string date_format)
	: format(format), date_format(date_format) {
}

FormattedLogSink::~FormattedLogSink() {
}

std::string FormattedLogSink::formatLogEntry(const LogMessage& message) {
	std::string formatted = format;

	std::time_t t = std::time(nullptr);
	char buffer[256];
	std::strftime(buffer, sizeof(buffer), date_format.c_str(), std::localtime(&t));
	formatted = util::replaceAll(formatted, "%(date)", std::string(buffer));

	formatted = util::replaceAll(formatted, "%(level)", LogLevelHelper::levelToString(message.level));
	formatted = util::replaceAll(formatted, "%(logger)", message.logger);
	formatted = util::replaceAll(formatted, "%(file)", message.file);
	formatted = util::replaceAll(formatted, "%(line)", util::str(message.line));
	formatted = util::replaceAll(formatted, "%(message)", message.message);
	return formatted;
}

void FormattedLogSink::setFormat(const std::string& format) {
	this->format = format;
}

void FormattedLogSink::setDateFormat(const std::string& date_format) {
	this->date_format = date_format;
}

void FormattedLogSink::sink(const LogMessage& message) {
	sinkFormatted(message, formatLogEntry(message));
}

void FormattedLogSink::sinkFormatted(const LogMessage& message,
		const std::string& formatted) {
}

LogOutputSink::LogOutputSink(std::string format, std::string date_format)
	: FormattedLogSink(format, date_format) {
}

LogOutputSink::~LogOutputSink() {
}

void LogOutputSink::sinkFormatted(const LogMessage& message,
		const std::string& formatted) {
	if (message.level < LogLevel::NOTICE || message.level == LogLevel::UNKNOWN)
		std::cerr << formatted << std::endl;
	else
		std::cout << formatted << std::endl;
}

LogFileSink::LogFileSink(const std::string& filename, std::string format,
		std::string date_format)
	: FormattedLogSink(format, date_format) {
	out.open(filename, std::fstream::out | std::fstream::app);
	/*
	if (!out) {
		// TODO handle error
	}
	*/
}

LogFileSink::~LogFileSink() {
	out.close();
}

void LogFileSink::sinkFormatted(const LogMessage& message,
		const std::string& formatted) {
	out << formatted << std::endl;
}

#ifdef HAVE_SYSLOG_H

LogSyslogSink::LogSyslogSink() {
	openlog("mapcrafter", 0, LOG_USER);
}

LogSyslogSink::~LogSyslogSink() {
	closelog();
}

void LogSyslogSink::sink(const LogMessage& message) {
	syslog(LogLevelHelper::levelToSyslog(message.level), message.message.c_str());
}

#endif

thread_ns::mutex Logging::instance_mutex;
std::shared_ptr<Logging> Logging::instance;

Logging::Logging()
	: global_verbosity(LogLevel::INFO), maximum_verbosity(global_verbosity) {
	reset();
}

Logging::~Logging() {
}

void Logging::setGlobalVerbosity(LogLevel level) {
	global_verbosity = level;
	updateMaximumVerbosity();
}

void Logging::setSinkVerbosity(const std::string& sink, LogLevel level) {
	sinks_verbosity[sink] = level;
	updateMaximumVerbosity();
}

LogLevel Logging::getSinkVerbosity(const std::string& sink) const {
	if (sinks_verbosity.count(sink))
		return sinks_verbosity.at(sink);
	return global_verbosity;
}

void Logging::addSink(const std::string& name, LogSink* sink) {
	sinks[name] = std::shared_ptr<LogSink>(sink);
}

void Logging::reset() {
	global_verbosity = maximum_verbosity = LogLevel::INFO;
	loggers.clear();
	sinks.clear();
	sinks_verbosity.clear();

	// short form for "%Y-%m-%d %H:%M:%S" is "%F %T", but that doesn't work with MinGW
	addSink("output", new LogOutputSink("%(date) [%(level)] [%(logger)] %(message)", "%Y-%m-%d %H:%M:%S"));
}

Logger& Logging::getLogger(const std::string& name) {
	thread_ns::unique_lock<thread_ns::mutex> lock(loggers_mutex);
	if (!loggers.count(name))
		loggers[name].reset(new Logger(name));
	return *loggers.at(name);
}

Logging& Logging::getInstance() {
	thread_ns::unique_lock<thread_ns::mutex> lock(instance_mutex);
	if (!instance)
		instance.reset(new Logging);
	return *instance;
}

void Logging::updateMaximumVerbosity() {
	LogLevel maximum = LogLevel::EMERGENCY;
	for (auto it = sinks.begin(); it != sinks.end(); ++it)
		maximum = std::max(maximum, getSinkVerbosity(it->first));
	maximum_verbosity = maximum;
}

void Logging::handleLogMessage(const LogMessage& message) {
	thread_ns::unique_lock<thread_ns::mutex> lock(handle_message_mutex);
	if (message.level > maximum_verbosity)
		return;
	for (auto it = sinks.begin(); it != sinks.end(); ++it) {
		if (message.level <= getSinkVerbosity(it->first))
			(*it->second).sink(message);
	}
}

} /* namespace util */
} /* namespace mapcrafter */
