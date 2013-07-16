set(SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
)
set(HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mc")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/render")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/render/rendermodes")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")

add_library(mapcraftercore SHARED ${SOURCE})
target_link_libraries(mapcraftercore ${Boost_IOSTREAMS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
	target_link_libraries(mapcraftercore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endif()

target_link_libraries(mapcraftercore png)
target_link_libraries(mapcraftercore pthread)

add_executable(mapcrafter mapcrafter.cpp)
target_link_libraries(mapcrafter mapcraftercore)
