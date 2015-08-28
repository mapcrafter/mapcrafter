set(SOURCE
    ${SOURCE}
    "${CMAKE_CURRENT_SOURCE_DIR}/dithering.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/palette.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quantization.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/scaling.cpp"
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
    "${CMAKE_CURRENT_SOURCE_DIR}/dithering.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/palette.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/quantization.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/scaling.h"
    PARENT_SCOPE
)
