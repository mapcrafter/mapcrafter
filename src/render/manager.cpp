/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "manager.h"

#include "worker.h"

#include <array>
#include <fstream>
#include <ctime>
#include <thread>
#include <pthread.h>

#if defined(__WIN32__) || defined(__WIN64__)
  #include <windows.h>
#endif

namespace mapcrafter {
namespace render {

MapSettings::MapSettings()
		: texture_size(12), tile_size(0), max_zoom(0),
		  render_unknown_blocks(0), render_leaves_transparent(0),
		  render_biomes(false) {
	rotations.resize(4, false);
	last_render.resize(4, 0);
}

/**
 * This method reads the map settings from a file.
 */
bool MapSettings::read(const std::string& filename) {
	config::ConfigFile config;
	if (!config.loadFile(filename))
		return false;

	config::ConfigSection& root = config.getRootSection();

	texture_size = root.get<int>("texture_size");
	tile_size = root.get<int>("tile_size");
	max_zoom = root.get<int>("max_zoom");

	render_unknown_blocks = root.get<bool>("render_unknown_blocks");
	render_leaves_transparent = root.get<bool>("render_leaves_transparent");
	render_biomes = root.get<bool>("render_biomes");

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		rotations[i] = config.hasSection("rotation", rotation_names[i]);
		if (rotations[i])
			last_render[i] = config.getSection("rotation", rotation_names[i]).get<int>("last_render");
	}

	return true;
}

/**
 * This method writes the map settings to a file.
 */
bool MapSettings::write(const std::string& filename) const {
	config::ConfigFile config;
	config::ConfigSection& root = config.getRootSection();

	root.set("texture_size", util::str(texture_size));
	root.set("tile_size", util::str(tile_size));
	root.set("max_zoom", util::str(max_zoom));

	root.set("render_unknown_blocks", util::str(render_unknown_blocks));
	root.set("render_leaves_transparent", util::str(render_leaves_transparent));
	root.set("render_biomes", util::str(render_biomes));

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		if (rotations[i])
			config.getSection("rotation", rotation_names[i]).set("last_render", util::str(last_render[i]));
	}

	return config.writeFile(filename);
}

bool MapSettings::equalsMapConfig(const config::MapSection& map) const {
	return texture_size == map.getTextureSize()
			&& render_unknown_blocks == map.renderUnknownBlocks()
			&& render_leaves_transparent == map.renderLeavesTransparent()
			&& render_biomes == map.renderBiomes();
}

MapSettings MapSettings::byMapConfig(const config::MapSection& map) {
	MapSettings settings;

	settings.texture_size = map.getTextureSize();
	settings.tile_size = map.getTextureSize() * 32;

	settings.render_unknown_blocks = map.renderUnknownBlocks();
	settings.render_leaves_transparent = map.renderLeavesTransparent();
	settings.render_biomes = map.renderBiomes();

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
	std::ifstream file(config.getTemplatePath(filename).c_str());
	if (!file)
		return false;
	std::stringstream ss;
	ss << file.rdbuf();
	file.close();
	std::string data = ss.str();

	for (std::map<std::string, std::string>::const_iterator it = vars.begin();
			it != vars.end(); ++it) {
		util::replaceAll(data, "{" + it->first + "}", it->second);
	}

	std::ofstream out(config.getOutputPath(filename).c_str());
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
	vars["worlds"] = confighelper.generateTemplateJavascript();

	return copyTemplateFile("index.html", vars);
}

/**
 * This method copies all template files to the output directory.
 */
void RenderManager::writeTemplates() const {
	if (!fs::is_directory(config.getTemplateDir())) {
		std::cout << "Warning: The template directory does not exist! Can't copy templates!"
				<< std::endl;
		return;
	}

	if (!writeTemplateIndexHtml())
		std::cout << "Warning: Unable to copy template file index.html!" << std::endl;

	if (!fs::exists(config.getOutputPath("markers.js"))
			&& !util::copyFile(config.getTemplatePath("markers.js"), config.getOutputPath("markers.js")))
		std::cout << "Warning: Unable to copy template file markers.js!" << std::endl;

	// copy all other files and directories
	fs::directory_iterator end;
	for (fs::directory_iterator it(config.getTemplateDir()); it != end;
			++it) {
		std::string filename = BOOST_FS_FILENAME(it->path());
		if (filename == "index.html"
				|| filename == "markers.js")
			continue;
		if (fs::is_regular_file(*it)) {
			if (!util::copyFile(*it, config.getOutputPath(filename)))
				std::cout << "Warning: Unable to copy template file " << filename << std::endl;
		} else if (fs::is_directory(*it)) {
			if (!util::copyDirectory(*it, config.getOutputPath(filename)))
				std::cout << "Warning: Unable to copy template directory " << filename
						<< std::endl;
		}
	}
}

