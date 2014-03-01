#include <vector>

#include "../util.h"
#include "../config/mapcrafterconfig.h"
#include "../mc/world.h"
#include "../mc/worldentities.h"

#include <string>
#include <vector>
#include <map>

namespace util = mapcrafter::util;
namespace config = mapcrafter::config;
namespace mc = mapcrafter::mc;

struct Marker {
	mc::BlockPos pos;
	std::string title, text;

	std::string toJSON() {
		std::string title_escaped = util::replaceAll(title, "\"", "\\\"");
		std::string text_escaped = util::replaceAll(text, "\"", "\\\"");

		std::string json = "{";
		json += "\"pos\": [" + util::str(pos.x) + "," + util::str(pos.z) + "," + util::str(pos.y) + "], ";
		json += "\"title\": \"" + title_escaped + "\", ";
		json += "\"text\": \"" + text_escaped + "\", ";
		return json + "}";
	}
};

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: ./mapcrafter_markers [configfile]" << std::endl;
		return 1;
	}

	std::string configfile = argv[1];

	config::MapcrafterConfig config;
	config::ValidationMap validation;
	bool ok = config.parse(configfile, validation);

	if (validation.size() > 0) {
		std::cerr << (ok ? "Some notes on your configuration file:"
				: "Your configuration file is invalid!") << std::endl;
		for (auto it = validation.begin(); it != validation.end(); ++it) {
			std::cerr << it->first << ":" << std::endl;
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				std::cerr << " - " << *it2 << std::endl;
			}
		}
	}

	std::map<std::string, std::map<std::string, std::vector<Marker> > > marker_groups;

	auto worlds = config.getWorlds();
	auto markers = config.getMarkers();
	for (auto world_it = worlds.begin(); world_it != worlds.end(); ++world_it) {
		mc::WorldCrop worldcrop = world_it->second.getWorldCrop();
		mc::World world(world_it->second.getInputDir().string(),
				world_it->second.getDimension());
		world.setWorldCrop(worldcrop);
		if (!world.load()) {
			std::cerr << "Error: Unable to load world " << world_it->first << "!" << std::endl;
			continue;
		}

		mc::WorldEntitiesCache entities(world);
		entities.update();

		std::vector<mc::SignEntity> signs = entities.getSigns(world.getWorldCrop());
		for (auto sign_it = signs.begin(); sign_it != signs.end(); ++sign_it) {
			// don't use signs not contained in the world boundaries
			if (!worldcrop.isBlockContainedXZ(sign_it->getPos())
					&& !worldcrop.isBlockContainedY(sign_it->getPos()))
				continue;
			for (auto marker_it = markers.begin(); marker_it != markers.end(); ++marker_it) {
				if (!marker_it->second.matchesSign(*sign_it))
					continue;
				Marker marker;
				marker.pos = sign_it->getPos();
				marker.title = marker_it->second.formatTitle(*sign_it);
				marker.text = marker_it->second.formatText(*sign_it);
				marker_groups[marker_it->first][world_it->second.getWorldName()].push_back(marker);
			}
		}
	}

	std::cout << "MARKERS = {" << std::endl;
	for (auto group_it = marker_groups.begin(); group_it != marker_groups.end();
			++group_it) {
		std::string group = group_it->first;
		config::MarkerSection marker_config = config.getMarker(group);
		std::cout << "  \"" << group << "\": {" << std::endl;
		std::cout << "    \"name\" : \"" << marker_config.getName() << "\"," << std::endl;
		if (!marker_config.getIcon().empty()) {
			std::cout << "    \"icon\" : \"" << marker_config.getIcon() << "\"," << std::endl;
			if (!marker_config.getIconSize().empty())
				std::cout << "    \"icon_size\" : " << marker_config.getIconSize() << "," << std::endl;
		}
		std::cout << "    \"markers\" : {" << std::endl;

		for (auto world_it = group_it->second.begin();
				world_it != group_it->second.end(); ++world_it) {
			std::cout << "      \"" << world_it->first << "\" : [" << std::endl;
			for (auto marker_it = world_it->second.begin();
					marker_it != world_it->second.end(); ++marker_it) {
				std::cout << "        " << marker_it->toJSON() << "," << std::endl;
			}
			std::cout << "      ]," << std::endl;
		}
		std::cout << "    }," << std::endl;
		std::cout << "  }," << std::endl;
	}
	std::cout << "};" << std::endl;
}
