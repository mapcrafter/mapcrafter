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

#include "../config/extended_ini.h"

#include <iostream>
#include <boost/test/unit_test.hpp>

namespace config = mapcrafter::config2;

BOOST_AUTO_TEST_CASE(config_test) {
	config::ConfigSection section("world", "myworld");
	section.set("foo", "bar");
	section.set("foo2", "test");
	section.remove("foo2");
	section.set("test", "73");
	section.set("test", "42");

	//std::cout << section << std::endl;
}
