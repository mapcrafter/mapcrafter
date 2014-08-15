/*
 * Copyright 2012-2014 Moritz Hilscher
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

#ifndef OTHER_H_
#define OTHER_H_

#include <string>
#include <sstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace util {

bool isBigEndian();
int16_t bigEndian16(int16_t x);
int32_t bigEndian32(int32_t x);
int64_t bigEndian64(int64_t x);

template <typename T>
std::string str(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

/**
 * A lazy function to convert different datatypes. It works by printing the value into a
 * string stream and reading the new datatype from it.
 *
 * The function throws a std::invalid_argument exception in case the string stream is in
 * an error state after the conversion. However, the behavior of the string stream here
 * is a bit strange. Because converting '42ff' would just result in '42', it is also
 * checked whether the whole string was processed (eof bit of string stream set).
 */
template <typename T>
T as(const std::string& from) {
	T to;
	std::stringstream ss(from);
	ss << from;
	ss >> to;
	if (!ss || !ss.eof())
		throw std::invalid_argument("Unable to parse '" + from + "'");
	return to;
}

template <>
std::string as(const std::string& from);

template <>
fs::path as(const std::string& from);

template <>
bool as<bool>(const std::string& from);

unsigned int parseHexNumber(const std::string& hex);

std::string trim(const std::string& str);
std::string escapeJSON(const std::string& str);
std::string capitalize(const std::string& str);

std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

bool startswith(const std::string& str, const std::string& start);
bool endswith(const std::string& str, const std::string& end);

template <typename T>
class Nullable {
public:
	Nullable() : null(true) {}
	Nullable(const T& value) : null(false), value(value) {}
	~Nullable() {}

	T get() const { return value; }
	void set(const T& value) { this->value = value; }

	bool isNull() const { return null; }
	void setNull() { null = true; }

	bool operator==(const T& other) const {
		if (null)
			return false;
		return value == other;
	}

private:
	bool null;
	T value;
};

} /* namespace util */
} /* namespace mapcrafter */
#endif /* OTHER_H_ */
