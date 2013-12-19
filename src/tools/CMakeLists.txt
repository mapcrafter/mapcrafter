add_executable(nbtdump nbtdump.cpp)
add_executable(testconfig testconfig.cpp)
add_executable(testtextures testtextures.cpp)

target_link_libraries(nbtdump mapcraftercore)
target_link_libraries(testconfig mapcraftercore)
target_link_libraries(testtextures mapcraftercore)

add_executable(regiontest regiontest.cpp)
target_link_libraries(regiontest mapcraftercore)

add_executable(merge_regions merge_regions.cpp)
target_link_libraries(merge_regions mapcraftercore)
