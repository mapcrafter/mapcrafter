/*
 * Copyright 2012 Moritz Hilscher
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

#include "mc/nbt.h"

#include <vector>
#include <map>
#include <sstream>
#include <boost/test/unit_test.hpp>

namespace nbt = mapcrafter::mc::nbt;

#define REQUIRE_TAG(tag, name) if(tag == NULL) { BOOST_ERROR("Tag '" + std::string(name) + "' not found!"); return; }

BOOST_AUTO_TEST_CASE(nbt_testIO) {
	std::stringstream stream;

	std::vector<std::string> listdata;
	listdata.push_back("This");
	listdata.push_back("is");
	listdata.push_back("a");
	listdata.push_back("test");
	listdata.push_back("list");
	listdata.push_back(".");

	nbt::NBTFile out("TestNBTFile");
	out.addTag("byte", new nbt::TagByte(42));
	out.addTag("short", new nbt::TagShort(1337));
	out.addTag("int", new nbt::TagInt(-23));
	out.addTag("long", new nbt::TagLong(123456));
	out.addTag("float", new nbt::TagFloat(3.1415926));
	out.addTag("double", new nbt::TagDouble(2.7182818));
	out.addTag("string", new nbt::TagString("foobar"));

	nbt::TagList* list = new nbt::TagList(nbt::TAG_STRING);
	for (int i = 0; i < listdata.size(); i++)
		list->payload.push_back(new nbt::TagString(listdata[i]));
	out.addTag("list", list);
	out.writeNBT(stream, nbt::GZIP);

	stream.seekg(0, std::ios_base::beg);

	nbt::NBTFile in;
	in.readNBT(stream, nbt::GZIP);
	nbt::TagByte* tag_byte = in.findTag<nbt::TagByte>("byte", nbt::TAG_BYTE);
	nbt::TagShort* tag_short = in.findTag<nbt::TagShort>("short", nbt::TAG_SHORT);
	nbt::TagInt* tag_int = in.findTag<nbt::TagInt>("int", nbt::TAG_INT);
	nbt::TagLong* tag_long = in.findTag<nbt::TagLong>("long", nbt::TAG_LONG);
	nbt::TagFloat* tag_float = in.findTag<nbt::TagFloat>("float", nbt::TAG_FLOAT);
	nbt::TagDouble* tag_double = in.findTag<nbt::TagDouble>("double", nbt::TAG_DOUBLE);
	nbt::TagString* tag_string = in.findTag<nbt::TagString>("string", nbt::TAG_STRING);
	nbt::TagList* tag_list = in.findTag<nbt::TagList>("list", nbt::TAG_LIST);

	REQUIRE_TAG(tag_byte, "byte");
	REQUIRE_TAG(tag_short, "short");
	REQUIRE_TAG(tag_int, "int");
	REQUIRE_TAG(tag_long, "long");
	REQUIRE_TAG(tag_float, "float");
	REQUIRE_TAG(tag_double, "double");
	REQUIRE_TAG(tag_string, "string");
	REQUIRE_TAG(tag_list, "list");

	BOOST_CHECK_EQUAL(tag_byte->payload, 42);
	BOOST_CHECK_EQUAL(tag_short->payload, 1337);
	BOOST_CHECK_EQUAL(tag_int->payload, -23);
	BOOST_CHECK_EQUAL(tag_long->payload, 123456);
	BOOST_CHECK_CLOSE(tag_float->payload, 3.1415926, 0.0001);
	BOOST_CHECK_CLOSE(tag_double->payload, 2.7182818, 0.0001);
	BOOST_CHECK_EQUAL(tag_string->payload, "foobar");

	BOOST_CHECK_EQUAL(tag_list->tag_type, nbt::TAG_STRING);
	BOOST_CHECK_EQUAL(listdata.size(), tag_list->payload.size());
	for(int i = 0; i < listdata.size(); i++) {
		BOOST_CHECK_EQUAL(tag_list->payload[i]->getType(), nbt::TAG_STRING);
		BOOST_CHECK(listdata[i] == ((nbt::TagString*) tag_list->payload[i])->payload);
	}

}
