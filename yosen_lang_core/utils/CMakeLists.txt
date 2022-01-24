set(CMAKE_CURRENT_SOURCE_DIR utils)
set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    UTILS_HEADERS

    ${cwd}/utils.h

    PARENT_SCOPE
)

set(
    UTILS_SOURCES

    ${cwd}/utils.cpp

    PARENT_SCOPE
)