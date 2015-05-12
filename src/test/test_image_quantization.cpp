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

#include "../mapcraftercore/renderer/image.h"
#include "../mapcraftercore/renderer/image/quantization.h"

#include <cstdlib>
#include <set>
#include <boost/test/unit_test.hpp>

namespace renderer = mapcrafter::renderer;
using namespace renderer;

int traverseCountOctree(const Octree* octree, const std::set<RGBAPixel>& colors) {
	int leaves = 0;
	if (octree->hasColor()) {
		leaves++;
		BOOST_CHECK(colors.count(octree->getColor()));
	}

	for (int i = 0; i < 8; i++) {
		if (octree->hasChildren(i))
			leaves += traverseCountOctree(octree->getChildren(i), colors);
	}
	return leaves;
}

void traverseReduceOctree(Octree* octree) {
	for (int i = 0; i < 8; i++) {
		if (!octree->hasChildren(i))
			continue;
		Octree* children = octree->getChildren(i);
		traverseReduceOctree(children);
	}
	if (!octree->isLeaf())
		octree->reduceColor();
}

BOOST_AUTO_TEST_CASE(image_quantization_octree) {
	std::srand(std::time(0));

	RGBAImage src(1000, 1000);
	src.readPNG("data/platypus.png");
	std::set<RGBAPixel> colors;
	int r = 0, g = 0, b = 0, count = 0;

	Octree octree;

	// create a random image and insert all colors into an octree
	for (int x = 0; x < src.getWidth(); x++) {
		for (int y = 0; y < src.getHeight(); y++) {
			RGBAPixel color = rgba(rand() % 256, rand() % 256, rand() % 256, 255);
			src.setPixel(x, y, color);
			colors.insert(src.getPixel(x, y));
			r += rgba_red(color);
			g += rgba_green(color);
			b += rgba_blue(color);
			count++;

			Octree::traverseToColor(&octree, color)->setColor(color);
		}
	}

	// make sure that all colors are inserted correctly
	BOOST_CHECK(!octree.isLeaf());
	BOOST_CHECK(!octree.hasColor());
	int color_count = traverseCountOctree(&octree, colors);
	BOOST_CHECK_EQUAL(color_count, colors.size());

	// reduce all colors up to the root of the tree
	// the color should be the overall average color
	traverseReduceOctree(&octree);
	BOOST_CHECK(octree.hasColor());

	RGBAPixel average1 = octree.getColor();
	RGBAPixel average2 = rgba(r / count, g / count, b / count, 255);
	BOOST_CHECK_EQUAL(average1, average2);
}

