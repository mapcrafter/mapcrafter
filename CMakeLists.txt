cmake_minimum_required(VERSION 2.6)
project(mapcrafter)

option(OPT_DEBUG "Sets debug compiler flags" OFF)
option(OPT_OPTIMIZE "Sets optimize compiler flags" ON)
option(OPT_PROFILE "Sets profile compiler flags" OFF)
option(OPT_BOOST_STATIC "Links boost statically" OFF)

if(OPT_DEBUG)
    add_definitions(-g)
endif()
if(OPT_OPTIMIZE)
    add_definitions(-O3)
endif()
if(OPT_PROFILE)
    add_definitions(-pg)
endif()

add_definitions(-Wall)
add_definitions(-std=c++0x)

if(OPT_BOOST_STATIC)
	set(Boost_USE_STATIC_LIBS ON)
	# we need zlib to link boost iostreams statically
	find_library(Z z)
endif()
find_package(Boost COMPONENTS iostreams system filesystem program_options REQUIRED)
find_package(Boost COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_library(PTHREAD pthread)
find_library(PNG png)

if("${PTHREAD}" EQUAL "PTHREAD-NOTFOUND")
    message(FATAL_ERROR "libpthread not found!")
endif()
if("${PNG}" EQUAL "PNG-NOTFOUND")
    message(FATAL_ERROR "libpng not found!")
endif()
if(OPT_BOOST_STATIC AND "${Z}" EQUAL "Z-NOTFOUND")
	message(FATAL_ERROR "zlib not found!")
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")
