set(CMAKE_CURRENT_SOURCE_DIR yosen_lang_core)

add_subdirectory(primitives)
add_subdirectory(utils)

set(TARGET_NAME yosen_lang_core)
add_library(
    ${TARGET_NAME} SHARED
    
    ${PRIMITIVES_HEADERS}
    ${PRIMITIVES_SOURCES}

    ${UTILS_HEADERS}
    ${UTILS_SOURCES}

    YosenCore.h
    YosenEnvironment.h
    YosenEnvironment.cpp
    YosenException.h
    YosenException.cpp
    StackFrame.h
    StackFrame.cpp
    RuntimeClassBuilder.h
    RuntimeClassBuilder.cpp
)

target_compile_definitions(${TARGET_NAME} PRIVATE YOSEN_BUILD)

# Linking against DL library on Linux only
if (LINUX)
    target_link_libraries(${TARGET_NAME} dl)
endif(LINUX)
