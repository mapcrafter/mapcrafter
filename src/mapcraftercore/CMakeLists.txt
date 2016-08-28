INCLUDE(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS -std=c++0x)
CHECK_CXX_SOURCE_COMPILES("int main() { void* p = nullptr; }" HAVE_NULLPTR)
CHECK_CXX_SOURCE_COMPILES("enum class Test { A=0, B=1, C=3 }; int main() { Test::A < Test::C; }" HAVE_ENUM_CLASS_COMPARISON)
CHECK_CXX_SOURCE_COMPILES("enum class Test; enum class Test { A, B }; int main() { Test::A == Test::B; }" HAVE_ENUM_CLASS_FORWARD_DECLARATION)

INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES("endian.h" HAVE_ENDIAN_H)
CHECK_INCLUDE_FILES("sys/endian.h" HAVE_SYS_ENDIAN_H)
CHECK_INCLUDE_FILES("sys/ioctl.h" HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILES("unistd.h" HAVE_UNISTD_H)
CHECK_INCLUDE_FILES("syslog.h" HAVE_SYSLOG_H)

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

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.h")

add_custom_target(version.cpp
    sh ./version.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM
)
if(NOT "${CMAKE_CURRENT_SOURCE_DIR}/version.cpp")
    execute_process(
        COMMAND sh ./version.sh
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

set(SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/version.cpp"
)
set(HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/util.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/compat")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/config")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/config/configsections")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mc")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer/overlays")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer/image")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer/renderviews/isometric")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/renderer/renderviews/topdown")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thread")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thread/impl")

add_library(mapcraftercore SHARED ${SOURCE})
add_dependencies(mapcraftercore version.cpp)

target_link_libraries(mapcraftercore "${Boost_IOSTREAMS_LIBRARY}" "${Boost_SYSTEM_LIBRARY}" "${Boost_FILESYSTEM_LIBRARY}")
if(OPT_USE_BOOST_THREAD)
    target_link_libraries(mapcraftercore "${Boost_THREAD_LIBRARY}")
endif()

if(OPT_LINK_DEPS_STATICALLY)
    target_link_libraries(mapcraftercore libpng.a libjpeg.a)
else()
    target_link_libraries(mapcraftercore ${PNG_LIBRARIES})
    target_link_libraries(mapcraftercore ${JPEG_LIBRARIES})
    target_link_libraries(mapcraftercore ${CMAKE_THREAD_LIBS_INIT})
endif()

if(OPT_LINK_BOOST_STATICALLY)
    if(OPT_LINK_DEPS_STATICALLY)
        target_link_libraries(mapcraftercore libz.a)
    else()
        target_link_libraries(mapcraftercore ${ZLIB_LIBRARIES})
    endif()
endif()

install(TARGETS mapcraftercore DESTINATION lib)

if(OPT_INSTALL_HEADERS)
    foreach(HEADER ${HEADERS})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" HEADER "${HEADER}")
        string(REGEX MATCH "(.*)[/\\]" DIR "${HEADER}")
        install(FILES "${HEADER}" DESTINATION "include/mapcraftercore/${DIR}")
    endforeach(HEADER)
endif()
