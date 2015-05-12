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

#ifndef IMAGE_QUANTIZATION_H_
#define IMAGE_QUANTIZATION_H_

#include "palette.h"
#include "../image.h"
#include "../../util.h"

namespace mapcrafter {
namespace renderer {

class Octree;

class Octree {
public:
	Octree(Octree* parent = nullptr);
	~Octree();

	Octree* getParent();
	const Octree* getParent() const;

	bool isLeaf() const;

	bool hasChildren(int index) const;
	int getChildrenCount() const;
	Octree* getChildren(int index);
	const Octree* getChildren(int index) const;

	bool hasColor() const;
	RGBAPixel getColor() const;
	void setColor(RGBAPixel color);
	void reduceColor();

	static Octree* findOrCreateNode(Octree* octree, RGBAPixel color);
	static const Octree* findNearestNode(const Octree* octree, RGBAPixel color);

protected:
	Octree* parent;
	Octree* children[8];

	int reference;
	int red, green, blue;
};

void imageColorQuantize(RGBAImage& image, int max_colors);

}
}

#endif /* IMAGE_QUANTIZATION_H_ */

