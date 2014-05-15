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

#ifndef LOGGING_H_
#define LOGGING_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#define LOG(level) mapcrafter::util::Logger::getLogger("default")->log(mapcrafter::util::LogLevel::level, __FILE__, __LINE__)
#define LOGN(level, logger) mapcrafter::util::Logger::getLogger(logger)->log(mapcrafter::util::LogLevel::level, __FILE__, __LINE__)

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

struct LogEntry {
	LogLevel level;
	std::string logger;
	std::string file;
	int line;

	std::string message;
};

class LogStream {
public:
	LogStream(LogLevel level, const std::string& logger, const std::string& file, int line);
	~LogStream();

	template<typename T>
	LogStream& operator<<(const T& t) {
		(*ss) << t;
		return *this;
	}

private:
	LogEntry entry;

	std::shared_ptr<std::stringstream> ss;
};

class Logger {
public:
	~Logger();

	LogStream log(LogLevel level, const std::string& file, int line);

	static Logger* getLogger(const std::string& name);

protected:
	Logger(const std::string& name);

	std::string name;

	static std::map<std::string, Logger*> loggers;
};

class LogSink {
public:
	LogSink();
	virtual ~LogSink();

	virtual void sink(const LogEntry& entry);
};

class FormattedLogSink : public LogSink {
public:
	FormattedLogSink(std::string format = "");
	virtual ~FormattedLogSink();

	void setFormat(const std::string& format);

	virtual void sink(const LogEntry& entry);
	virtual void sinkFormatted(const LogEntry& entry, const std::string& formatted);

protected:
	std::string format;

	std::string formatLogEntry(const LogEntry& entry);
};

class LogOutputSink : public FormattedLogSink {
public:
	LogOutputSink(std::string format = "");
	virtual ~LogOutputSink();

	virtual void sinkFormatted(const LogEntry& entry, const std::string& formatted);
};

// TODO
class LogFileSink : public FormattedLogSink {
};

// TODO
class LogSyslogSink : public LogSink {
};

class LogManager {
public:
	~LogManager();

	void addSink(LogSink* sink);

	void handleLogEntry(const LogEntry& entry);

	static LogManager* getInstance();

protected:
	LogManager();

	std::vector<std::shared_ptr<LogSink> > sinks;

	static LogManager* instance;
};

} /* namespace util */
} /* namespace mapcrafter */

#endif /* LOGGING_H_ */
