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

#include <exception>
#include <map>
#include <vector>
#include <stdint.h>
#include <string>
#include <sstream>

namespace mapcrafter {
namespace mc {
namespace nbt {

enum TagType {
	TAG_END,
	TAG_BYTE,
	TAG_SHORT,
	TAG_INT,
	TAG_LONG,
	TAG_FLOAT,
	TAG_DOUBLE,
	TAG_BYTE_ARRAY,
	TAG_STRING,
	TAG_LIST,
	TAG_COMPOUND,
	TAG_INT_ARRAY
};

enum CompressionType {
	NO_COMPRESSION = 0, GZIP = 1, ZLIB = 2
};

template<typename T>
void dumpTag(std::ostream& stream, const char* type, bool named,
		const std::string& name, const std::string& indendation, T payload);

class NBTError: public std::exception {
private:
	std::string message;
public:
	NBTError(const char* message);
	NBTError(const std::string& message);
	~NBTError() throw ();

	virtual const char* what() const throw ();
};

class NBTTag {
protected:
	TagType type;
	bool named;
	bool writeType;
	std::string name;
public:
	NBTTag();
	NBTTag(TagType type);
	virtual ~NBTTag();

	const TagType& getType() const;

	bool isNamed() const;
	void setNamed(bool named);

	bool isWriteType() const;
	void setWriteType(bool writeType);

	const std::string& getName() const;
	void setName(const std::string& name, bool setNamed = true);

	virtual NBTTag& read(std::istream& stream);
	virtual void write(std::ostream& stream) const;
	virtual void dump(std::ostream& stream, const std::string& indendation = "") const;
};

class TagEnd: public NBTTag {
public:
	TagEnd() : NBTTag(TAG_END) {}
};

class TagByte: public NBTTag {
public:
	TagByte() : NBTTag(TAG_BYTE), payload(0) {}
	TagByte(int8_t payload) : NBTTag(TAG_BYTE), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	int8_t payload;
};

class TagShort: public NBTTag {
public:
	TagShort() : NBTTag(TAG_SHORT), payload(0) {}
	TagShort(int16_t payload) : NBTTag(TAG_SHORT), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	int16_t payload;
};

class TagInt: public NBTTag {
public:
	TagInt() : NBTTag(TAG_INT), payload(0) {}
	TagInt(int32_t payload) : NBTTag(TAG_INT), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	int32_t payload;
};

class TagLong: public NBTTag {
public:
	TagLong() : NBTTag(TAG_LONG), payload(0) {}
	TagLong(int64_t payload) : NBTTag(TAG_LONG), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	int64_t payload;
};

class TagFloat: public NBTTag {
public:
	TagFloat() : NBTTag(TAG_FLOAT), payload(0) {}
	TagFloat(float payload) : NBTTag(TAG_FLOAT), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	float payload;
};

class TagDouble: public NBTTag {
public:
	TagDouble() : NBTTag(TAG_DOUBLE), payload(0) {}
	TagDouble(double payload) : NBTTag(TAG_DOUBLE), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	double payload;
};

class TagByteArray: public NBTTag {
public:
	TagByteArray() : NBTTag(TAG_BYTE_ARRAY) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	std::vector<int8_t> payload;
};

class TagString: public NBTTag {
public:
	TagString() : NBTTag(TAG_STRING) {}
	TagString(const std::string& payload) : NBTTag(TAG_STRING), payload(payload) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	std::string payload;
};

class TagList: public NBTTag {
public:
	TagList() : NBTTag(TAG_LIST), tag_type(-1) {}
	TagList(int tag_type) : NBTTag(TAG_LIST), tag_type(tag_type) {}
	~TagList();

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	int tag_type;
	std::vector<NBTTag*> payload;
};

class TagCompound: public NBTTag {
public:
	TagCompound() : NBTTag(TAG_COMPOUND) { setName(""); }
	TagCompound(const std::string& name) : NBTTag(TAG_COMPOUND) { setName(name); }
	~TagCompound();

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	NBTTag* findTag(const std::string& name) const;

	template <typename T>
	T* findTag(const std::string& name, TagType type) const {
		NBTTag* tag = findTag(name);
		if (tag != NULL && tag->getType() == type)
			return (T*) tag;
		return NULL;
	}
	void addTag(const std::string& name, NBTTag* tag);

	std::map<std::string, NBTTag*> payload;
};

class TagIntArray: public NBTTag {
public:
	TagIntArray() : NBTTag(TAG_INT_ARRAY) {}

	NBTTag& read(std::istream& stream);
	void write(std::ostream& stream) const;
	void dump(std::ostream& stream, const std::string& indendation = "") const;

	std::vector<int32_t> payload;
};

class NBTFile: public TagCompound {
private:
	void decompressStream(std::istream& stream, std::stringstream& decompressed,
	        CompressionType compression);
public:
	NBTFile();
	NBTFile(const std::string name) : TagCompound(name) {}
	~NBTFile();

	void readCompressed(std::istream& stream, CompressionType compression = GZIP);
	void readNBT(std::istream& stream, CompressionType compression = GZIP);
	void readNBT(const char* filename, CompressionType compression = GZIP);
	void readNBT(const char* buffer, size_t len, CompressionType compression = GZIP);

	void writeNBT(std::ostream& stream, CompressionType compression = GZIP);
	void writeNBT(const char* filename, CompressionType compression = GZIP);
};

NBTTag* createTag(int type);

}
}
}

#endif /* NBT_H_ */
