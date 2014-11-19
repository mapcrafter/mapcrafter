/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "renderview.h"

#include "blockimages.h"
#include "tileset.h"
#include "tilerenderer.h"

namespace mapcrafter {
namespace renderer {

IsometricRenderView::IsometricRenderView() {
}

IsometricRenderView::~IsometricRenderView() {
}

BlockImages* IsometricRenderView::createBlockImages() const {
	return new IsometricBlockImages();
}

TileSet* IsometricRenderView::createTileSet() const {
	return new IsometricTileSet();
}

TileRenderer* IsometricRenderView::createTileRenderer() const {
	return new IsometricTileRenderer();
}

int IsometricRenderView::getBlockSizeFactor() const {
	return 2;
}

int IsometricRenderView::getTileSizeFactor() const {
	return 16;
}

} /* namespace renderer */
} /* namespace mapcrafter */
