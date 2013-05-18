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

#include "mc/nbt.h"

#include "util.h"

#include <iostream>
#include <fstream>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <endian.h>

namespace mapcrafter {
namespace mc {
namespace nbt {

template<typename T>
void dumpTag(std::ostream& stream, const char* type, bool named,
		const std::string& name, const std::string& indendation, T payload) {
	stream << indendation << type;
	if (named)
		stream << "(\"" << name << "\")";
	stream << ": " << payload << std::endl;
}

NBTError::NBTError(const char* message)
		: message(message) {
}

NBTError::NBTError(const std::string& message)
		: message(message) {
}

NBTError::~NBTError() throw () {
}

const char* NBTError::what() const throw () {
	return message.c_str();
}

NBTTag::NBTTag()
		: named(false), writeType(true) {
}

NBTTag::NBTTag(TagType type)
		: type(type), named(false), writeType(true) {
}

NBTTag::~NBTTag() {
}

const TagType& NBTTag::getType() const {
	return type;
}

bool NBTTag::isNamed() const {
	return named;
}

void NBTTag::setNamed(bool named) {
	this->named = named;
}

bool NBTTag::isWriteType() const {
	return writeType;
}

void NBTTag::setWriteType(bool writeType) {
	this->writeType = writeType;
}

const std::string& NBTTag::getName() const {
	return name;
}

void NBTTag::setName(const std::string& name, bool setNamed) {
	if (setNamed) {
		this->named = true;
	}
	this->name = name;
}

NBTTag& NBTTag::read(std::istream& stream) {
	return *this;
}

void NBTTag::write(std::ostream& stream) const {
	if (writeType)
		stream << (int8_t) type;
	if (named) {
		TagString nameTag(name);
		nameTag.setWriteType(false);
		nameTag.write(stream);
	}
}

void NBTTag::dump(std::ostream& stream, const std::string& indendation) const {
}

NBTTag& TagByte::read(std::istream& stream) {
	stream.read((char*) &payload, sizeof(payload));
	return *this;
}

void TagByte::write(std::ostream& stream) const {
	NBTTag::write(stream);
	stream << payload;
}

void TagByte::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_Byte", isNamed(), getName(), indendation, (int) payload);
}

NBTTag& TagShort::read(std::istream& stream) {
	stream.read((char*) &payload, sizeof(payload));
	payload = be16toh(payload);
	return *this;
}

void TagShort::write(std::ostream& stream) const {
	NBTTag::write(stream);
	int16_t tmp = htobe16(payload);
	stream.write((char*) &tmp, sizeof(payload));
}

void TagShort::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_Short", isNamed(), getName(), indendation, payload);
}

NBTTag& TagInt::read(std::istream& stream) {
	stream.read((char*) &payload, sizeof(payload));
	payload = be32toh(payload);
	return *this;
}

void TagInt::write(std::ostream& stream) const {
	NBTTag::write(stream);
	int32_t tmp = htobe32(payload);
	stream.write((char*) &tmp, sizeof(payload));
}

void TagInt::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_Int", isNamed(), getName(), indendation, payload);
}

NBTTag& TagLong::read(std::istream& stream) {
	stream.read((char*) &payload, sizeof(payload));
	payload = be64toh(payload);
	return *this;
}

void TagLong::write(std::ostream& stream) const {
	NBTTag::write(stream);
	int64_t tmp = htobe64(payload);
	stream.write((char*) &tmp, sizeof(payload));
}

void TagLong::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_Long", isNamed(), getName(), indendation, payload);
}

NBTTag& TagFloat::read(std::istream& stream) {
	union {
		int32_t tmp;
		float myfloat;
	};
	stream.read(reinterpret_cast<char*>(&tmp), sizeof(int32_t));
	tmp = be32toh(tmp);
	payload = myfloat;
	return *this;
}

void TagFloat::write(std::ostream& stream) const {
	NBTTag::write(stream);
	union {
		int32_t tmp;
		float myfloat;
	};
	myfloat = payload;
	tmp = htobe32(tmp);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(int32_t));
}

void TagFloat::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_Float", isNamed(), getName(), indendation, payload);
}

NBTTag& TagDouble::read(std::istream& stream) {
	union {
		int64_t tmp;
		double mydouble;
	};
	stream.read(reinterpret_cast<char*>(&tmp), sizeof(int64_t));
	tmp = be64toh(tmp);
	payload = mydouble;
	return *this;
}

void TagDouble::write(std::ostream& stream) const {
	NBTTag::write(stream);
	union {
		int64_t tmp;
		double myfloat;
	};
	myfloat = payload;
	tmp = htobe64(tmp);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(int64_t));
}

