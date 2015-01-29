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

#ifndef MANAGER_H_
#define MANAGER_H_

#include "tilerenderer.h"
#include "tileset.h"
#include "../config/mapcrafterconfig.h"
#include "../config/mapcrafterconfighelper.h"
#include "../mc/world.h"
#include "../mc/worldcache.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace config = mapcrafter::config;

namespace mapcrafter {
namespace renderer {

/**
 * This are the render options from the command line.
 */
struct RenderOpts {
	fs::path logging_config;
	bool batch;

	fs::path config;
	std::vector<std::string> render_skip, render_auto, render_force;
	bool skip_all;
	int jobs;
};

/**
 * This does the whole rendering process.
 */
class RenderManager {
private:
	RenderOpts opts;
	config::MapcrafterConfig config;
	config::MapcrafterConfigHelper confighelper;

	bool copyTemplateFile(const std::string& filename,
	        const std::map<std::string, std::string>& vars) const;
	bool copyTemplateFile(const std::string& filename) const;

	bool writeTemplateIndexHtml() const;
	void writeTemplates() const;

	void increaseMaxZoom(const fs::path& dir, std::string image_format,
			int jpeg_quality = 85) const;

public:
	RenderManager(const RenderOpts& opts);

	bool run();
};

}
}

#endif /* MANAGER_H_ */
