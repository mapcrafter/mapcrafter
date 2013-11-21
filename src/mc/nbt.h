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

#ifndef NBT_H_
#define NBT_H_

#include "../util.h"

#include <iostream>
#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mapcrafter {
namespace mc {
namespace nbt {

class NBTError : public std::runtime_error {
public:
	NBTError(const std::string& message = "")
		: std::runtime_error(message) {}
};

class InvalidTagCast : public NBTError {
public:
	InvalidTagCast(const std::string& message = "")
		: NBTError(message) {}
};

class TagNotFound : public NBTError {
public:
	TagNotFound(const std::string& message = "")
		: NBTError(message) {}
};

// only for reference
// use the TagXXX::TAG_TYPE constants instead
enum class TagType : int8_t {
	TAG_END = 0,
	TAG_BYTE = 1,
	TAG_SHORT = 2,
	TAG_INT = 3,
	TAG_LONG = 4,
	TAG_FLOAT = 5,
	TAG_DOUBLE = 6,
	TAG_BYTE_ARRAY = 7,
	TAG_STRING = 8,
	TAG_LIST = 9,
	TAG_COMPOUND = 10,
	TAG_INT_ARRAY = 11
};

enum class Compression {
	NO_COMPRESSION = 0, GZIP = 1, ZLIB = 2
};

static const char* TAG_NAMES[] = {
	"TAG_End",
	"TAG_Byte",
	"TAG_Short",
	"TAG_Int",
	"TAG_Long",
	"TAG_Float",
	"TAG_Double",
	"TAG_Byte_Array",
	"TAG_String",
	"TAG_List",
	"TAG_Compound",
	"TAG_Int_Array",
};

template<typename T>
void dumpTag(std::ostream& stream, const std::string& indendation, T tag) {
	dumpTag(stream, indendation, tag, tag.payload);
}

template<typename T, typename P>
void dumpTag(std::ostream& stream, const std::string& indendation, T tag, P payloadrepr) {
	const char* type = "TAG_Unknown";
	if (tag.getType() >= 0 && tag.getType() <= 11)
		type = TAG_NAMES[tag.getType()];
	stream << indendation << type;
	if (tag.isNamed())
		stream << "(\"" << tag.getName() << "\")";
	stream << ": " << payloadrepr << std::endl;
}

namespace nbtstream {
template<typename T>
T read(std::istream& stream);

template<typename T>
void write(std::ostream& stream, T t);
}

class Tag {
protected:
	int8_t type;
	bool named;
	bool write_type;
	std::string name;
public:
	Tag(int8_t type = -1);
	virtual ~Tag();

	int8_t getType() const;
	
	template<typename T>
	T& cast() {
		if (type == T::TAG_TYPE)
			return dynamic_cast<T&>(*this);
		throw InvalidTagCast();
	}

	template<typename T>
	const T& cast() const {
		if (type == T::TAG_TYPE)
			return dynamic_cast<const T&>(*this);
		throw InvalidTagCast();
	}

	bool isWriteType() const;
	void setWriteType(bool write_type);
	
	bool isNamed() const;
	void setNamed(bool named);

	const std::string& getName() const;
	void setName(const std::string& name, bool set_named = true);

	virtual Tag& read(std::istream& stream);
	virtual void write(std::ostream& stream) const;
	virtual void dump(std::ostream& stream, const std::string& indendation = "") const;
};

class TagEnd: public Tag {
public:
	TagEnd() : Tag(TAG_TYPE) {}
	
	static const int8_t TAG_TYPE = (int8_t) TagType::TAG_END;
};

template<typename T, TagType tag_type>
class ScalarTag: public Tag {
public:
	ScalarTag(T payload = 0) : Tag(TAG_TYPE), payload(payload) {}

	Tag& read(std::istream& stream) {
		payload = nbtstream::read<T>(stream);
		return *this;
	}

	void write(std::ostream& stream) const {
		Tag::write(stream);
		nbtstream::write<T>(stream, payload);
	}

	void dump(std::ostream& stream, const std::string& indendation = "") const {
		if (std::is_same<T, int8_t>::value)
			dumpTag(stream, indendation, *this, static_cast<int>(payload));
		else
			dumpTag(stream, indendation, *this);
	}

	T payload;
	
	static const int8_t TAG_TYPE = (int8_t) tag_type;
};

typedef ScalarTag<int8_t, TagType::TAG_BYTE> TagByte;
typedef ScalarTag<int16_t, TagType::TAG_SHORT> TagShort;
typedef ScalarTag<int32_t, TagType::TAG_INT> TagInt;
typedef ScalarTag<int64_t, TagType::TAG_LONG> TagLong;
typedef ScalarTag<float, TagType::TAG_FLOAT> TagFloat;
typedef ScalarTag<double, TagType::TAG_DOUBLE> TagDouble;

template<typename T, TagType tag_type>
class TagArray: public Tag {
public:
	TagArray() : Tag(TAG_TYPE) {}
	TagArray(const std::vector<T>& payload) : Tag(TAG_TYPE), payload(payload) {}

