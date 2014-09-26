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

#ifndef UTIL_H_
#define UTIL_H_

#include "config.h"

// some compatibility stuff, definitions

#ifndef HAVE_NULLPTR
// official workaround for nullptr
// see also http://stackoverflow.com/questions/2419800/can-nullptr-be-emulated-in-gcc
const                        // this is a const object...
class {
public:
  template <class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template <class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr
#endif

// shortcut to detect Windows
#if defined(__WIN32__) || defined(__WIN64__) || defined(_WIN32) || defined(_WIN64)
#  define OS_WINDOWS
#endif

// include compat/*.h here if all files need it
#include "compat/boost.h"

#include "util/filesystem.h"
#include "util/logging.h"
#include "util/progress.h"
#include "util/math.h"
#include "util/other.h"
#include "util/terminal.h"
#include "util/utf8.h"

#endif /* UTIL_H_ */
