set(CMAKE_CURRENT_SOURCE_DIR yosen_lang_std)

set(TARGET_NAME stdio)
add_library(
    ${TARGET_NAME} SHARED

    io_module_init.cpp

    yosen_std_io.h
    yosen_std_io.cpp
)
target_include_directories(${TARGET_NAME} PUBLIC yosen_lang_core)
target_link_libraries(${TARGET_NAME} yosen_lang_core)
