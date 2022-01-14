cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_SOURCE_DIR src/interpreter)

set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    INTERPRETER_HEADERS

    ${cwd}/Interpreter.h
    ${cwd}/Compiler.h
    ${cwd}/opcodes.h
    ${cwd}/StackFrame.h
    ${cwd}/YosenStandardLibrary.h

    PARENT_SCOPE
)

set(
    INTERPRETER_SOURCES

    ${cwd}/Interpreter.cpp
    ${cwd}/Compiler.cpp
    ${cwd}/YosenStandardLibrary.cpp

    PARENT_SCOPE
)