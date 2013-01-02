/*
 * Copyright 2012 Moritz Hilscher
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

#include "render/render.h"
#include "render/tile.h"
#include "util.h"

#include <fstream>
#include <ctime>
#include <boost/date_time.hpp>
#include <pthread.h>

namespace mapcrafter {
namespace render {

std::string RenderOpts::dataPath(const std::string& path) const {
	return (data_dir / path).string();
}

std::string RenderOpts::outputPath(const std::string& path) const {
	return (output_dir / path).string();
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
		if (key.compare("texture_size") == 0)
			texture_size = atoi(value.c_str());
		else if (key.compare("tile_size") == 0)
			tile_size = atoi(value.c_str());
		else if (key.compare("max_zoom") == 0)
			max_zoom = atoi(value.c_str());
		else if (key.compare("render_unknown_blocks") == 0) {
			if (value.compare("1") == 0)
				render_unknown_blocks = true;
			else
				render_unknown_blocks = false;
		} else if (key.compare("render_leaves_transparent") == 0) {
			if (value.compare("1") == 0)
				render_leaves_transparent = true;
			else
				render_leaves_transparent = false;
		} else if (key.compare("last_render") == 0)
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
	file << "tile_size " << tile_size << std::endl;
	file << "max_zoom " << max_zoom << std::endl;
	file << "render_unknown_blocks " << render_unknown_blocks << std::endl;
	file << "render_leaves_transparent " << render_leaves_transparent << std::endl;
	file << "last_render " << last_render << std::endl;
	file.close();

	return true;
}

RenderManager::RenderManager(const RenderOpts& opts)
		: opts(opts) {
}

/**
 * This method copies a file from the template directory to the output directory and
 * replaces the variables from the map.
 */
bool RenderManager::copyTemplateFile(const std::string& filename,
		std::map<std::string, std::string> vars) {
	std::ifstream file(opts.dataPath("template/" + filename).c_str());
	if (!file)
		return false;
	std::stringstream ss;
	ss << file.rdbuf();
	file.close();
	std::string data = ss.str();

	for (std::map<std::string, std::string>::iterator it = vars.begin(); it != vars.end();
			++it) {
		replaceAll(data, "{" + it->first + "}", it->second);
	}

	std::ofstream out(opts.outputPath(filename).c_str());
	if (!out)
		return false;
	out << data;
	out.close();
	return true;
}

/**
 * This method copies all template files to the output directory.
 */
void RenderManager::writeTemplates(const TileSet& tiles) {
	// the variables for the index.html
	std::map<std::string, std::string> vars;
	vars["textureSize"] = str(textures.getTextureSize());
	vars["tileSize"] = str(textures.getTileSize());
	vars["maxZoom"] = str(tiles.getMaxZoom());
	vars["lastRender"] = str(time(NULL));

	if (!copyTemplateFile("index.html", vars))
		std::cout << "Warning: Unable to copy template file index.html!" << std::endl;

	// the variables for the markers.js - still in work
	vars.clear();
	vars["markers"] = "";
	if (!copyTemplateFile("markers.js", vars))
		std::cout << "Warning: Unable to copy template file markers.js!" << std::endl;

	// copy all other files and directories
	fs::path to(opts.output_dir);
	fs::directory_iterator end;
	for (fs::directory_iterator it(fs::path(opts.dataPath("template"))); it != end;
			++it) {
		std::string filename = it->path().filename().string();
		if (filename.compare("index.html") == 0 || filename.compare("markers.js") == 0)
			continue;
		if (fs::is_regular_file(*it)) {
			if (!copyFile(*it, to / filename))
				std::cout << "Warning: Unable to copy file " << filename << std::endl;
		} else if (fs::is_directory(*it)) {
			if (!copyDirectory(*it, to / filename))
				std::cout << "Warning: Unable to copy directory " << filename
						<< std::endl;
		}
	}
}

