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

#include "render/manager.h"

#include <fstream>
#include <ctime>
#include <pthread.h>

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
	ConfigFile config;
	if (!config.loadFile(filename))
		return false;

	texture_size = config.get<int>("", "texture_size");
	tile_size = config.get<int>("", "tile_size");
	max_zoom = config.get<int>("", "max_zoom");

	render_unknown_blocks = config.get<bool>("", "render_unknown_blocks");
	render_leaves_transparent = config.get<bool>("", "render_leaves_transparent");
	render_biomes = config.get<bool>("", "render_biomes");

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		rotations[i] = config.hasSection(rotation_names[i]);
		if (rotations[i])
			last_render[i] = config.get<int>(rotation_names[i], "last_render");
	}

	return true;
}

/**
 * This method writes the map settings to a file.
 */
bool MapSettings::write(const std::string& filename) const {
	ConfigFile config;

	config.set("", "texture_size", str(texture_size));
	config.set("", "tile_size", str(tile_size));
	config.set("", "max_zoom", str(max_zoom));

	config.set("", "render_unknown_blocks", str(render_unknown_blocks));
	config.set("", "render_leaves_transparent", str(render_leaves_transparent));
	config.set("", "render_biomes", str(render_biomes));

	std::string rotation_names[4] = {"tl", "tr", "br", "bl"};
	for (int i = 0; i < 4; i++) {
		if (rotations[i])
			config.set(rotation_names[i], "last_render", str(last_render[i]));
	}

	return config.writeFile(filename);
}

bool MapSettings::equalsConfig(const RenderWorldConfig& config) const {
	return texture_size == config.texture_size
			&& render_unknown_blocks == config.render_unknown_blocks
			&& render_leaves_transparent == config.render_leaves_transparent
			&& render_biomes == config.render_biomes;
}

MapSettings MapSettings::byConfig(const RenderWorldConfig& config) {
	MapSettings settings;

	settings.texture_size = config.texture_size;
	settings.tile_size = config.texture_size * 32;

	settings.render_unknown_blocks = config.render_unknown_blocks;
	settings.render_leaves_transparent = config.render_leaves_transparent;
	settings.render_biomes = config.render_biomes;

	for (std::set<int>::const_iterator it = config.rotations.begin();
			it != config.rotations.end(); ++it)
		settings.rotations[*it] = true;

	return settings;
}

/**
 * Saves a tile image.
 */
void saveTile(const fs::path& output_dir, const Path& path, const Image& tile) {
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
void renderRecursive(RecursiveRenderSettings& settings, const Path& path, Image& tile) {
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
		// this tile is a render tile, render it (if we have a renderer)
		if (settings.renderer == NULL)
			return;
		settings.renderer->renderTile(path.getTilePos(), tile);
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
		replaceAll(data, "{" + it->first + "}", it->second);
	}

	std::ofstream out(config.getOutputPath(filename).c_str());
	if (!out)
		return false;
	out << data;
	out.close();
	return true;
}

