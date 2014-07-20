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

#ifndef BOOST_THREAD_H_
#define BOOST_THREAD_H_

#ifdef OPT_USE_BOOST_THREAD
#  include <boost/thread.hpp>
#  define THREAD_NS boost
#else
#  include <condition_variable>
#  include <mutex>
#  define THREAD_NS std
#endif

#endif /* BOOST_THREAD_H_ */