void TagDouble::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_Double", isNamed(), getName(), indendation, payload);
}

NBTTag& TagByteArray::read(std::istream& stream) {
	int length = ((TagInt&) TagInt().read(stream)).payload;
	payload.resize(length);
	stream.read((char*) &payload[0], length);
	return *this;
}

void TagByteArray::write(std::ostream& stream) const {
	NBTTag::write(stream);
	TagInt length(payload.size());
	length.setWriteType(false);
	length.write(stream);
	TagByte byte;
	byte.setWriteType(false);
	for (int i = 0; i < length.payload; i++) {
		byte.payload = payload.at(i);
		byte.write(stream);
	}
}

void TagByteArray::dump(std::ostream& stream, const std::string& indendation) const {
	stream << indendation << "TAG_ByteArray";
	if (named) {
		stream << "(\"" << name << "\")";
	}
	stream << ": " << payload.size() << " entries" << std::endl;
}

NBTTag& TagString::read(std::istream& stream) {
	short length = ((TagShort&) TagShort().read(stream)).payload;
	payload.resize(length);
	stream.read(&payload[0], length);
	return *this;
}

void TagString::write(std::ostream& stream) const {
	NBTTag::write(stream);
	TagShort length(payload.size());
	length.setWriteType(false);
	length.write(stream);
	stream.write(payload.c_str(), length.payload);
}

void TagString::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, "TAG_String", isNamed(), getName(), indendation, payload);
}

TagList::~TagList() {
	for (std::vector<NBTTag*>::iterator it = payload.begin(); it != payload.end(); ++it)
		delete *it;
}

NBTTag& TagList::read(std::istream& stream) {
	tag_type = ((TagByte&) TagByte().read(stream)).payload;
	int length = ((TagInt&) TagInt().read(stream)).payload;
	for (int i = 0; i < length; i++) {
		NBTTag* tag = createTag(tag_type);
		if (tag != NULL) {
			tag->read(stream);
			tag->setWriteType(false);
			payload.push_back(tag);
		}
	}
	return *this;
}

void TagList::write(std::ostream& stream) const {
	NBTTag::write(stream);
	TagByte typeTag(tag_type);
	TagInt length(payload.size());
	typeTag.setWriteType(false);
	length.setWriteType(false);
	typeTag.write(stream);
	length.write(stream);
	for (std::vector<NBTTag*>::const_iterator it = payload.begin(); it != payload.end();
	        ++it) {
		(*it)->setWriteType(false);
		(*it)->write(stream);
	}
}

void TagList::dump(std::ostream& stream, const std::string& indendation) const {
	stream << indendation << "TAG_List";
	if (named) {
		stream << "(\"" << name << "\")";
	}
	stream << ": " << payload.size() << " entries of type " << tag_type << std::endl;
	stream << indendation << "{" << std::endl;
	for (std::vector<NBTTag*>::const_iterator it = payload.begin(); it != payload.end();
	        ++it)
		(*it)->dump(stream, indendation + "   ");
	stream << indendation << "{" << std::endl;
}

TagCompound::~TagCompound() {
	for (std::map<std::string, NBTTag*>::iterator it = payload.begin();
			it != payload.end(); ++it) {
		if (it->second != NULL)
			delete it->second;
	}
}

NBTTag& TagCompound::read(std::istream& stream) {
	TagByte byteTag;
	TagString stringTag;
	while (1) {
		byteTag.read(stream);
		int tagType = byteTag.payload;
		if (tagType == TAG_END)
			break;
		stringTag.read(stream);
		std::string name = stringTag.payload;
		NBTTag* tag = createTag(tagType);
		if (tag != NULL) {
			tag->read(stream);
			tag->setName(name);
			payload[name] = tag;
		}
	}
	return *this;
}

void TagCompound::write(std::ostream& stream) const {
	NBTTag::write(stream);
	std::map<std::string, NBTTag*>::const_iterator it = payload.begin();
	for (; it != payload.end(); ++it) {
		it->second->write(stream);
	}
	stream << (int8_t) TAG_END;
}

void TagCompound::dump(std::ostream& stream, const std::string& indendation) const {
	stream << indendation << "TAG_Compound";
	if (named) {
		stream << "(\"" << name << "\")";
	}
	stream << ": " << payload.size() << " entries" << std::endl;
	stream << indendation << "{" << std::endl;
	std::map<std::string, NBTTag*>::const_iterator it = payload.begin();
	for (; it != payload.end(); ++it) {
		it->second->dump(stream, indendation + "   ");
	}
	stream << indendation << "}" << std::endl;
}

NBTTag* TagCompound::findTag(const std::string& name) const {
	std::map<std::string, NBTTag*>::const_iterator it = payload.find(name);
	if (it != payload.end()) {
		return it->second;
	}
	return NULL;
}

