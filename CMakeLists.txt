cmake_minimum_required(VERSION 2.6)
project(mapcrafter)

option(OPT_DEBUG "Sets debug compiler flags" OFF)
option(OPT_OPTIMIZE "Sets optimize compiler flags" ON)
option(OPT_PROFILE "Sets profile compiler flags" OFF)
option(OPT_USE_BOOST_THREAD "Uses boost thread instead of C++11 threads" OFF)
option(OPT_SKIP_TESTS "Skip compiling the boost unittests" OFF)
option(OPT_LINK_DEPS_STATICALLY "Links all dependencies (libpng, libjpeg, boost...) statically" OFF)
option(OPT_LINK_BOOST_STATICALLY "Links boost statically" OFF)
option(OPT_BOOST_STATIC "Links boost statically (deprecated, use OPT_LINK_BOOST_STATICALLY)" OFF)
option(OPT_INSTALL_HEADERS "Installs libmapcraftercore header files" ON)

if(OPT_BOOST_STATIC)
    set(OPT_LINK_BOOST_STATICALLY ON)
endif()
if(OPT_LINK_DEPS_STATICALLY)
    set(OPT_LINK_BOOST_STATICALLY ON)
endif()

if(OPT_DEBUG)
    add_definitions(-g)
else()
    add_definitions(-DNDEBUG)
endif()
if(OPT_OPTIMIZE)
    add_definitions(-O3)
endif()
if(OPT_PROFILE)
    add_definitions(-pg)
endif()

add_definitions(-Wall)

# http://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    add_definitions(-std=c++11)
elseif(COMPILER_SUPPORTS_CXX0X)
    add_definitions(-std=c++0x)
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(OPT_LINK_BOOST_STATICALLY)
    set(Boost_USE_STATIC_LIBS ON)
    # we need zlib to link boost iostreams statically
    find_package(ZLIB REQUIRED)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

find_package(Boost COMPONENTS iostreams system filesystem program_options REQUIRED)
if(OPT_USE_BOOST_THREAD)
    find_package(Boost COMPONENTS thread REQUIRED)
else()
    find_package(Threads REQUIRED)
    include_directories(${Threads_INCLUDE_DIRS})
endif()
if(NOT OPT_SKIP_TESTS)
    find_package(Boost COMPONENTS unit_test_framework)
    if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
        set(OPT_SKIP_TESTS ON)
        message("Boost Unit Test Framework not found. Skipping the tests.")
    endif()
endif()
include_directories(${Boost_INCLUDE_DIRS})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
# ${JPEG_INCLUDE_DIRS} somehow doesn't work
include_directories(${JPEG_INCLUDE_DIR})

add_custom_target(
    build_pngquant
    COMMAND ./configure --extra-cflags=-fPIC && make
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/pngquant-2.7.1")

add_library(imagequant STATIC IMPORTED)
set_property(TARGET imagequant APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imagequant PROPERTIES
    IMPORTED_LOCATION_NOCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/lib/pngquant-2.7.1/lib/libimagequant.a")

add_dependencies(imagequant build_pngquant)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")