/**
 * This method increases the max zoom of a rendered map and makes the necessary changes
 * on the tile tree.
 */
void RenderManager::increaseMaxZoom(const fs::path& dir) const {
	if (fs::exists(dir / "1")) {
		// at first rename the directories 1 2 3 4 (zoom level 0) and make new directories
		util::moveFile(dir / "1", dir / "1_");
		fs::create_directories(dir / "1");
		// then move the old tile trees one zoom level deeper
		util::moveFile(dir / "1_", dir / "1/4");
		// also move the images of the directories
		util::moveFile(dir / "1.png", dir / "1/4.png");
	}

	// do the same for the other directories
	if (fs::exists(dir / "2")) {
		util::moveFile(dir / "2", dir / "2_");
		fs::create_directories(dir / "2");
		util::moveFile(dir / "2_", dir / "2/3");
		util::moveFile(dir / "2.png", dir / "2/3.png");
	}
	
	if (fs::exists(dir / "3")) {
		util::moveFile(dir / "3", dir / "3_");
		fs::create_directories(dir / "3");
		util::moveFile(dir / "3_", dir / "3/2");
		util::moveFile(dir / "3.png", dir / "3/2.png");
	}
	
	if (fs::exists(dir / "4")) {
		util::moveFile(dir / "4", dir / "4_");
		fs::create_directories(dir / "4");
		util::moveFile(dir / "4_", dir / "4/1");
		util::moveFile(dir / "4.png", dir / "4/1.png");
	}

	// now read the images, which belong to the new directories
	Image img1, img2, img3, img4;
	img1.readPNG((dir / "1/4.png").string());
	img2.readPNG((dir / "2/3.png").string());
	img3.readPNG((dir / "3/2.png").string());
	img4.readPNG((dir / "4/1.png").string());

	int s = img1.getWidth();
	// create images for the new directories
	Image new1(s, s), new2(s, s), new3(s, s), new4(s, s);
	Image old1, old2, old3, old4;
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
	new1.writePNG((dir / "1.png").string());
	new2.writePNG((dir / "2.png").string());
	new3.writePNG((dir / "3.png").string());
	new4.writePNG((dir / "4.png").string());

	// don't forget the base.png
	Image base_big(2*s, 2*s), base;
	base_big.simpleblit(new1, 0, 0);
	base_big.simpleblit(new2, s, 0);
	base_big.simpleblit(new3, 0, s);
	base_big.simpleblit(new4, s, s);
	base_big.resizeHalf(base);
	base.writePNG((dir / "base.png").string());
}

/**
 * Renders render tiles and composite tiles.
 */
void RenderManager::render(const config::MapSection& map_config, const std::string& output_dir,
		const mc::World& world, std::shared_ptr<TileSet> tileset,
		std::shared_ptr<BlockImages> blockimages) {
	if(tileset->getRequiredCompositeTilesCount() == 0) {
		std::cout << "No tiles need to get rendered." << std::endl;
		return;
	}

	// check if only one thread
	if (opts.jobs == 1) {
		std::cout << "Rendering " << tileset->getRequiredRenderTilesCount();
		std::cout << " tiles on max zoom level " << tileset->getDepth() << "." << std::endl;

		// create cache for the worker
		std::shared_ptr<mc::WorldCache> cache(new mc::WorldCache(world));

		// create the worker and set required data
		RenderWorker worker;
		worker.setWorld(cache, tileset);
		worker.setMapConfig(blockimages, map_config, output_dir);

		std::set<TilePath> tiles, tiles_skip;
		tiles.insert(TilePath());
		worker.setWork(tiles, tiles_skip);

		std::shared_ptr<util::ProgressBar> progress(new util::ProgressBar(0, !opts.batch));
		worker.setProgressHandler(progress);

		// start rendering
		worker();
		progress->finish();
	} else {
		renderMultithreaded(map_config, output_dir, world, tileset, blockimages);
	}
}