	Tag& read(std::istream& stream) {
		int32_t length = nbtstream::read<int32_t>(stream);
		payload.resize(length);
		if (std::is_same<T, int8_t>::value)
			stream.read(reinterpret_cast<char*>(&payload[0]), length * sizeof(T));
		else {
			for (int32_t i = 0; i < length; i++)
				payload[i] = nbtstream::read<T>(stream);
		}
		return *this;
	}
	
	void write(std::ostream& stream) const {
		Tag::write(stream);
		nbtstream::write<int32_t>(stream, payload.size());
		if (std::is_same<T, int8_t>::value)
			stream.write(reinterpret_cast<const char*>(&payload[0]), payload.size() * sizeof(T));
		else {
			for (size_t i = 0; i < payload.size(); i++)
				nbtstream::write<T>(stream, payload[i]);
		}
	}
	
	void dump(std::ostream& stream, const std::string& indendation = "") const {
		dumpTag(stream, indendation, *this, util::str(payload.size()) + " entries");
	}

	std::vector<T> payload;
	
	static const int8_t TAG_TYPE = (int8_t) tag_type;
};

typedef TagArray<int8_t, TagType::TAG_BYTE_ARRAY> TagByteArray;
typedef TagArray<int32_t, TagType::TAG_INT_ARRAY> TagIntArray;

class TagString: public Tag {
public:
	TagString() : Tag(TAG_TYPE) {}
	TagString(const std::string& payload) : Tag(TAG_TYPE), payload(payload) {}

	Tag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	std::string payload;
	
	static const int8_t TAG_TYPE = (int8_t) TagType::TAG_STRING;
};

// use shared_ptr in gcc <= 4.4.* instead of unique_ptr,
// because there are some bugs
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4
# define TagPtrType std::shared_ptr
#else
# define TagPtrType std::unique_ptr
#endif

typedef TagPtrType<Tag> TagPtr;

class TagList: public Tag {
public:
	TagList(int8_t tag_type = -1) : Tag(TAG_TYPE), tag_type(tag_type) {}
	~TagList();

	Tag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	int8_t tag_type;
	std::vector<TagPtr> payload;
	
	static const int8_t TAG_TYPE = (int8_t) TagType::TAG_LIST;
};

class TagCompound: public Tag {
public:
	TagCompound(const std::string& name = "") : Tag(TAG_TYPE) { setName(name); }
	~TagCompound();

	Tag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	bool hasTag(const std::string& name) const;
	
	template<typename T>
	bool hasTag(const std::string& name) const {
		if (!hasTag(name))
			return false;
		return payload.at(name)->getType() == T::TAG_TYPE;
	}
	
	template<typename T>
	bool hasArray(const std::string& name, int32_t len = -1) const {
		static_assert(std::is_same<T, TagByteArray>::value
				|| std::is_same<T, TagIntArray>::value,
			"Only TagByteArray and TagIntArray are allowed as template argument!");
		if (!hasTag<T>(name))
			return false;
		T& tag = payload.at(name)->cast<T>();
		return len == -1 || (unsigned) len == tag.payload.size();
	}
	
	template<typename T>
	bool hasList(const std::string& name, int32_t len = -1) const {
		if (!hasTag<TagList>(name))
			return false;
		TagList& tag = payload.at(name)->cast<TagList>();
		return tag.tag_type == T::TAG_TYPE && (len == -1 || (unsigned) len == tag.payload.size());
	}

	Tag& findTag(const std::string& name);
	const Tag& findTag(const std::string& name) const;
	
	template<typename T>
	T& findTag(const std::string& name) {
		return findTag(name).cast<T>();
	}

	template<typename T>
	const T& findTag(const std::string& name) const {
		return findTag(name).cast<T>();
	}

	void addTag(const std::string& name, TagPtr tag);

	std::map<std::string, TagPtr> payload;
	
	static const int8_t TAG_TYPE = (int8_t) TagType::TAG_COMPOUND;
};

class NBTFile: public TagCompound {
private:
	void decompressStream(std::istream& stream, std::stringstream& decompressed,
	        Compression compression);
public:
	NBTFile();
	NBTFile(const std::string name) : TagCompound(name) {}
	~NBTFile();

	void readCompressed(std::istream& stream, Compression compression = Compression::GZIP);
	void readNBT(std::istream& stream, Compression compression = Compression::GZIP);
	void readNBT(const char* filename, Compression compression = Compression::GZIP);
	void readNBT(const char* buffer, size_t len, Compression compression = Compression::GZIP);

	void writeNBT(std::ostream& stream, Compression compression = Compression::GZIP);
	void writeNBT(const char* filename, Compression compression = Compression::GZIP);
};

Tag* createTag(int8_t type);

template<typename T, typename ... Args>
TagPtrType<T> tag(Args ... args) {
	static_assert(std::is_base_of<Tag, T>::value, "The template type is not a subclass of Tag!");
	return TagPtrType<T>(new T(args...));
}

}
}
}

#endif /* NBT_H_ */
