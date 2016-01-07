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

#ifndef COMPAT_BOOST_H_
#define COMPAT_BOOST_H_

#include <boost/filesystem.hpp>

#if BOOST_FILESYSTEM_VERSION == 2
# define OLD_BOOST_FILESYSTEM 42
#endif

#ifndef BOOST_FILESYSTEM_VERSION
# define OLD_BOOST_FILESYSTEM 42
#endif

#ifdef OLD_BOOST_FILESYSTEM
# define BOOST_FS_FILENAME(p) (p).filename()
# define BOOST_FS_ABSOLUTE(p, b) fs::complete((p), (b))
# define BOOST_FS_ABSOLUTE1(p) fs::complete((p))
#else
# define BOOST_FS_FILENAME(p) (p).filename().string()
# define BOOST_FS_ABSOLUTE(p, b) fs::absolute((p), (b))
# define BOOST_FS_ABSOLUTE1(p) fs::absolute((p))
#endif

#endif /* COMPAT_BOOST_H_ */
