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

add_executable(mapcrafter_export mapcrafter_export.cpp)
target_link_libraries(mapcrafter_export mapcraftercore "${Boost_PROGRAM_OPTIONS_LIBRARY}")
install(TARGETS mapcrafter_export DESTINATION bin)

add_executable(mapcrafter mapcrafter.cpp)
target_link_libraries(mapcrafter mapcraftercore "${Boost_PROGRAM_OPTIONS_LIBRARY}")
install(TARGETS mapcrafter DESTINATION bin)

add_executable(mapcrafter_markers mapcrafter_markers.cpp)
target_link_libraries(mapcrafter_markers mapcraftercore "${Boost_PROGRAM_OPTIONS_LIBRARY}")
install(TARGETS mapcrafter_markers DESTINATION bin)

install(FILES logging.conf DESTINATION ../etc/mapcrafter)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/template" DESTINATION share/mapcrafter)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/blocks" DESTINATION share/mapcrafter)
