add_executable(nbtdump nbtdump.cpp)
target_link_libraries(nbtdump mapcraftercore)

add_executable(testconfig testconfig.cpp)
target_link_libraries(testconfig mapcraftercore)

add_executable(testtextures testtextures.cpp)
target_link_libraries(testtextures mapcraftercore ${Boost_PROGRAM_OPTIONS_LIBRARY})
