cmake_minimum_required(VERSION 3.5)

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

set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})

add_subdirectory(yosen_lang)
add_subdirectory(yosen_lang_core)
add_subdirectory(yosen_lang_std)

add_custom_target(Tests
    SOURCES
        tests/main.ys
)
