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

#ifndef UTF8_H_
#define UTF8_H_

#include <string>

namespace mapcrafter {
namespace util {

std::string convertUnicodeEscapeSequence(const std::string& escape_sequence);
std::string replaceUnicodeEscapeSequences(const std::string& string);

} /* namespace util */
} /* namespace mapcrafter */

#endif /* UTF8_H_ */
