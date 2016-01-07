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

#include "../mapcraftercore/config/iniconfig.h"
#include "../mapcraftercore/config/mapcrafterconfig.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/test/unit_test.hpp>

namespace config = mapcrafter::config;

BOOST_AUTO_TEST_CASE(config_test) {
	/*
	config::ConfigSection section("world", "myworld");
	section.set("foo", "bar");
	section.set("foo2", "test");
	section.remove("foo2");
	section.set("test", "73");
	section.set("test", "42");
	*/

	/*
	config::ConfigSection section("test", "");
	section.set("foo", "42");

	config::ConfigFile c;

	config::ValidationMessage msg;
	if (!c.loadFile("test.conf", msg))
		std::cout << msg << std::endl;
	else {
		c.getRootSection().set("hello", "world");
		c.addSection(section);
		c.write(std::cout);
	}
	*/
}

BOOST_AUTO_TEST_CASE(config_testReadWrite) {
	config::INIConfig c;
	c.loadFile("data/config/test.conf");
	std::ifstream in("data/config/test.conf");
	std::string in_data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

	std::ostringstream out;
	c.write(out);
	std::string out_data = out.rdbuf()->str();

	BOOST_CHECK_EQUAL(in_data, out_data);
}

BOOST_AUTO_TEST_CASE(config_testFieldValidation) {
	config::ValidationList validation;

	// test the behavior of loading config entries from different sections
	// into the field objects
	config::INIConfigSection section1, section2, section3, section4;
	section2.set("test", "foobar");
	section4.set("test", "42");

	config::Field<std::string> field, field2;
	BOOST_CHECK(!field.isLoaded());
	BOOST_CHECK(!field.require(validation, "error"));

	field.load("test", section2.get("test"), validation);
	BOOST_CHECK(field.isLoaded());
	BOOST_CHECK_EQUAL(field.getValue(), "foobar");

	field.load("test", section4.get("test"), validation);
	BOOST_CHECK(field.isLoaded());
	BOOST_CHECK_EQUAL(field.getValue(), "42");

	field2.setDefault("default");
	BOOST_CHECK(field2.isLoaded());
	BOOST_CHECK_EQUAL(field2.getValue(), "default");

	field2.load("test", section2.get("test"), validation);
	BOOST_CHECK(field2.isLoaded());
	BOOST_CHECK_EQUAL(field2.getValue(), "foobar");
}
