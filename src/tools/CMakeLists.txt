add_executable(testtextures testtextures.cpp)
add_executable(nbtdump nbtdump.cpp)

target_link_libraries(testtextures mapcraftercore)
target_link_libraries(nbtdump mapcraftercore)
