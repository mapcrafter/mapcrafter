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

#include "manager.h"

#include "renderviews/isometric/blockimages.h"
#include "renderviews/isometric/renderview.h"
#include "renderviews/topdown/renderview.h"
#include "tilerenderworker.h"
#include "../config/loggingconfig.h"
#include "../thread/impl/singlethread.h"
#include "../thread/impl/multithreading.h"
#include "../thread/dispatcher.h"
#include "../util.h"
#include "../version.h"

#include <cstring>
#include <ctime>
#include <array>
#include <fstream>
#include <memory>
#include <thread>

namespace mapcrafter {
namespace renderer {

MapSettings::MapSettings()
	: texture_size(12), image_format("png"), lighting_intensity(1.0),
	  render_unknown_blocks(0), render_leaves_transparent(0), render_biomes(false),
	  max_zoom(0) {
	for (int i = 0; i < 4; i++) {
		rotations[i] = false;
		last_render[i] = 0;
		tile_offsets[i] = TilePos(0, 0);
	}
}

/**
 * This method reads the map settings from a file.
 */
bool MapSettings::read(const fs::path& filename) {
	config::INIConfig config;
	try {
		config.loadFile(filename.string());
	} catch (config::INIConfigError& exception) {
		LOG(WARNING) << "Unable to read map.settings file '" << filename.string() << "': "
				<< exception.what();
		return false;
	}

	config::INIConfigSection& root = config.getRootSection();

	// don't set default values for new options here, that's done in syncMapConfig
	if (root.has("texture_size"))
		texture_size.set(root.get<int>("texture_size"));
	if (root.has("image_format"))
		image_format.set(root.get<std::string>("image_format"));
	if (root.has("lighting_intensity"))
		lighting_intensity.set(root.get<double>("lighting_intensity"));
	// exception for default value of cave_high_contrast
	// according to config defaults to true, but for older maps it's false
	cave_high_contrast.set(root.get<bool>("cave_high_contrast", false));
	if (root.has("render_unknown_blocks"))
		render_unknown_blocks.set(root.get<bool>("render_unknown_blocks"));
	if (root.has("render_leaves_transparent"))
		render_leaves_transparent.set(root.get<bool>("render_leaves_transparent"));
	if (root.has("render_biomes"))
		render_biomes.set(root.get<bool>("render_biomes"));

	max_zoom = root.get<int>("max_zoom");

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		rotations[i] = config.hasSection("rotation", rotation_names[i]);
		if (rotations[i]) {
			auto section = config.getSection("rotation", rotation_names[i]);
			last_render[i] = section.get<int>("last_render");
			int offset_x = section.get<int>("tile_offset_x", 0);
			int offset_y = section.get<int>("tile_offset_y", 0);
			tile_offsets[i] = TilePos(offset_x, offset_y);
		}
	}

	return true;
}

/**
 * This method writes the map settings to a file.
 */
bool MapSettings::write(const fs::path& filename) const {
	config::INIConfig config;
	config::INIConfigSection& root = config.getRootSection();

	root.set("texture_size", util::str(texture_size.get()));
	root.set("image_format", image_format.get());
	root.set("lighting_intensity", util::str(lighting_intensity.get()));
	root.set("cave_high_contrast", util::str(cave_high_contrast.get()));
	root.set("render_unknown_blocks", util::str(render_unknown_blocks.get()));
	root.set("render_leaves_transparent", util::str(render_leaves_transparent.get()));
	root.set("render_biomes", util::str(render_biomes.get()));

	root.set("max_zoom", util::str(max_zoom));

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		if (rotations[i]) {
			auto& section = config.getSection("rotation", rotation_names[i]);
			section.set("last_render", util::str(last_render[i]));
			section.set("tile_offset_x", util::str(tile_offsets[i].getX()));
			section.set("tile_offset_y", util::str(tile_offsets[i].getY()));
		}
	}

	try {
		config.writeFile(filename.string());
	} catch (config::INIConfigError& exception) {
		LOG(WARNING) << "Unable to write map.settings file '" << filename.string() << "': "
				<< exception.what();
		return false;
	}
	return true;
}

