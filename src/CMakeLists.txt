INCLUDE(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS -std=c++0x)
CHECK_CXX_SOURCE_COMPILES("int main() { void* p = nullptr; }" HAVE_NULLPTR)

INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES("endian.h" HAVE_ENDIAN_H)
CHECK_INCLUDE_FILES("sys/endian.h" HAVE_SYS_ENDIAN_H)

if(HAVE_SYS_ENDIAN_H)
    set(HAVE_ENDIAN_H ON)
    set(ENDIAN_H_FREEBSD ON)
else()
    set(ENDIAN_H_FREEBSD OFF)
endif()

if(HAVE_ENDIAN_H)
    if(HAVE_SYS_ENDIAN_H)
        CHECK_CXX_SOURCE_COMPILES("#include <sys/endian.h>\n int main() { be16toh(42); be32toh(73); be64toh(21); }" HAVE_ENDIAN_CONVERSION)
    else()
        CHECK_CXX_SOURCE_COMPILES("#include <endian.h>\n int main() { be16toh(42); be32toh(73); be64toh(21); }" HAVE_ENDIAN_CONVERSION)
    endif()
    if(NOT HAVE_ENDIAN_CONVERSION)
        set(HAVE_ENDIAN_H OFF)
    endif()
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

set(SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
)
set(HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
	${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/config")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mc")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/render")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/render/rendermodes")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")

add_library(mapcraftercore SHARED ${SOURCE})
target_link_libraries(mapcraftercore ${Boost_IOSTREAMS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
	target_link_libraries(mapcraftercore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endif()

target_link_libraries(mapcraftercore png)
target_link_libraries(mapcraftercore pthread)

add_executable(mapcrafter mapcrafter.cpp)
target_link_libraries(mapcrafter mapcraftercore)
