cmake_minimum_required(VERSION 3.0)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
endif()

add_definitions(-DUNICODE -D_UNICODE)

project(YosenLang)

include_directories("src")
add_subdirectory(src)

add_executable(
    YosenLang
    
    ${YOSENLANG_HEADERS}
    ${YOSENLANG_SOURCES}
)

add_custom_target(Tests
    SOURCES
        tests/main.ys
)