void RenderManager::writeStats(int time_took) {
	/*
	 std::cout << "Writing stats..." << std::endl;

	 std::ofstream file(opts.outputPath("statistics.html").c_str(), std::ios::trunc);
	 if (!file) {
	 std::cerr << "Error: Can't open file " << opts.outputPath("statistics.html")
	 << " for the statistics!" << std::endl;
	 return;
	 }
	 // TODO use strftime here
	 //boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	 file << "<h1>Statistics</h1>";
	 file << "<h2>Render statistics</h2><table>";
	 //file << "<tr><th>Last render: </th><td>" << now << "</td></tr>";
	 file << "<tr><th>Time took: </th><td>" << time_took << " seconds</td></tr>";
	 file << "</table>";

	 int chunks = 42;
	 file << "<h2>World statistics</h2>";
	 file << "<table>";
	 file << "<tr><th>Regions: </th><td>" << world.getRegionCount() << "</td></tr>";
	 file << "<tr><th>Chunks: </th><td>" << chunks << "</td></tr>";
	 file << "<tr><th>Area: </th><td>" << (chunks * 16 * 16) / 1000000.
	 << " km^2</td></tr>";
	 file << "</table><br />";
	 file.close();
	 */
}

/**
 * This method increases the max zoom of a rendered map and makes the necessary changes
 * on the tile tree.
 */
