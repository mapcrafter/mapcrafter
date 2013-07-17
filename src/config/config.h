/*
 * config.h
 *
 *  Created on: 17.07.2013
 *      Author: moritz
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "ini.h"

#include <string>
#include <vector>
#include <set>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config {

static std::string ROTATION_NAMES[4] = {"top-left", "top-right", "bottom-right", "bottom-left"};
static std::string ROTATION_NAMES_SHORT[4] = {"tl", "tr", "br", "bl"};

int stringToRotation(const std::string& rotation, std::string names[4] = ROTATION_NAMES);

struct RenderWorldConfig {
	std::string name_short, name_long;

	std::string input_dir;
	std::string textures_dir;

	std::set<int> rotations;
	std::vector<std::string> unknown_rotations;

	std::string rendermode;

	int texture_size;
	bool render_unknown_blocks;
	bool render_leaves_transparent;
	bool render_biomes;

	std::string incremental_detection;

	std::vector<int> render_behaviors;

	RenderWorldConfig();

	void readFromConfig(const fs::path& dir, const ConfigFile& config,
			const std::string& section);
	bool checkValid(std::vector<std::string>& errors) const;

	bool canSkip() const;
	bool isCompleteRenderForce() const;

	void print(std::ostream& stream) const;

	static const int RENDER_SKIP = 0;
	static const int RENDER_AUTO = 1;
	static const int RENDER_FORCE = 2;
};

class RenderConfigParser {
private:
	ConfigFile config;

	std::string output_dir;
	std::string template_dir;

	RenderWorldConfig default_config;
	std::vector<RenderWorldConfig> worlds;
	std::vector<int> worlds_max_zoom;

public:
	RenderConfigParser();
	~RenderConfigParser();

	bool loadFile(const std::string& filename);
	bool checkValid() const;

	void setRenderBehaviors(bool skip_all, const std::string& render_skip,
			const std::string& render_auto, const std::string& render_force);

	const std::vector<RenderWorldConfig>& getWorlds() const;

	fs::path getOutputDir() const;
	fs::path getTemplateDir() const;
	std::string getOutputPath(std::string file) const;
	std::string getTemplatePath(std::string file) const;

	void setWorldMaxZoom(size_t world, int max_zoom);
	std::string generateJavascript() const;
};

} /* namespace config */
} /* namespace mapcrafter */
#endif /* CONFIG_H_ */
