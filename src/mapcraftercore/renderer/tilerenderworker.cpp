/*
 * Copyright 2012-2016 Moritz Hilscher
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

#include "tilerenderworker.h"

#include "blockimages.h"
#include "image.h"
#include "rendermode.h"
#include "renderview.h"
#include "tilerenderer.h"
#include "tileset.h"
#include "../mc/worldcache.h"
#include "../util.h"

namespace mapcrafter {
namespace renderer {

void RenderContext::initializeTileRenderer() {
	world_cache.reset(new mc::WorldCache(world));
	render_mode.reset(createRenderMode(world_config, map_config, world.getRotation()));
	tile_renderer.reset(render_view->createTileRenderer(block_images,
			map_config.getTileWidth(), world_cache.get(), render_mode.get()));
	render_view->configureTileRenderer(tile_renderer.get(), world_config, map_config);
}

TileRenderWorker::TileRenderWorker()
	: progress(nullptr) {
}

TileRenderWorker::~TileRenderWorker() {
}

void TileRenderWorker::setRenderContext(const RenderContext& context) {
	render_context = context;
}

void TileRenderWorker::setRenderWork(const RenderWork& work) {
	render_work = work;
	render_work_result = RenderWorkResult();
	render_work_result.render_work = work;
}

const RenderWorkResult& TileRenderWorker::getRenderWorkResult() const {
	return render_work_result;
}

void TileRenderWorker::setProgressHandler(util::IProgressHandler* progress) {
	this->progress = progress;
}

void TileRenderWorker::saveTile(const TilePath& tile, const RGBAImage& image) {
	bool png = render_context.map_config.getImageFormat() == config::ImageFormat::PNG;
	bool png_indexed = render_context.map_config.isPNGIndexed();
	std::string suffix = std::string(".") + render_context.map_config.getImageFormatSuffix();
	std::string filename = tile.toString() + suffix;
	if (tile.getDepth() == 0)
		filename = std::string("base") + suffix;
	fs::path file = render_context.output_dir / filename;
	if (!fs::exists(file.parent_path()))
		fs::create_directories(file.parent_path());

	if ((png && !png_indexed) && !image.writePNG(file.string()))
		LOG(WARNING) << "Unable to write '" << file.string() << "'.";

	if ((png && png_indexed) && !image.writeIndexedPNG(file.string()))
		LOG(WARNING) << "Unable to write '" << file.string() << "'.";

	config::Color bg = render_context.background_color;
	if (!png && !image.writeJPEG(file.string(),
			render_context.map_config.getJPEGQuality(), rgba(bg.red, bg.green, bg.blue, 255)))
		LOG(WARNING) << "Unable to write '" << file.string() << "'.";
}

void TileRenderWorker::renderRecursive(const TilePath& tile, RGBAImage& image) {
	// if this is tile is not required or we should skip it, try to load it from file
	if (!render_context.tile_set->isTileRequired(tile)
			|| render_work.tiles_skip.count(tile)) {
		bool png = render_context.map_config.getImageFormat() == config::ImageFormat::PNG;
		fs::path file = render_context.output_dir
				/ (tile.toString() + "." + render_context.map_config.getImageFormatSuffix());
		if ((png && image.readPNG(file.string()))
				|| (!png && image.readJPEG(file.string()))) {
			if (render_work.tiles_skip.count(tile) && progress != nullptr)
				progress->setValue(progress->getValue()
						+ render_context.tile_set->getContainingRenderTiles(tile));
			return;
		}

		LOG(WARNING) << "Unable to read tile '" << tile.toString()
				<< "', I will just render it again.";
	}

	if (tile.getDepth() == render_context.tile_set->getDepth()) {
		// this tile is a render tile, render it
		render_context.tile_renderer->renderTile(tile.getTilePos()
				+ render_context.tile_set->getTileOffset(), image);
		render_work_result.tiles_rendered++;

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
		if (progress != nullptr)
			progress->setValue(progress->getValue() + 1);
	} else {
		// this tile is a composite tile, we need to compose it from its children
		// just check, if children 1, 2, 3, 4 exists, render it, resize it to the half size
		// and blit it to the properly position
		//int size = render_context.map_config.getTextureSize() * 32 * TILE_WIDTH;
		// TODO
		int size = render_context.tile_renderer->getTileSize();
		image.setSize(size, size);

		RGBAImage other;
		RGBAImage resized;
		if (render_context.tile_set->hasTile(tile + 1)) {
			renderRecursive(tile + 1, other);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			image.simpleAlphaBlit(resized, 0, 0);
			other.clear();
		}
		if (render_context.tile_set->hasTile(tile + 2)) {
			renderRecursive(tile + 2, other);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			image.simpleAlphaBlit(resized, size / 2, 0);
			other.clear();
		}
		if (render_context.tile_set->hasTile(tile + 3)) {
			renderRecursive(tile + 3, other);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			image.simpleAlphaBlit(resized, 0, size / 2);
			other.clear();
		}
		if (render_context.tile_set->hasTile(tile + 4)) {
			renderRecursive(tile + 4, other);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			image.simpleAlphaBlit(resized, size / 2, size / 2);
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

void TileRenderWorker::operator()() {
	int work = 0;
	for (auto it = render_work.tiles.begin(); it != render_work.tiles.end(); ++it)
		work += render_context.tile_set->getContainingRenderTiles(*it);
	if (progress != nullptr) {
		progress->setMax(work);
		progress->setValue(0);
	}
	
	RGBAImage image;
	// iterate through the start composite tiles
	for (auto it = render_work.tiles.begin(); it != render_work.tiles.end(); ++it) {
		// render this composite tile
		renderRecursive(*it, image);

		// clear image
		image.clear();
	}
}

} /* namespace render */
} /* namespace mapcrafter */
