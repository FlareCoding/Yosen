#pragma once
#include <primitives/YosenObject.h>

namespace yosen
{
    class RuntimeClassBuilder
    {
    public:
        // Name of the class
        std::string class_name;

        // Map of all runtime functions
        std::map<std::string, ys_runtime_function_t> runtime_functions;

        // List of all member variables with default values
        std::map<std::string, YosenObject*> member_variables;

        // Creates and registers a class builder function in the environment
        YOSENAPI void create_runtime_class();
    };
}
