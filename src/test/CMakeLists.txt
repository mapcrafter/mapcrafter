if(NOT OPT_SKIP_TESTS)
	add_executable(test_all test_all.cpp test_config.cpp test_image.cpp test_nbt.cpp test_pos.cpp test_tile.cpp)
	target_link_libraries(test_all mapcraftercore)
endif()
