#include "Interpreter.h"
#include "YosenStandardLibrary.h"
#include "../utils/utils.h"

namespace yosen
{
    void Interpreter::init()
    {
        YosenObject_Null = allocate_object<YosenObject>();
        YosenObject_Null->string_repr = "null";

        YosenStandardLibrary::load_functions(this);

        // It will get freed automatically when the
        // entry point's stack frame gets destroyed.
        executable_path_obj = allocate_object<YosenString>(utils::get_current_executable_path());
    }

    void Interpreter::shutdown()
    {
        compiler.shutdown();

        free_object(YosenObject_Null);
    }

    void Interpreter::register_native_runtime_function(const std::string& name, yosen_function_t fn)
    {
        native_runtime_functions[name] = fn;
    }

    void Interpreter::run(std::string& source)
    {
        // Compile the entire source
        compiler.compile_source(source);

        // For testing
        utils::set_terminal_color_green();

        std::string entry_point_name = "main";

        if (compiler.is_user_defined_function(entry_point_name))
        {
            // Get the entry point (main function)
            auto& entry_point = compiler.get_user_defined_function(entry_point_name);

            // Setup the path as the first command-line parameter
            entry_point.stack_frame.params[0].second = executable_path_obj;

            // Create an empty parameter stack to be used by the function for future functions
            parameter_stacks.push({});

            // Execute the bytecode of the function
            execute_function(entry_point.stack_frame, entry_point.bytecode);

            // Free the memory allocated for the stack frame
            deallocate_frame(entry_point.stack_frame);

            // If the return register is not empty, deallocate the existing object
            if (registers[RegisterType::ReturnRegister] != nullptr)
            {
                free_object(registers[RegisterType::ReturnRegister]);
                registers[RegisterType::ReturnRegister] = nullptr;
            }
        }
        else
        {
            printf("Error: entry point not found\n");
        }

        utils::set_terminal_color_default();
    }

    void Interpreter::execute_function(StackFrame& frame, bytecode_t& bytecode)
	{
		for (size_t i = 0; i < bytecode.size();)
		{
			auto incr = execute_instruction(frame, &bytecode[i]);
			i += incr;
		}
	}

    void Interpreter::deallocate_frame(StackFrame& stack_frame)
    {
        // Deallocate incoming parameters
        for (auto& [name, obj] : stack_frame.params)
            if (obj) free_object(obj);

        // Deallocate vars
        for (auto& [key, obj] : stack_frame.vars)
        {
            if (obj) free_object(obj);
            
            stack_frame.vars[key] = YosenObject_Null->clone();
        }

        // Deallocate pushed variables
        for (auto& obj : parameter_stacks.top())
            if (obj) free_object(obj);

        parameter_stacks.pop();
    }
	
	size_t Interpreter::execute_instruction(StackFrame& frame, opcodes::opcode_t* ops)
	{
        size_t opcount = 1;
        auto op = ops[0];

        switch (op)
        {
        case opcodes::LOAD:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &frame.vars[operand];
            break;
        }
        case opcodes::LOAD_CONST:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &frame.constants[operand];
            break;
        }
        case opcodes::LOAD_PARAM:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &frame.params[operand].second;
            break;
        }
        case opcodes::STORE:
        {
            // Operand is the key of the variable being modified in the vars map
            auto operand = ops[1];
            opcount = 2;

            // Retrieve the original object
            auto original_object = frame.vars[operand];

            // Assign the new object
            frame.vars[operand] = (*LLOref)->clone();

            // Free the original object
            free_object(original_object);

            break;
        }
        case opcodes::REG_LOAD:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &registers[static_cast<RegisterType>(operand)];
            break;
        }
        case opcodes::REG_STORE:
        {
            // Operand is the key of the register into which the loaded object has to be copied
            auto operand = ops[1];
            opcount = 2;

            // Retrieve the original object
            auto original_object = registers[static_cast<RegisterType>(operand)];

            // Copy the object into the correct register
            registers[static_cast<RegisterType>(operand)] = (*LLOref)->clone();

            // Free the original object
            if (original_object)
                free_object(original_object);

            break;
        }
        case opcodes::PUSH:
        {
            // Copy the last loaded object onto the parameter stack
            parameter_stacks.top().push_back((*LLOref)->clone());
            break;
        }
        case opcodes::CALL:
        {
            auto fn_index = ops[1];
            auto owner_var = ops[2];
            auto fn_name = frame.function_names[fn_index];
            opcount = 3;

            // Process parameters
            auto& parameter_stack = parameter_stacks.top();
            auto param_count = parameter_stack.size();
            YosenTuple* param_pack = allocate_object<YosenTuple>(parameter_stack);

            YosenObject* return_val = nullptr;

            if (owner_var)
            {
                // Member function
                auto caller_object = frame.vars[owner_var];

                if (caller_object->has_member_function(fn_name))
                {
                    auto fn = caller_object->get_member_function(fn_name);
                    return_val = fn(param_pack);

                    // If the return register is not empty, deallocate the existing object
                    if (registers[RegisterType::ReturnRegister] != nullptr)
                        free_object(registers[RegisterType::ReturnRegister]);

                    // Move the return value into the return register
                    registers[RegisterType::ReturnRegister] = return_val;
                }
                else
                {
                    printf("Exception> Object has no member function \"%s\"\n", fn_name.c_str());
                }
            }
            else
            {
                // Check for a user-defined function
                if (compiler.is_user_defined_function(fn_name))
                {
                    auto& fn = compiler.get_user_defined_function(fn_name);

                    // Setup function's parameters from
                    // the current function's parameter stack.
                    if (fn.stack_frame.params.size())
                    {
                        if (fn.stack_frame.params.size() != param_count)
                        {
                            throw "Incorrect number of parameters passed";
                        }

                        for (size_t i = 0; i < param_count; ++i)
                            fn.stack_frame.params[i].second = param_pack->items[param_count - 1 - i]->clone();
                    }

                    // Create an empty parameter stack to be used by the function for future functions
                    parameter_stacks.push({});

                    // Run the user function (return register will automatically be updated
                    execute_function(fn.stack_frame, fn.bytecode);

                    // Deallocate the user function's stack frame
                    deallocate_frame(fn.stack_frame);
                }

                // Check for a native function
                else if (native_runtime_functions.find(fn_name) != native_runtime_functions.end())
                {
                    auto fn = native_runtime_functions.at(fn_name);
                    return_val = fn(param_pack);

                    // If the return register is not empty, deallocate the existing object
                    if (registers[RegisterType::ReturnRegister] != nullptr)
                        free_object(registers[RegisterType::ReturnRegister]);

                    // Move the return value into the return register
                    registers[RegisterType::ReturnRegister] = return_val;
                }
                else
                {
                    printf("Function not found: \"%s\"\n", fn_name.c_str());
                }
            }

            // Deallocate the parameter pack object
            free_object(param_pack);

            // Clear the parameter stack
            parameter_stack.clear();

            break;
        }
        default: break;
        }

        return opcount;
	}
}
