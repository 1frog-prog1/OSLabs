set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${INCROOT}/func_lib/func.h

    ${SRCROOT}/func_lib/func.cpp

)

add_library(func_lib ${SRC})
target_include_directories(func_lib PUBLIC ${INCROOT})
set_target_properties(func_lib PROPERTIES LINKER_LANGUAGE CXX)
