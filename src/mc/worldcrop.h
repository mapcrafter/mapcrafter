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
	Bounds() : min_set(false), max_set(false) {}
	~Bounds() {}

	void setMin(T min) { this->min = min; min_set = true; }
	void setMax(T max) { this->max = max; max_set = true; }

	void resetMin() { min_set = false; }
	void resetMax() { max_set = false; }

	bool contains(T value) const {
		// case 1: no borders
		// value is definitely included
		if (!min_set && !max_set)
			return true;

		// case 2: only a minimum border
		// value is included if value >= minimum
		if (min_set && !max_set)
			return value >= min;

		// case 3: only a maximum border
		// value is included if value <= maximum
		if (max_set && !min_set)
			return value <= max;

		// case 3: two borders
		// value is included if value >= minimum and value <= maximum
		return min <= value && value <= max;
	}

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
