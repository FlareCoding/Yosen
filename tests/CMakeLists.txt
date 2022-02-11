set(CMAKE_CURRENT_SOURCE_DIR tests)

add_custom_target(tests
    SOURCES
        main.ys
        conditionals.ys
        loops.ys
        primitives.ys
        fibb.ys
        classes.ys
        global_vars.ys
        multi_file_test/logger.ys
        multi_file_test/util.ys
        multi_file_test/main.ys
)