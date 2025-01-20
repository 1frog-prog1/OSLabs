set(INCROOT "${PROJECT_SOURCE_DIR}/include/common")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src/common")

set(SRC
    ${INCROOT}/common.h
    ${INCROOT}/exception.h
    ${INCROOT}/file.h
    ${INCROOT}/format.h
    ${INCROOT}/log.h

    ${SRCROOT}/exception.cpp
    ${SRCROOT}/file.cpp
    ${SRCROOT}/log.cpp
)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building common library...")
    add_library(common ${SRC})
    target_include_directories(common PUBLIC ${INCROOT})
    set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Common library not built")
endif()
