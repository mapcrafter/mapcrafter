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
		: texture_size(12), rotation(0), tile_size(0), max_zoom(0),
		  render_unknown_blocks(0), render_leaves_transparent(0),
		  render_biomes(false), last_render(-1) {
}

/**
 * This method reads the map settings from a file.
 */
bool MapSettings::read(const std::string& filename) {
	std::ifstream file;
	file.open(filename.c_str());
	if (!file)
		return false;

	std::string line;
	// read line for line
	while (std::getline(file, line)) {
		std::string::size_type i = line.find(" ", 0);
		if (i == std::string::npos)
			continue;
		// split line by a ' ' into key and value
		// and save the value
		std::string key = line.substr(0, i);
		std::string value = line.substr(i + 1, line.size() - 1);
		if (key == "texture_size")
			texture_size = atoi(value.c_str());
		else if (key == "rotation")
			rotation = atoi(value.c_str());
		else if (key == "tile_size")
			tile_size = atoi(value.c_str());
		else if (key == "max_zoom")
			max_zoom = atoi(value.c_str());
		else if (key == "render_unknown_blocks") {
			render_unknown_blocks = value == "1";
		} else if (key == "render_leaves_transparent") {
			render_leaves_transparent = value == "1";
		} else if (key == "render_biomes") {
			render_biomes = value == "1";
		} else if (key == "last_render")
			last_render = atoi(value.c_str());
	}

	return true;
}

/**
 * This method writes the map settings to a file.
 */
bool MapSettings::write(const std::string& filename) const {
	std::ofstream file(filename.c_str());
	if (!file)
		return false;

	file << "texture_size " << texture_size << std::endl;
	file << "rotation " << rotation << std::endl;
	file << "tile_size " << tile_size << std::endl;
	file << "max_zoom " << max_zoom << std::endl;
	file << "render_unknown_blocks " << render_unknown_blocks << std::endl;
	file << "render_leaves_transparent " << render_leaves_transparent << std::endl;
	file << "render_biomes " << render_biomes << std::endl;
	file << "last_render " << last_render << std::endl;
	file.close();

	return true;
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
	} else if (path.getDepth() == settings.tiles.getMaxZoom()) {
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
void RenderManager::increaseMaxZoom() {
	fs::path out = config.getOutputDir();

	// at first rename the directories 1 2 3 4 (zoom level 0) and make new directories
	for (int i = 1; i <= 4; i++) {
		moveFile(out / str(i), out / (str(i) + "_"));
		fs::create_directory(out / str(i));
	}

	// then move the old tile trees one zoom level deeper
	moveFile(out / "1_", out / "1/4");
	moveFile(out / "2_", out / "2/3");
	moveFile(out / "3_", out / "3/2");
	moveFile(out / "4_", out / "4/1");
	// also move the images of the directories
	moveFile(out / "1.png", out / "1/4.png");
	moveFile(out / "2.png", out / "2/3.png");
	moveFile(out / "3.png", out / "3/2.png");
	moveFile(out / "4.png", out / "4/1.png");

	// now read the images, which belong to the new directories
	Image img1, img2, img3, img4;
	img1.readPNG((out / "1/4.png").string());
	img2.readPNG((out / "2/3.png").string());
	img3.readPNG((out / "3/2.png").string());
	img4.readPNG((out / "4/1.png").string());

	//int s = settings.tile_size;
	int s = 12;
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
	new1.writePNG((out / "1.png").string());
	new2.writePNG((out / "2.png").string());
	new3.writePNG((out / "3.png").string());
	new4.writePNG((out / "4.png").string());

	// don't forget the base.png
	Image base_big(2*s, 2*s), base;
	base_big.simpleblit(new1, 0, 0);
	base_big.simpleblit(new2, s, 0);
	base_big.simpleblit(new3, 0, s);
	base_big.simpleblit(new4, s, s);
	base_big.resizeHalf(base);
	base.writePNG((out / "base.png").string());
}

/**
 * Renders render tiles and composite tiles.
 */
void RenderManager::render(const mc::World& world, const TileSet& tiles,
		const BlockImages& images, const std::string& output_dir) {
	if(tiles.getRequiredCompositeTilesCount() == 0) {
		std::cout << "No tiles need to get rendered." << std::endl;
		return;
	}

	// check if only one thread
	if (opts.jobs == 1) {
		std::cout << "Rendering " << tiles.getRequiredRenderTilesCount()
				<< " tiles on max zoom level " << tiles.getMaxZoom()
				<< "." << std::endl;

		// create needed things for recursiv render method
		mc::WorldCache cache(world);
		TileRenderer renderer(cache, images, true);
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
		renderMultithreaded(world, tiles, images, output_dir);
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
		const BlockImages& images, const std::string& output_dir) {
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
		TileRenderer* renderer = new TileRenderer(*cache, images, true);
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
				<< " tiles on max zoom level " << tiles.getMaxZoom() << "." << std::endl;

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

bool RenderManager::renderWorld(size_t index, const RenderWorldConfig& world_config,
		int rotation) {
	mc::World world;
	if (!world.load(world_config.input_dir, rotation)) {
		std::cerr << "Unable to load the world!" << std::endl;
		return false;
	}

	std::cout << "Creating block images..." << std::endl;
	BlockImages images;
	images.setSettings(world_config.texture_size, rotation, true, true);
	if (!images.loadAll(world_config.textures_dir)) {
		std::cerr << "Unable to create block images!" << std::endl;
		return false;
	}

	std::cout << "Scanning world..." << std::endl;
	TileSet tileset(world, 0);
	config.setMapZoomlevel(index, rotation, tileset.getMaxZoom());
	writeTemplateIndexHtml();

	std::cout << "Start rendering..." << std::endl;

	std::string rotations[] = {"tl", "tr", "br", "bl"};
	std::string output_dir = config.getOutputPath(world_config.name_short + "/" + rotations[rotation]);
	render(world, tileset, images, output_dir);

	return true;
}

/**
 * Starts the whole rendering thing.
 */
bool RenderManager::run() {
	if (!config.loadFile(opts.config_file)) {
		std::cerr << "Error: Unable to read config file!" << std::endl;
		return false;
	}
	if (!config.checkValid())
		return false;

	if (!fs::is_directory(config.getOutputDir())
		&& !fs::create_directories(config.getOutputDir())) {
		std::cerr << "Error: Unable to create output directory!" << std::endl;
		return false;
	}

	std::vector<RenderWorldConfig> worlds = config.getWorlds();

	std::string rotations[] = {"top left", "top right", "bottom right", "bottom left"};
	int i_to = worlds.size();

	int start_all = time(NULL);

	writeTemplates();

	for (size_t i = 0; i < worlds.size(); i++) {
		RenderWorldConfig world = worlds[i];
		int i_from = i+1;
		std::cout << "(" << i_from << "/" << i_to << ") Rendering world "
				<< world.name_short << " (" << world.name_long << "):"
				<< std::endl;

		int j_from = 1;
		int j_to = world.rotations.size();
		for (std::set<int>::iterator it = world.rotations.begin();
				it != world.rotations.end(); ++it) {
			std::cout << "(" << i_from << "." << j_from << "/" << i_from << "."
					<< j_to << ") Rendering rotation " << rotations[*it]
					<< ":" << std::endl;

			int start = time(NULL);
			if (!renderWorld(i, worlds[i], *it)) {
				std::cerr << "Skipping remaining rotations." << std::endl << std::endl;
				break;
			}

			int took = time(NULL) - start;
			std::cout << "Rendering rotation " << rotations[*it] << " took " << took
					<< " seconds." << std::endl << std::endl;

			j_from++;
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
