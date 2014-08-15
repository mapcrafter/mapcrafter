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

#ifndef MAPCRAFTERCONFIG_H_
#define MAPCRAFTERCONFIG_H_

#include "validation.h"
#include "sections/base.h"
#include "sections/log.h"
#include "sections/map.h"
#include "sections/marker.h"
#include "sections/world.h"
#include "../util.h"

#include <map>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

struct Color {
	std::string hex;
	uint8_t red, green, blue;
};

/**
 * Customized section factory that passes the config directory to the section objects.
 */
template <typename T>
class MapcrafterConfigSectionFactory {
public:
	MapcrafterConfigSectionFactory(fs::path config_dir = "");
	~MapcrafterConfigSectionFactory();

	T operator()() const;

private:
	fs::path config_dir;
};

class MapcrafterConfigRootSection : public ConfigSectionBase {
public:
	MapcrafterConfigRootSection();
	~MapcrafterConfigRootSection();

	virtual std::string getPrettyName() const;
	virtual void dump(std::ostream& out) const;

	void setConfigDir(const fs::path& config_dir);

	fs::path getOutputDir() const;
	fs::path getTemplateDir() const;
	Color getBackgroundColor() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	fs::path config_dir;

	Field<fs::path> output_dir, template_dir;
	Field<Color> background_color;
};

class MapcrafterConfig {
public:
	MapcrafterConfig();
	~MapcrafterConfig();

	ValidationMap parse(const std::string& filename);
	void dump(std::ostream& out) const;

	void configureLogging() const;

	fs::path getOutputDir() const;
	fs::path getTemplateDir() const;
	fs::path getOutputPath(const std::string& path) const;
	fs::path getTemplatePath(const std::string& path) const;

	Color getBackgroundColor() const;

	bool hasWorld(const std::string& world) const;
	const std::map<std::string, WorldSection>& getWorlds() const;
	const WorldSection& getWorld(const std::string& world) const;

	bool hasMap(const std::string& map) const;
	const std::vector<MapSection>& getMaps() const;
	const MapSection& getMap(const std::string& map) const;

	bool hasMarker(const std::string marker) const;
	const std::vector<MarkerSection>& getMarkers() const;
	const MarkerSection& getMarker(const std::string& marker) const;

private:
	WorldSection world_global;
	MapSection map_global;
	MarkerSection marker_global;

	MapcrafterConfigRootSection root_section;
	std::map<std::string, WorldSection> worlds;
	std::vector<MapSection> maps;
	std::vector<MarkerSection> markers;
	std::vector<LogSection> log_sinks;
};

template <typename T>
MapcrafterConfigSectionFactory<T>::MapcrafterConfigSectionFactory(fs::path config_dir)
	: config_dir(config_dir) {
}

template <typename T>
MapcrafterConfigSectionFactory<T>::~MapcrafterConfigSectionFactory() {
}

template <typename T>
T MapcrafterConfigSectionFactory<T>::operator()() const {
	T section;
	section.setConfigDir(config_dir);
	return section;
}

} /* namespace config */
} /* namespace mapcrafter */
#endif /* MAPCRAFTERCONFIG_H_ */
