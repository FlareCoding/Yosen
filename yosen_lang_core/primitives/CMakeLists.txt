set(CMAKE_CURRENT_SOURCE_DIR primitives)
set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    PRIMITIVES_HEADERS

    ${cwd}/YosenObject.h
    ${cwd}/YosenBoolean.h
    ${cwd}/YosenInteger.h
    ${cwd}/YosenFloat.h
    ${cwd}/YosenString.h
    ${cwd}/YosenTuple.h
    ${cwd}/YosenList.h
    ${cwd}/primitives.h

    PARENT_SCOPE
)

set(
    PRIMITIVES_SOURCES

    ${cwd}/YosenObject.cpp
    ${cwd}/YosenBoolean.cpp
    ${cwd}/YosenInteger.cpp
    ${cwd}/YosenFloat.cpp
    ${cwd}/YosenString.cpp
    ${cwd}/YosenTuple.cpp
    ${cwd}/YosenList.cpp

    PARENT_SCOPE
)