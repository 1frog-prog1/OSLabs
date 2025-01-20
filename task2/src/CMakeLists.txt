add_subdirectory(subprocess)

set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${SRCROOT}/main.cpp
)

add_executable(main ${SRC})
target_include_directories(main PUBLIC ${INCROOT})
target_link_libraries(main PUBLIC subprocess)
set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
