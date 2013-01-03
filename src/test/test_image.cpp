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

#include "render/image.h"

#include <cstdlib>
#include <boost/test/unit_test.hpp>

namespace render = mapcrafter::render;

BOOST_AUTO_TEST_CASE(image_testIO) {
	render::Image src(400, 200);
	render::Image dest;

	for(int x = 0; x < src.getWidth(); x++) {
		for(int y = 0; y < src.getHeight(); y++) {
			src.setPixel(x, y, render::rgba(rand() % 256, rand() % 256, rand() % 256, rand() % 256));
		}
	}

	if(!src.writePNG("test.png"))
		BOOST_ERROR("Unable to write image!");
	if(!dest.readPNG("test.png"))
		BOOST_ERROR("Unable to read image!");

	BOOST_CHECK_EQUAL(dest.getWidth(), src.getWidth());
	BOOST_CHECK_EQUAL(dest.getHeight(), src.getHeight());

	for(int x = 0; x < dest.getWidth(); x++) {
		for(int y = 0; y < dest.getHeight(); y++) {
			if(src.getPixel(x, y) != dest.getPixel(x, y))
				BOOST_ERROR("Images arent equal!");
		}
	}
}
