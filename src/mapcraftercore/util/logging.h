/*
 * logging.h
 *
 *  Created on: 15.05.2014
 *      Author: moritz
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>

#define LOG(level) mapcrafter::util::Logger::getLogger("default")->log(mapcrafter::util::LogLevel::level)

namespace mapcrafter {
namespace util {

/**
 * Log levels according to syslog.
 */
enum class LogLevel {
	// System is unusable
	EMERGENCY = 0,
	// Action must be taken immediately
	ALERT = 1,
	// Critical conditions
	FATAL = 2, // or "critical"
	// Error conditions
	ERROR = 3,
	// Warning conditions
	WARNING = 4,
	// Normal but significant condition
	NOTICE = 5,
	// Informational messages
	INFO = 6,
	// Debug-level messages
	DEBUG = 7,
	// Unknown level, only used for levelFromString method
	UNKNOWN = 8,
};

class LogLevelHelper {
public:
	static LogLevel levelFromString(const std::string& str);
	static std::string levelToString(LogLevel level);
};

class Logger;

class LoggingStream {
public:
	LoggingStream(Logger* logger, LogLevel level);
	~LoggingStream();

	template<typename T>
	LoggingStream& operator<<(const T& t) {
		(*ss) << t;
		return *this;
	}

private:
	Logger* logger;
	LogLevel level;

	std::shared_ptr<std::stringstream> ss;
};

class Logger {
public:
	~Logger();

	LoggingStream log(LogLevel level);

	static Logger* getLogger(const std::string& name);

protected:
	Logger(const std::string& name);

	static std::map<std::string, Logger*> loggers;
};

} /* namespace util */
} /* namespace mapcrafter */

#endif /* LOGGING_H_ */
