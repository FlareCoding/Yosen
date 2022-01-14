cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_SOURCE_DIR src)

set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(interpreter)
add_subdirectory(parser)
add_subdirectory(native_objects)
add_subdirectory(utils)

set(
    YOSENLANG_HEADERS

    ${INTERPRETER_HEADERS}
    ${PARSER_HEADERS}
    ${NATIVE_OBJECTS_HEADERS}
    ${UTILS_HEADERS}

    PARENT_SCOPE
)

set(
    YOSENLANG_SOURCES

    ${INTERPRETER_SOURCES}
    ${PARSER_SOURCES}
    ${NATIVE_OBJECTS_SOURCES}
    ${UTILS_SOURCES}

    ${cwd}/main.cpp

    PARENT_SCOPE
)
