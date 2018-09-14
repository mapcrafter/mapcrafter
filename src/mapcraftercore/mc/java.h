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

#ifndef JAVA_H_
#define JAVA_H_

#include <array>
#include <cmath>

namespace mapcrafter {
namespace mc {

/**
 * Very minimal implementation of Java Random class.
 *
 * Seems to work reliably only for setting a seed and returning first integer < 10
 * (for slime chunk detection), everything else yields different results than Java random.
 */
class JavaRandom {
public:
	JavaRandom();
	~JavaRandom();

	void setSeed(long long seed);
	int next(int bits);
	int nextInt(int max);

protected:
	long long seed;
};

/**
 * Simplex noise implementation - with a little help from:
 * http://staffwww.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java
 * Thanks!
 */
class JavaSimplexGenerator {
public:
	double getValue(double x, double y) const;

protected:
	static const int GRADIENTS[12][3];
	static const int PERMUTATIONS[512];
	
	static constexpr double F2 = 0.5 * (sqrt(3) - 1.0);
	static constexpr double G2 = (3.0 - sqrt(3)) / 6.0;
};

}
}

#endif

