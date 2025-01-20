add_subdirectory(subprocess)
add_subdirectory(cache)

set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${INCROOT}/common/public.h

    ${SRCROOT}/main.cpp
)

add_executable(main ${SRC})
target_include_directories(main PUBLIC ${INCROOT})
target_link_libraries(main PUBLIC subprocess cache)
set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