void TagCompound::addTag(const std::string& name, NBTTag* tag) {
	tag->setName(name);
	payload[name] = tag;
}

NBTTag& TagIntArray::read(std::istream& stream) {
	int length = ((TagInt&) TagInt().read(stream)).payload;
	for (int i = 0; i < length; i++) {
		int32_t element;
		stream.read((char*) &element, sizeof(element));
		element = be32toh(element);
		payload.push_back(element);
	}
	return *this;
}

void TagIntArray::write(std::ostream& stream) const {
	NBTTag::write(stream);
	TagInt length(payload.size());
	length.write(stream);
	TagInt element;
	element.setWriteType(false);
	for (std::vector<int32_t>::const_iterator it = payload.begin(); it != payload.end();
	        ++it) {
		element.payload = *it;
		element.write(stream);
	}
}

void TagIntArray::dump(std::ostream& stream, const std::string& indendation) const {
	stream << indendation << "TAG_IntArray";
	if (named) {
		stream << "(\"" << name << "\")";
	}
	stream << ": " << payload.size() << " entries" << std::endl;
}

NBTFile::NBTFile() {
}

NBTFile::~NBTFile() {
}

void NBTFile::decompressStream(std::istream& stream, std::stringstream& decompressed,
        CompressionType compression) {
	if (compression != GZIP && compression != ZLIB) {
		decompressed << stream.rdbuf();
		return;
	}
	boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
	if (compression == GZIP) {
		in.push(boost::iostreams::gzip_decompressor());
	} else if (compression == ZLIB) {
		in.push(boost::iostreams::zlib_decompressor());
	}
	try {
		in.push(stream);
		boost::iostreams::copy(in, decompressed);
	} catch (boost::iostreams::gzip_error &e) {
		throw NBTError(
		        "Error while decompressing gzip data: " + std::string(e.what()) + " ("
		                + str(e.error()) + ")");
	} catch (boost::iostreams::zlib_error &e) {
		throw NBTError(
		        "Error while decompressing zlib data: " + std::string(e.what()) + " ("
		                + str(e.error()) + ")");
	}
}

void NBTFile::readCompressed(std::istream& stream, CompressionType compression) {
	std::stringstream decompressed(std::ios::in | std::ios::out | std::ios::binary);
	decompressStream(stream, decompressed, compression);
	int type = ((TagByte&) TagByte().read(decompressed)).payload;
	if (type != TAG_COMPOUND) {
		throw NBTError("First tag is not a tag compound!");
	}
	std::string name = ((TagString&) TagString().read(decompressed)).payload;
	TagCompound::read(decompressed);
	setName(name);
}

void NBTFile::readNBT(std::istream& stream, CompressionType compression) {
	readCompressed(stream, compression);
}

void NBTFile::readNBT(const char* filename, CompressionType compression) {
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		throw NBTError(
		        std::string("Unable to open file '").append(filename).append("'!"));
	}
	readCompressed(file, compression);
	file.close();
}

void NBTFile::readNBT(const char* buffer, size_t len, CompressionType compression) {
	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	stream.write(buffer, len);
	readCompressed(stream, compression);
}

void NBTFile::writeNBT(std::ostream& stream, CompressionType compression) {
	std::stringstream in(std::ios::in | std::ios::out | std::ios::binary);
	boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
	if (compression == GZIP) {
		out.push(boost::iostreams::gzip_compressor());
	} else if (compression == ZLIB) {
		out.push(boost::iostreams::zlib_compressor());
	} else {
		write(stream);
		return;
	}
	out.push(in);
	write(in);
	boost::iostreams::copy(out, stream);
}

void NBTFile::writeNBT(const char* filename, CompressionType compression) {
	std::ofstream file(filename, std::ios::binary);
	if (!file)
		throw NBTError("Unable to open file '" + std::string(filename) + "'!");
	writeNBT(file, compression);
	file.close();
}

NBTTag* createTag(int type) {
	switch (type) {
	case TAG_BYTE:
		return new TagByte();
	case TAG_SHORT:
		return new TagShort();
	case TAG_INT:
		return new TagInt();
	case TAG_LONG:
		return new TagLong();
	case TAG_FLOAT:
		return new TagFloat();
	case TAG_DOUBLE:
		return new TagDouble();
	case TAG_BYTE_ARRAY:
		return new TagByteArray();
	case TAG_STRING:
		return new TagString();
	case TAG_LIST:
		return new TagList();
	case TAG_COMPOUND:
		return new TagCompound();
	case TAG_INT_ARRAY:
		return new TagIntArray();
	default:
		return NULL;
	}
}

}
}
}