bool RenderManager::writeTemplateIndexHtml() const {
	std::map<std::string, std::string> vars;
	vars["worlds"] = config.generateJavascript();

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
	if (!copyTemplateFile("markers.js"))
		std::cout << "Warning: Unable to copy template file markers.js!" << std::endl;

	// copy all other files and directories
	fs::directory_iterator end;
	for (fs::directory_iterator it(config.getTemplateDir()); it != end;
			++it) {
#ifdef OLD_BOOST
		std::string filename = it->path().filename();
#else
		std::string filename = it->path().filename().string();
#endif
		if (filename.compare("index.html") == 0 || filename.compare("markers.js") == 0)
			continue;
		if (fs::is_regular_file(*it)) {
			if (!copyFile(*it, config.getOutputPath(filename)))
				std::cout << "Warning: Unable to copy template file " << filename << std::endl;
		} else if (fs::is_directory(*it)) {
			if (!copyDirectory(*it, config.getOutputPath(filename)))
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
	// at first rename the directories 1 2 3 4 (zoom level 0) and make new directories
	for (int i = 1; i <= 4; i++) {
		moveFile(dir / str(i), dir / (str(i) + "_"));
		fs::create_directory(dir / str(i));
	}

	// then move the old tile trees one zoom level deeper
	moveFile(dir / "1_", dir / "1/4");
	moveFile(dir / "2_", dir / "2/3");
	moveFile(dir / "3_", dir / "3/2");
	moveFile(dir / "4_", dir / "4/1");
	// also move the images of the directories
	moveFile(dir / "1.png", dir / "1/4.png");
	moveFile(dir / "2.png", dir / "2/3.png");
	moveFile(dir / "3.png", dir / "3/2.png");
	moveFile(dir / "4.png", dir / "4/1.png");

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
void RenderManager::render(const mc::World& world, const TileSet& tiles,
		const BlockImages& images, const std::string& output_dir, bool biomes) {
	if(tiles.getRequiredCompositeTilesCount() == 0) {
		std::cout << "No tiles need to get rendered." << std::endl;
		return;
	}

	// check if only one thread
	if (opts.jobs == 1) {
		std::cout << "Rendering " << tiles.getRequiredRenderTilesCount()
				<< " tiles on max zoom level " << tiles.getDepth()
				<< "." << std::endl;

		// create needed things for recursiv render method
		mc::WorldCache cache(world);
		TileRenderer renderer(cache, images, biomes);
		RecursiveRenderSettings settings(tiles, &renderer);

		settings.tile_size = images.getTileSize();
		settings.output_dir = output_dir;

		settings.show_progress = true;
		settings.progress_bar = ProgressBar(tiles.getRequiredRenderTilesCount(), !opts.batch);

		Image tile;
		// then render just everything recursive
		renderRecursive(settings, Path(), tile);
		settings.progress_bar.finish();
	} else {
		renderMultithreaded(world, tiles, images, output_dir, biomes);
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
	for (std::set<Path>::const_iterator it = settings->tiles.begin();
			it != settings->tiles.end(); ++it) {

		// render this composite tile
		renderRecursive(*settings->render_settings, *it, tile);

		// clear image, increase progress
		tile.clear();
		settings->base_progress += settings->render_settings->progress;
		settings->render_settings->progress = 0;
	}

	settings->finished = true;
	pthread_exit(NULL);
}

/**
 * This method starts the render threads when multithreading is enabled.
 */
void RenderManager::renderMultithreaded(const mc::World& world, const TileSet& tiles,
		const BlockImages& images, const std::string& output_dir, bool biomes) {
	// a list of workers
	std::vector<std::map<Path, int> > workers;
	// find task/worker assignemt
	int remaining = tiles.findRenderTasks(opts.jobs, workers);

	// create render settings for the remaining composite tiles at the end
	RecursiveRenderSettings remaining_settings(tiles, NULL);
	remaining_settings.tile_size = images.getTileSize();
	remaining_settings.output_dir = output_dir;
	remaining_settings.show_progress = true;

	// list of threads
	std::vector<pthread_t> threads;
	std::vector<RenderWorkerSettings*> worker_settings;
	threads.resize(opts.jobs);
	worker_settings.resize(opts.jobs);
	for (int i = 0; i < opts.jobs; i++) {
		// create all informations needed for the worker
		// every thread has his own cache
		mc::WorldCache* cache = new mc::WorldCache(world);
		TileRenderer* renderer = new TileRenderer(*cache, images, biomes);
		RecursiveRenderSettings* render_settings =
				new RecursiveRenderSettings(tiles, renderer);
		render_settings->tile_size = images.getTileSize();
		render_settings->output_dir = output_dir;
		render_settings->show_progress = false;

		RenderWorkerSettings* settings = new RenderWorkerSettings;
		settings->thread = i;
		settings->render_settings = render_settings;

		// add tasks to thread
		int sum = 0;
		for (std::map<Path, int>::iterator it = workers[i].begin(); it != workers[i].end();
				++it) {
			sum += it->second;
			settings->tiles.insert(it->first);
			remaining_settings.skip_tiles.insert(it->first);
		}

		std::cout << "Thread " << i << " renders " << sum
				<< " tiles on max zoom level " << tiles.getDepth() << "." << std::endl;

		worker_settings[i] = settings;
		// start thread
		pthread_create(&threads[i], NULL, runWorker, (void*) settings);
	}

	ProgressBar progress(tiles.getRequiredRenderTilesCount(), !opts.batch);
	// loop while the render threads are running
	while (1) {
		sleep(1);

		// check if threads are running and update progress_bar
		int sum = 0;
		bool running = false;
		for (int i = 0; i < opts.jobs; i++) {
			sum += worker_settings[i]->base_progress
					+ worker_settings[i]->render_settings->progress;
			running = running || !worker_settings[i]->finished;
		}
		progress.update(sum);
		if (!running)
			break;
	}
	progress.finish();

	// render remaining composite tiles
	std::cout << "Rendering remaining " << remaining << " composite tiles" << std::endl;
	Image tile;
	remaining_settings.progress_bar = ProgressBar(remaining, !opts.batch);
	renderRecursive(remaining_settings, Path(), tile);
	remaining_settings.progress_bar.finish();
}

/**
 * Starts the whole rendering thing.
 */
bool RenderManager::run() {
	// load the configuration file
	if (!config.loadFile(opts.config_file)) {
		std::cerr << "Error: Unable to read config file!" << std::endl;
		return false;
	}
	// validate the configuration file
	if (!config.checkValid())
		return false;
	// set the maps to render/skip/force-render from the command line options
	config.setRenderBehaviors(opts.skip_all, opts.render_skip, opts.render_auto,
			opts.render_force);

	// we need an output directory
	if (!fs::is_directory(config.getOutputDir())
			&& !fs::create_directories(config.getOutputDir())) {
		std::cerr << "Error: Unable to create output directory!" << std::endl;
		return false;
	}

	// get the maps to render
	std::vector<RenderWorldConfig> world_configs = config.getWorlds();

	// check for already existing rendered maps
	// and get the (old) zoom levels for the template,
	// so the user can still view the other maps while rendering
	for (size_t i = 0; i < world_configs.size(); i++) {
		MapSettings settings;
		if (settings.read(config.getOutputPath(world_configs[i].name_short + "/map.settings")))
			config.setWorldMaxZoom(i, settings.max_zoom);
	}

	// write all template files
	writeTemplates();

	int i_to = world_configs.size();
	int start_all = time(NULL);

	// go through all maps
	for (size_t i = 0; i < world_configs.size(); i++) {
		RenderWorldConfig world = world_configs[i];
		// continue, if all rotations for this map are skipped
		if (world.canSkip())
			continue;

		int i_from = i+1;
		std::cout << "(" << i_from << "/" << i_to << ") Rendering map "
				<< world.name_short << " (\"" << world.name_long << "\"):"
				<< std::endl;

		std::string settings_filename = config.getOutputPath(world.name_short + "/map.settings");
		MapSettings settings;
		// check if we have already an old settings file,
		// but ignore the settings file if the whole world is force-rendered
		bool old_settings = !world.isCompleteRenderForce() && fs::exists(settings_filename);
		if (old_settings) {
			if (!settings.read(settings_filename)) {
				std::cerr << "Error: Unable to load old map.settings file!"
						<< std::endl << std::endl;
				continue;
			}

			// check if the config file was not changed when rendering incrementally
			if (!settings.equalsConfig(world)) {
				std::cerr << "Error: The configuration does not equal the settings of the already rendered map." << std::endl;
				std::cerr << "Force-render the whole map (" << world.name_short
						<< ") or reset the configuration to the old settings."
						<< std::endl << std::endl;
				continue;
			}

			// for force-render rotations, set the last render time to 0
			// to render all tiles
			for (int i = 0; i < 4; i++)
				if (world.render_behaviors[i] == RenderWorldConfig::RENDER_FORCE)
					settings.last_render[i] = 0;
		} else {
			// if we don't have a settings file or force-render the whole map
			// -> create a new one
			settings = MapSettings::byConfig(world);
		}

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
			tilesets[*it] = TileSet(worlds[*it], settings.last_render[*it]);
			depth = std::max(depth, tilesets[*it].getMinDepth());
		}

		if (!world_ok)
			continue;

		// check if the max zoom level has increased
		if (old_settings && settings.max_zoom < depth) {
			std::cout << "The max zoom level was increased from " << settings.max_zoom
					<< " to " << depth << "." << std::endl;
			std::cout << "I will move some files around..." << std::endl;
		}

		// now go through the rotations and set the max zoom level
		for (std::set<int>::iterator it = world.rotations.begin();
				it != world.rotations.end(); ++it) {
			tilesets[*it].setDepth(depth);

			// check if this rotation was already rendered on a lower max zoom level
			// -> then: increase max zoom level
			if (old_settings && settings.rotations[*it] && settings.max_zoom < depth) {
				for (int i = settings.max_zoom; i < depth; i++)
					increaseMaxZoom(config.getOutputDir() / world.name_short / ROTATION_NAMES_SHORT[*it]);
			}
		}

		// now write the max zoom level to the settings file
		settings.max_zoom = depth;
		settings.write(settings_filename);
		// and also to the template
		config.setWorldMaxZoom(i, depth);
		writeTemplateIndexHtml();

		// go through the rotations and render them
		int j_from = 0;
		int j_to = world.rotations.size();
		for (std::set<int>::iterator it = world.rotations.begin();
				it != world.rotations.end(); ++it) {
			j_from++;

			// continue if we should skip this rotation
			if (world.render_behaviors[*it] == RenderWorldConfig::RENDER_SKIP)
				continue;

			std::cout << "(" << i_from << "." << j_from << "/" << i_from << "."
					<< j_to << ") Rendering rotation " << ROTATION_NAMES[*it]
					<< ":" << std::endl;

			if (settings.last_render[*it] != 0) {
				time_t t = settings.last_render[*it];
				char buffer[100];
				strftime(buffer, 100, "%d %b %Y, %H:%M:%S", localtime(&t));
				std::cout << "Last rendering was on " << buffer << "." << std::endl;
			}

			int start = time(NULL);

			// create block images and render the world
			BlockImages images;
			images.setSettings(world.texture_size, *it, world.render_unknown_blocks,
					world.render_leaves_transparent);
			if (!images.loadAll(world.textures_dir)) {
				std::cerr << "Unable to create block images!" << std::endl;
				std::cerr << "Skipping remaining rotations." << std::endl << std::endl;
				break;
			}

			std::string output_dir = config.getOutputPath(world.name_short + "/"
					+ ROTATION_NAMES_SHORT[*it]);
			render(worlds[*it], tilesets[*it], images, output_dir, world.render_biomes);

			// update the settings file
			settings.rotations[*it] = true;
			settings.last_render[*it] = start_scanning;
			settings.write(settings_filename);

			int took = time(NULL) - start;
			std::cout << "(" << i_from << "." << j_from << "/" << i_from << "."
					<< j_to << ") Rendering rotation " << ROTATION_NAMES[*it]
					<< " took " << took << " seconds." << std::endl << std::endl;

		}
	}

	int took_all = time(NULL) - start_all;
	std::cout << "Rendering all worlds took " << took_all << " seconds." << std::endl;

	std::cout << std::endl << "Finished.....aaand it's gone!" << std::endl;
	return true;

	/*
	std::cout << "Starting renderer for world " << opts.input_dir << "." << std::endl;

	// load map settings if this a incremental render
	if (opts.incremental && !settings.read(opts.outputPath("map.settings"))) {
		std::cerr << "Error: Unable to read map.settings file." << std::endl;
		std::cerr << "Can't render incrementally." << std::endl;
		return false;
	}

	// if not incremental, set default map settings
	if (!opts.incremental) {
		settings.last_render = -1;
		settings.texture_size = opts.texture_size;
		settings.rotation = opts.rotation;
		settings.render_unknown_blocks = opts.render_unknown_blocks;
		settings.render_leaves_transparent = opts.render_leaves_transparent;
		settings.render_biomes = opts.render_biomes;
	} else {
		time_t now = settings.last_render;
		char buffer[100];
		strftime(buffer, 100, "%d %b %Y, %H:%M:%S", localtime(&now));
		std::cout << "Rendering incrementally since " << buffer << "." << std::endl;
	}

	// load world
	if (!world.load(opts.input_dir.string(), settings.rotation)) {
		std::cerr << "Error: Unable to load the world!" << std::endl;
		return false;
	}

	// make sure, output directory exists
	if (!fs::exists(opts.output_dir) && !fs::create_directories(opts.output_dir)) {
		std::cerr << "Error: Unable to create output dir!" << std::endl;
		return false;
	}

	// give the textures some settings
	images.setSettings(settings.texture_size, settings.rotation,
			settings.render_unknown_blocks, settings.render_leaves_transparent);
	// try to load all textures
	if (!images.loadChests(opts.texturePath("chest.png"), opts.texturePath("largechest.png"),
			opts.texturePath("enderchest.png"))) {
		std::cerr << "Error: Unable to load chest.png, largechest.png or enderchest.png!"
				<< std::endl;
		return false;
	} else if (!images.loadColors(opts.texturePath("foliagecolor.png"),
			opts.texturePath("grasscolor.png"))) {
		std::cerr << "Error: Unable to load foliagecolor.png or grasscolor.png!"
				<< std::endl;
		return false;
	} else if (!images.loadOther(opts.texturePath("fire.png"),
			opts.texturePath("endportal.png"))) {
		std::cerr << "Error: Unable to load fire.png or endportal.png!" << std::endl;
		return false;
	} else if (!images.loadBlocks(opts.texturePath("blocks"))) {
		std::cerr << "Error: Unable to load block textures!" << std::endl;
		return false;
	}

	std::cout << "Setting texture size to " << settings.texture_size << "." << std::endl;
	settings.tile_size = images.getTileSize();

	int render_start = time(NULL);

	// scan world for tiles
	std::cout << "Scanning world..." << std::endl;
	TileSet tiles(world, settings.last_render);
	// check if zoom level has changed, do some tile movements if yes
	if (opts.incremental && tiles.getMaxZoom() > settings.max_zoom) {
		std::cout << "The max zoom has changed since last render, let's move some files around."
				<< std::endl;
		for (int i = 0; i < tiles.getMaxZoom() - settings.max_zoom; i++)
			increaseMaxZoom();
	}
	// save settings, templates
	settings.max_zoom = tiles.getMaxZoom();
	settings.write(opts.outputPath("map.settings"));
	writeTemplates(settings);

	// start the real rendering
	render(tiles);

	// in the end, get needed time and set the start time to last render time
	int took = time(NULL) - render_start;
	settings.last_render = render_start;
	settings.write(opts.outputPath("map.settings"));

	std::cout << "Finished rendering after " << took << " seconds!" << std::endl;

	//writeStats(took);
	std::cout << std::endl << "Finished.....aaand it's gone!" << std::endl;
	return true;
	*/
}

}
}
