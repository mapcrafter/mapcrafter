/*
 * Copyright 2012-2014 Moritz Hilscher
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

template<typename T>
std::string str(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

/**
 * A lazy function to convert different datatypes.
 * Works by printing the value into a string stream and reading the new datatype from it.
 */
template<typename T>
T as(const std::string& from) {
	T to;
	std::stringstream ss(from);
	ss << from;
	ss >> to;
	return to;
}

template<>
std::string as(const std::string& from);

template<>
fs::path as(const std::string& from);

template<>
bool as<bool>(const std::string& from);

std::string trim(const std::string& str);
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

bool startswith(const std::string& str, const std::string& start);
bool endswith(const std::string& str, const std::string& end);

} /* namespace util */
} /* namespace mapcrafter */
#endif /* OTHER_H_ */
