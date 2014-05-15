/*
 * logging.cpp
 *
 *  Created on: 15.05.2014
 *      Author: moritz
 */

#include "logging.h"

namespace mapcrafter {
namespace util {

LoggingStream::LoggingStream(Logger* logger)
	: logger(logger), ss(new std::stringstream) {
}

LoggingStream::~LoggingStream() {
	std::cout << "log: " << ss->str() << std::endl;
}

std::map<std::string, Logger*> Logger::loggers;

Logger::Logger(const std::string& name) {
}

Logger::~Logger() {
}

LoggingStream Logger::log() {
	return LoggingStream(this);
}

Logger* Logger::getLogger(const std::string& name) {
	if (!loggers.count(name))
		loggers[name] = new Logger(name);
	return loggers.at(name);
}

} /* namespace util */
} /* namespace mapcrafter */
