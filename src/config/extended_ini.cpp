#include "extended_ini.h"

namespace mapcrafter {
namespace config2 {

bool ConfigSection::has(const std::string& key) const {
	return false;
}

std::string ConfigSection::get(const std::string& key, const std::string& default_value) const {
	return "";
}

void ConfigSection::set(const std::string& key, const std::string& value) {
}

void ConfigSection::remove(const std::string& key) {
}

bool ConfigFile::load(std::istream& in, ValidationMessage& msg) {
	return true;
}

bool ConfigFile::loadFile(const std::string& filename, ValidationMessage& msg) {
	return true;
}

bool ConfigFile::write(std::ostream& out) const {
	return true;
}

bool ConfigFile::writeFile(const std::string& filename) const {
	return true;
}

const ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) const {
	return empty_section;
}

ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) {
	return empty_section;
}

void ConfigFile::addSection(const std::string& type, const std::string& name) {
}

void ConfigFile::removeSection(const std::string& type, const std::string& name) {
}

} /* namespace config */
} /* namespace mapcrafter */
