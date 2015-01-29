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

#ifndef LOGGINGCONFIG_H_
#define LOGGINGCONFIG_H_

#include "validation.h"
#include "sections/base.h"
#include "sections/log.h"

#include <vector>

namespace mapcrafter {
namespace config {

/**
 * Class to parse the global logging configuration file.
 */
class LoggingConfig {
public:
	LoggingConfig();
	~LoggingConfig();

	ValidationMap parse(const std::string& filename);

	const std::vector<LogSection>& getLogSections();

	/**
	 * Parses/validates a global logging configuration file and configures the log sinks.
	 * Use the util::findLoggingConfigFile() to automatically find the config file.
	 */
	static void configureLogging(const fs::path& logging_config);

private:
	std::vector<LogSection> log_sections;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* LOGGINGCONFIG_H_ */
