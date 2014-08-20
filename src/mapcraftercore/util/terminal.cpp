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

#include "terminal.h"

#include "../config.h"

#include <cstdio>

#if defined(__WIN32__) || defined(__WIN64__) || defined(_WIN32) || defined(_WIN64)
#  include <io.h>
#  include <windows.h>
#  define OS_WIN
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

namespace mapcrafter {
namespace util {

bool isOutTTY() {
#ifdef OS_WIN
	return _isatty(_fileno(stdout));
#else
	return isatty(fileno(stdout));
#endif
}

setcolor::setcolor(int type, int color)
	: type(type), color(color) {
}

std::ostream& setcolor::operator()(std::ostream& out) const {
	if (!isAvailable())
		return out;
	return out << "\033[1;" << type + color << "m";
}

std::ostream& setcolor::reset(std::ostream& out) {
	if (!isAvailable())
		return out;
	return out << "\033[1;0m";
}

bool setcolor::isAvailable() {
	if (!available_initialized) {
		available_initialized = true;
		available = isOutTTY();
	}
	return available;
}

bool setcolor::available_initialized = false;
bool setcolor::available = false;

std::ostream& operator<<(std::ostream& out, const setcolor& color) {
	return color(out);
}

} /* namespace util */
} /* namespace mapcrafter */
