/*
 * Copyright 2012-2015 Moritz Hilscher
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

// shortcut to detect Windows
#if defined(__WIN32__) || defined(__WIN64__) || defined(_WIN32) || defined(_WIN64)
#  define OS_WINDOWS
#endif

// include compat/*.h here if all files need it
#include "compat/boost.h"
#include "compat/nullptr.h"

#include "util/filesystem.h"
#include "util/json.h"
#include "util/logging.h"
#include "util/progress.h"
#include "util/math.h"
#include "util/other.h"
#include "util/terminal.h"

#endif /* UTIL_H_ */