/**
 * This method starts the render threads when multithreading is enabled.
 */
void RenderManager::renderMultithreaded(const config::MapSection& map_config,
		const std::string& output_dir, const mc::World& world, std::shared_ptr<TileSet> tileset,
		std::shared_ptr<BlockImages> blockimages) {
	std::cout << "Rendering " << tileset->getRequiredRenderTilesCount();
	std::cout << " render tiles on zoom level " << tileset->getDepth() << "." << std::endl;

	// a list of workers
	std::vector<std::map<TilePath, int> > workers;
	// find task/worker assignemt
	int remaining = tileset->findRenderTasks(opts.jobs, workers);

	std::vector<std::thread> threads;
	std::vector<std::shared_ptr<mc::WorldCache>> threads_worldcache;
	std::vector<std::shared_ptr<util::IProgressHandler>> threads_progress;
	std::vector<std::shared_ptr<bool>> threads_finished;

	std::set<TilePath> remaining_tiles, remaining_tiles_skip;
	remaining_tiles.insert(TilePath());

	for (int i = 0; i < opts.jobs; i++) {
		std::shared_ptr<mc::WorldCache> worldcache(new mc::WorldCache(world));
		std::shared_ptr<util::IProgressHandler> progress(new util::DummyProgressHandler);
		std::shared_ptr<bool> finished(new bool);

		threads_worldcache.push_back(worldcache);
		threads_progress.push_back(progress);
		threads_finished.push_back(finished);

		RenderWorker worker;
		worker.setWorld(worldcache, tileset);
		worker.setMapConfig(blockimages, map_config, output_dir);

		std::set<TilePath> tiles, tiles_skip;
		int work = 0;
		for (auto it = workers[i].begin(); it != workers[i].end(); ++it) {
			tiles.insert(it->first);
			remaining_tiles_skip.insert(it->first);
			work += it->second;
		}
		worker.setWork(tiles, tiles_skip);
		worker.setProgressHandler(progress, finished);

		threads.push_back(std::thread(worker));

		std::cout << "Thread " << i << " renders " << work << " tiles." << std::endl;
	}

	util::ProgressBar progress;
	progress.setMax(tileset->getContainingRenderTiles(TilePath()));

	while (1) {
		int value = 0;
		bool finished = true;
		for (int i = 0; i < opts.jobs; i++) {
			value += threads_progress[i]->getValue();
			finished = finished && *threads_finished[i];
		}
		progress.setValue(value);

		if (finished)
			break;

		sleep(1);
	}

	// make sure all threads are finished
	for (int i = 0;  i < opts.jobs; i++)
		threads[i].join();
	progress.finish();

	std::cout << "Rendering remaining " << remaining << " composite tiles." << std::endl;

	RenderWorker worker;
	worker.setWorld(std::make_shared<mc::WorldCache>(world), tileset);
	worker.setMapConfig(blockimages, map_config, output_dir);
	worker.setWork(remaining_tiles, remaining_tiles_skip);

	std::shared_ptr<util::ProgressBar> remaining_progress(new util::ProgressBar);
	worker.setProgressHandler(remaining_progress);
	worker();
	remaining_progress->finish();
}

/**
 * Starts the whole rendering thing.
 */
