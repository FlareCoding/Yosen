cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_SOURCE_DIR src/native_objects)

set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    NATIVE_OBJECTS_HEADERS

    ${cwd}/YosenObject.h
    ${cwd}/YosenInteger.h
    ${cwd}/YosenString.h
    ${cwd}/YosenTuple.h

    PARENT_SCOPE
)

set(
    NATIVE_OBJECTS_SOURCES

    ${cwd}/YosenObject.cpp
    ${cwd}/YosenInteger.cpp
    ${cwd}/YosenString.cpp
    ${cwd}/YosenTuple.cpp

    PARENT_SCOPE
)