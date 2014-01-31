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

#include "world.h"

#include <array>
#include <string>
#include <vector>

#ifndef WORLDHELPER_H_
#define WORLDHELPER_H_

namespace mapcrafter {
namespace mc {

class Sign {
public:
	typedef std::array<std::string, 4> Lines;

	Sign();
	Sign(const mc::BlockPos& pos, const Lines& lines);
	~Sign();

	/**
	 * Returns the position of the sign.
	 */
	const mc::BlockPos& getPos() const;

	/**
	 * Returns the four lines of the sign.
	 */
	const Sign::Lines& getLines() const;

	/**
	 * Returns the text of the sign -- the not-empty lines joined with a separative space.
	 */
	const std::string& getText() const;

private:
	mc::BlockPos pos;

	Lines lines;
	std::string text;
};

std::vector<Sign> findSignsInWorld(const mc::World& world);

} /* namespace mc */
} /* namespace mapcrafter */

#endif /* WORLDHELPER_H_ */
