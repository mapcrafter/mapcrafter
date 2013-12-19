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

#include "nbt.h"

#include <fstream>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

namespace mapcrafter {
namespace mc {
namespace nbt {

namespace nbtstream {
template<typename T>
T read(std::istream& stream) {
}

template<>
int8_t read<int8_t>(std::istream& stream) {
	int8_t value;
	stream.read(reinterpret_cast<char*>(&value), sizeof(value));
	return value;
}

template<>
int16_t read<int16_t>(std::istream& stream) {
	int16_t value;
	stream.read(reinterpret_cast<char*>(&value), sizeof(value));
	return util::bigEndian16(value);
}

template<>
int32_t read<int32_t>(std::istream& stream) {
	int32_t value;
	stream.read(reinterpret_cast<char*>(&value), sizeof(value));
	return util::bigEndian32(value);
}

template<>
int64_t read<int64_t>(std::istream& stream) {
	int64_t value;
	stream.read(reinterpret_cast<char*>(&value), sizeof(value));
	return util::bigEndian64(value);
}

template<>
float read<float>(std::istream& stream) {
	union {
		int32_t tmp;
		float myfloat;
	};
	stream.read(reinterpret_cast<char*>(&tmp), sizeof(int32_t));
	tmp = util::bigEndian32(tmp);
	return myfloat;
}

template<>
double read<double>(std::istream& stream) {
	union {
		int64_t tmp;
		double mydouble;
	};
	stream.read(reinterpret_cast<char*>(&tmp), sizeof(int64_t));
	tmp = util::bigEndian64(tmp);
	return mydouble;
}

template<>
std::string read<std::string>(std::istream& stream) {
	std::string value;
	int16_t length = read<int16_t>(stream);
	value.resize(length);
	stream.read(&value[0], length);
	return value;
}

template<typename T>
void write(std::ostream& stream, T value) {
}

template<>
void write<int8_t>(std::ostream& stream, int8_t value) {
	stream.write(reinterpret_cast<char*>(&value), sizeof(value));
}

template<>
void write<int16_t>(std::ostream& stream, int16_t value) {
	int16_t tmp = util::bigEndian16(value);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(value));
}

template<>
void write<int32_t>(std::ostream& stream, int32_t value) {
	int32_t tmp = util::bigEndian32(value);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(value));
}

template<>
void write<int64_t>(std::ostream& stream, int64_t value) {
	int64_t tmp = util::bigEndian64(value);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(value));
}

template<>
void write<float>(std::ostream& stream, float value) {
	union {
		int32_t tmp;
		float myfloat;
	};
	myfloat = value;
	tmp = util::bigEndian32(tmp);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(int32_t));
}

template<>
void write<double>(std::ostream& stream, double value) {
	union {
		int64_t tmp;
		double myfloat;
	};
	myfloat = value;
	tmp = util::bigEndian64(tmp);
	stream.write(reinterpret_cast<char*>(&tmp), sizeof(int64_t));
}

template<>
void write<std::string>(std::ostream& stream, std::string value) {
	write<int16_t>(stream, value.size());
	stream.write(value.c_str(), value.size());
}
}

Tag::Tag(int8_t type)
	: type(type), named(false), write_type(true) {
}

Tag::~Tag() {
}

int8_t Tag::getType() const {
	return type;
}

bool Tag::isWriteType() const {
	return write_type;
}

void Tag::setWriteType(bool write_type) {
	this->write_type = write_type;
}

bool Tag::isNamed() const {
	return named;
}

void Tag::setNamed(bool named) {
	this->named = named;
}

const std::string& Tag::getName() const {
	return name;
}

void Tag::setName(const std::string& name, bool set_named) {
	if (set_named)
		this->named = true;
	this->name = name;
}

Tag& Tag::read(std::istream& stream) {
	return *this;
}

void Tag::write(std::ostream& stream) const {
	if (write_type)
		nbtstream::write<int8_t>(stream, type);
	if (named)
		nbtstream::write<std::string>(stream, name);
}

void Tag::dump(std::ostream& stream, const std::string& indendation) const {
}

Tag* Tag::clone() const {
	return new Tag(*this);
}

Tag& TagString::read(std::istream& stream) {
	payload = nbtstream::read<std::string>(stream);
	return *this;
}

