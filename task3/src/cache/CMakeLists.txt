set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${INCROOT}/cache/file.h
    ${INCROOT}/cache/cache.h

    ${SRCROOT}/cache/file.cpp
    ${SRCROOT}/cache/cache.cpp
)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building cache library...")
    add_library(cache ${SRC})
    target_include_directories(cache PUBLIC ${INCROOT})
    set_target_properties(cache PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Cache library not built")
endif()
