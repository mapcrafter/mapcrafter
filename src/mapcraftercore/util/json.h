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

#ifndef JSON_H_
#define JSON_H_

#include "picojson.h"

#include <string>

namespace mapcrafter {
namespace util {

class JSONError : public std::runtime_error {
public:
	JSONError(const std::string& message = "")
		: std::runtime_error(message) {}
};

template <typename T>
T json_get(const picojson::object& object, const std::string& key) {
	if (object.count(key) && object.at(key).is<T>())
		return object.at(key).get<T>();
	throw JSONError("Unable to find/convert '" + key + "' of json object!");
}

}
}

#endif /* JSON_H_ */
