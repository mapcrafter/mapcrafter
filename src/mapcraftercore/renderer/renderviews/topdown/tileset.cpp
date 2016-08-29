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

#include "tileset.h"

#include "../../../mc/chunk.h"
#include "../../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

TopdownTileSet::TopdownTileSet(int tile_width)
	: TileSet(tile_width) {
}

TopdownTileSet::~TopdownTileSet() {
}

void TopdownTileSet::mapChunkToTiles(const mc::ChunkPos& chunk,
		std::set<TilePos>& tiles) {
	// make sure we render towards -infinity
	int x = std::floor((float) chunk.x / getTileWidth());
	int y = std::floor((float) chunk.z / getTileWidth());
	tiles.insert(TilePos(x, y));
}

}
}
