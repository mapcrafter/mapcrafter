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

#include "worldcrop.h"

namespace mapcrafter {
namespace mc {

WorldCrop::WorldCrop()
	: type(RECTANGULAR), radius(0) {
}

WorldCrop::~WorldCrop() {
}

void WorldCrop::setMinY(int value) {
}

void WorldCrop::setMaxY(int value) {
}

void WorldCrop::setMinX(int value) {
}

void WorldCrop::setMaxX(int value) {
}

void WorldCrop::setMinZ(int value) {
}

void WorldCrop::setMaxZ(int value) {
}

void WorldCrop::setCenter(const BlockPos& pos) {
}

void WorldCrop::setRadius(int radius) {
}

bool WorldCrop::isRegionContained(const mc::RegionPos& region) const {
	return true;
}

bool WorldCrop::isChunkContained(const mc::ChunkPos& chunk) const {
	return true;
}

}
}
