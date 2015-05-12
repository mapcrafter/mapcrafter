set(SOURCE
    ${SOURCE}
    "${CMAKE_CURRENT_SOURCE_DIR}/dither.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/palette.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quantization.cpp"
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
    "${CMAKE_CURRENT_SOURCE_DIR}/dither.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/palette.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/quantization.h"
    PARENT_SCOPE
)
