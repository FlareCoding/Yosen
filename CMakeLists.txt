cmake_minimum_required(VERSION 3.0)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
endif()

add_definitions(-DUNICODE -D_UNICODE)

project(yosen_lang)
include_directories(yosen_lang_core)

set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})

message("----------------------------------------------")
message("-- CMake  ${CMAKE_VERSION}")
message("-- Build  ${CMAKE_SYSTEM_NAME}")
message("-- OutDir ${OUTPUT_PATH}")
message("")
message("")

message("-- Building the language core system")
add_subdirectory(yosen_lang_core)

message("-- Building the standard library")
add_subdirectory(yosen_lang_std)

message("-- Building the interpreter")
add_subdirectory(yosen_lang)

message("-- Creating tests")
add_subdirectory(tests)

message("")
message("----------------------------------------------")
