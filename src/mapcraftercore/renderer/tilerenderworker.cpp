/*
 * Copyright 2012-2015 Moritz Hilscher
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
	overlays = createOverlays(world_config, map_config, world.getRotation());
	tile_renderer.reset(render_view->createTileRenderer(block_images,
			map_config.getTileWidth(), world_cache.get(), render_mode.get(), overlays));
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

void TileRenderWorker::saveTile(const TilePath& tile, const std::string& type, const RGBAImage& image) {
	bool png = render_context.map_config.getImageFormat() == config::ImageFormat::PNG;
	bool png_indexed = render_context.map_config.isPNGIndexed();
	std::string suffix = std::string(".") + render_context.map_config.getImageFormatSuffix();
	std::string filename = tile.toString() + suffix;
	if (tile.getDepth() == 0)
		filename = std::string("base") + suffix;
	fs::path file = render_context.output_dir / type / filename;
	if (!fs::exists(file.branch_path()))
		fs::create_directories(file.branch_path());

	if ((png && !png_indexed) && !image.writePNG(file.string()))
		LOG(WARNING) << "Unable to write '" << file.string() << "'.";

	if ((png && png_indexed) && !image.writeIndexedPNG(file.string()))
		LOG(WARNING) << "Unable to write '" << file.string() << "'.";

	config::Color bg = render_context.background_color;
	if (!png && !image.writeJPEG(file.string(),
			render_context.map_config.getJPEGQuality(), rgba(bg.red, bg.green, bg.blue, 255)))
		LOG(WARNING) << "Unable to write '" << file.string() << "'.";
}

void TileRenderWorker::renderRecursive(const TilePath& path, RGBAImage& tile, std::vector<RGBAImage>& overlay_tiles) {
	// TODO overlays!
	assert(overlay_tiles.size() == render_context.overlays.size());

	// if this is tile is not required or we should skip it, try to load it from file
	if (!render_context.tile_set->isTileRequired(path) || render_work.tiles_skip.count(path)) {
		bool png = render_context.map_config.getImageFormat() == config::ImageFormat::PNG;
		bool ok = true;
		for (size_t i = 0; i <= overlay_tiles.size(); i++) {
			std::string type;
			if (i == overlay_tiles.size())
				type = "terrain";
			else
				type = std::string("overlay_") + render_context.overlays[i]->getName();
			RGBAImage& image = tile;
			if (i != overlay_tiles.size())
				image = overlay_tiles[i];
			fs::path file = render_context.output_dir / type
					/ (path.toString() + "." + render_context.map_config.getImageFormatSuffix());
			if ((png && image.readPNG(file.string()))
					|| (!png && image.readJPEG(file.string()))) {
				if (i == overlay_tiles.size() && render_work.tiles_skip.count(path) && progress != nullptr)
					progress->setValue(progress->getValue()
							+ render_context.tile_set->getContainingRenderTiles(path));
			} else {
				ok = false;
				break;
			}
		}

		// return if all tiles are okay (they are now read from disk)
		// render them later on in this method if a tile was not okay
		if (!ok)
			LOG(WARNING) << "Unable to read tile '" << path.toString()
				<< "', I will just render it again.";
		else
			return;
	}

	if (path.getDepth() == render_context.tile_set->getDepth()) {
		// this tile is a render tile, render it
		render_context.tile_renderer->renderTile(path.getTilePos()
				+ render_context.tile_set->getTileOffset(), tile, overlay_tiles);
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
		saveTile(path, "terrain", tile);
		for (size_t i = 0; i < overlay_tiles.size(); i++) {
			std::string type = "overlay_" + render_context.overlays[i]->getName();
			saveTile(path, type, overlay_tiles[i]);
		}

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
		tile.setSize(size, size);
		for (size_t i = 0; i < overlay_tiles.size(); i++)
			overlay_tiles[i].setSize(size, size);

		RGBAImage other;
		RGBAImage resized;
		std::vector<RGBAImage> other_overlays(overlay_tiles.size());
		if (render_context.tile_set->hasTile(path + 1)) {
			renderRecursive(path + 1, other, other_overlays);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			tile.simpleAlphaBlit(resized, 0, 0);
			other.clear();
		
			for (size_t i = 0; i < other_overlays.size(); i++) {
				other_overlays[i].resize(resized, 0, 0, InterpolationType::HALF);
				overlay_tiles[i].simpleAlphaBlit(resized, 0, 0);
				other_overlays[i].clear();
			}
		}
		if (render_context.tile_set->hasTile(path + 2)) {
			renderRecursive(path + 2, other, other_overlays);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			tile.simpleAlphaBlit(resized, size / 2, 0);
			for (size_t i = 0; i < other_overlays.size(); i++) {
				other_overlays[i].resize(resized, 0, 0, InterpolationType::HALF);
				overlay_tiles[i].simpleAlphaBlit(resized, size / 2, 0);
				other_overlays[i].clear();
			}
			other.clear();
		}
		if (render_context.tile_set->hasTile(path + 3)) {
			renderRecursive(path + 3, other, other_overlays);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			tile.simpleAlphaBlit(resized, 0, size / 2);
			for (size_t i = 0; i < other_overlays.size(); i++) {
				other_overlays[i].resize(resized, 0, 0, InterpolationType::HALF);
				overlay_tiles[i].simpleAlphaBlit(resized, 0, size / 2);
				other_overlays[i].clear();
			}
			other.clear();
		}
		if (render_context.tile_set->hasTile(path + 4)) {
			renderRecursive(path + 4, other, other_overlays);
			other.resize(resized, 0, 0, InterpolationType::HALF);
			tile.simpleAlphaBlit(resized, size / 2, size / 2);
			for (size_t i = 0; i < other_overlays.size(); i++) {
				other_overlays[i].resize(resized, 0, 0, InterpolationType::HALF);
				overlay_tiles[i].simpleAlphaBlit(resized, size / 2, size / 2);
				other_overlays[i].clear();
			}
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
		saveTile(path, "terrain", tile);
		for (size_t i = 0; i < overlay_tiles.size(); i++) {
			std::string type = "overlay_" + render_context.overlays[i]->getName();
			saveTile(path, type, overlay_tiles[i]);
		}
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
	
	RGBAImage tile;
	std::vector<RGBAImage> overlay_images(render_context.overlays.size());
	// iterate through the start composite tiles
	for (auto it = render_work.tiles.begin(); it != render_work.tiles.end(); ++it) {
		// render this composite tile
		renderRecursive(*it, tile, overlay_images);

		// clear image
		tile.clear();
		for (size_t i = 0; i < overlay_images.size(); i++)
			overlay_images.clear();
	}
}

} /* namespace render */
} /* namespace mapcrafter */
