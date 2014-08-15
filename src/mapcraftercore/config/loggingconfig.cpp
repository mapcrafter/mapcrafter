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

#include "loggingconfig.h"

#include "configparser.h"
#include "iniconfig.h"

namespace mapcrafter {
namespace config {

LoggingConfig::LoggingConfig() {
}

LoggingConfig::~LoggingConfig() {
}

ValidationMap LoggingConfig::parse(const std::string& filename) {
	ValidationMap validation;

	INIConfig config;
	try {
		config.loadFile(filename);
	} catch (INIConfigError& exception) {
		validation.section("Configuration file").error(exception.what());
		return validation;
	}

	fs::path config_dir = BOOST_FS_ABSOLUTE1(fs::path(filename)).parent_path();
	//root_section.setConfigDir(config_dir);

	ConfigSectionBase root_section;

	ConfigParser parser(config);
	parser.parseRootSection(root_section);
	parser.parseSections(log_sections, "log", ConfigDirSectionFactory<LogSection>(config_dir));
	parser.validate();
	validation = parser.getValidation();
	return validation;
}

const std::vector<LogSection>& LoggingConfig::getLogSections() {
	return log_sections;
}

void LoggingConfig::configureLogging() {
	fs::path logging_conf = util::findLoggingConfigFile();
	if (logging_conf.empty()) {
		LOG(WARNING) << "Unable to find a global logging configuration file!";
		return;
	}

	LoggingConfig config;
	ValidationMap validation = config.parse(logging_conf.string());
	if (!validation.isEmpty()) {
		LOG(WARNING) << "There is a problem parsing the global logging configuration file.";
		validation.log();
	}
	if (validation.isCritical())
		return;

	for (auto it = config.log_sections.begin(); it != config.log_sections.end(); ++it)
		it->configureLogging();
}

} /* namespace config */
} /* namespace mapcrafter */
