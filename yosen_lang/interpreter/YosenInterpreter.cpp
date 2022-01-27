#include "YosenInterpreter.h"
#include <iostream>

namespace yosen::utils
{
    std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
    {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
    {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
    {
        return ltrim(rtrim(s, t), t);
    }
}

namespace yosen
{
    void YosenInterpreter::init()
	{
		// Initialize the current environment
		YosenEnvironment::init();

		m_env = &YosenEnvironment::get();

        // Register keyboard interrupt handler
        utils::set_keyboard_interrupt_handler([this]() {
            m_env->throw_exception(KeyboardInterruptException());
        });

        // Register the exception handler
        m_env->register_exception_handler([this](const YosenException& ex) {
            main_exception_handler(ex);
        });

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
		utils::log_colored(
			utils::ConsoleColor::Green,
			"[*] Yosen Interpreter Initialized\n"
		);
#endif
	}
	
	void YosenInterpreter::shutdown()
	{
        for (auto& stack_frame : m_allocated_stack_frames)
        {
            // Deallocated created resources off the stack frame
            deallocate_frame(stack_frame);

            // Completely destroy the last resources on the stack frame
            destroy_stack_frame(stack_frame);
        }

        // If the return register is not empty, deallocate the existing object
        if (registers[RegisterType::ReturnRegister] != nullptr)
        {
            free_object(registers[RegisterType::ReturnRegister]);
            registers[RegisterType::ReturnRegister] = nullptr;
        }

        // If the allocated object register is not empty, deallocate the existing object
        if (registers[RegisterType::AllocatedObjectRegister] != nullptr)
        {
            free_object(registers[RegisterType::AllocatedObjectRegister]);
            registers[RegisterType::AllocatedObjectRegister] = nullptr;
        }

		// Shutdown the environment
		m_env->shutdown();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
		utils::log_colored(
			utils::ConsoleColor::Green,
			"[*] Interpreter Shutdown, total objects left allocated is {%zi}, exiting...\n",
			__yosen_get_total_allocated_objects()
		);
#endif
	}

    void YosenInterpreter::main_exception_handler(const YosenException& ex)
    {
        utils::log_colored(
            utils::ConsoleColor::Yellow,
            "%s\n",
            ex.to_string().c_str()
        );

        shutdown();
        exit(1);
    }

    void YosenInterpreter::run_source(std::string& source)
    {
        StackFrame frame;
        bytecode_t bytecode;

        /* EXAMPLE CODE:
        func main() {
            var str = "Yosen";
            println(str);

            str = new std::TestStdClass();
            println(str);
            str.test_fn();
        }
        */

        // For testing purposes
        using namespace opcodes;

        frame.name = "main";

        frame.add_variable("null");
        frame.add_variable("str");

        frame.function_names = {
            "println",
            "test_fn"
        };
        frame.class_names = {
            "std::TestStdClass"
        };
        frame.constant_keys = {
            { "Yosen", 0 }
        };
        frame.constants = {
            { 0, allocate_object<YosenString>("Yosen") }
        };

        bytecode = {
            // var str = "Yosen";
            LOAD_CONST, 0,
            STORE,      1,

            // println(str);
            LOAD, 1,
            PUSH,
            CALL, 0, 0x00,

            // str = new std::TestStdClass();
            ALLOC_OBJECT,   0,
            REG_LOAD,       1,
            STORE,          1,

            // println(str);
            LOAD, 1,
            PUSH,
            CALL, 0, 0x00,

            // str.test_fn();
            LOAD, 1,
            CALL, 1, 0x01,

            // *Implicit* return YosenObject_Null
            LOAD, 0x00,
            RETURN
        };

        // Create an empty parameter stack to be used by the function for future functions
        parameter_stacks.push({});

        // Run the bytecode
        execute_bytecode(frame, bytecode);

        // Deallocated created resources off the stack frame
        deallocate_frame(frame);

        // Completely destroy the last resources on the stack frame
        destroy_stack_frame(frame);

        // If the return register is not empty, deallocate the existing object
        if (registers[RegisterType::ReturnRegister] != nullptr)
        {
            free_object(registers[RegisterType::ReturnRegister]);
            registers[RegisterType::ReturnRegister] = nullptr;
        }

        // If the allocated object register is not empty, deallocate the existing object
        if (registers[RegisterType::AllocatedObjectRegister] != nullptr)
        {
            free_object(registers[RegisterType::AllocatedObjectRegister]);
            registers[RegisterType::AllocatedObjectRegister] = nullptr;
        }
    }

    std::string YosenInterpreter::read_block_source(const std::string& header, const std::string& tab_space)
    {
        std::string result = header + "\n";
        char last_char = '\0';
        std::string next_tab_space = tab_space + "\t";

        while (last_char != '}')
        {
            printf("%s", tab_space.c_str());
            std::string input;
            std::getline(std::cin, input);
            utils::trim(input);

            last_char = input.back();

            if (last_char == '{')
                input = read_block_source(input, next_tab_space);

            result += input + "\n";
        }

        return result;
    }
	
	void YosenInterpreter::run_interactive_shell()
	{
        // Create an empty parameter stack to be used by the global function
        parameter_stacks.push({});

        StackFrame global_stack_frame;
        global_stack_frame.name = "__ys_global_stack_frame";

        // Register the stack frame
        m_allocated_stack_frames.push_back(global_stack_frame);

        while (true)
        {
            printf("$> ");

            std::string input;
            std::getline(std::cin, input);

            utils::trim(input);

            if (input.empty())
                continue;

            if (input == "exit")
                break;

            if (input.back() == '{')
            {
                input = read_block_source(input, "   ");
                printf("\n");
            }

            auto& stack_frame = m_allocated_stack_frames[0];

            auto bytecode = m_compiler.compile_single_statement(input, stack_frame);
            execute_bytecode(stack_frame, bytecode);
        }
	}

	void YosenInterpreter::deallocate_frame(StackFrame& stack_frame)
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
	
	void YosenInterpreter::destroy_stack_frame(StackFrame& stack_frame)
	{
		// Deallocate constants
		for (auto& [key, obj] : stack_frame.constants)
			if (obj) free_object(obj);

		stack_frame.constants.clear();

		// Deallocate variables
		for (auto& [key, obj] : stack_frame.vars)
			if (obj) free_object(obj);

		stack_frame.vars.clear();
		stack_frame.params.clear();
	}
	
	void YosenInterpreter::execute_bytecode(StackFrame& stack_frame, bytecode_t& bytecode)
	{
        for (size_t i = 0; i < bytecode.size();)
        {
            auto incr = execute_instruction(stack_frame, &bytecode[i]);
            i += incr;
        }
	}

	size_t YosenInterpreter::execute_instruction(StackFrame& stack_frame, opcodes::opcode_t* ops)
	{
        size_t opcount = 1;
        auto op = ops[0];

        switch (op)
        {
        case opcodes::LOAD:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &stack_frame.vars[operand];
            break;
        }
        case opcodes::LOAD_CONST:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &stack_frame.constants[operand];
            break;
        }
        case opcodes::LOAD_PARAM:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &stack_frame.params[operand].second;
            break;
        }
        case opcodes::STORE:
        {
            // Operand is the key of the variable being modified in the vars map
            auto operand = ops[1];
            opcount = 2;

            // Retrieve the original object
            auto original_object = stack_frame.vars[operand];

            // Assign the new object
            stack_frame.vars[operand] = (*LLOref)->clone();

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
        case opcodes::IMPORT_LIB:
        {
            // Operand is the key of the register into which the loaded object has to be copied
            auto operand = ops[1];
            opcount = 2;

            auto lib_name = stack_frame.imported_library_names[operand];
            m_env->load_yosen_module(lib_name);

            break;
        }
        case opcodes::ALLOC_OBJECT:
        {
            // Operand is the key of the register into which the loaded object has to be copied
            auto operand = ops[1];
            opcount = 2;

            auto class_name = stack_frame.class_names[operand];

            if (!m_env->is_class_name(class_name))
            {
                auto ex_reason = "Class name \"" + class_name + "\" not found";
                m_env->throw_exception(RuntimeException(ex_reason));
            }

            // Process parameters
            auto& parameter_stack = parameter_stacks.top();
            auto param_count = parameter_stack.size();
            YosenTuple* param_pack = allocate_object<YosenTuple>(parameter_stack);

            // Instantiate the class
            auto instance = m_env->construct_class_instance(class_name, param_pack);

            // Deallocate the parameter pack object
            free_object(param_pack);

            // Clear the parameter stack
            parameter_stack.clear();

            // Retrieve the original object
            auto original_object = registers[RegisterType::AllocatedObjectRegister];

            // Copy the object into the register
            registers[RegisterType::AllocatedObjectRegister] = instance;

            // Free the original object
            if (original_object)
                free_object(original_object);

            break;
        }
        case opcodes::CALL:
        {
            auto fn_index = ops[1];
            auto caller_var = ops[2];
            auto fn_name = stack_frame.function_names[fn_index];
            opcount = 3;

            // Process parameters
            auto& parameter_stack = parameter_stacks.top();
            auto param_count = parameter_stack.size();
            YosenTuple* param_pack = allocate_object<YosenTuple>(parameter_stack);

            YosenObject* return_val = nullptr;

            if (caller_var)
            {
                // Member function
                auto caller_object = *LLOref;

                // Check if it's a native member function
                if (caller_object->has_member_native_function(fn_name))
                {
                    return_val = caller_object->call_member_native_function(fn_name, param_pack);

                    // If the return register is not empty, deallocate the existing object
                    if (registers[RegisterType::ReturnRegister] != nullptr)
                        free_object(registers[RegisterType::ReturnRegister]);

                    // Move the return value into the return register
                    registers[RegisterType::ReturnRegister] = return_val;
                }
                else
                {
                    auto ex_reason = "Member function \"" + fn_name + "\" not found";
                    m_env->throw_exception(RuntimeException(ex_reason));
                }
            }
            else
            {
                // Check for a user-defined function
                //if (compiler.is_user_defined_function(fn_name))
                //{
                //    auto& fn = compiler.get_user_defined_function(fn_name);

                //    // Setup function's parameters from
                //    // the current function's parameter stack.
                //    if (fn.stack_frame.params.size())
                //    {
                //        if (fn.stack_frame.params.size() != param_count)
                //        {
                //            throw "Incorrect number of parameters passed";
                //        }

                //        for (size_t i = 0; i < param_count; ++i)
                //            fn.stack_frame.params[i].second = param_pack->items[param_count - 1 - i]->clone();
                //    }

                //    // Create an empty parameter stack to be used by the function for future functions
                //    parameter_stacks.push({});

                //    // Run the user function (return register will automatically be updated
                //    execute_function(fn.stack_frame, fn.bytecode);

                //    // Deallocate the user function's stack frame
                //    deallocate_frame(fn.stack_frame);
                //}

                // Check for a native function
                if (m_env->is_static_native_function(fn_name))
                {
                    auto fn = m_env->get_static_native_function(fn_name);
                    return_val = fn(param_pack);

                    // If the return register is not empty, deallocate the existing object
                    if (registers[RegisterType::ReturnRegister] != nullptr)
                        free_object(registers[RegisterType::ReturnRegister]);

                    // Move the return value into the return register
                    registers[RegisterType::ReturnRegister] = return_val;
                }
                else
                {
                    auto ex_reason = "Static function \"" + fn_name + "\" not found";
                    m_env->throw_exception(RuntimeException(ex_reason));
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