bool RenderManager::run() {

	// ###
	// ### First big step: Load/parse/validate the configuration file
	// ###

	config::ValidationMap validation;
	bool ok = config.parse(opts.config_file, validation);

	// show infos/warnings/errors if configuration file has something
	if (validation.size() > 0) {
		if (ok)
			std::cout << "Some notes on your configuration file:" << std::endl;
		else
			std::cout << "Your configuration file is invalid!" << std::endl;
		for (auto section_it = validation.begin(); section_it != validation.end(); ++section_it) {
			if (section_it->second.empty())
				continue;
			std::cout << section_it->first << ":" << std::endl;
			for (auto message_it = section_it->second.begin(); message_it != section_it->second.end(); ++message_it)
				std::cout << " - " << *message_it << std::endl;
		}
	}
	if (!ok)
		return false;

	// an output directory would be nice -- create one if it does not exist
	if (!fs::is_directory(config.getOutputDir()) && !fs::create_directories(config.getOutputDir())) {
		std::cerr << "Error: Unable to create output directory!" << std::endl;
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
	std::map<std::string, std::array<std::shared_ptr<TileSet>, 4> > tilesets;

	// go through all maps and:
	// 1. - find out which rotations are needed for which world
	// 2. - and check if there are already rendered maps
	//    - get the old max zoom levels of these maps for the template
	//    -> so the user can still view his already rendered maps while new ones are rendering
	for (auto map_it = config_maps.begin(); map_it != config_maps.end(); ++map_it) {
		confighelper.setUsedRotations(map_it->getWorld(), map_it->getRotations());
		MapSettings settings;
		if (settings.read(config.getOutputPath(map_it->getShortName() + "/map.settings")))
			confighelper.setMapZoomlevel(map_it->getShortName(), settings.max_zoom);
	}

	// ###
	// ### Second big step: Scan the worlds
	// ###

	std::cout << "Scanning worlds..." << std::endl;
	for (auto world_it = config_worlds.begin(); world_it != config_worlds.end(); ++world_it) {
		std::string world_name = world_it->first;

		// at first check if we really need to scan this world
		// scan only if there is at least one map which...
		//  ... is rendered with this world and which
		//  ... is not set to skip
		bool used = false;
		for (auto map_it = config_maps.begin(); map_it != config_maps.end(); ++map_it) {
			if (!confighelper.isCompleteRenderSkip(map_it->getShortName())) {
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
			mc::World world;
			if (!world.load(world_it->second.getInputDir().string(), *rotation_it)) {
				std::cerr << "Unable to load world " << world_name << "!" << std::endl;
				return false;
			}
			// create a tileset for this world
			std::shared_ptr<TileSet> tileset(new TileSet(world));
			// update the highest max zoom level
			zoomlevels_max = std::max(zoomlevels_max, tileset->getMinDepth());

			// set world- and tileset object in the map
			worlds[world_name][*rotation_it] = world;
			tilesets[world_name][*rotation_it] = tileset;
		}

		// now apply this highest max zoom level
		for (auto rotation_it = rotations.begin(); rotation_it != rotations.end(); ++rotation_it)
			tilesets[world_name][*rotation_it]->setDepth(zoomlevels_max);
		// also give this highest max zoom level to the config helper
		confighelper.setWorldZoomlevel(world_name, zoomlevels_max);
	}

	// write all template files
	writeTemplates();

	// ###
	// ### Third big step: Render the maps
	// ###

	int i_to = config_maps.size();
	int start_all = time(NULL);

	// go through all maps
	for (size_t i = 0; i < config_maps.size(); i++) {
		config::MapSection map = config_maps[i];
		std::string map_name = map.getShortName();
		std::string world_name = map.getWorld();
		// continue, if all rotations for this map are skipped
		if (confighelper.isCompleteRenderSkip(map_name))
			continue;

		int i_from = i+1;
		std::cout << "(" << i_from << "/" << i_to << ") Rendering map " << map.getShortName();
		std::cout << " (\"" << map.getLongName() << "\"):" << std::endl;

		if (!fs::is_directory(config.getOutputDir() / map_name))
			fs::create_directories(config.getOutputDir() / map_name);

		std::string settings_filename = config.getOutputPath(map_name + "/map.settings");
		MapSettings settings;
		// check if we have already an old settings file,
		// but ignore the settings file if the whole map is force-rendered
		bool old_settings = !confighelper.isCompleteRenderForce(map_name) && fs::exists(settings_filename);
		if (old_settings) {
			if (!settings.read(settings_filename)) {
				std::cerr << "Error: Unable to load old map.settings file!" << std::endl << std::endl;
				continue;
			}

			// check if the config file was not changed when rendering incrementally
			if (!settings.equalsMapConfig(map)) {
				std::cerr << "Error: The configuration does not equal the settings of the already rendered map." << std::endl;
				std::cerr << "Force-render the whole map (" << map_name
						<< ") or reset the configuration to the old settings."
						<< std::endl << std::endl;
				continue;
			}

			// for force-render rotations, set the last render time to 0
			// to render all tiles
			for (int i = 0; i < 4; i++)
				if (confighelper.getRenderBehavior(map_name, i) == config::MapcrafterConfigHelper::RENDER_FORCE)
					settings.last_render[i] = 0;
		} else {
			// if we don't have a settings file or if we should force-render the whole map
			// -> create a new settings file
			settings = MapSettings::byMapConfig(map);
		}

		int start_scanning = time(NULL);

		auto rotations = map.getRotations();
		int world_zoomlevels = confighelper.getWorldZoomlevel(world_name);
		// check if the max zoom level has increased
		if (old_settings && settings.max_zoom < world_zoomlevels) {
			std::cout << "The max zoom level was increased from " << settings.max_zoom;
			std::cout << " to " << world_zoomlevels << "." << std::endl;
			std::cout << "I will move some files around..." << std::endl;

			for (auto rotation_it = rotations.begin(); rotation_it != rotations.end();
					++rotation_it) {
				std::string output_dir = config.getOutputPath(map_name + "/" + config::ROTATION_NAMES_SHORT[*rotation_it]);
				for (int i = settings.max_zoom; i < world_zoomlevels; i++)
					increaseMaxZoom(output_dir);
			}
		}

		// now write the zoomlevel to the settings file
		settings.max_zoom = world_zoomlevels;
		settings.write(settings_filename);
		// and also to the template
		confighelper.setMapZoomlevel(map_name, settings.max_zoom);
		writeTemplateIndexHtml();

		// go through the rotations and render them
		int j_from = 0;
		int j_to = rotations.size();
		for (auto it = rotations.begin(); it != rotations.end(); ++it) {
			j_from++;

			int rotation = *it;

			// continue if we should skip this rotation
			if (confighelper.getRenderBehavior(map_name, rotation)
					== config::MapcrafterConfigHelper::RENDER_SKIP)
				continue;

			std::cout << "(" << i_from << "." << j_from << "/" << i_from << ".";
			std::cout << j_to << ") Rendering rotation " << config::ROTATION_NAMES[rotation];
			std::cout << ":" << std::endl;

			if (settings.last_render[rotation] != 0) {
				time_t t = settings.last_render[rotation];
				char buffer[100];
				strftime(buffer, 100, "%d %b %Y, %H:%M:%S", localtime(&t));
				std::cout << "Last rendering was on " << buffer << "." << std::endl;
			}

			std::string output_dir = config.getOutputPath(map_name + "/" + config::ROTATION_NAMES_SHORT[rotation]);
			// if incremental render: scan which tiles might have changed
			std::shared_ptr<TileSet> tileset(new TileSet(*tilesets[world_name][rotation]));
			if (confighelper.getRenderBehavior(map_name, rotation)
					== config::MapcrafterConfigHelper::RENDER_AUTO) {
				std::cout << "Scanning required tiles..." << std::endl;
				//if (map.get.incremental_detection == "filetimes")
					tileset->scanRequiredByFiletimes(output_dir);
				//else
				//	tileset->scanRequiredByTimestamp(settings.last_render[rotation]);
			}

			int start = time(NULL);

			// create block images and render the world
			std::shared_ptr<BlockImages> blockimages(new BlockImages);
			blockimages->setSettings(map.getTextureSize(), rotation, map.renderUnknownBlocks(),
					map.renderLeavesTransparent(), map.getRendermode());
			if (!blockimages->loadAll(map.getTextureDir().string())) {
				std::cerr << "Skipping remaining rotations." << std::endl << std::endl;
				break;
			}

			render(map, output_dir, worlds[world_name][rotation], tileset, blockimages);

			// update the settings file
			settings.rotations[rotation] = true;
			settings.last_render[rotation] = start_scanning;
			settings.write(settings_filename);

			int took = time(NULL) - start;
			std::cout << "(" << i_from << "." << j_from << "/" << i_from << "." << j_to;
			std::cout << ") Rendering rotation " << config::ROTATION_NAMES[*it];
			std::cout << " took " << took << " seconds." << std::endl << std::endl;

		}
	}

	int took_all = time(NULL) - start_all;
	std::cout << "Rendering all worlds took " << took_all << " seconds." << std::endl;

	std::cout << std::endl << "Finished.....aaand it's gone!" << std::endl;
	return true;
}

}
}
