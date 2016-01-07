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

#include "../mapcraftercore/renderer/image.h"
#include "../mapcraftercore/renderer/image/quantization.h"

#include <cstdlib>
#include <set>
#include <boost/test/unit_test.hpp>

namespace renderer = mapcrafter::renderer;
using namespace renderer;

void traverseCheckOctree(const Octree* octree) {
	// root <-> level = 0
	BOOST_CHECK(!octree->isRoot() == (octree->getLevel() == 0));
	BOOST_CHECK(octree->isRoot() == !octree->isLeaf());
	BOOST_CHECK(octree->hasColor() == octree->isLeaf());
	for (int i = 0; i < 16; i++) {
		if (octree->hasChildren(i))
			traverseCheckOctree(octree->getChildren(i));
	}
}

void traverseReduceOctree(Octree* octree) {
	for (int i = 0; i < 16; i++) {
		if (!octree->hasChildren(i))
			continue;
		traverseReduceOctree(octree->getChildren(i));
	}
	if (octree->isLeaf() && !octree->isRoot()) {
		octree->reduceToParent();
		delete octree;
	}
}

void testOctreeWithImage(const RGBAImage& image) {
	std::set<RGBAPixel> colors;
	int r = 0, g = 0, b = 0, count = 0;

	Octree octree;

	// insert all pixels into an octree
	for (int x = 0; x < image.getWidth(); x++) {
		for (int y = 0; y < image.getHeight(); y++) {
			RGBAPixel color = image.getPixel(x, y);
			colors.insert(color);
			r += rgba_red(color);
			g += rgba_green(color);
			b += rgba_blue(color);
			count++;

			Octree::findOrCreateNode(&octree, color)->setColor(color);
		}
	}

	// make sure that all colors are inserted correctly
	BOOST_CHECK(octree.isRoot() && !octree.isLeaf());
	BOOST_CHECK(!octree.hasColor());

	// reduce all colors up to the root of the tree
	// the color should be the overall average color
	traverseReduceOctree(&octree);
	BOOST_CHECK(octree.hasColor());

	RGBAPixel average1 = octree.getColor();
	RGBAPixel average2 = rgba(r / count, g / count, b / count, 255);
	BOOST_CHECK_EQUAL(average1, average2);

	BOOST_TEST_MESSAGE("Overall colors: " << colors.size());
	BOOST_TEST_MESSAGE("Pixels per color: " << (double) (image.getWidth() * image.getHeight()) / colors.size());
	BOOST_TEST_MESSAGE("Average color: " << (int) rgba_red(average1) << ","
			<< (int) rgba_green(average1) << "," << (int) rgba_blue(average1));
}

RGBAPixel randomColor() {
	return rgba(rand() % 256, rand() % 256, rand() % 256, rand() % 256);
}

void testPalette(Palette& palette, bool extended) {
	auto colors = palette.getColors();
	for (size_t i = 0; i < colors.size(); i++) {
		int j = palette.getNearestColor(colors[i]);
		// same color might be multiple times in the palette, that's why not i == j
		BOOST_CHECK_EQUAL(colors[i], colors[j]);
	}

	if (!extended)
		return;

	SimplePalette palette2(colors);
	for (size_t i = 0; i < colors.size() * 5; i++) {
		RGBAPixel color = randomColor();
		int found1 = palette.getNearestColor(color);
		int found2 = palette2.getNearestColor(color);
		BOOST_CHECK_EQUAL(colors[found1], colors[found2]);
	}
}

BOOST_AUTO_TEST_CASE(image_palette) {
	std::vector<RGBAPixel> colors = { randomColor() };
	OctreePalette palette1(colors);
	OctreePalette2 palette2(colors);
	testPalette(palette1, false);
	testPalette(palette2, true);

	for (int i = 0; i < 255; i++)
		colors.push_back(randomColor());
	OctreePalette palette3(colors);
	OctreePalette2 palette4(colors);
	testPalette(palette3, false);
	testPalette(palette4, true);
}

BOOST_AUTO_TEST_CASE(image_quantization_octree) {
	std::srand(std::time(0));

	// create a random image to test octree with
	BOOST_TEST_MESSAGE("Testing random image.");
	RGBAImage random(1000, 1000);
	for (int x = 0; x < random.getWidth(); x++)
		for (int y = 0; y < random.getHeight(); y++)
			random.setPixel(x, y, rgba(rand() % 256, rand() % 256, rand() % 256, 255));
	testOctreeWithImage(random);

	// and also check it with this cute platypus
	BOOST_TEST_MESSAGE("Testing platypus.");
	RGBAImage platypus;
	platypus.readPNG("data/platypus.png");
	testOctreeWithImage(platypus);
}

