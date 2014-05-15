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

#define LOG(level) mapcrafter::util::Logger::getLogger("default")->log()

namespace mapcrafter {
namespace util {

class Logger;

class LoggingStream {
public:
	LoggingStream(Logger* logger);
	~LoggingStream();

	template<typename T>
	LoggingStream& operator<<(const T& t) {
		(*ss) << t;
		return *this;
	}

private:
	Logger* logger;

	std::shared_ptr<std::stringstream> ss;
};

class Logger {
public:
	~Logger();

	LoggingStream log();

	static Logger* getLogger(const std::string& name);

protected:
	Logger(const std::string& name);

	static std::map<std::string, Logger*> loggers;
};

} /* namespace util */
} /* namespace mapcrafter */

#endif /* LOGGING_H_ */