void TagString::write(std::ostream& stream) const {
	Tag::write(stream);
	nbtstream::write<std::string>(stream, payload);
}

void TagString::dump(std::ostream& stream, const std::string& indendation) const {
	dumpTag(stream, indendation, *this);
}

Tag* TagString::clone() const {
	return new TagString(*this);
}

TagList::TagList(const TagList& other)
	: TagList() {
	*this = other;
}

TagList::~TagList() {
}

void TagList::operator=(const TagList& other) {
	name = other.name;
	named = other.named;

	tag_type = other.tag_type;

	payload.clear();
	for (auto it = other.payload.begin(); it != other.payload.end(); ++it)
		payload.push_back(TagPtr((*it)->clone()));
}

Tag& TagList::read(std::istream& stream) {
	tag_type = nbtstream::read<int8_t>(stream);
	int32_t length = nbtstream::read<int32_t>(stream);
	for (int32_t i = 0; i < length; i++) {
		Tag* tag = createTag(tag_type);
		if (tag == nullptr)
			throw NBTError(std::string("Unknown tag type with id ") + util::str(static_cast<int>(tag_type))
						   + ". NBT data stream may be corrupted.");
		tag->read(stream);
		tag->setWriteType(false);
		tag->setNamed(false);
		payload.push_back(TagPtrType<Tag>(tag));
	}
	return *this;
}

void TagList::write(std::ostream& stream) const {
	Tag::write(stream);
	nbtstream::write<int8_t>(stream, tag_type);
	nbtstream::write<int32_t>(stream, payload.size());
	for (auto it = payload.begin(); it != payload.end(); ++it) {
		(*it)->setWriteType(false);
		(*it)->setNamed(false);
		(*it)->write(stream);
	}
}

void TagList::dump(std::ostream& stream, const std::string& indendation) const {
	stream << indendation << "TAG_List";
	if (named)
		stream << "(\"" << name << "\")";
	stream << ": " << payload.size() << " entries of type " << static_cast<int>(tag_type) << std::endl;
	stream << indendation << "{" << std::endl;
	for (auto it = payload.begin(); it != payload.end(); ++it)
		(*it)->dump(stream, indendation + "   ");
	stream << indendation << "}" << std::endl;
}

Tag* TagList::clone() const {
	return new TagList(*this);
}

TagCompound::TagCompound(const TagCompound& other)
	: TagCompound() {
	*this = other;
}

TagCompound::~TagCompound() {
}

void TagCompound::operator=(const TagCompound& other) {
	name = other.name;
	named = other.named;

	payload.clear();
	for (auto it = other.payload.begin(); it != other.payload.end(); ++it)
		payload[it->first] = TagPtr(it->second->clone());
}

Tag& TagCompound::read(std::istream& stream) {
	while (1) {
		int8_t tag_type = nbtstream::read<int8_t>(stream);
		if (tag_type == TagEnd::TAG_TYPE)
			break;
		std::string name = nbtstream::read<std::string>(stream);
		Tag* tag = createTag(tag_type);
		if (tag == nullptr)
			throw NBTError(std::string("Unknown tag type with id ") + util::str(static_cast<int>(tag_type))
						   + ". NBT data stream may be corrupted.");
		tag->read(stream);
		tag->setName(name);
		tag->setWriteType(true);
		payload[name] = TagPtr(tag);
	}
	return *this;
}

void TagCompound::write(std::ostream& stream) const {
	Tag::write(stream);
	for (auto it = payload.begin(); it != payload.end(); ++it) {
		it->second->setWriteType(true);
		it->second->setNamed(true);
		it->second->write(stream);
	}
	nbtstream::write<int8_t>(stream, TagEnd::TAG_TYPE);
}

void TagCompound::dump(std::ostream& stream, const std::string& indendation) const {
	stream << indendation << "TAG_Compound";
	if (named)
		stream << "(\"" << name << "\")";
	stream << ": " << payload.size() << " entries" << std::endl;
	stream << indendation << "{" << std::endl;
	for (auto it = payload.begin(); it != payload.end(); ++it)
		it->second->dump(stream, indendation + "   ");
	stream << indendation << "}" << std::endl;
}

Tag* TagCompound::clone() const {
	return new TagCompound(*this);
}

bool TagCompound::hasTag(const std::string& name) const {
	return payload.count(name);
}

