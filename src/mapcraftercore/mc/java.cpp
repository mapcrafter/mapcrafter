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

#include "java.h"

namespace mapcrafter {
namespace mc {

JavaRandom::JavaRandom()
	: seed(0) {
}

JavaRandom::~JavaRandom() {
}

void JavaRandom::setSeed(long long seed) {
	this->seed = (seed ^ 0x5EECE66DLL) & ((1LL << 48) - 1);
}

int JavaRandom::next(int bits) {
	seed = (seed * 0x5EECE66DLL + 0xBLL) & ((1LL << 48) - 1);
	return (int) (seed >> (48 - bits));
}

int JavaRandom::nextInt(int max) {
	// invalid max
	if (max <= 0)
		return -1;
	if ((max & -max) == max) // i.e., n is a power of 2
		return (int) ((max * (long long) next(31)) >> 31);
	int bits, val;
	do {
		bits = next(31);
		val = bits % max;
	} while (bits - val + (max - 1) < 0);
	return val;
}

const int JavaSimplexGenerator::GRADIENTS[12][3] = {
	{ 1, 1, 0},
	{-1, 1, 0},
	{ 1,-1, 0},
	{-1,-1, 0},
	{ 1, 0, 1},
	{-1, 0, 1},
	{ 1, 0,-1},
	{-1, 0,-1},
	{ 0, 1, 1},
	{ 0,-1, 1},
	{ 0, 1,-1},
	{ 0,-1,-1}
};

// If you are an Mojang/Microsoft employee reading this:
// These numbers appeared to me in a dream
// I don't know how, but they help to reproduce the swamp grass colors just like in Minecraft ;)
const int JavaSimplexGenerator::PERMUTATIONS[512] = {
	64, 175, 124, 148, 10, 239, 244, 91, 138, 73, 228, 171, 27, 134, 77, 122,
	238, 196, 202, 181, 211, 7, 49, 173, 48, 165, 120, 217, 129, 56, 153, 8,
	140, 141, 21, 130, 71, 100, 132, 23, 176, 250, 29, 104, 149, 159, 180, 237,
	247, 11, 252, 241, 14, 2, 219, 75, 178, 151, 233, 251, 103, 45, 52, 201,
	222, 18, 223, 88, 136, 34, 227, 235, 35, 160, 0, 131, 51, 214, 39, 216,
	207, 26, 137, 185, 41, 13, 249, 54, 112, 5, 66, 242, 157, 158, 28, 89, 86,
	192, 172, 17, 69, 204, 38, 221, 65, 166, 9, 226, 33, 30, 84, 240, 59, 224,
	127, 108, 92, 146, 99, 195, 255, 98, 126, 4, 133, 236, 189, 121, 144, 183,
	80, 109, 191, 218, 161, 53, 25, 93, 72, 150, 163, 234, 205, 152, 61, 37,
	197, 78, 81, 32, 85, 70, 187, 63, 96, 115, 117, 184, 139, 79, 74, 46, 188,
	182, 76, 31, 174, 57, 68, 198, 90, 245, 230, 106, 94, 212, 190, 16, 200,
	213, 206, 44, 43, 215, 231, 12, 177, 203, 220, 24, 170, 19, 209, 82, 95,
	125, 194, 248, 208, 55, 67, 1, 87, 110, 135, 162, 128, 3, 60, 225, 15, 186,
	232, 145, 119, 142, 113, 154, 102, 164, 42, 156, 210, 22, 253, 147, 169,
	193, 83, 143, 118, 123, 254, 167, 111, 114, 6, 50, 40, 199, 179, 246, 20,
	107, 168, 97, 229, 101, 155, 62, 47, 58, 116, 243, 105, 36, 64, 175, 124,
	148, 10, 239, 244, 91, 138, 73, 228, 171, 27, 134, 77, 122, 238, 196, 202,
	181, 211, 7, 49, 173, 48, 165, 120, 217, 129, 56, 153, 8, 140, 141, 21,
	130, 71, 100, 132, 23, 176, 250, 29, 104, 149, 159, 180, 237, 247, 11, 252,
	241, 14, 2, 219, 75, 178, 151, 233, 251, 103, 45, 52, 201, 222, 18, 223,
	88, 136, 34, 227, 235, 35, 160, 0, 131, 51, 214, 39, 216, 207, 26, 137,
	185, 41, 13, 249, 54, 112, 5, 66, 242, 157, 158, 28, 89, 86, 192, 172, 17,
	69, 204, 38, 221, 65, 166, 9, 226, 33, 30, 84, 240, 59, 224, 127, 108, 92,
	146, 99, 195, 255, 98, 126, 4, 133, 236, 189, 121, 144, 183, 80, 109, 191,
	218, 161, 53, 25, 93, 72, 150, 163, 234, 205, 152, 61, 37, 197, 78, 81, 32,
	85, 70, 187, 63, 96, 115, 117, 184, 139, 79, 74, 46, 188, 182, 76, 31, 174,
	57, 68, 198, 90, 245, 230, 106, 94, 212, 190, 16, 200, 213, 206, 44, 43,
	215, 231, 12, 177, 203, 220, 24, 170, 19, 209, 82, 95, 125, 194, 248, 208,
	55, 67, 1, 87, 110, 135, 162, 128, 3, 60, 225, 15, 186, 232, 145, 119, 142,
	113, 154, 102, 164, 42, 156, 210, 22, 253, 147, 169, 193, 83, 143, 118,
	123, 254, 167, 111, 114, 6, 50, 40, 199, 179, 246, 20, 107, 168, 97, 229,
	101, 155, 62, 47, 58, 116, 243, 105, 36
};


namespace {

double dot(const int* v, double x, double y) {
	return (double) v[0] * x + (double) v[1] * y;
}

}

double JavaSimplexGenerator::getValue(double x, double y) const {
	// Once again, have a look at:
	// http://staffwww.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java
	
	double s = (x + y) * F2;
	int i = std::floor(x + s);
	int j = std::floor(y + s);

	double t = (double) (i + j) * G2;
	double X0 = (double)i - t;
	double Y0 = (double)j - t;
	double x0 = x - X0;
	double y0 = y - Y0;
	
	int i1, j1;
	if (x0 > y0) {
		i1 = 1;
		j1 = 0;
	} else {
		i1 = 0;
		j1 = 1;
	}

	double x1 = x0 - (double) i1 + G2;
	double y1 = y0 - (double) j1 + G2;
	double x2 = x0 - 1.0 + 2.0 * G2;
	double y2 = y0 - 1.0 + 2.0 * G2;
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = PERMUTATIONS[ii + PERMUTATIONS[jj]] % 12;
	int gi1 = PERMUTATIONS[ii + i1 + PERMUTATIONS[jj + j1]] % 12;
	int gi2 = PERMUTATIONS[ii + 1 + PERMUTATIONS[jj + 1]] % 12;
	
	double t0 = 0.5 - x0 * x0 - y0 * y0;
	double n0;
	if (t0 < 0.0) {
		n0 = 0.0;
	} else {
		t0 = t0 * t0;
		n0 = t0 * t0 * dot(GRADIENTS[gi0], x0, y0);
	}

	double t1 = 0.5 - x1 * x1 - y1 * y1;
	double n1;
	if (t1 < 0.0) {
		n1 = 0.0;
	} else {
		t1 = t1 * t1;
		n1 = t1 * t1 * dot(GRADIENTS[gi1], x1, y1);
	}

	double t2 = 0.5 - x2 * x2 - y2 * y2;
	double n2;
	if (t2 < 0.0) {
		n2 = 0.0;
	} else {
		t2 = t2 * t2;
		n2 = t2 * t2 * dot(GRADIENTS[gi2], x2, y2);
	}

	return 70.0 * (n0 + n1 + n2);
}

}
}

