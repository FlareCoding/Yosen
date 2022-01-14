cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_SOURCE_DIR src/utils)

set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    UTILS_HEADERS

    ${cwd}/utils.h
    ${cwd}/json/json11.hpp

    PARENT_SCOPE
)

set(
    UTILS_SOURCES

    ${cwd}/utils.cpp
    ${cwd}/json/json11.cpp

    PARENT_SCOPE
)