Tag& TagCompound::findTag(const std::string& name) {
	if (!hasTag(name))
		throw TagNotFound();
	return *payload.at(name);
}

const Tag& TagCompound::findTag(const std::string& name) const {
	if (!hasTag(name))
		throw TagNotFound();
	return *payload.at(name);
}

void TagCompound::addTag(const std::string& name, const Tag& tag) {
	Tag* tag_ptr = tag.clone();
	tag_ptr->setName(name);
	tag_ptr->setWriteType(true);
	payload[name] = TagPtr(tag_ptr);
}

NBTFile::NBTFile() {
}

NBTFile::~NBTFile() {
}

void NBTFile::decompressStream(std::istream& stream, std::stringstream& decompressed,
        Compression compression) {
	if (compression == Compression::NO_COMPRESSION) {
		decompressed << stream.rdbuf();
		return;
	}
	boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
	if (compression == Compression::GZIP) {
		in.push(boost::iostreams::gzip_decompressor());
	} else if (compression == Compression::ZLIB) {
		in.push(boost::iostreams::zlib_decompressor());
	}
	try {
		in.push(stream);
		boost::iostreams::copy(in, decompressed);
	} catch (boost::iostreams::gzip_error &e) {
		throw NBTError(
		        "Error while decompressing gzip data: " + std::string(e.what()) + " ("
		                + util::str(e.error()) + ")");
	} catch (boost::iostreams::zlib_error &e) {
		throw NBTError(
		        "Error while decompressing zlib data: " + std::string(e.what()) + " ("
		                + util::str(e.error()) + ")");
	}
}

void NBTFile::readCompressed(std::istream& stream, Compression compression) {
	std::stringstream decompressed(std::ios::in | std::ios::out | std::ios::binary);
	decompressStream(stream, decompressed, compression);
	int8_t type = ((TagByte&) TagByte().read(decompressed)).payload;
	if (type != TagCompound::TAG_TYPE)
		throw NBTError("First tag is not a tag compound!");
	std::string name = ((TagString&) TagString().read(decompressed)).payload;
	TagCompound::read(decompressed);
	setName(name);
}

void NBTFile::readNBT(std::istream& stream, Compression compression) {
	readCompressed(stream, compression);
}

void NBTFile::readNBT(const char* filename, Compression compression) {
	std::ifstream file(filename, std::ios::binary);
	if (!file)
		throw NBTError(std::string("Unable to open file '") + filename + "'!");
	readCompressed(file, compression);
	file.close();
}

void NBTFile::readNBT(const char* buffer, size_t len, Compression compression) {
	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	stream.write(buffer, len);
	readCompressed(stream, compression);
}

void NBTFile::writeNBT(std::ostream& stream, Compression compression) {
	std::stringstream in(std::ios::in | std::ios::out | std::ios::binary);
	boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
	if (compression == Compression::GZIP) {
		out.push(boost::iostreams::gzip_compressor());
	} else if (compression == Compression::ZLIB) {
		out.push(boost::iostreams::zlib_compressor());
	} else {
		write(stream);
		return;
	}
	out.push(in);
	write(in);
	boost::iostreams::copy(out, stream);
}

void NBTFile::writeNBT(const char* filename, Compression compression) {
	std::ofstream file(filename, std::ios::binary);
	if (!file)
		throw NBTError(std::string("Unable to open file '") + filename + "'!");
	writeNBT(file, compression);
	file.close();
}

Tag* createTag(int8_t type) {
	switch (type) {
	case TagByte::TAG_TYPE:
		return new TagByte;
	case TagShort::TAG_TYPE:
		return new TagShort;
	case TagInt::TAG_TYPE:
		return new TagInt;
	case TagLong::TAG_TYPE:
		return new TagLong;
	case TagFloat::TAG_TYPE:
		return new TagFloat;
	case TagDouble::TAG_TYPE:
		return new TagDouble;
	case TagByteArray::TAG_TYPE:
		return new TagByteArray;
	case TagString::TAG_TYPE:
		return new TagString;
	case TagList::TAG_TYPE:
		return new TagList;
	case TagCompound::TAG_TYPE:
		return new TagCompound;
	case TagIntArray::TAG_TYPE:
		return new TagIntArray;
	default:
		return nullptr;
	}
}

}
}
}
