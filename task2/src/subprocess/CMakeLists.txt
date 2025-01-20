set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${INCROOT}/subprocess/subprocess.h

    ${SRCROOT}/subprocess/subprocess.cpp

)

add_library(subprocess ${SRC})
target_include_directories(subprocess PUBLIC ${INCROOT})
set_target_properties(subprocess PROPERTIES LINKER_LANGUAGE CXX)
