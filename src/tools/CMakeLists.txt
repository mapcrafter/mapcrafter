add_executable(nbtdump nbtdump.cpp)
target_link_libraries(nbtdump mapcraftercore)

add_executable(testconfig testconfig.cpp)
target_link_libraries(testconfig mapcraftercore)

install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mapcrafter_textures.py" DESTINATION bin)
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mapcrafter_png-it.py" DESTINATION bin)
