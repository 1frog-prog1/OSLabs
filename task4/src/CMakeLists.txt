add_subdirectory(serial)
add_subdirectory(cache)

set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${INCROOT}/common/public.h
    ${INCROOT}/temperature_storage.h

    ${SRCROOT}/temperature_storage.cpp
    ${SRCROOT}/main.cpp
)

add_executable(main ${SRC})
target_include_directories(main PUBLIC ${INCROOT})
target_link_libraries(main PUBLIC serial cache)
set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
