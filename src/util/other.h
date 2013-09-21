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

void trim(std::string& str);
void replaceAll(std::string& str, const std::string& from, const std::string& to);

/**
 * Does a rotated shift by m bytes to the right. n is the length of the bits.
 * Example: 0b0010 >> 2 = 0b1000
 */
template<typename T>
T rotate_shift_r(T x, int m, int n) {
	return (x >> m) | ((x & ((1 << m) - 1)) << (n - m));
}

/**
 * Does a rotated shift by m bytes to the left. n is the length of the bits.
 * Example: 0b0100 << 2 = 0b0001
 */
template<typename T>
T rotate_shift_l(T x, int m, int n) {
	return ((x << m) & ((1 << n) - 1)) | (x >> (n - m));
}

} /* namespace util */
} /* namespace mapcrafter */
#endif /* OTHER_H_ */
