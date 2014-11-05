add_executable(nbtdump nbtdump.cpp)
target_link_libraries(nbtdump mapcraftercore)

add_executable(testconfig testconfig.cpp)
target_link_libraries(testconfig mapcraftercore)

add_executable(testtextures testtextures.cpp)
target_link_libraries(testtextures mapcraftercore "${Boost_PROGRAM_OPTIONS_LIBRARY}")

add_executable(merge_azeroth merge_azeroth.cpp)
target_link_libraries(merge_azeroth mapcraftercore)

add_executable(merge_regions merge_regions.cpp)
target_link_libraries(merge_regions mapcraftercore)

add_executable(touch_regions touch_regions.cpp)
target_link_libraries(touch_regions mapcraftercore)

install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mapcrafter_textures.py" DESTINATION bin)
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mapcrafter_png-it.py" DESTINATION bin)
