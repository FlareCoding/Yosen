set(CMAKE_CURRENT_SOURCE_DIR yosen_lang)

set(TARGET_NAME yosen_lang)

add_subdirectory(parser)
add_subdirectory(interpreter)

add_executable(
    ${TARGET_NAME}

    ${PARSER_HEADERS}
    ${PARSER_SOURCES}
    
    ${INTERPRETER_HEADERS}
    ${INTERPRETER_SOURCES}

    main.cpp
)

target_link_libraries(${TARGET_NAME} yosen_lang_core)
target_include_directories(${TARGET_NAME} PUBLIC .)