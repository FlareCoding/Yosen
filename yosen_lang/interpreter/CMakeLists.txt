set(CMAKE_CURRENT_SOURCE_DIR interpreter)
set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    INTERPRETER_HEADERS

    ${cwd}/YosenInterpreter.h

    PARENT_SCOPE
)

set(
    INTERPRETER_SOURCES

    ${cwd}/YosenInterpreter.cpp

    PARENT_SCOPE
)