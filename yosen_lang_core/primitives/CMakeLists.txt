set(CMAKE_CURRENT_SOURCE_DIR primitives)
set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    PRIMITIVES_HEADERS

    ${cwd}/YosenObject.h
    ${cwd}/YosenInteger.h
    ${cwd}/YosenString.h
    ${cwd}/YosenTuple.h

    PARENT_SCOPE
)

set(
    PRIMITIVES_SOURCES

    ${cwd}/YosenObject.cpp
    ${cwd}/YosenInteger.cpp
    ${cwd}/YosenString.cpp
    ${cwd}/YosenTuple.cpp

    PARENT_SCOPE
)