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

#include "worker.h"

namespace mapcrafter {
namespace render {

RenderWorker::RenderWorker()
		: progress(new util::DummyProgressHandler) {

}

RenderWorker::~RenderWorker() {

}

void RenderWorker::setWorld(std::shared_ptr<mc::WorldCache> worldcache,
		std::shared_ptr<TileSet> tileset) {
	this->worldcache = worldcache;
	this->tileset = tileset;
}

void RenderWorker::setMapConfig(std::shared_ptr<BlockImages> blockimages,
		const config2::MapSection map_config,
		const fs::path& map_output_dir) {
	this->blockimages = blockimages;
	this->map_config = map_config;
	this->map_output_dir = map_output_dir;
}

void RenderWorker::setWork(const std::set<TilePath>& tiles, const std::set<TilePath>& tiles_skip) {
	this->tiles = tiles;
	this->tiles_skip = tiles_skip;
}

void RenderWorker::setProgressHandler(std::shared_ptr<util::IProgressHandler> progress,
		std::shared_ptr<bool> finished) {
	this->progress = progress;
	this->finished = finished;
}

void RenderWorker::saveTile(const TilePath& tile, const Image& image) {
	std::string filename = tile.toString() + ".png";
	if (tile.getDepth() == 0)
		filename = "base.png";
	fs::path file = map_output_dir / filename;
	if (!fs::exists(file.branch_path()))
		fs::create_directories(file.branch_path());
	if (!image.writePNG(file.string()))
		std::cout << "Unable to write " << file.string() << std::endl;
}

void RenderWorker::renderRecursive(const TilePath& tile, Image& image) {
	// if this is tile is not required or we should skip it, load it from file
	if (!tileset->isTileRequired(tile) || tiles_skip.count(tile)) {
		fs::path file = map_output_dir / (tile.toString() + ".png");
		if (!image.readPNG(file.string())) {
			std::cerr << "Unable to read tile " << tile.toString() << " from " << file << std::endl;
			std::cerr << tileset->isTileRequired(tile) << " " << tiles_skip.count(tile) << std::endl;
		}
	} else if (tile.getDepth() == tileset->getDepth()) {
		// this tile is a render tile, render it
		renderer.renderTile(tile.getTilePos(), image);

		/*
		// draws a border on the tile
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
		saveTile(tile, image);

		// update progress
		progress->setValue(progress->getValue() + 1);
	} else {
		// this tile is a composite tile, we need to compose it from its children
		// just check, if children 1, 2, 3, 4 exists, render it, resize it to the half size
		// and blit it to the properly position
		int size = map_config.getTextureSize() * 32 * TILE_WIDTH;
		image.setSize(size, size);

		Image other;
		Image resized;
		if (tileset->hasTile(tile + 1)) {
			renderRecursive(tile + 1, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, 0, 0);
			other.clear();
		}
		if (tileset->hasTile(tile + 2)) {
			renderRecursive(tile + 2, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, size / 2, 0);
			other.clear();
		}
		if (tileset->hasTile(tile + 3)) {
			renderRecursive(tile + 3, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, 0, size / 2);
			other.clear();
		}
		if (tileset->hasTile(tile + 4)) {
			renderRecursive(tile + 4, other);
			other.resizeHalf(resized);
			image.simpleblit(resized, size / 2, size / 2);
		}

		/*
		// draws a border on the tile
		for (int x = 0; x < size; x++)
			for (int y = 0; y < size; y++) {
				if (x < 5 || x > size-5)
					tile.setPixel(x, y, rgba(255, 0, 0, 255));
				if (y < 5 || y > size-5)
					tile.setPixel(x, y, rgba(255, 0, 0, 255));
			}
		*/

		// then save the tile
		saveTile(tile, image);
	}
}

void RenderWorker::operator()() {
	renderer = TileRenderer(worldcache, blockimages, map_config);
	
	int work = 0;
	for (auto it = tiles.begin(); it != tiles.end(); ++it)
		work += tileset->getContainingRenderTiles(*it);
	progress->setMax(work);
	progress->setValue(0);
	*finished = false;
	
	Image image;
	// iterate through the start composite tiles
	for (auto it = tiles.begin(); it != tiles.end(); ++it) {
		// render this composite tile
		renderRecursive(*it, image);

		// clear image
		image.clear();
	}

	*finished = true;
}

} /* namespace render */
} /* namespace mapcrafter */