bool MapSettings::syncMapConfig(const config::MapSection& map) {
	if (texture_size.isNull())
		texture_size.set(map.getTextureSize());
	if (image_format.isNull())
		image_format.set(map.getImageFormatSuffix());
	if (lighting_intensity.isNull())
		lighting_intensity.set(map.getLightingIntensity());
	if (cave_high_contrast.isNull())
		cave_high_contrast.set(map.hasCaveHighContrast());
	if (render_unknown_blocks.isNull())
		render_unknown_blocks.set(map.renderUnknownBlocks());
	if (render_leaves_transparent.isNull())
		render_leaves_transparent.set(map.renderLeavesTransparent());
	if (render_biomes.isNull())
		render_biomes.set(map.renderBiomes());

	bool changed = true;
	bool force_required = false;
	if (texture_size.get() != map.getTextureSize()) {
		LOG(ERROR) << "You changed the texture size from " << texture_size.get()
				<< " to " << map.getTextureSize() << ".";
		force_required = true;
	} else if (image_format.get() != map.getImageFormatSuffix()) {
		LOG(ERROR) << "You changed the image format from " << image_format.get()
				<< " to " << map.getImageFormatSuffix() << ".";
		LOG(ERROR) << "Force-render the whole map in order for the new "
				<< "configuration to come into effect and delete the images "
				<< "generated with the other image format.";
		force_required = true;
		return false;
	} else if (!util::floatingPointEquals(lighting_intensity.get(), map.getLightingIntensity())) {
		LOG(WARNING) << "You changed the lighting intensity from "
				<< lighting_intensity.get() << " to " << map.getLightingIntensity() << ".";
	} else if (cave_high_contrast.get() != map.hasCaveHighContrast()) {
		LOG(WARNING) << "You have changed the cave high contrast mode from "
				<< util::str(cave_high_contrast.get()) << " to "
				<< util::str(map.hasCaveHighContrast()) << ".";
	} else if (render_unknown_blocks.get() != map.renderUnknownBlocks()) {
		LOG(WARNING) << "You changed the rendering of unknown blocks from "
				<< util::str(render_unknown_blocks.get()) << " to "
				<< util::str(map.renderUnknownBlocks()) << ".";
	} else if (render_leaves_transparent.get() != map.renderLeavesTransparent()) {
		LOG(WARNING) << "You changed the rendering of transparent leaves from "
				<< util::str(render_leaves_transparent.get()) << " to "
				<< util::str(map.renderLeavesTransparent()) << ".";
	} else if (render_biomes.get() != map.renderBiomes()) {
		LOG(WARNING) << "You changed the rendering of biomes from "
				<< util::str(render_biomes.get()) << " to "
				<< util::str(map.renderBiomes()) << ".";
	} else {
		changed = false;
	}

	if (changed) {
		(force_required ? LOG(ERROR) : LOG(WARNING))
				<< "Force-render the whole map in order for the new "
				<< "configuration to come into effect.";
	}

	return !(changed && force_required);
}

MapSettings MapSettings::byMapConfig(const config::MapSection& map) {
	MapSettings settings;

	settings.texture_size.set(map.getTextureSize());
	settings.image_format.set(map.getImageFormatSuffix());
	settings.lighting_intensity.set(map.getLightingIntensity());
	settings.cave_high_contrast.set(map.hasCaveHighContrast());
	settings.render_unknown_blocks.set(map.renderUnknownBlocks());
	settings.render_leaves_transparent.set(map.renderLeavesTransparent());
	settings.render_biomes.set(map.renderBiomes());

	auto rotations = map.getRotations();
	for (auto it = rotations.begin(); it != rotations.end(); ++it)
		settings.rotations[*it] = true;

	return settings;
}

RenderManager::RenderManager(const RenderOpts& opts)
	: opts(opts) {
}

/**
 * This method copies a file from the template directory to the output directory and
 * replaces the variables from the map.
 */
bool RenderManager::copyTemplateFile(const std::string& filename,
		const std::map<std::string, std::string>& vars) const {
	std::ifstream file(config.getTemplatePath(filename).string().c_str());
	if (!file)
		return false;
	std::stringstream ss;
	ss << file.rdbuf();
	file.close();
	std::string data = ss.str();

	for (std::map<std::string, std::string>::const_iterator it = vars.begin();
			it != vars.end(); ++it) {
		data = util::replaceAll(data, "{" + it->first + "}", it->second);
	}

	std::ofstream out(config.getOutputPath(filename).string().c_str());
	if (!out)
		return false;
	out << data;
	out.close();
	return true;
}

