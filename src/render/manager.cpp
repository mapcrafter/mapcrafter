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

bool MapSettings::read(const std::string& filename) {
	std::ifstream file;
	file.open(filename.c_str());
	if (!file)
		return false;

	std::string line;
	while (std::getline(file, line)) {
		std::string::size_type i = line.find(" ", 0);
		if (i == std::string::npos)
			continue;
		std::string key = line.substr(0, i);
		std::string value = line.substr(i + 1, line.size() - 1);
		//std::cout << "read config " << key << ":" << value << std::endl;
		if (key.compare("texture_size") == 0)
			texture_size = atoi(value.c_str());
		else if (key.compare("tile_size") == 0)
			tile_size = atoi(value.c_str());
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

bool MapSettings::write(const std::string& filename) const {
	std::ofstream file(filename.c_str());
	if (!file)
		return false;

	file << "texture_size " << texture_size << std::endl;
	file << "tile_size " << tile_size << std::endl;
	file << "render_unknown_blocks " << render_unknown_blocks << std::endl;
	file << "render_leaves_transparent " << render_leaves_transparent << std::endl;
	file << "last_render " << last_render << std::endl;
	file.close();

	return true;
}

RenderManager::RenderManager(const RenderOpts& opts)
		: opts(opts) {
}

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

void RenderManager::writeTemplates(const TileSet& tiles) {
	std::map<std::string, std::string> vars;
	vars["textureSize"] = str(textures.getTextureSize());
	vars["tileSize"] = str(textures.getTileSize());
	vars["maxZoom"] = str(tiles.getMaxZoom());
	vars["lastRender"] = str(time(NULL));

	if (!copyTemplateFile("index.html", vars))
		std::cout << "Warning: Unable to copy template file index.html!" << std::endl;

	vars.clear();
	vars["markers"] = "";
	if (!copyTemplateFile("markers.js", vars))
		std::cout << "Warning: Unable to copy template file markers.js!" << std::endl;

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

void RenderManager::render(const TileSet& tiles) {
	if (tiles.getRequiredCompositeTilesCount() != 0) {
		std::cout << "Rendering " << tiles.getRequiredRenderTilesCount()
		        << " tiles on zoom level " << tiles.getMaxZoom() << "." << std::endl;
		renderBaseTiles(tiles);
	} else {
		std::cout << "No render tiles need to get rendered." << std::endl;
	}

	if (tiles.getRequiredCompositeTilesCount() != 0) {
		std::cout << "Rendering " << tiles.getRequiredCompositeTilesCount()
		        << " tiles on other zoom levels." << std::endl;
		renderCompositeTiles(tiles);
	} else {
		std::cout << "No composite tiles need to get rendered." << std::endl;
	}
}

void* runWorker(void* settings_ptr) {
	RenderWorkerSettings* settings = (RenderWorkerSettings*) settings_ptr;

	Image tile;
	TileRenderer renderer(*settings->worldcache, *settings->textures);
	for (std::set<TilePos>::const_iterator it = settings->render_tiles.begin();
	        it != settings->render_tiles.end(); ++it) {
		renderer.renderTile(*it, tile);

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

void RenderManager::renderBaseTiles(const TileSet& tiles) {
	std::set<TilePos> render_tiles = tiles.getRequiredRenderTiles();
	int depth = tiles.getMaxZoom();

	if (opts.jobs == 1) {
		mc::WorldCache cache(world);
		TileRenderer renderer(cache, textures);
		ProgressBar progress(render_tiles.size());
		int i = 0;
		for (std::set<TilePos>::const_iterator it = render_tiles.begin();
		        it != render_tiles.end(); ++it) {
			Image tile;
			renderer.renderTile(*it, tile);
			saveTile(Path::byTilePos(*it, depth), tile);
			i++;
			progress.update(i);
		}
		progress.finish();

		//cache.getRegionCacheStats().print("Region cache");
		//cache.getChunkCacheStats().print("Chunk cache");
	} else {
		std::vector<TilePos> render_tiles_list;

		std::vector<pthread_t> threads;
		std::vector<RenderWorkerSettings*> worker_settings;
		threads.resize(opts.jobs);
		worker_settings.resize(opts.jobs);

		int progress_count = 0;

		for (std::set<TilePos>::iterator it = render_tiles.begin();
		        it != render_tiles.end(); ++it)
			render_tiles_list.push_back(*it);

		int size = render_tiles.size() / opts.jobs;
		for (int i = 0; i < opts.jobs; i++) {
			std::set<TilePos> worker_tiles;
			int start = size * i;
			int end = size * i + size;
			if (i == opts.jobs - 1)
				end = render_tiles.size();
			for (int j = start; j < end; j++)
				worker_tiles.insert(render_tiles_list[j]);
			pthread_mutex_lock(&testmutex);
			std::cout << "Thread " << i << " renders [" << start << ":" << end << "] = "
			        << (end - start) << " tiles" << std::endl;
			pthread_mutex_unlock(&testmutex);

			mc::WorldCache* worldcache = new mc::WorldCache(world);
			RenderWorkerSettings* settings = new RenderWorkerSettings;
			settings->thread = i;
			settings->worldcache = worldcache;
			settings->textures = &textures;
			settings->depth = depth;
			settings->output_dir = opts.output_dir;
			settings->render_tiles = worker_tiles;

			worker_settings[i] = settings;
			pthread_create(&threads[i], NULL, runWorker, (void*) settings);
		}

		ProgressBar progress(render_tiles.size());
		while (1) {
			sleep(1);

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

void RenderManager::renderCompositeTiles(const TileSet& tiles) {
	Image base;
	ProgressBar progress(tiles.getRequiredCompositeTilesCount());
	int current_progress = 0;
	renderCompositeTile(tiles, Path(), base, progress, current_progress);
	progress.finish();
}

void RenderManager::renderCompositeTile(const TileSet& tiles, const Path& path,
        Image& tile, ProgressBar& progress, int& current_progress) {
	if (path.getDepth() == tiles.getMaxZoom()) {
		TilePos pos = path.getTilePos();
		std::string file = opts.outputPath(path.toString()) + ".png";
		if (!tile.readPNG(file)) {
			//auto available = tiles.getAvailableRenderTiles();
			//auto required = tiles.getRequiredRenderTiles();
			std::cerr << "Unable to read tile " << path.toString() << " at " << pos.getX()
			        << ":" << pos.getY() << " from " << file << std::endl;
			//std::cout << "avail=" << available.count(path.getTilePos()) << " required=" << required.count(path.getTilePos()) << std::endl;
		}

		//for(int x = 0; x < tileSize; x++) {
		//	for(int y = 0; y < tileSize; y++) {
		//		if(x < 10 || tileSize-10 < x || y < 10 || tileSize-10 < y)
		//			tile.setPixel(x, y, rgba(0, 0, 0, 255));
		//	}
		//}
	} else if (!tiles.isTileRequired(path)) {
		std::string file = opts.outputPath(path.toString()) + ".png";
		if (!tile.readPNG(file)) {
			//auto available = tiles.getAvailableCompositeTiles();
			//auto required = tiles.getRequiredCompositeTiles();
			std::cerr << "Unable to read composite tile " << path.toString() << " from "
			        << file << std::endl;
			//std::cout << "avail=" << available.count(path) << " required=" << required.count(path) << std::endl;
		}
	} else {
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
		saveTile(path, tile);
		current_progress++;
		progress.update(current_progress);
	}
}

void RenderManager::saveTile(const Path& path, Image& tile) const {
	std::string filename = path.toString() + ".png";
	if (path.getDepth() == 0)
		filename = "base.png";
	fs::path file = opts.output_dir / filename;
	if (!fs::exists(file.branch_path()))
		fs::create_directories(file.branch_path());
	if (!tile.writePNG(file.string()))
		std::cout << "Unable to write " << file.string() << std::endl;
}

bool RenderManager::run() {
	std::cout << "Starting renderer for world " << opts.input_dir << "." << std::endl;

	if (!world.load(opts.input_dir.string())) {
		std::cerr << "Error: Unable to load the world!" << std::endl;
		return false;
	}

	if (!fs::exists(opts.output_dir) && !fs::create_directories(opts.output_dir)) {
		std::cerr << "Error: Unable to create output dir!" << std::endl;
		return false;
	}

	if (opts.incremental && !settings.read(opts.outputPath("map.settings"))) {
		std::cerr << "Error: Unable to read map.settings file.";
		std::cerr << "Can't render incrementally.";
		return false;
	}

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

	textures.setSettings(settings.texture_size, settings.render_unknown_blocks,
	        settings.render_leaves_transparent);
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
	settings.write(opts.outputPath("map.settings"));
	int render_start = time(NULL);

	TileSet tiles(world, settings.last_render);
	writeTemplates(tiles);

	render(tiles);

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
