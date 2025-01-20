set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${INCROOT}/serial/serial.h

    ${SRCROOT}/serial/serial.cpp

)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building serial library...")
    add_library(serial ${SRC})
    target_include_directories(serial PUBLIC ${INCROOT})
    set_target_properties(serial PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Serial library not built")
endif()
