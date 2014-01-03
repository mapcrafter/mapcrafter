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


#ifndef MATH_H_
#define MATH_H_

namespace mapcrafter {
namespace util {

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

/**
 * Does a floor division a/b towards -infinity.
 */
template<typename T>
T floordiv(T a, T b) {
	if (a >= 0)
		return a / b;
	return (a - b + 1) / b;
}

} /* namespace util */
} /* namespace mapcrafter */

#endif /* MATH_H_ */
