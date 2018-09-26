#add_executable(nbtdump nbtdump.cpp)
#target_link_libraries(nbtdump mapcraftercore-legacy)

#add_executable(testconfig testconfig.cpp)
#target_link_libraries(testconfig mapcraftercore-legacy)

#add_executable(testtextures testtextures.cpp)
#target_link_libraries(testtextures mapcraftercore-legacy "${Boost_PROGRAM_OPTIONS_LIBRARY}")

#add_executable(mapcraftertest mapcraftertest.cpp)
#target_link_libraries(mapcraftertest mapcraftercore-legacy)

install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mapcrafter-legacy-textures.py" DESTINATION bin)
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mapcrafter-legacy-png-it.py" DESTINATION bin)
