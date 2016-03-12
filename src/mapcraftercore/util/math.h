/*
 * Copyright 2012-2016 Moritz Hilscher
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
#include <limits>

namespace mapcrafter {
namespace util {

/**
 * Does a rotated shift by m bytes to the right. n is the length of the bits.
 * Example: 0b0010 >> 2 = 0b1000
 */
template <typename T>
T rotateShiftRight(T x, int m, int n) {
	return (x >> m) | ((x & ((1 << m) - 1)) << (n - m));
}

/**
 * Does a rotated shift by m bytes to the left. n is the length of the bits.
 * Example: 0b0100 << 2 = 0b0001
 */
template <typename T>
T rotateShiftLeft(T x, int m, int n) {
	return ((x << m) & ((1 << n) - 1)) | (x >> (n - m));
}

/**
 * Does a floor division a/b towards -infinity.
 */
template <typename T>
T floordiv(T a, T b) {
	if (a >= 0)
		return a / b;
	return (a - b + 1) / b;
}

/**
 * Checks whether two floating point numbers are equal with a small deviation.
 */
template <typename T>
bool floatingPointEquals(T a, T b, T epsilon = 0.0001) {
	return std::abs(a - b) < epsilon;
}

/**
 * Binary constants helper.
 */
template <unsigned long N>
struct binary {
	// prepend higher bits to lowest bit
	static const unsigned long value = binary<N / 10>::value << 1 | N % 10;
};

template <unsigned long N>
const unsigned long binary<N>::value;

/**
 * Specialization, terminates recursion.
 */
template <>
struct binary<0> {
	static const unsigned long value = 0;
};

/**
 * A 1-dimensional interval.
 * Has either two limits, one limit (minimum or maximum) or no limit.
 */
template <typename T>
class Interval1D {
public:
	Interval1D(T min = MIN_DEFAULT, T max = MAX_DEFAULT);

	/**
	 * Sets the minimum/maximum limit.
	 */
	void setMin(T min);
	void setMax(T max);

	/**
	 * Resets the minimum/maximum, i.e. sets it to infinity (or -infinity for minimum).
	 */
	void resetMin();
	void resetMax();

	/**
	 * Returns whether a specific value is within in the bounds.
	 */
	bool contains(T value) const;

	static const T MIN_DEFAULT = std::numeric_limits<T>::lowest();
	static const T MAX_DEFAULT = std::numeric_limits<T>::max();

private:
	// minimum, maximum
	T min, max;
};

/**
 * A 3-dimensional interval consisting of three 1-dimension intervals.
 */
template <typename T>
class Interval3D {
public:
	Interval3D(const Interval1D<T>& interval_x = Interval1D<T>(),
			const Interval1D<T>& interval_z = Interval1D<T>(),
			const Interval1D<T>& interval_y = Interval1D<T>());

	void setIntervalX(Interval1D<T> interval_x);
	void setIntervalZ(Interval1D<T> interval_z);
	void setIntervalY(Interval1D<T> interval_y);

	bool contains(T x, T z, T y) const;

protected:
	Interval1D<T> interval_x, interval_z, interval_y;
};

template <typename T>
Interval1D<T>::Interval1D(T min, T max)
	: min(min), max(max) {
}

template <typename T>
void Interval1D<T>::setMin(T min) {
	this->min = min;
}

template <typename T>
void Interval1D<T>::setMax(T max) {
	this->max = max;
}

template <typename T>
void Interval1D<T>::resetMin() {
	min = MIN_DEFAULT;
}

template <typename T>
void Interval1D<T>::resetMax() {
	max = MAX_DEFAULT;
}

template <typename T>
bool Interval1D<T>::contains(T value) const {
	return min <= value && value <= max;
}

template <typename T>
Interval3D<T>::Interval3D(const Interval1D<T>& interval_x, const Interval1D<T>& interval_z,
		const Interval1D<T>& interval_y)
	: interval_x(interval_x), interval_z(interval_z), interval_y(interval_y) {
}

template <typename T>
void Interval3D<T>::setIntervalX(Interval1D<T> interval_x) {
	this->interval_x = interval_x;
}

template <typename T>
void Interval3D<T>::setIntervalZ(Interval1D<T> interval_z) {
	this->interval_z = interval_z;
}

template <typename T>
void Interval3D<T>::setIntervalY(Interval1D<T> interval_y) {
	this->interval_y = interval_y;
}

template <typename T>
bool Interval3D<T>::contains(T x, T z, T y) const {
	return interval_x.contains(x) && interval_z.contains(z) && interval_y.contains(y);
}

} /* namespace util */
} /* namespace mapcrafter */

#endif /* MATH_H_ */
