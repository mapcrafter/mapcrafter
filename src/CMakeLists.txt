add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mapcraftercore")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")

add_executable(mapcrafter mapcrafter.cpp)
target_link_libraries(mapcrafter mapcraftercore)
install(TARGETS mapcrafter DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/template DESTINATION share/mapcrafter)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/textures DESTINATION share/mapcrafter)
