cmake_minimum_required(VERSION 2.6)
project(mapcrafter)

option(OPT_DEBUG "Sets debug compiler flags" OFF)
option(OPT_OPTIMIZE "Sets optimize compiler flags" ON)
option(OPT_PROFILE "Sets profile compiler flags" OFF)
option(OPT_SKIP_TESTS "Skip compiling the boost unittests" OFF)
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
	find_package(ZLIB REQUIRED)
endif()

find_package(Boost COMPONENTS iostreams system filesystem program_options REQUIRED)
if(NOT OPT_SKIP_TESTS)
    find_package(Boost COMPONENTS unit_test_framework)
    if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
        set(OPT_SKIP_TESTS ON)
        message("Boost Unit Test Framework not found. Skipping the tests.")
    endif()
endif()
include_directories(${Boost_INCLUDE_DIRS})

find_package(Threads REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")
