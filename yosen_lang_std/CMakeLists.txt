set(CMAKE_CURRENT_SOURCE_DIR yosen_lang_std)

set(TARGET_NAME yosen_std_math)
add_library(
    ${TARGET_NAME} SHARED

    module_init.cpp

    YosenStandardLibrary.h
    YosenStandardLibrary.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC yosen_lang_core)
target_link_libraries(${TARGET_NAME} yosen_lang_core)