bool RenderManager::copyTemplateFile(const std::string& filename) const {
	std::map<std::string, std::string> vars;
	return copyTemplateFile(filename, vars);
}

bool RenderManager::writeTemplateIndexHtml() const {
	std::map<std::string, std::string> vars;
	vars["version"] = MAPCRAFTER_VERSION;
	if (strlen(MAPCRAFTER_GITVERSION))
		vars["version"] += std::string(" (") + MAPCRAFTER_GITVERSION + ")";

	time_t t = std::time(nullptr);
	char buffer[256];
	std::strftime(buffer, sizeof(buffer), "%d.%m.%Y, %H:%M:%S", std::localtime(&t));
	vars["lastUpdate"] = buffer;

	vars["backgroundColor"] = config.getBackgroundColor().hex;

	return copyTemplateFile("index.html", vars);
}

bool RenderManager::writeTemplateConfig() const {
	std::ofstream out(config.getOutputPath("config.js").string().c_str());
	if (!out)
		return false;
	std::string config = util::trim(confighelper.generateTemplateJavascript());
	out << "var CONFIG = " << config << ";";
	out.close();
	return true;
}

/**
 * This method copies all template files to the output directory.
 */
void RenderManager::writeTemplates() const {
	if (!fs::is_directory(config.getTemplateDir())) {
		LOG(WARNING) << "The template directory does not exist! Can't copy templates!";
		return;
	}

	if (!writeTemplateIndexHtml())
		LOG(WARNING) << "Warning: Unable to copy template file index.html!";
	if (!writeTemplateConfig())
		LOG(WARNING) << "Warning: Unable to write config.js file!";

	if (!fs::exists(config.getOutputPath("markers.js"))
			&& !util::copyFile(config.getTemplatePath("markers.js"), config.getOutputPath("markers.js")))
		LOG(WARNING) << "Unable to copy template file markers.js!";

	// copy all other files and directories
	fs::directory_iterator end;
	for (fs::directory_iterator it(config.getTemplateDir()); it != end;
			++it) {
		std::string filename = BOOST_FS_FILENAME(it->path());
		// do not copy the index.html
		if (filename == "index.html")
			continue;
		// and do not overwrite markers.js and markers-generated.js
		if ((filename == "markers.js" || filename == "markers-generated.js")
				&& fs::exists(config.getOutputPath(filename)))
			continue;
		if (fs::is_regular_file(*it)) {
			if (!util::copyFile(*it, config.getOutputPath(filename)))
				LOG(WARNING) << "Unable to copy template file " << filename;
		} else if (fs::is_directory(*it)) {
			if (!util::copyDirectory(*it, config.getOutputPath(filename)))
				LOG(WARNING) << "Unable to copy template directory " << filename;
		}
	}
}

/**
 * This method increases the max zoom of a rendered map and makes the necessary changes
 * on the tile tree.
 */
