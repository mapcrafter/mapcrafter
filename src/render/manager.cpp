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
	config2::ConfigFile config;
	if (!config.loadFile(filename))
		return false;

	config2::ConfigSection& root = config.getRootSection();

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
	config2::ConfigFile config;
	config2::ConfigSection& root = config.getRootSection();

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

bool MapSettings::equalsMapConfig(const config2::MapSection& map) const {
	return texture_size == map.getTextureSize()
			&& render_unknown_blocks == map.renderUnknownBlocks()
			&& render_leaves_transparent == map.renderLeavesTransparent()
			&& render_biomes == map.renderBiomes();
}

MapSettings MapSettings::byMapConfig(const config2::MapSection& map) {
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

/**
 * Saves a tile image.
 */
void saveTile(const fs::path& output_dir, const TilePath& path, const Image& tile) {
	std::string filename = path.toString() + ".png";
	if (path.getDepth() == 0)
		filename = "base.png";
	fs::path file = output_dir / filename;
	if (!fs::exists(file.branch_path()))
		fs::create_directories(file.branch_path());
	if (!tile.writePNG(file.string()))
		std::cout << "Unable to write " << file.string() << std::endl;
}

/**
 * This function renders tiles recursive.
 */
void renderRecursive(RecursiveRenderSettings& settings, const TilePath& path, Image& tile) {
	// if this is tile is not required or we should skip it, load it from file
	if (!settings.tiles.isTileRequired(path)
			|| settings.skip_tiles.count(path) == 1) {
		fs::path file = settings.output_dir / (path.toString() + ".png");
		if (!tile.readPNG(file.string())) {
			std::cerr << "Unable to read tile " << path.toString()
				<< " from " << file << std::endl;
			std::cerr << settings.tiles.isTileRequired(path) << " " << settings.skip_tiles.count(path) << std::endl;
		}
	} else if (path.getDepth() == settings.tiles.getDepth()) {
		// this tile is a render tile, render it
		settings.renderer.renderTile(path.getTilePos(), tile);

		/*
		int size = settings.tile_size;
		for (int x = 0; x < size; x++)
			for (int y = 0; y < size; y++) {
				if (x < 5 || x > size-5)
					tile.setPixel(x, y, rgba(0, 0, 255, 255));
				if (y < 5 || y > size-5)
					tile.setPixel(x, y, rgba(0, 0, 255, 255));
			}
		*/

		// save it
		saveTile(settings.output_dir, path, tile);

		// update progress
		settings.progress++;
		if (settings.show_progress) {
			settings.progress_bar.update(settings.progress);
		}
	} else {
		// this tile is a composite tile, we need to compose it from its children
		// just check, if children 1, 2, 3, 4 exists, render it, resize it to the half size
		// and blit it to the properly position
		int size = settings.tile_size;
		tile.setSize(size, size);

		Image other;
		Image resized;
		if (settings.tiles.hasTile(path + 1)) {
			renderRecursive(settings, path + 1, other);
			//renderCompositeTile(tiles, path + 1, other, progress_bar, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, 0, 0);
			other.clear();
		}
		if (settings.tiles.hasTile(path + 2)) {
			renderRecursive(settings, path + 2, other);
			//renderCompositeTile(tiles, path + 2, other, progress_bar, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, size / 2, 0);
			other.clear();
		}
		if (settings.tiles.hasTile(path + 3)) {
			renderRecursive(settings, path + 3, other);
			//renderCompositeTile(tiles, path + 3, other, progress_bar, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, 0, size / 2);
			other.clear();
		}
		if (settings.tiles.hasTile(path + 4)) {
			renderRecursive(settings, path + 4, other);
			//renderCompositeTile(tiles, path + 4, other, progress_bar, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, size / 2, size / 2);
		}

		/*
		for (int x = 0; x < size; x++)
			for (int y = 0; y < size; y++) {
				if (x < 5 || x > size-5)
					tile.setPixel(x, y, rgba(255, 0, 0, 255));
				if (y < 5 || y > size-5)
					tile.setPixel(x, y, rgba(255, 0, 0, 255));
			}
		*/

		// then save tile
		saveTile(settings.output_dir, path, tile);
	}
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
void RenderManager::render(const config2::MapSection& map, const std::string& output_dir,
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
		worker.setMapConfig(blockimages, map, output_dir);

		std::set<TilePath> tiles, tiles_skip;
		tiles.insert(TilePath());
		worker.setWork(tiles, tiles_skip);

		std::shared_ptr<util::ProgressBar> progress(new util::ProgressBar(0, !opts.batch));
		worker.setProgressHandler(progress);

		// start rendering
		worker();
		progress->finish();
	} else {
		renderMultithreaded(map, output_dir, world, tileset, blockimages);
	}
}

/**
 * This function runs a worker thread.
 */
void* runWorker(void* settings_ptr) {
	// get the worker settings
	RenderWorkerSettings* settings = (RenderWorkerSettings*) settings_ptr;

	Image tile;
	// iterate through the start composite tiles
	for (std::set<TilePath>::const_iterator it = settings->tiles.begin();
			it != settings->tiles.end(); ++it) {

		// render this composite tile
		renderRecursive(settings->render_settings, *it, tile);

		// clear image, increase progress
		tile.clear();
		settings->base_progress += settings->render_settings.progress;
		settings->render_settings.progress = 0;
	}

	settings->finished = true;
	pthread_exit(NULL);
}

/**
 * This method starts the render threads when multithreading is enabled.
 */
void RenderManager::renderMultithreaded(const config2::MapSection& map,
		const std::string& output_dir, const mc::World& world, std::shared_ptr<TileSet> tileset,
		std::shared_ptr<BlockImages> blockimages) {
	// a list of workers
	std::vector<std::map<TilePath, int> > workers;
	// find task/worker assignemt
	int remaining = tileset->findRenderTasks(opts.jobs, workers);

	// create render settings for the remaining composite tiles at the end
	RecursiveRenderSettings remaining_settings(*tileset, TileRenderer());
	remaining_settings.tile_size = blockimages->getTileSize();
	remaining_settings.output_dir = output_dir;
	remaining_settings.show_progress = true;

	// list of threads/workers
	std::vector<pthread_t> threads(opts.jobs);
	std::vector<RenderWorkerSettings*> worker_settings;
	std::vector<TileRenderer*> worker_renderers;

	for (int i = 0; i < opts.jobs; i++) {
		// create all informations needed for the worker

		std::shared_ptr<mc::WorldCache> cache(new mc::WorldCache(world));
		TileRenderer* renderer = new TileRenderer(cache, blockimages, map);
		RecursiveRenderSettings render_settings(*tileset, *renderer);
		render_settings.tile_size = blockimages->getTileSize();
		render_settings.output_dir = output_dir;
		render_settings.show_progress = false;

		RenderWorkerSettings* settings = new RenderWorkerSettings;
		settings->thread = i;
		settings->render_settings = render_settings;

		// add tasks to thread
		int sum = 0;
		for (std::map<TilePath, int>::iterator it = workers[i].begin(); it != workers[i].end();
				++it) {
			sum += it->second;
			settings->tiles.insert(it->first);
			remaining_settings.skip_tiles.insert(it->first);
		}

		std::cout << "Thread " << i << " renders " << sum
				<< " tiles on max zoom level " << tileset->getDepth() << "." << std::endl;

		worker_settings.push_back(settings);
		worker_renderers.push_back(renderer);

		// start thread
		pthread_create(&threads[i], NULL, runWorker, (void*) settings);
	}

	util::ProgressBar progress(tileset->getRequiredRenderTilesCount(), !opts.batch);
	// loop while the render threads are running
	while (1) {
	
#if defined(__WIN32__) || defined(__WIN64__)
		Sleep(1);
#else
		sleep(1);
#endif

		// check if threads are running and update progress_bar
		int sum = 0;
		bool running = false;
		for (int i = 0; i < opts.jobs; i++) {
			sum += worker_settings[i]->base_progress
					+ worker_settings[i]->render_settings.progress;
			running = running || !worker_settings[i]->finished;
		}
		progress.update(sum);
		if (!running)
			break;
	}
	progress.finish();

	// free some memory used by the workers
	for (int i = 0; i < opts.jobs; i++) {
		delete worker_settings[i];
		delete worker_renderers[i];
	}

	// render remaining composite tiles
	std::cout << "Rendering remaining " << remaining << " composite tiles" << std::endl;
	Image tile;
	remaining_settings.progress_bar = util::ProgressBar(remaining, !opts.batch);
	renderRecursive(remaining_settings, TilePath(), tile);
	remaining_settings.progress_bar.finish();
}

/**
 * Starts the whole rendering thing.
 */
bool RenderManager::run() {

	config2::ValidationMap validation;
	bool ok = config.parse(opts.config_file, validation);

	if (validation.size() > 0) {
		std::cout << (ok ? "Some notes on your configuration file:" : "Your configuration file is invalid!") << std::endl;
		for (auto it = validation.begin(); it != validation.end(); ++it) {
			if (it->second.empty())
				continue;
			std::cout << it->first << ":" << std::endl;
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
				std::cout << " - " << *it2 << std::endl;
		}
	}
	if (!ok)
		return false;

	/*
	// set the maps to render/skip/force-render from the command line options
	config.setRenderBehaviors(opts.skip_all, opts.render_skip, opts.render_auto,
			opts.render_force);
	*/

	// we need an output directory
	if (!fs::is_directory(config.getOutputDir())
			&& !fs::create_directories(config.getOutputDir())) {
		std::cerr << "Error: Unable to create output directory!" << std::endl;
		return false;
	}

	confighelper = config2::MapcrafterConfigHelper(config);
	confighelper.parseRenderBehaviors(opts.skip_all, opts.render_skip, opts.render_auto, opts.render_force);
	auto config_worlds = config.getWorlds();
	auto config_maps = config.getMaps();

	std::map<std::string, std::array<mc::World, 4> > worlds;
	std::map<std::string, std::array<std::shared_ptr<TileSet>, 4> > tilesets;

	// find out which rotations are needed for which world
	// ...and...
	// check for already existing rendered maps
	// and get the (old) zoom levels for the template,
	// so the user can still view the other maps while rendering
	for (auto it = config_maps.begin(); it != config_maps.end(); ++it) {
		confighelper.setUsedRotations(it->getWorld(), it->getRotations());

		MapSettings settings;
		if (settings.read(config.getOutputPath(it->getShortName() + "/map.settings")))
			confighelper.setMapZoomlevel(it->getShortName(), settings.max_zoom);
	}

	std::cout << "Scanning worlds..." << std::endl;

	for (auto it = config_worlds.begin(); it != config_worlds.end(); ++it) {
		int zoomlevels_max = 0;
		auto rotations = confighelper.getUsedRotations(it->first);
		for (auto it2 = rotations.begin(); it2 != rotations.end(); ++it2) {
			mc::World world;
			if (!world.load(it->second.getInputDir().string(), *it2)) {
				std::cerr << "Unable to load world " << it->first << "!" << std::endl;
				return false;
			}
			std::shared_ptr<TileSet> tileset(new TileSet(world));
			zoomlevels_max = std::max(zoomlevels_max, tileset->getMinDepth());

			worlds[it->first][*it2] = world;
			tilesets[it->first][*it2] = tileset;
		}

		for (auto it2 = rotations.begin(); it2 != rotations.end(); ++it2) {
			tilesets[it->first][*it2]->setDepth(zoomlevels_max);
		}
		confighelper.setWorldZoomlevel(it->first, zoomlevels_max);
	}

	// write all template files
	writeTemplates();

	int i_to = config_maps.size();
	int start_all = time(NULL);

	// go through all maps
	for (size_t i = 0; i < config_maps.size(); i++) {
		config2::MapSection map = config_maps[i];
		std::string mapname = map.getShortName();
		std::string worldname = map.getWorld();
		// continue, if all rotations for this map are skipped
		if (confighelper.isCompleteRenderSkip(mapname))
			continue;

		int i_from = i+1;
		std::cout << "(" << i_from << "/" << i_to << ") Rendering map "
				<< map.getShortName() << " (\"" << map.getLongName() << "\"):"
				<< std::endl;

		if (!fs::is_directory(config.getOutputDir() / mapname))
			fs::create_directories(config.getOutputDir() / mapname);

		std::string settings_filename = config.getOutputPath(mapname + "/map.settings");
		MapSettings settings;
		// check if we have already an old settings file,
		// but ignore the settings file if the whole world is force-rendered
		bool old_settings = !confighelper.isCompleteRenderForce(mapname) && fs::exists(settings_filename);
		if (old_settings) {
			if (!settings.read(settings_filename)) {
				std::cerr << "Error: Unable to load old map.settings file!" << std::endl << std::endl;
				continue;
			}

			// check if the config file was not changed when rendering incrementally
			if (!settings.equalsMapConfig(map)) {
				std::cerr << "Error: The configuration does not equal the settings of the already rendered map." << std::endl;
				std::cerr << "Force-render the whole map (" << mapname
						<< ") or reset the configuration to the old settings."
						<< std::endl << std::endl;
				continue;
			}

			// for force-render rotations, set the last render time to 0
			// to render all tiles
			for (int i = 0; i < 4; i++)
				if (confighelper.getRenderBehavior(mapname, i) == config2::MapcrafterConfigHelper::RENDER_FORCE)
					settings.last_render[i] = 0;
		} else {
			// if we don't have a settings file or force-render the whole map
			// -> create a new one
			settings = MapSettings::byMapConfig(map);
		}

		int start_scanning = time(NULL);
		/*
		// scan the different rotated versions of the world
		// find the highest max zoom level of these tilesets to use this for all rotations
		// all rotations should have the same max zoom level
		// to allow a nice interactively rotateable map
		std::cout << "Scanning the world..." << std::endl;

		mc::World worlds[4];
		TileSet tilesets[4];
		bool world_ok = true;

		int start_scanning = time(NULL);
		int depth = 0;
		for (std::set<int>::iterator it = world.rotations.begin();
				it != world.rotations.end(); ++it) {
			if (!worlds[*it].load(world.input_dir, *it)) {
				std::cerr << "Unable to load the world!" << std::endl;
				std::cerr << "Skipping this world." << std::endl << std::endl;
				world_ok = false;
				break;
			}
			tilesets[*it] = TileSet(worlds[*it]/*, settings.last_render[*it]*//*);
			depth = std::max(depth, tilesets[*it].getMinDepth());
		}

		if (!world_ok)
			continue;
		*/

		int world_zoomlevels = confighelper.getWorldZoomlevel(worldname);
		// check if the max zoom level has increased
		if (old_settings && settings.max_zoom < world_zoomlevels) {
			std::cout << "The max zoom level was increased from " << settings.max_zoom
					<< " to " << world_zoomlevels << "." << std::endl;
			std::cout << "I will move some files around..." << std::endl;
		}

		/*
		// now go through the rotations and set the max zoom level
		for (std::set<int>::iterator it = world.rotations.begin();
				it != world.rotations.end(); ++it) {
			tilesets[*it].setDepth(depth);

			fs::path output_dir = config.getOutputDir() / world.name_short / config::ROTATION_NAMES_SHORT[*it];
			// check if this rotation was already rendered on a lower max zoom level
			// -> then: increase max zoom level
			if (old_settings && settings.rotations[*it] && settings.max_zoom < depth) {
				for (int i = settings.max_zoom; i < depth; i++)
					increaseMaxZoom(output_dir);
			}

			// if this is an incremental rendering...
			if (world.render_behaviors[*it] == config::RenderWorldConfig::RENDER_AUTO) {
				// ...scan the required tiles
				if (world.incremental_detection == "filetimes")
					tilesets[*it].scanRequiredByFiletimes(output_dir);
				else
					tilesets[*it].scanRequiredByTimestamp(settings.last_render[*it]);
			}
		}
		*/

		// now write the zoomlevel to the settings file
		settings.max_zoom = world_zoomlevels;
		settings.write(settings_filename);
		// and also to the template
		confighelper.setMapZoomlevel(mapname, settings.max_zoom);
		writeTemplateIndexHtml();

		// go through the rotations and render them
		int j_from = 0;
		int j_to = map.getRotations().size();
		auto rotations = map.getRotations();
		for (auto it = rotations.begin(); it != rotations.end(); ++it) {
			j_from++;

			int rotation = *it;

			// continue if we should skip this rotation
			if (confighelper.getRenderBehavior(map.getShortName(), rotation)
					== config2::MapcrafterConfigHelper::RENDER_SKIP)
				continue;

			std::cout << "(" << i_from << "." << j_from << "/" << i_from << "."
					<< j_to << ") Rendering rotation " << config2::ROTATION_NAMES[rotation]
					<< ":" << std::endl;

			if (settings.last_render[rotation] != 0) {
				time_t t = settings.last_render[*it];
				char buffer[100];
				strftime(buffer, 100, "%d %b %Y, %H:%M:%S", localtime(&t));
				std::cout << "Last rendering was on " << buffer << "." << std::endl;
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

			std::string output_dir = config.getOutputPath(mapname + "/" + config2::ROTATION_NAMES_SHORT[rotation]);
			render(map, output_dir, worlds[worldname][rotation], tilesets[worldname][rotation], blockimages);

			// update the settings file
			settings.rotations[rotation] = true;
			settings.last_render[rotation] = start_scanning;
			settings.write(settings_filename);

			int took = time(NULL) - start;
			std::cout << "(" << i_from << "." << j_from << "/" << i_from << "."
					<< j_to << ") Rendering rotation " << config::ROTATION_NAMES[*it]
					<< " took " << took << " seconds." << std::endl << std::endl;

		}
	}

	int took_all = time(NULL) - start_all;
	std::cout << "Rendering all worlds took " << took_all << " seconds." << std::endl;

	std::cout << std::endl << "Finished.....aaand it's gone!" << std::endl;
	return true;
}

}
}
