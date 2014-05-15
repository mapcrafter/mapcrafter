/*
 * logging.cpp
 *
 *  Created on: 15.05.2014
 *      Author: moritz
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

LoggingStream::LoggingStream(Logger* logger, LogLevel level)
	: logger(logger), level(level), ss(new std::stringstream) {
}

LoggingStream::~LoggingStream() {
	std::cout << "[" << LogLevelHelper::levelToString(level) << "] " << ss->str() << std::endl;
}

std::map<std::string, Logger*> Logger::loggers;

Logger::Logger(const std::string& name) {
}

Logger::~Logger() {
}

LoggingStream Logger::log(LogLevel level) {
	return LoggingStream(this, level);
}

Logger* Logger::getLogger(const std::string& name) {
	if (!loggers.count(name))
		loggers[name] = new Logger(name);
	return loggers.at(name);
}

} /* namespace util */
} /* namespace mapcrafter */
