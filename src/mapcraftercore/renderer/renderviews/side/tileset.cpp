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

SideTileSet::SideTileSet(int tile_width)
	: TileSet(tile_width) {
}

SideTileSet::~SideTileSet() {
}

void SideTileSet::mapChunkToTiles(const mc::ChunkPos& chunk,
		std::set<TilePos>& tiles) {
	for (int i = 0; i < mc::CHUNK_HEIGHT; i++) {
		// make sure we render towards -infinity
		int x = std::floor((float) chunk.x / getTileWidth());
		int y0 = std::floor((float) (chunk.z + mc::CHUNK_HEIGHT-i-1 + 1) / getTileWidth());
		int y1 = std::floor((float) (chunk.z + mc::CHUNK_HEIGHT-i-1 + 0) / getTileWidth());
		tiles.insert(TilePos(x, y0));
		tiles.insert(TilePos(x, y1));
	}
}

}
}
