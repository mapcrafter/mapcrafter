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

#ifndef TILERENDERWORKER_H_
#define TILERENDERWORKER_H_

#include "rendermodes/overlay.h"
#include "../config/mapcrafterconfig.h"
#include "../config/configsections/map.h"
#include "../config/configsections/overlay.h"
#include "../config/configsections/world.h"
#include "../mc/world.h"

#include <memory>
#include <set>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {

namespace mc {
class WorldCache;
}

namespace renderer {

class BlockImages;
class RenderMode;
class RenderView;
class RGBAImage;
class TilePath;
class TileRenderer;
class TileSet;

struct RenderContext {
	fs::path output_dir;
	config::Color background_color;
	config::WorldSection world_config;
	config::MapSection map_config;
	std::map<std::string, std::shared_ptr<config::OverlaySection>> overlays_config;

	RenderView* render_view;
	BlockImages* block_images;
	TileSet* tile_set;
	mc::World world;

	std::shared_ptr<mc::WorldCache> world_cache;
	std::shared_ptr<RenderMode> render_mode;
	std::shared_ptr<OverlayRenderMode> hardcode_overlay;
	std::vector<std::shared_ptr<OverlayRenderMode>> overlays;
	std::shared_ptr<TileRenderer> tile_renderer;

	/**
	 * Creates/initializes the world cache and tile renderer with the render view and
	 * other supplied objects (block images, tile set, world).
	 *
	 * This is method is already called in the render management code, but you can copy
	 * the render context and call this method again if you need multiple tile renderers
	 * (for multithreading for example).
	 */
	void initializeTileRenderer();
};

struct RenderWork {
	std::set<renderer::TilePath> tiles, tiles_skip;
};

struct RenderWorkResult {
	RenderWorkResult() : tiles_rendered(0) {}

	RenderWork render_work;

	int tiles_rendered;
};

class TileRenderWorker {
public:
	TileRenderWorker();
	~TileRenderWorker();

	void setRenderContext(const RenderContext& context);
	void setRenderWork(const RenderWork& work);
	const RenderWorkResult& getRenderWorkResult() const;

	void setProgressHandler(util::IProgressHandler* progress);

	void saveTile(const TilePath& path, const std::string& type, const RGBAImage& tile, const ImageFormat& image_format);
	void renderRecursive(const TilePath& path, RGBAImage& tile, std::vector<RGBAImage>& overlay_tiles);

	void operator()();

private:
	RenderContext render_context;
	RenderWork render_work;
	RenderWorkResult render_work_result;

	// progress handler
	util::IProgressHandler* progress;
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* TILERENDERWORKER_H_ */
