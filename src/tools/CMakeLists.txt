add_executable(nbtdump nbtdump.cpp)
add_executable(testconfig testconfig.cpp)
add_executable(testtextures testtextures.cpp)

target_link_libraries(nbtdump mapcraftercore)
target_link_libraries(testconfig mapcraftercore)
target_link_libraries(testtextures mapcraftercore)
