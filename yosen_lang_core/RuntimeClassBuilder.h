#pragma once
#include <primitives/YosenObject.h>

namespace yosen
{
    class RuntimeClassBuilder
    {
    public:
        std::string class_name;

        // Map of all runtime functions
        std::map<std::string, ys_runtime_function_t> runtime_functions;

        // Creates and registers a class builder function in the environment
        YOSENAPI void create_runtime_class();
    };
}
