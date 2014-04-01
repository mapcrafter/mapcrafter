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


#ifndef MATH_H_
#define MATH_H_

#include <cmath>

namespace mapcrafter {
namespace util {

/**
 * Does a rotated shift by m bytes to the right. n is the length of the bits.
 * Example: 0b0010 >> 2 = 0b1000
 */
template<typename T>
T rotateShiftRight(T x, int m, int n) {
	return (x >> m) | ((x & ((1 << m) - 1)) << (n - m));
}

/**
 * Does a rotated shift by m bytes to the left. n is the length of the bits.
 * Example: 0b0100 << 2 = 0b0001
 */
template<typename T>
T rotateShiftLeft(T x, int m, int n) {
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

/**
 * Checks whether to floating point numbers are equal with a small deviation.
 */
template<typename T>
bool floatingPointEquals(T a, T b, T epsilon = 0.0001) {
	return std::abs(a - b) < epsilon;
}

} /* namespace util */
} /* namespace mapcrafter */

#endif /* MATH_H_ */
