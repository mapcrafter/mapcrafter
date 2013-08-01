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

#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <sstream>
#include <string>
#include <boost/filesystem.hpp>

#if BOOST_FILESYSTEM_VERSION == 2
	#define OLD_BOOST_FILESYSTEM 42
#endif

#ifndef BOOST_FILESYSTEM_VERSION
	#define OLD_BOOST_FILESYSTEM 42
#endif

#ifdef OLD_BOOST_FILESYSTEM
	#define BOOST_FS_FILENAME(p) (p).filename()
	#define BOOST_FS_ABSOLUTE(p, b) fs::complete((p), (b))
#else
	#define BOOST_FS_FILENAME(p) (p).filename().string()
	#define BOOST_FS_ABSOLUTE(p, b) fs::absolute((p), (b))
#endif

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

namespace fs = boost::filesystem;

namespace mapcrafter {

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

bool copyFile(const fs::path& from, const fs::path& to);
bool copyDirectory(const fs::path& from, const fs::path& to);
bool moveFile(const fs::path& from, const fs::path& to);

/**
 * Shows a nice command line progress bar.
 */
class ProgressBar {
private:
	// the maximum value of the progress
	int max;
	// animated? if yes, it updates the progress bar and makes it "animated"
	// but not good if you pipe the output in a file, so you can disable it
	bool animated;

	// the time of the start of progress
	int start;
	// time of last update
	int last_update;
	// value of last update
	int last_value;
	// current percentage of last update
	int last_percent;
public:
	ProgressBar();
	ProgressBar(int max, bool animated);

	void setMax(int max);
	int getMax() const;

	void setAnimated(bool animated);
	bool isAnimated() const;

	void update(int value);
	void finish();
};

}

#endif /* UTIL_H_ */
