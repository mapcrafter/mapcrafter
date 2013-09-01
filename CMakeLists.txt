cmake_minimum_required(VERSION 2.6)
project(mapcrafter)

option(OPT_DEBUG "Debug" OFF)
option(OPT_OPTIMIZE "Optimize" ON)
option(OPT_PROFILE "Profile" OFF)

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

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")
