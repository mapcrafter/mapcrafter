add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mapcraftercore")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools")

add_custom_target(runtests
    ./test_all --log_level=test_suite
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test" VERBATIM
)

add_custom_target(gen_texture_code
    sh tools/gen_texture_code.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

#add_executable(mapcrafter_export mapcrafter_export.cpp)
#target_link_libraries(mapcrafter_export mapcraftercore "${Boost_PROGRAM_OPTIONS_LIBRARY}")
#install(TARGETS mapcrafter_export DESTINATION bin)

add_executable(mapcrafter-legacy mapcrafter.cpp)
target_link_libraries(mapcrafter-legacy mapcraftercore-legacy "${Boost_PROGRAM_OPTIONS_LIBRARY}")
install(TARGETS mapcrafter-legacy DESTINATION bin)

add_executable(mapcrafter-legacy-markers mapcrafter_markers.cpp)
target_link_libraries(mapcrafter-legacy-markers mapcraftercore-legacy "${Boost_PROGRAM_OPTIONS_LIBRARY}")
install(TARGETS mapcrafter-legacy-markers DESTINATION bin)

install(FILES logging.conf DESTINATION ../etc/mapcrafter-legacy)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/template" DESTINATION share/mapcrafter-legacy)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/textures" DESTINATION share/mapcrafter-legacy)