void RenderManager::increaseMaxZoom(const fs::path& dir,
		std::string image_format, int jpeg_quality) const {
	if (fs::exists(dir / "1")) {
		// at first rename the directories 1 2 3 4 (zoom level 0) and make new directories
		util::moveFile(dir / "1", dir / "1_");
		fs::create_directories(dir / "1");
		// then move the old tile trees one zoom level deeper
		util::moveFile(dir / "1_", dir / "1/4");
		// also move the images of the directories
		util::moveFile(dir / (std::string("1.") + image_format),
				dir / (std::string("1/4.") + image_format));
	}

	// do the same for the other directories
	if (fs::exists(dir / "2")) {
		util::moveFile(dir / "2", dir / "2_");
		fs::create_directories(dir / "2");
		util::moveFile(dir / "2_", dir / "2/3");
		util::moveFile(dir / (std::string("2.") + image_format),
				dir / (std::string("2/3.") + image_format));
	}
	
	if (fs::exists(dir / "3")) {
		util::moveFile(dir / "3", dir / "3_");
		fs::create_directories(dir / "3");
		util::moveFile(dir / "3_", dir / "3/2");
		util::moveFile(dir / (std::string("3.") + image_format),
				dir / (std::string("3/2.") + image_format));
	}
	
	if (fs::exists(dir / "4")) {
		util::moveFile(dir / "4", dir / "4_");
		fs::create_directories(dir / "4");
		util::moveFile(dir / "4_", dir / "4/1");
		util::moveFile(dir / (std::string("4.") + image_format),
				dir / (std::string("4/1.") + image_format));
	}

	// now read the images, which belong to the new directories
	RGBAImage img1, img2, img3, img4;
	if (image_format == "png") {
		img1.readPNG((dir / "1/4.png").string());
		img2.readPNG((dir / "2/3.png").string());
		img3.readPNG((dir / "3/2.png").string());
		img4.readPNG((dir / "4/1.png").string());
	} else {
		img1.readJPEG((dir / "1/4.jpg").string());
		img2.readJPEG((dir / "2/3.jpg").string());
		img3.readJPEG((dir / "3/2.jpg").string());
		img4.readJPEG((dir / "4/1.jpg").string());
	}

	int s = img1.getWidth();
	// create images for the new directories
	RGBAImage new1(s, s), new2(s, s), new3(s, s), new4(s, s);
	RGBAImage old1, old2, old3, old4;
	// resize the old images...
	img1.resizeHalf(old1);
	img2.resizeHalf(old2);
	img3.resizeHalf(old3);
	img4.resizeHalf(old4);

	// ...to blit them to the images of the new directories
	new1.simpleblit(old1, s/2, s/2);
	new2.simpleblit(old2, 0, s/2);
	new3.simpleblit(old3, s/2, 0);
	new4.simpleblit(old4, 0, 0);

	// now save the new images in the output directory
	if (image_format == "png") {
		new1.writePNG((dir / "1.png").string());
		new2.writePNG((dir / "2.png").string());
		new3.writePNG((dir / "3.png").string());
		new4.writePNG((dir / "4.png").string());
	} else {
		new1.writeJPEG((dir / "1.jpg").string(), jpeg_quality);
		new2.writeJPEG((dir / "2.jpg").string(), jpeg_quality);
		new3.writeJPEG((dir / "3.jpg").string(), jpeg_quality);
		new4.writeJPEG((dir / "4.jpg").string(), jpeg_quality);
	}

	// don't forget the base.png
	RGBAImage base_big(2*s, 2*s), base;
	base_big.simpleblit(new1, 0, 0);
	base_big.simpleblit(new2, s, 0);
	base_big.simpleblit(new3, 0, s);
	base_big.simpleblit(new4, s, s);
	base_big.resizeHalf(base);
	if (image_format == "png")
		base.writePNG((dir / "base.png").string());
	else
		base.writeJPEG((dir / "base.jpg").string(), jpeg_quality);
}

/**
 * Starts the whole rendering thing.
 */
