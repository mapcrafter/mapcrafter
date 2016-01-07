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

#include "../mapcraftercore/mc/nbt.h"

#include <vector>
#include <map>
#include <sstream>
#include <boost/test/unit_test.hpp>

namespace nbt = mapcrafter::mc::nbt;

#define REQUIRE_TAG(found, name) if(!found) { BOOST_ERROR("Tag '" + std::string(name) + "' not found!"); return; }

BOOST_AUTO_TEST_CASE(nbt_testIO) {
	std::vector<std::string> list_data = {"This", "is", "a", "test", "list", "."};
	std::vector<int32_t> intarray_data = {1, 1, 2, 3, 5, 8, 13, 21};
	std::vector<int8_t> bytearray_data = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
	
	nbt::Compression compressions[] = {
		nbt::Compression::NO_COMPRESSION,
		nbt::Compression::GZIP,
		nbt::Compression::ZLIB
	};
	for (size_t i = 0; i < 3; i++) {
		nbt::Compression compression = compressions[i];
		BOOST_TEST_MESSAGE(std::string("Testing NBT with") + (compression == nbt::Compression::NO_COMPRESSION ? "out compression." : (compression == nbt::Compression::GZIP ? " Gzip compression." : " Zlib compression.")));
		
		std::stringstream stream;
		
		nbt::NBTFile out("TestNBTFile");
		out.addTag("byte", nbt::TagByte(42));
		out.addTag("short", nbt::TagShort(1337));
		out.addTag("int", nbt::TagInt(-23));
		out.addTag("long", nbt::TagLong(123456));
		out.addTag("float", nbt::TagFloat(3.1415926));
		out.addTag("double", nbt::TagDouble(2.7182818));
		out.addTag("string", nbt::TagString("foobar"));

		nbt::TagList list(nbt::TagString::TAG_TYPE);
		for (size_t i = 0; i < list_data.size(); i++)
			list.payload.push_back(nbt::TagPtr(new nbt::TagString(list_data[i])));
		out.addTag("list", list);
		out.addTag("bytearray", nbt::TagByteArray(bytearray_data));
		out.addTag("intarray", nbt::TagIntArray(intarray_data));
		out.addTag("compound", out);
		
		//out.dump(std::cout);
		out.writeNBT(stream, compression);

		stream.seekg(0, std::ios_base::beg);
		nbt::NBTFile in;
		in.readNBT(stream, compression);
		//in.dump(std::cout);
		
		REQUIRE_TAG(in.hasTag<nbt::TagByte>("byte"), "byte");
		REQUIRE_TAG(in.hasTag<nbt::TagShort>("short"), "short");
		REQUIRE_TAG(in.hasTag<nbt::TagInt>("int"), "int");
		REQUIRE_TAG(in.hasTag<nbt::TagLong>("long"), "long");
		REQUIRE_TAG(in.hasTag<nbt::TagFloat>("float"), "float");
		REQUIRE_TAG(in.hasTag<nbt::TagDouble>("double"), "double");
		REQUIRE_TAG(in.hasTag<nbt::TagString>("string"), "string");
		REQUIRE_TAG(in.hasList<nbt::TagString>("list", list_data.size()), "list");
		REQUIRE_TAG(in.hasArray<nbt::TagByteArray>("bytearray", bytearray_data.size()), "bytearray");
		REQUIRE_TAG(in.hasArray<nbt::TagIntArray>("intarray", intarray_data.size()), "intarray");

		BOOST_CHECK_EQUAL(in.findTag<nbt::TagByte>("byte").payload, 42);
		BOOST_CHECK_EQUAL(in.findTag<nbt::TagShort>("short").payload, 1337);
		BOOST_CHECK_EQUAL(in.findTag<nbt::TagInt>("int").payload, -23);
		BOOST_CHECK_EQUAL(in.findTag<nbt::TagLong>("long").payload, 123456);
		BOOST_CHECK_CLOSE(in.findTag<nbt::TagFloat>("float").payload, 3.1415926, 0.0001);
		BOOST_CHECK_CLOSE(in.findTag<nbt::TagDouble>("double").payload, 2.7182818, 0.0001);
		BOOST_CHECK_EQUAL(in.findTag<nbt::TagString>("string").payload, "foobar");
		
		nbt::TagList& tag_list = in.findTag<nbt::TagList>("list");
		for (size_t i = 0; i < list_data.size(); i++) {
			BOOST_CHECK(tag_list.payload[i]->getType() == nbt::TagString::TAG_TYPE);
			BOOST_CHECK(list_data[i] == tag_list.payload[i]->cast<nbt::TagString>().payload);
		}
		
		BOOST_CHECK(bytearray_data == in.findTag<nbt::TagByteArray>("bytearray").payload);
		BOOST_CHECK(intarray_data == in.findTag<nbt::TagIntArray>("intarray").payload);
	}
}
