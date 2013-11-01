#ifndef WORKER_H_
#define WORKER_H_

#include "../util.h"

#include "../config/mapcrafter_config.h"

#include "../mc/cache.h"

#include "blockimages.h"
#include "tile.h"
#include "render.h"

#include <memory>
#include <set>

namespace mapcrafter {
namespace render {

class Worker {
private:
	std::shared_ptr<mc::WorldCache> world;
	std::shared_ptr<TileSet> tileset;
	std::shared_ptr<BlockImages> blockimages;
	TileRenderer renderer;

	config2::MapcrafterConfigFile config;
	config2::MapSection map_config;
	std::set<TilePath> tiles, tiles_skip;
public:
	Worker();
	~Worker();

	void setWorld(std::shared_ptr<mc::WorldCache> world,
			std::shared_ptr<TileSet> tileset,
			std::shared_ptr<BlockImages> blockimages);
	void setWork(const config2::MapcrafterConfigFile& config,
			const config2::MapSection& map_config,
			const std::set<TilePath>& tiles, const std::set<TilePath>& tiles_skip);

	void operator()();
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* WORKER_H_ */