bool RenderManager::run() {

	// TODO
	std::shared_ptr<RenderView> render_view(new IsometricRenderView);
	//std::shared_ptr<RenderView> render_view(new TopdownRenderView);

	// ###
	// ### First big step: Load/parse/validate the configuration file
	// ###

	config::ValidationMap validation = config.parse(opts.config.string());

	// show infos/warnings/errors if configuration file has something
	if (!validation.isEmpty()) {
		if (validation.isCritical())
			LOG(FATAL) << "Unable to parse configuration file:";
		else
			LOG(WARNING) << "There is a problem parsing the configuration file:";
		validation.log();
		LOG(WARNING) << "Please have a look at the documentation.";
	}
	if (validation.isCritical())
		return false;

	// parse global logging configuration file and configure logging
	config::LoggingConfig::configureLogging(opts.logging_config);

	// configure logging from this configuration file
	config.configureLogging();

	// an output directory would be nice -- create one if it does not exist
	if (!fs::is_directory(config.getOutputDir()) && !fs::create_directories(config.getOutputDir())) {
		LOG(FATAL) << "Error: Unable to create output directory!";
		return false;
	}

	// create a helper for the configuration
	confighelper = config::MapcrafterConfigHelper(config);
	// set the render behaviors the user specified with the -rsaf flags
	confighelper.parseRenderBehaviors(opts.skip_all, opts.render_skip, opts.render_auto, opts.render_force);

	// and get the maps and worlds of the configuration
	auto config_worlds = config.getWorlds();
	auto config_maps = config.getMaps();
	// maps for world- and tileset objects
	std::map<std::string, std::array<mc::World, 4> > worlds;
	std::map<std::string, std::array<std::shared_ptr<TileSet>, 4> > tile_sets;

	// go through all maps and:
	// 1. - find out which rotations are needed for which world
	// 2. - and check if there are already rendered maps
	//    - get the old max zoom levels of these maps for the template
	//    -> so the user can still view his already rendered maps while new ones are rendering
	for (auto map_it = config_maps.begin(); map_it != config_maps.end(); ++map_it) {
		confighelper.setUsedRotations(map_it->getWorld(), map_it->getRotations());
		// TODO add this to the point where TileRenderer is available
		// and add tile_size to MapSettings
		//confighelper.setMapTileSize(map_it->getShortName(), render_view->getTileSize(map_it->getTextureSize(), 1));
		fs::path settings_file = config.getOutputPath(map_it->getShortName() + "/map.settings");
		if (!fs::exists(settings_file))
			continue;
		MapSettings settings;
		if (settings.read(settings_file)) {
			confighelper.setMapZoomlevel(map_it->getShortName(), settings.max_zoom);
			for (int i = 0; i < 4; i++)
				confighelper.setWorldTileOffset(map_it->getWorld(), i, settings.tile_offsets[i]);
		}
	}

	// ###
	// ### Second big step: Scan the worlds
	// ###

	LOG(INFO) << "Scanning worlds...";
	for (auto world_it = config_worlds.begin(); world_it != config_worlds.end(); ++world_it) {
		std::string world_name = world_it->first;

		// at first check if we really need to scan this world
		// scan only if there is at least one map which...
		//  ... is rendered with this world and which
		//  ... is not set to skip
		bool used = false;
		for (auto map_it = config_maps.begin(); map_it != config_maps.end(); ++map_it) {
			if (map_it->getWorld() == world_name &&
					!confighelper.isCompleteRenderSkip(map_it->getShortName())) {
				used = true;
				break;
			}
		}
		if (!used)
			continue;

		// scan the different rotated versions of the world
		// -> the rotations which are used by maps, so not necessarily all rotations
		// find the highest max zoom level of these tilesets to use this for all rotations
		// -> all rotations should have the same max zoom level
		//    to allow a nice interactively rotatable map
		int zoomlevels_max = 0;
		auto rotations = confighelper.getUsedRotations(world_name);
		for (auto rotation_it = rotations.begin(); rotation_it != rotations.end(); ++rotation_it) {
			// load the world
			mc::World world(world_it->second.getInputDir().string(),
					world_it->second.getDimension());
			world.setRotation(*rotation_it);
			world.setWorldCrop(world_it->second.getWorldCrop());
			if (!world.load()) {
				LOG(FATAL) << "Unable to load world " << world_name << "!";
				return false;
			}
			// create a tileset for this world
			std::shared_ptr<TileSet> tile_set(render_view->createTileSet());
			// and scan for tiles of this world,
			// we automatically center the tiles for cropped worlds, but only...
			//  - the circular cropped ones and
			//  - the ones with complete specified x- AND z-bounds
			if (world_it->second.needsWorldCentering()) {
				TilePos tile_offset;
				tile_set->scan(world, true, tile_offset);
				confighelper.setWorldTileOffset(world_name, *rotation_it, tile_offset);
			} else {
				tile_set->scan(world);
			}
			// update the highest max zoom level
			zoomlevels_max = std::max(zoomlevels_max, tile_set->getMinDepth());

			// set world- and tileset object in the map
			worlds[world_name][*rotation_it] = world;
			tile_sets[world_name][*rotation_it] = tile_set;
		}

		// now apply this highest max zoom level
		for (auto rotation_it = rotations.begin(); rotation_it != rotations.end(); ++rotation_it)
			tile_sets[world_name][*rotation_it]->setDepth(zoomlevels_max);
		// also give this highest max zoom level to the config helper
		confighelper.setWorldZoomlevel(world_name, zoomlevels_max);
	}

	// write all template files
	writeTemplates();

	// ###
	// ### Third big step: Render the maps
	// ###

	// some progress and timing stuff
	int progress_maps_all = config_maps.size();
	int time_start_all = std::time(nullptr);

	// go through all maps
	for (size_t i = 0; i < config_maps.size(); i++) {
		// get things like map section, map/world name
		config::MapSection map = config_maps[i];
		std::string map_name = map.getShortName();
		std::string world_name = map.getWorld();

		// continue if all rotations of this map are skipped
		if (confighelper.isCompleteRenderSkip(map_name))
			continue;

		int progress_maps = i+1;
		LOG(INFO) << "[" << progress_maps << "/" << progress_maps_all << "] "
				<< "Rendering map " << map.getShortName() << " (\""
				<< map.getLongName() << "\"):";

		// check again if the output directory for the tiles of this map exists
		if (!fs::is_directory(config.getOutputDir() / map_name))
			fs::create_directories(config.getOutputDir() / map_name);

		// check if we have already an old settings file,
		// but ignore the settings file if the whole map is force-rendered
		MapSettings settings;
		fs::path settings_file = config.getOutputPath(map_name + "/map.settings");
		bool old_settings = !confighelper.isCompleteRenderForce(map_name)
				&& fs::exists(settings_file);
		if (old_settings) {
			// try to read the map.settings filename
			if (!settings.read(settings_file)) {
				LOG(ERROR) << "Unable to load old map.settings file!"
						<< "You have to force-render the whole map.";
				continue;
			}

			// check whether the config file was changed when rendering incrementally
			if (!settings.syncMapConfig(map))
				continue;

			// for force-render rotations
			// -> set the last render time to 0 -> to render all tiles
			for (int i = 0; i < 4; i++)
				if (confighelper.getRenderBehavior(map_name, i)
						== config::MapcrafterConfigHelper::RENDER_FORCE)
					settings.last_render[i] = 0;
		} else {
			// if we don't have a settings file or if we should force-render the whole map
			// -> create a new settings file
			settings = MapSettings::byMapConfig(map);
		}

		std::time_t start_scanning = std::time(nullptr);

		auto rotations = map.getRotations();
		// get the max zoom level calculated with the current tile set
		int world_zoomlevels = confighelper.getWorldZoomlevel(world_name);
		// check if the zoom level of the world has increased
		// since the map was rendered last time (if it was already rendered)
		if (old_settings && settings.max_zoom < world_zoomlevels) {
			LOG(INFO) << "The max zoom level was increased from " << settings.max_zoom
					<< " to " << world_zoomlevels << ".";
			LOG(INFO) << "I will move some files around...";

			// if zoom level has increased, increase zoom levels of tile sets
			for (auto rotation_it = rotations.begin(); rotation_it != rotations.end();
					++rotation_it) {
				fs::path output_dir = config.getOutputPath(map_name + "/"
						+ config::ROTATION_NAMES_SHORT[*rotation_it]);
				for (int i = settings.max_zoom; i < world_zoomlevels; i++)
					increaseMaxZoom(output_dir, map.getImageFormatSuffix());
			}
		}

		// also write the tile offsets to the map settings file
		// to have them next time available even if we don't render/scan this world
		for (int rotation = 0; rotation < 4; rotation++)
			settings.tile_offsets[rotation] = confighelper.getWorldTileOffset(world_name, rotation);

		// now write the (possibly new) max zoom level to the settings file
		settings.max_zoom = world_zoomlevels;
		settings.write(settings_file);
		// and also update the template with the max zoom level
		confighelper.setMapZoomlevel(map_name, settings.max_zoom);
		writeTemplateConfig();

		// again some progress stuff
		int progress_rotations = 0;
		int progress_rotations_all = rotations.size();

		// now go through the rotations and render them
		for (auto rotation_it = rotations.begin(); rotation_it != rotations.end();
				++rotation_it) {
			progress_rotations++;

			int rotation = *rotation_it;

			// continue if we should skip this rotation
			if (confighelper.getRenderBehavior(map_name, rotation)
					== config::MapcrafterConfigHelper::RENDER_SKIP)
				continue;

			LOG(INFO) << "[" << progress_maps << "." << progress_rotations << "/"
					<< progress_maps << "." << progress_rotations_all << "] "
					<< "Rendering rotation " << config::ROTATION_NAMES[rotation] << ":";

			// output a small notice if we render this map incrementally
			if (settings.last_render[rotation] != 0) {
				std::time_t t = settings.last_render[rotation];
				char buffer[256];
				std::strftime(buffer, sizeof(buffer), "%d %b %Y, %H:%M:%S", std::localtime(&t));
				LOG(INFO) << "Last rendering was on " << buffer << ".";
			}

			fs::path output_dir = config.getOutputPath(map_name + "/"
					+ config::ROTATION_NAMES_SHORT[rotation]);
			// if incremental render scan which tiles might have changed
			std::shared_ptr<TileSet> tile_set(render_view->createTileSet());
			// TODO ewwwwwwww
			tile_set->operator=(*tile_sets[world_name][rotation]);
			if (confighelper.getRenderBehavior(map_name, rotation)
					== config::MapcrafterConfigHelper::RENDER_AUTO) {
				LOG(INFO) << "Scanning required tiles...";
				// use the incremental check specified in the config
				if (map.useImageModificationTimes())
					tile_set->scanRequiredByFiletimes(output_dir,
							map.getImageFormatSuffix());
				else
					tile_set->scanRequiredByTimestamp(settings.last_render[rotation]);
			}

			std::time_t time_start = std::time(nullptr);

			// create block images
			std::shared_ptr<BlockImages> block_images(render_view->createBlockImages());
			block_images->setSettings(map.getTextureSize(), rotation, map.renderUnknownBlocks(),
					map.renderLeavesTransparent(), map.getRendermode());
			// if textures do not work, it does not make much sense
			// to try the other rotations with the same textures
			if (!block_images->loadAll(map.getTextureDir().string())) {
				LOG(ERROR) << "Skipping remaining rotations.";
				break;
			}

			// render the map
			if (tile_set->getRequiredRenderTilesCount() == 0) {
				LOG(INFO) << "No tiles need to get rendered.";
				continue;
			}

			std::shared_ptr<mc::WorldCache> world_cache(new mc::WorldCache(worlds[world_name][rotation]));
			RenderModes render_modes;
			createRenderModes(config.getWorld(map.getWorld()), map, render_modes);
			std::shared_ptr<TileRenderer> tile_renderer(render_view->createTileRenderer(block_images, world_cache, render_modes));
			tile_renderer->setRenderBiomes(map.renderBiomes());

			// TODO isometric tile renderer --> setUsePreblitWater

			// TODO maybe set only once per map?
			confighelper.setMapTileSize(map_name, tile_renderer->getTileSize());
			writeTemplateConfig();

			RenderContext context;
			context.output_dir = output_dir;
			context.background_color = config.getBackgroundColor();
			context.world_config = config.getWorld(map.getWorld());
			context.map_config = map;
			context.tile_set = tile_set;
			context.tile_renderer = tile_renderer;

			std::shared_ptr<thread::Dispatcher> dispatcher;
			if (opts.jobs == 1)
				dispatcher = std::make_shared<thread::SingleThreadDispatcher>();
			else
				dispatcher = std::make_shared<thread::MultiThreadingDispatcher>(opts.jobs);

			std::shared_ptr<util::MultiplexingProgressHandler> progress(new util::MultiplexingProgressHandler);

			util::ProgressBar* progress_bar = nullptr;
			if (opts.batch || !util::isOutTTY()) {
				util::Logging::getInstance().setSinkLogProgress("__output__", true);
			} else {
				progress_bar = new util::ProgressBar;
				progress->addHandler(progress_bar);
			}

			util::LogOutputProgressHandler* log_output = new util::LogOutputProgressHandler;
			progress->addHandler(log_output);

			dispatcher->dispatch(context, progress);
			if (progress_bar != nullptr)
				progress_bar->finish();

			// update the settings file with last render time
			settings.rotations[rotation] = true;
			settings.last_render[rotation] = start_scanning;
			settings.write(settings_file);

			std::time_t took = std::time(nullptr) - time_start;
			LOG(INFO) << "[" << progress_maps << "." << progress_rotations << "/"
					<< progress_maps << "." << progress_rotations_all << "] "
					<< "Rendering rotation " << config::ROTATION_NAMES[*rotation_it]
					<< " took " << took << " seconds.";

		}
	}

	std::time_t took_all = std::time(nullptr) - time_start_all;
	LOG(INFO) << "Rendering all worlds took " << took_all << " seconds.";
	LOG(INFO) << "Finished.....aaand it's gone!";
	return true;
}

}
}
