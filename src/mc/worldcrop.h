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

#ifndef WORLDCROP_H_
#define WORLDCROP_H_

#include "pos.h"

namespace mapcrafter {
namespace mc {

template <typename T>
class Bounds {
public:
	Bounds();
	~Bounds();

	void setMin(T min);
	void setMax(T max);

	void resetMin();
	void resetMax();

	bool contains(T value) const;

private:
	T min, max;
	bool min_set, max_set;
};

class WorldCrop {
public:
	WorldCrop();
	~WorldCrop();

	void setMinX(int value);
	void setMaxX(int value);

	void setMinZ(int value);
	void setMaxZ(int value);

	void setMinY(int value);
	void setMaxY(int value);

	bool isRegionContained(const mc::RegionPos& region) const;
	bool isChunkContained(const mc::ChunkPos& chunk) const;

private:
	Bounds<int> bounds_x, bounds_z, bounds_y;
};

}
}

#endif /* WORLDCROP_H_ */
