set(CMAKE_CURRENT_SOURCE_DIR interpreter)
set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    INTERPRETER_HEADERS

    ${cwd}/YosenInterpreter.h
    ${cwd}/YosenCompiler.h
    ${cwd}/opcodes.h

    PARENT_SCOPE
)

set(
    INTERPRETER_SOURCES

    ${cwd}/YosenInterpreter.cpp
    ${cwd}/YosenCompiler.cpp

    PARENT_SCOPE
)