void RenderManager::increaseMaxZoom() {
	fs::path out = opts.output_dir;

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

	int s = settings.tile_size;
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
void RenderManager::render(const TileSet& tiles) {
	// check if there are render tiles and render them
	if (tiles.getRequiredRenderTilesCount() != 0) {
		std::cout << "Rendering " << tiles.getRequiredRenderTilesCount()
				<< " tiles on zoom level " << tiles.getMaxZoom() << "." << std::endl;
		renderBaseTiles(tiles);
	} else {
		std::cout << "No render tiles need to get rendered." << std::endl;
	}

	// check if there are composite tiles and render them
	if (tiles.getRequiredCompositeTilesCount() != 0) {
		std::cout << "Rendering " << tiles.getRequiredCompositeTilesCount()
				<< " tiles on other zoom levels." << std::endl;
		renderCompositeTiles(tiles);
	} else {
		std::cout << "No composite tiles need to get rendered." << std::endl;
	}
}

/**
 * This function runs a worker thread.
 */
void* runWorker(void* settings_ptr) {
	// get the worker settings
	RenderWorkerSettings* settings = (RenderWorkerSettings*) settings_ptr;

	Image tile;
	// create a tile renderer
	TileRenderer renderer(*settings->worldcache, *settings->textures);
	// iterate through the render tiles
	for (std::set<TilePos>::const_iterator it = settings->render_tiles.begin();
			it != settings->render_tiles.end(); ++it) {
		// render the tile
		renderer.renderTile(*it, tile);

		// get the filename of the tile and save it
		Path path = Path::byTilePos(*it, settings->depth);
		std::string filename = path.toString() + ".png";
		if (path.getDepth() == 0)
			filename = "base.png";
		fs::path file = settings->output_dir / filename;
		if (!fs::exists(file.branch_path()))
			fs::create_directories(file.branch_path());
		if (!tile.writePNG(file.string()))
			std::cout << "Unable to write " << file.string() << std::endl;

		tile.clear();
		settings->progress++;
	}

	settings->finished = true;
	pthread_exit(NULL);
}

/**
 * This method renders the render tiles.
 */
void RenderManager::renderBaseTiles(const TileSet& tiles) {
	// get the required render tiles and the max zoom level
	std::set<TilePos> render_tiles = tiles.getRequiredRenderTiles();
	int depth = tiles.getMaxZoom();

	// if the render should only render with one thread
	if (opts.jobs == 1) {
		// create cache, tile renderer, and a nice progress bar
		mc::WorldCache cache(world);
		TileRenderer renderer(cache, textures);
		ProgressBar progress(render_tiles.size(), !opts.batch);
		int i = 0;
		// go through all required tiles and just render, save them
		for (std::set<TilePos>::const_iterator it = render_tiles.begin();
				it != render_tiles.end(); ++it) {
			Image tile;
			renderer.renderTile(*it, tile);
			saveTile(Path::byTilePos(*it, depth), tile);
			i++;
			progress.update(i);
		}
		progress.finish();
	} else {
		// a list with render tiles converted from the set
		std::vector<TilePos> render_tiles_list;
		for (std::set<TilePos>::iterator it = render_tiles.begin();
					it != render_tiles.end(); ++it)
				render_tiles_list.push_back(*it);

		// a list of threads and their worker settings
		std::vector<pthread_t> threads;
		std::vector<RenderWorkerSettings*> worker_settings;
		threads.resize(opts.jobs);
		worker_settings.resize(opts.jobs);

		// count of tiles, a thread has to render
		int size = render_tiles.size() / opts.jobs;
		// go through all threads
		for (int i = 0; i < opts.jobs; i++) {
			// get the render tiles for this thread
			// TODO here maybe some better assignment
			std::set<TilePos> worker_tiles;
			int start = size * i;
			int end = size * i + size;
			if (i == opts.jobs - 1)
				end = render_tiles.size();
			for (int j = start; j < end; j++)
				worker_tiles.insert(render_tiles_list[j]);
			std::cout << "Thread " << i << " renders [" << start << ":" << end << "] = "
					<< (end - start) << " tiles" << std::endl;

			// create all informations needed for the worker
			// every thread has an own cache
			mc::WorldCache* worldcache = new mc::WorldCache(world);
			RenderWorkerSettings* settings = new RenderWorkerSettings;
			settings->thread = i;
			settings->worldcache = worldcache;
			settings->textures = &textures;
			settings->depth = depth;
			settings->output_dir = opts.output_dir;
			settings->render_tiles = worker_tiles;

			worker_settings[i] = settings;
			// start thread
			pthread_create(&threads[i], NULL, runWorker, (void*) settings);
		}

		ProgressBar progress(render_tiles.size(), !opts.batch);
		// loop while the render threads are running
		while (1) {
			sleep(1);

			// check if threads are running and update progress
			int sum = 0;
			bool running = false;
			for (int i = 0; i < opts.jobs; i++) {
				sum += worker_settings[i]->progress;
				running = running || !worker_settings[i]->finished;
			}
			progress.update(sum);
			if (!running)
				break;
		}
		progress.finish();
	}
}

/**
 * This method renders the composite tiles.
 */
void RenderManager::renderCompositeTiles(const TileSet& tiles) {
	Image base;
	ProgressBar progress(tiles.getRequiredCompositeTilesCount(), !opts.batch);
	int current_progress = 0;
	// start recursively rendering with the base tile at zoom level 0
	renderCompositeTile(tiles, Path(), base, progress, current_progress);
	progress.finish();
}

/**
 * This method does the main composite tile rendering work.
 */
void RenderManager::renderCompositeTile(const TileSet& tiles, const Path& path,
		Image& tile, ProgressBar& progress, int& current_progress) {
	// if this tile is a render tile, read it from disk
	if (path.getDepth() == tiles.getMaxZoom()) {
		TilePos pos = path.getTilePos();
		std::string file = opts.outputPath(path.toString()) + ".png";
		if (!tile.readPNG(file)) {
			std::cerr << "Unable to read tile " << path.toString() << " at " << pos.getX()
					<< ":" << pos.getY() << " from " << file << std::endl;
		}
	// if this tile is a composite tile, but not required, read it also from disk
	} else if (!tiles.isTileRequired(path)) {
		std::string file = opts.outputPath(path.toString()) + ".png";
		if (!tile.readPNG(file)) {
			std::cerr << "Unable to read composite tile " << path.toString() << " from "
					<< file << std::endl;
		}
	} else {
		// this tile is a composite tile, we need to compose it from its children
		// just check, if children 1, 2, 3, 4 exists, render it, resize it to the half size
		// and blit it to the properly position
		int tile_size = textures.getTileSize();
		tile.setSize(tile_size, tile_size);

		Image other;
		Image resized;
		if (tiles.hasTile(path + 1)) {
			renderCompositeTile(tiles, path + 1, other, progress, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, 0, 0);
			other.clear();
		}
		if (tiles.hasTile(path + 2)) {
			renderCompositeTile(tiles, path + 2, other, progress, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, tile_size / 2, 0);
			other.clear();
		}
		if (tiles.hasTile(path + 3)) {
			renderCompositeTile(tiles, path + 3, other, progress, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, 0, tile_size / 2);
			other.clear();
		}
		if (tiles.hasTile(path + 4)) {
			renderCompositeTile(tiles, path + 4, other, progress, current_progress);
			other.resizeHalf(resized);
			tile.simpleblit(resized, tile_size / 2, tile_size / 2);
		}
		// then save tile, increase progress
		saveTile(path, tile);
		current_progress++;
		progress.update(current_progress);
	}
}

/**
 * This method saves a tile image to a file.
 */
void RenderManager::saveTile(const Path& path, Image& tile) const {
	// get the filename
	std::string filename = path.toString() + ".png";
	// zoom level 0 is "base.png"
	if (path.getDepth() == 0)
		filename = "base.png";
	// then add output directory
	fs::path file = opts.output_dir / filename;
	// check if directory exists, if not, create it
	if (!fs::exists(file.branch_path()))
		fs::create_directories(file.branch_path());
	// then save it
	if (!tile.writePNG(file.string()))
		std::cout << "Unable to write " << file.string() << std::endl;
}

/**
 * Starts the render manager.
 */
bool RenderManager::run() {
	std::cout << "Starting renderer for world " << opts.input_dir << "." << std::endl;

	// load world
	if (!world.load(opts.input_dir.string())) {
		std::cerr << "Error: Unable to load the world!" << std::endl;
		return false;
	}

	// make sure, output directory exists
	if (!fs::exists(opts.output_dir) && !fs::create_directories(opts.output_dir)) {
		std::cerr << "Error: Unable to create output dir!" << std::endl;
		return false;
	}

	// load map settings if this a incremental render
	if (opts.incremental && !settings.read(opts.outputPath("map.settings"))) {
		std::cerr << "Error: Unable to read map.settings file.";
		std::cerr << "Can't render incrementally.";
		return false;
	}

	// if not incremental, set default map settings
	if (!opts.incremental) {
		settings.last_render = -1;
		settings.texture_size = opts.texture_size;
		settings.render_unknown_blocks = opts.render_unknown_blocks;
		settings.render_leaves_transparent = opts.render_leaves_transparent;
	} else {
		time_t now = settings.last_render;
		char buffer[100];
		strftime(buffer, 100, "%d %b %Y, %H:%M:%S", localtime(&now));
		std::cout << "Rendering incrementally since " << buffer << "." << std::endl;
	}

	// give the textures some settings
	textures.setSettings(settings.texture_size, settings.render_unknown_blocks,
			settings.render_leaves_transparent);
	// try to load all textures
	if (!textures.loadChests(opts.dataPath("chest.png"), opts.dataPath("largechest.png"),
			opts.dataPath("enderchest.png"))) {
		std::cerr << "Error Unable to load chest.png, largechest.png or enderchest.png!"
				<< std::endl;
		return false;
	} else if (!textures.loadOther(opts.dataPath("fire.png"),
			opts.dataPath("endportal.png"))) {
		std::cerr << "Error Unable to load fire.png or endportal.png!" << std::endl;
		return false;
	} else if (!textures.loadBlocks(opts.dataPath("terrain.png"))) {
		std::cerr << "Error: Unable to load terrain.png!" << std::endl;
		return false;
	}

	std::cout << "Setting texture size to " << settings.texture_size << "." << std::endl;
	settings.tile_size = textures.getTileSize();

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
	writeTemplates(tiles);

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
}

}
}
