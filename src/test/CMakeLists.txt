if(NOT OPT_SKIP_TESTS)
    add_executable(test_all test_all.cpp test_config.cpp test_image.cpp test_image_quantization.cpp test_misc.cpp test_nbt.cpp test_pos.cpp test_region.cpp test_tile.cpp test_util.cpp test_worldcrop.cpp)
    target_link_libraries(test_all mapcraftercore "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
endif()
