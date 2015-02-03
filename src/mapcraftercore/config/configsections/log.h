/*
 * Copyright 2012-2015 Moritz Hilscher
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

#ifndef LOG_H_
#define LOG_H_

#include "../configsection.h"
#include "../validation.h"
#include "../../util.h"

#include <iostream>
#include <string>

namespace mapcrafter {
namespace config {

enum class LogSinkType {
	OUTPUT,
	FILE,
	SYSLOG
};

std::ostream& operator<<(std::ostream& out, LogSinkType sink_type);

class LogSection : public ConfigSection {
public:
	LogSection();
	~LogSection();

	virtual std::string getPrettyName() const;
	virtual void dump(std::ostream& out) const;

	void setConfigDir(const fs::path& config_dir);
	void configureLogging() const;

	LogSinkType getType() const;
	util::LogLevel getVerbosity() const;
	bool getLogProgress() const;

	// only for output, file log
	std::string getFormat() const;
	std::string getDateFormat() const;

	// only for file log
	fs::path getFile() const;

	// only for syslog
	bool isEnabled() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	fs::path config_dir;

	Field<LogSinkType> type;
	Field<util::LogLevel> verbosity;
	Field<bool> log_progress;

	// only for output, file log
	Field<std::string> format, date_format;

	// only for file log
	Field<fs::path> file;
};

}
}

#endif /* LOG_H_ */
