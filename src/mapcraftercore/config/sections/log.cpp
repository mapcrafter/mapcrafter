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

template <>
config::LogSinkType as<config::LogSinkType>(const std::string& from) {
	if (from == "output")
		return config::LogSinkType::OUTPUT;
	else if (from == "file")
		return config::LogSinkType::FILE;
	else if (from == "syslog")
		return config::LogSinkType::SYSLOG;
	throw std::invalid_argument("Must be 'output', 'file' or 'syslog'!");
}

template <>
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

std::ostream& operator<<(std::ostream& out, LogSinkType sink_type) {
	if (sink_type == LogSinkType::OUTPUT)
		out << "output";
	else if (sink_type == LogSinkType::FILE)
		out << "file";
	else if (sink_type == LogSinkType::SYSLOG)
		out << "syslog";
	return out;
}

LogSection::LogSection() {
}

LogSection::~LogSection() {
}

std::string LogSection::getPrettyName() const {
	if (isGlobal())
		return "Global log section " + getSectionName();
	return "Log section '" + getSectionName() + "'";
}

void LogSection::dump(std::ostream& out) const {
	out << getPrettyName() << ":" << std::endl;
	out << "  type = " << getType() << std::endl;
	out << "  verbosity = " << getVerbosity() << std::endl;
	out << "  log_progress = " << util::strBool(getLogProgress()) << std::endl;

	if (getType() == LogSinkType::OUTPUT || getType() == LogSinkType::FILE) {
		out << "  format = " << getFormat() << std::endl;
		out << "  date_format = " << getDateFormat() << std::endl;
	}
	if (getType() == LogSinkType::FILE)
		out << "  file = " << getFile() << std::endl;
}

void LogSection::configureLogging() const {
	std::string sink_name = "__" + util::str(getType()) + "__";
	if (getType() == LogSinkType::FILE)
		sink_name = getSectionName();

	util::Logging& logging = util::Logging::getInstance();
	if (verbosity.isLoaded())
		logging.setSinkVerbosity(sink_name, verbosity.getValue());
	if (log_progress.isLoaded())
		logging.setSinkLogProgress(sink_name, log_progress.getValue());

	if (getType() == LogSinkType::FILE) {
		if (logging.getSink(sink_name) != nullptr)
			LOG(WARNING) << "Unable to configure file log '" << file.getValue()
					<< "'. Sink name '" << sink_name << "' is already in use!";
		else
			logging.setSink(sink_name, new util::LogFileSink(file.getValue().string()));
	}

	if (getType() == LogSinkType::OUTPUT || getType() == LogSinkType::FILE) {
		util::LogSink* sink_ptr = logging.getSink(sink_name);
		util::FormattedLogSink* sink = dynamic_cast<util::FormattedLogSink*>(sink_ptr);
		if (sink != nullptr) {
			if (format.isLoaded())
				sink->setFormat(format.getValue());
			if (date_format.isLoaded())
				sink->setDateFormat(date_format.getValue());
		} else
			LOG(WARNING) << "Unable to configure log sink '" << sink_name << "'!";
	}

#ifdef HAVE_SYSLOG_H
	if (getType() == LogSinkType::SYSLOG && logging.getSink(sink_name) == nullptr)
		logging.setSink(sink_name, new util::LogSyslogSink);
#endif
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
	// don't set defaults here, they are set by the logging class
	// if something is set in the logging config file,
	// it is overwritten in the logging class then
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
	std::string section_name = getSectionName();
	if (!section_name.empty() && section_name[0] == '_')
		validation.error("Invalid section name '" + section_name + "'! "
				"Log section names must not start with an underscore!");
	if (!type.require(validation, "You have to specify a log sink type ('type')!"))
		return;
#ifndef HAVE_SYSLOG_H
	if (getType() == LogSinkType::SYSLOG)
		validation.error("You have set the log type to syslog, but syslog is not "
				"available on your platform!");
#endif
	if (getType() == LogSinkType::FILE)
		file.require(validation, "You have to specify a log file ('file')!");
}

} /* namespace config */
} /* namespace mapcrafter */
