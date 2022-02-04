#include "StackFrame.h"

namespace yosen
{
    void StackFrame::add_variable(const std::string& name, YosenObject* obj)
    {
        // Get the next available variable key
        uint32_t var_key = (uint32_t)var_keys.size();

        // Create a variable key entry for the variable name
        var_keys.insert({ name, var_key });

        // Create an entry in the variables map
        vars.insert({ var_key, obj ? obj : YosenObject_Null->clone() });
    }
    
    bool StackFrame::has_variable(const std::string& name) const
    {
        return var_keys.find(name) == var_keys.end();
    }
    
    void StackFrame::add_function_name(const std::string& name)
    {
        // Create a variable key entry for the variable name
        function_names.push_back(name);
    }
    
    size_t StackFrame::get_function_index(const std::string& name)
    {
        auto it = std::find(
            function_names.begin(),
            function_names.end(),
            name
        );

        if (it != function_names.end())
            return it - function_names.begin();

        return -1;
    }
    
    size_t StackFrame::get_class_name_index(const std::string& name)
    {
        auto it = std::find(
            class_names.begin(),
            class_names.end(),
            name
        );

        if (it != class_names.end())
            return it - class_names.begin();

        return -1;
    }
    
    size_t StackFrame::get_imported_lib_name_index(const std::string& name)
    {
        auto it = std::find(
            imported_library_names.begin(),
            imported_library_names.end(),
            name
        );

        if (it != imported_library_names.end())
            return it - imported_library_names.begin();

        return -1;
    }

    std::shared_ptr<StackFrame> StackFrame::clone()
    {
        auto new_frame = allocate_stack_frame();

        // Copy the name
        new_frame->name = this->name;

        // Copy parameters
        for (auto& param : this->params)
            new_frame->params.push_back({ param.first, param.second ? param.second->clone() : nullptr });

        // Copy var keys
        new_frame->var_keys = this->var_keys;

        // Copy variables
        for (auto& var : this->vars)
            new_frame->vars[var.first] = var.second->clone();

        // Copy constant keys
        new_frame->constant_keys = this->constant_keys;

        // Copy constants
        for (auto& constant : this->constants)
            new_frame->constants[constant.first] = constant.second->clone();

        // Copy function names
        new_frame->function_names = this->function_names;

        // Copy class names
        new_frame->class_names = this->class_names;

        // Copy imported library names
        new_frame->imported_library_names = this->imported_library_names;

        return new_frame;
    }
    
    StackFramePtr allocate_stack_frame()
    {
        return std::make_shared<StackFrame>();
    }
}
