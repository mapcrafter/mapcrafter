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

#include "tileset.h"

#include "../../../mc/chunk.h"
#include "../../../mc/pos.h"

namespace mapcrafter {
namespace renderer {

IsometricTileSet::IsometricTileSet(int tile_width)
	: TileSet(tile_width) {
}

namespace {

/**
 * Calculates the tiles a row and column covers.
 */
void addRowColTiles(int row, int col, int tile_width, std::set<TilePos>& tiles) {
	// the tiles are 2 * TILE_WIDTH columns wide
	// and 4 * TILE_WIDTH row tall
	// calculate the approximate position of the tile
	int x = col / (2 * tile_width);
	int y = row / (4 * tile_width);

	// add this tile
	tiles.insert(TilePos(x, y));

	// check if this row/col is on the border of two tiles
	bool edge_col = col % (2 * tile_width) == 0;
	bool edge_row = row % (4 * tile_width) == 0;
	// if yes, we have to add the neighbor tiles
	if (edge_col)
		tiles.insert(TilePos(x-1, y));
	if (edge_row)
		tiles.insert(TilePos(x, y-1));
	if (edge_col && edge_row)
		tiles.insert(TilePos(x-1, y-1));
}

}

void IsometricTileSet::mapChunkToTiles(const mc::ChunkPos& chunk,
		std::set<TilePos>& tiles) {
	// at first get row and column of the top of the chunk
	int row = chunk.getRow();
	int col = chunk.getCol();

	// then we go through all sections of the chunk plus one on the bottom side
	// and add the tiles the individual sections cover,

	// plus one on the bottom side because with chunk section is here
	// only the top of a chunk section meant
	for (int i = 0; i <= mc::CHUNK_HEIGHT; i++)
		addRowColTiles(row + 2*i, col, getTileWidth(), tiles);
}

}
}
