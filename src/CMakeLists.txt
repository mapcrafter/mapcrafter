cmake_minimum_required(VERSION 2.6)
project(mapcrafter)

add_definitions(-O3)
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


include_directories("${PROJECT_SOURCE_DIR}")

set(SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
)
set(HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
)

add_subdirectory("${PROJECT_SOURCE_DIR}/mc")
add_subdirectory("${PROJECT_SOURCE_DIR}/render")
add_subdirectory("${PROJECT_SOURCE_DIR}/render/rendermodes")
add_subdirectory("${PROJECT_SOURCE_DIR}/tools")
add_subdirectory("${PROJECT_SOURCE_DIR}/test")

add_library(mapcraftercore STATIC ${SOURCE} ${HEADERS})
target_link_libraries(mapcraftercore ${Boost_IOSTREAMS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
	target_link_libraries(mapcraftercore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endif()

target_link_libraries(mapcraftercore png)
target_link_libraries(mapcraftercore pthread)

add_executable(mapcrafter mapcrafter.cpp)
target_link_libraries(mapcrafter mapcraftercore)
