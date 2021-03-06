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
        // Destroy compiler resources
        m_compiler.shutdown();

        for (auto& stack_frame : m_allocated_stack_frames)
        {
            // Deallocated created resources off the stack frame
            deallocate_stack_frame(stack_frame);

            // Completely destroy the last resources on the stack frame
            destroy_stack_frame(stack_frame);
        }

        // Destroy all objects on the operations stack
        for (auto& obj : m_operation_stack_objects)
            free_object(obj);

        // If the return register is not empty, deallocate the existing object
        if (m_registers[RegisterType::ReturnRegister] != nullptr)
        {
            free_object(m_registers[RegisterType::ReturnRegister]);
            m_registers[RegisterType::ReturnRegister] = nullptr;
        }

        // If the allocated object register is not empty, deallocate the existing object
        if (m_registers[RegisterType::AllocatedObjectRegister] != nullptr)
        {
            free_object(m_registers[RegisterType::AllocatedObjectRegister]);
            m_registers[RegisterType::AllocatedObjectRegister] = nullptr;
        }

        // If the return register is not empty, deallocate the existing object
        if (m_registers[RegisterType::TemporaryObjectRegister] != nullptr)
        {
            free_object(m_registers[RegisterType::TemporaryObjectRegister]);
            m_registers[RegisterType::TemporaryObjectRegister] = nullptr;
        }

        // Destroy the entry point argument object if it was used
        if (m_entry_point_args)
            free_object(m_entry_point_args);

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
        trace_call_stack();

        utils::log_colored(
            utils::ConsoleColor::Yellow,
            "%s\n",
            ex.to_string().c_str()
        );

        if (m_interactive_mode)
        {
            m_interactive_shell_exception_occured = true;
            return;
        }

        shutdown();
        exit(1);
    }

    void YosenInterpreter::trace_call_stack()
    {
        utils::log_colored(
            utils::ConsoleColor::Yellow,
            "Callstack:\n"
        );
        
        for (auto& name : m_call_stack)
        {
            utils::log_colored(
                utils::ConsoleColor::Yellow,
                "<>\t%s\n",
                name.c_str()
            );
        }
    }

    void YosenInterpreter::run_source(std::string& source, const std::vector<std::string>& cmd_arguments)
    {
        // Compile the source code
        auto source_directory = std::filesystem::path(cmd_arguments.at(0)).parent_path().string();
        auto program_source = m_compiler.compile_source(source, source_directory);

        // Create an empty parameter stack to be used by the function for future functions
        m_parameter_stacks.push({});

        // Register all the functions
        for (auto& fn : program_source.runtime_functions)
        {
            auto& stack_frame = fn.first;
            m_allocated_stack_frames.push_back(stack_frame);
            m_env->register_static_runtime_function(stack_frame->name, fn);
        }

        // Get the entry point
        std::string entry_point_name = "main";

        if (!m_env->is_static_runtime_function(entry_point_name))
        {
            m_env->throw_exception(
                RuntimeException("Could not find entry point \"" + entry_point_name + "\"")
            );
        }

        // Get the entry point function stack frame
        auto [stack_frame, bytecode] = m_env->get_static_runtime_function(entry_point_name);

        // Prepare the args parameter value
        if (stack_frame->params.size())
        {
            std::vector<YosenObject*> args;

            for (auto& arg : cmd_arguments)
                args.push_back(allocate_object<YosenString>(arg));

            stack_frame->params[0].second = allocate_object<YosenList>(args);
        }

        // Execute the entry point function
        execute_bytecode(stack_frame, bytecode);
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
        // Tell the interpreter that it is
        // now running in the interactive console mode.
        m_interactive_mode = true;

        // Create an empty parameter stack to be used by the global function
        m_parameter_stacks.push({});

        StackFramePtr global_stack_frame = allocate_stack_frame();
        global_stack_frame->name = "__ys_global_stack_frame";

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

            auto& stack_frame = global_stack_frame;

            auto bytecode = m_compiler.compile_single_statement(input, stack_frame);
            execute_bytecode(stack_frame, bytecode);
        }
	}

	void YosenInterpreter::deallocate_stack_frame(StackFramePtr stack_frame)
	{
		// Deallocate incoming parameters
        for (auto& [name, obj] : stack_frame->params)
        {
            if (obj)
                free_object(obj);

            obj = nullptr;
        }

		// Deallocate vars
		for (auto& [key, obj] : stack_frame->vars)
		{
			if (obj) free_object(obj);

			stack_frame->vars[key] = YosenObject_Null->clone();
		}

		// Deallocate pushed variables
		for (auto& obj : m_parameter_stacks.top())
			if (obj) free_object(obj);

        // Deallocate disposed objects
        for (auto& obj : stack_frame->disposed_objects)
            free_object(obj);

        stack_frame->disposed_objects.clear();
	}
	
	void YosenInterpreter::destroy_stack_frame(StackFramePtr stack_frame)
	{
		// Deallocate constants
		for (auto& [key, obj] : stack_frame->constants)
			if (obj) free_object(obj);

		stack_frame->constants.clear();

		// Deallocate variables
		for (auto& [key, obj] : stack_frame->vars)
			if (obj) free_object(obj);

		stack_frame->vars.clear();
		stack_frame->params.clear();
	}
	
	void YosenInterpreter::execute_bytecode(StackFramePtr stack_frame, bytecode_t& bytecode)
	{
        for (size_t i = 0; i < bytecode.size();)
        {
            // If an exception occurs in the interactive console mode,
            // then the shell should abandon processing the current statement.
            if (m_interactive_mode &&
                m_interactive_shell_exception_occured)
            {
                m_interactive_shell_exception_occured = false;
                break;
            }

            auto incr = execute_instruction(stack_frame, &bytecode[i], i, bytecode.size());
            i += incr;
        }
	}

	size_t YosenInterpreter::execute_instruction(StackFramePtr stack_frame, opcodes::opcode_t* ops, size_t& current_instruction, size_t instruction_count)
	{
        size_t opcount = 1;
        auto op = ops[0];

        switch (op)
        {
        case opcodes::LOAD:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &stack_frame->vars[operand];
            break;
        }
        case opcodes::LOAD_CONST:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &stack_frame->constants[operand];
            break;
        }
        case opcodes::LOAD_PARAM:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &stack_frame->params[operand].second;
            break;
        }
        case opcodes::STORE:
        {
            // Operand is the key of the variable being modified in the vars map
            auto operand = ops[1];
            opcount = 2;

            // Retrieve the original object
            auto original_object = stack_frame->vars[operand];

            // Assign the new object
            stack_frame->vars[operand] = (*LLOref)->clone();

            // Free the original object
            free_object(original_object);

            break;
        }
        case opcodes::LOAD_MEMBER:
        {
            // Operand is the index of the member variable name
            auto operand = ops[1];
            opcount = 2;

            // Get member variable name
            auto var_name = stack_frame->member_variable_names[operand];

            // Get the caller object
            auto caller_obj = m_operation_stack_objects.back();

            if (!caller_obj->has_member_variable(var_name))
            {
                auto ex_reason = "Member variable \"" + var_name + "\" not found for class \"" + caller_obj->runtime_name() + "\"";
                m_env->throw_exception(RuntimeException(ex_reason));
                return 0;
            }

            // Get the member variable object
            auto member_var = caller_obj->get_member_variable(var_name);

            // Store the object in a temporary object register and load it into the LLOref.
            // Retrieve the original object.
            auto original_object = m_registers[RegisterType::TemporaryObjectRegister];

            // Copy the object into the correct register by reference
            m_registers[RegisterType::TemporaryObjectRegister] = allocate_object<YosenReference>(member_var);

            // Free the original object
            if (original_object)
                free_object(original_object);

            // Load the member object into LLOref
            LLOref = &static_cast<YosenReference*>(m_registers[RegisterType::TemporaryObjectRegister])->obj;

            break;
        }
        case opcodes::STORE_MEMBER:
        {
            // Operand is the index of the member variable name
            auto operand = ops[1];
            opcount = 2;

            // Get member variable name
            auto var_name = stack_frame->member_variable_names[operand];

            // Get the caller object
            auto caller_obj = m_operation_stack_objects.back();

            if (!caller_obj->has_member_variable(var_name))
            {
                auto ex_reason = "Member variable \"" + var_name + "\" not found for class \"" + caller_obj->runtime_name() + "\"";
                m_env->throw_exception(RuntimeException(ex_reason));
                return 0;
            }

            // Retrieve the original object
            auto original_object = caller_obj->get_member_variable(var_name);

            // Set the member variable object
            caller_obj->set_member_variable(var_name, (*LLOref)->clone());

            // Free the original object
            free_object(original_object);

            break;
        }
        case opcodes::LOAD_GLOBAL:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &m_env->get_global_variable(operand);
            break;
        }
        case opcodes::STORE_GLOBAL:
        {
            // Operand is the key of the variable being modified in the vars map
            auto operand = ops[1];
            opcount = 2;

            // Retrieve the original object
            auto original_object = m_env->get_global_variable(operand);

            // Assign the new object
            m_env->set_global_variable(operand, (*LLOref)->clone());

            // Free the original object
            free_object(original_object);

            break;
        }
        case opcodes::REG_LOAD:
        {
            auto operand = ops[1];
            opcount = 2;

            LLOref = &m_registers[static_cast<RegisterType>(operand)];
            break;
        }
        case opcodes::REG_STORE:
        {
            // Operand is the key of the register into which the loaded object has to be copied
            auto operand = ops[1];
            opcount = 2;

            // Retrieve the original object
            auto original_object = m_registers[static_cast<RegisterType>(operand)];

            // Copy the object into the correct register
            m_registers[static_cast<RegisterType>(operand)] = (*LLOref)->clone();

            // Free the original object
            if (original_object)
                free_object(original_object);

            break;
        }
        case opcodes::PUSH:
        {
            // Copy the last loaded object onto the parameter stack
            m_parameter_stacks.top().push_back((*LLOref)->clone());
            break;
        }
        case opcodes::POP:
        {
            // Free the object before popping
            auto& obj = m_parameter_stacks.top().back();
            free_object(obj);

            // Pop the last object from the parameter stack
            m_parameter_stacks.top().pop_back();
            break;
        }
        case opcodes::PUSH_OP:
        {
            // Copy the last loaded object into the operation objects list
            m_operation_stack_objects.push_back((*LLOref)->clone());
            break;
        }
        case opcodes::PUSH_OP_NO_CLONE:
        {
            // Moves the last loaded object onto the operation objects list
            m_operation_stack_objects.push_back(*LLOref);
            break;
        }
        case opcodes::POP_OP:
        {
            // Free the object before popping
            auto& obj = m_operation_stack_objects.back();
            free_object(obj);

            // Pop the last object from the list of operation objects
            m_operation_stack_objects.pop_back();
            break;
        }
        case opcodes::POP_OP_NO_FREE:
        {
            // Pop the last object from the list of operation objects
            m_operation_stack_objects.pop_back();
            break;
        }
        case opcodes::ADD:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BinOpAdd);
            break;
        }
        case opcodes::SUB:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BinOpSub);
            break;
        }
        case opcodes::MUL:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BinOpMul);
            break;
        }
        case opcodes::DIV:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BinOpDiv);
            break;
        }
        case opcodes::MOD:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BinOpMod);
            break;
        }
        case opcodes::EQU:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BoolOpEqu);
            break;
        }
        case opcodes::NOTEQU:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BoolOpNotEqu);
            break;
        }
        case opcodes::GREATER:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BoolOpGreaterThan);
            break;
        }
        case opcodes::LESS:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BoolOpLessThan);
            break;
        }
        case opcodes::OR:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BoolOpOr);
            break;
        }
        case opcodes::AND:
        {
            execute_runtime_operator_instruction(RuntimeOperator::BoolOpAnd);
            break;
        }
        case opcodes::JMP:
        {
            // Operand is the index of instruction to jump to
            auto operand = ops[1];
            opcount = 2;

            // Adjust the current instruction to
            // the value that should be jumped to.
            current_instruction = operand;

            // Increment of instructions should be 0
            // since we already jump to the right instruction.
            return 0;
        }
        case opcodes::JMP_IF_FALSE:
        {
            // Operand is the index of instruction to jump to if boolean expression evaluates to false
            auto operand = ops[1];
            opcount = 2;

            if (!LLOref || strcmp((*LLOref)->runtime_name(), "Boolean") != 0)
            {
                auto ex_reason = "conditional expression is not a boolean";
                m_env->throw_exception(RuntimeException(ex_reason));
                return 0;
            }

            // Get the boolean value of the loaded conditional expression
            auto value = static_cast<YosenBoolean*>((*LLOref))->value;

            if (!value)
            {
                // Adjust the current instruction to
                // the value that should be jumped to.
                current_instruction = operand;

                // Increment of instructions should be 0
                // since we already jump to the right instruction.
                return 0;
            }

            break;
        }
        case opcodes::IMPORT_LIB:
        {
            // Operand is the key of the register into which the loaded object has to be copied
            auto operand = ops[1];
            opcount = 2;

            auto lib_name = stack_frame->imported_library_names[operand];
            m_env->load_yosen_module(lib_name);

            break;
        }
        case opcodes::ALLOC_OBJECT:
        {
            // Operand is the key of the register into which the loaded object has to be copied
            auto operand = ops[1];
            opcount = 2;

            auto class_name = stack_frame->class_names[operand];

            if (!m_env->is_class_name(class_name))
            {
                auto ex_reason = "Class name \"" + class_name + "\" not found";
                m_env->throw_exception(RuntimeException(ex_reason));
                return 0;
            }

            // Process parameters
            auto& parameter_stack = m_parameter_stacks.top();
            auto param_count = parameter_stack.size();

            // Reverse the param stack
            std::reverse(parameter_stack.begin(), parameter_stack.end());

            YosenTuple* param_pack = allocate_object<YosenTuple>(parameter_stack);

            // Instantiate the class
            auto instance = m_env->construct_class_instance(class_name, param_pack);

            // In case of a user-defined class, there may be a constructor,
            // if it exists, call it.
            if (instance->has_member_runtime_function(class_name))
            {
                // Add the constructor to the call stack
                m_call_stack.push_back(class_name + "::constructor");

                auto fn = instance->get_member_runtime_function(class_name);

                auto fn_stack_frame = fn.first->clone();
                auto& fn_bytecode = fn.second;

                // Adjust the param count to account for the "self" object
                ++param_count;

                // Setup function's parameters from
                // the current function's parameter stack.
                if (fn_stack_frame->params.size())
                {
                    if (fn_stack_frame->params.size() != param_count)
                    {
                        auto ex_reason = "Constructor for class \"" + class_name + "\" expected " +
                            std::to_string(fn_stack_frame->params.size()) +
                            " arguments, received " + std::to_string(param_count) +
                            " arguments";

                        // Destroy the stack frame since it's a clone
                        destroy_stack_frame(fn_stack_frame);

                        // Deallocate the parameter pack object
                        free_object(param_pack);

                        // Clear the parameter stack
                        parameter_stack.clear();

                        // Destroy the allocated instance
                        free_object(instance);

                        m_env->throw_exception(RuntimeException(ex_reason));
                        return 0;
                    }

                    // Assign the caller object as the first parameter in the param pack (self)
                    fn_stack_frame->params[0].second = allocate_object<YosenReference>(instance);

                    for (size_t i = 0; i < param_count - 1; ++i)
                        fn_stack_frame->params[i + 1].second = param_pack->items[param_count - 2 - i]->clone();

                    // Set the used parameter count, excluding the "self" parameter
                    param_pack->items_used = fn_stack_frame->params.size() - 1;
                }

                // Create an empty parameter stack to be used by the function for future functions
                m_parameter_stacks.push({});

                // Run the user function (return register will automatically be updated
                execute_bytecode(fn_stack_frame, fn_bytecode);

                // Get the instance object from the reference again
                instance = static_cast<YosenReference*>(fn_stack_frame->params[0].second)->obj;

                // Deallocate the user function's stack frame
                deallocate_stack_frame(fn_stack_frame);

                // Destroy the stack frame since it's a clone
                destroy_stack_frame(fn_stack_frame);

                // Pop the functions's parameter stack
                m_parameter_stacks.pop();

                // Reverse the parameters for the runtime function case
                std::reverse(param_pack->items.begin(), param_pack->items.end());

                // Pop the constructor off the call stack
                m_call_stack.pop_back();
            }

            size_t used_params = param_pack->items_used;
            size_t params_to_remove = param_pack->items.size() - used_params;

            // Deallocate the parameter pack object
            for (size_t i = 0; i < params_to_remove; ++i)
                param_pack->items.pop_back();

            free_object(param_pack);

            // Remove the used objects from the parameter stack
            for (size_t i = 0; i < used_params; ++i)
                parameter_stack.pop_back();

            // Retrieve the original object
            auto original_object = m_registers[RegisterType::AllocatedObjectRegister];

            // Copy the object into the register
            m_registers[RegisterType::AllocatedObjectRegister] = instance;

            // Free the original object
            if (original_object)
                free_object(original_object);

            break;
        }
        case opcodes::RET:
        {
            // Move the instruction pointer to the end
            current_instruction = instruction_count;
            return 0;
        }
        case opcodes::SET_RUNTIME_FLAG:
        {
            // Operand is flag to be set
            auto operand = ops[1];
            opcount = 2;

            m_runtime_flag = static_cast<RuntimeFlag>(operand);
            break;
        }
        case opcodes::CALL:
        {
            auto fn_index = ops[1];
            auto has_caller = ops[2];
            auto fn_name = stack_frame->function_names[fn_index];
            opcount = 3;

            // Process parameters
            auto& parameter_stack = m_parameter_stacks.top();
            auto param_count = parameter_stack.size();

            YosenTuple* param_pack = allocate_object<YosenTuple>(parameter_stack);
            YosenObject* return_val = nullptr;

            if (has_caller)
            {
                // Member function
                auto caller_object = *LLOref;

                // Push the function name to the call stack
                auto caller_type = caller_object->runtime_name();
                
                // Get the actual name of the object under the reference
                YosenObject* dummy_caller = caller_object;
                while (strcmp(caller_type, "Ref") == 0)
                {
                    caller_type = static_cast<YosenReference*>(dummy_caller)->obj->runtime_name();
                    dummy_caller = static_cast<YosenReference*>(dummy_caller)->obj;
                }

                m_call_stack.push_back(caller_type + std::string("::") + fn_name);

                // Check if it's a native member function
                if (caller_object->has_member_native_function(fn_name))
                {
                    return_val = caller_object->call_member_native_function(fn_name, param_pack);

                    // If the return register is not empty, deallocate the existing object
                    if (m_registers[RegisterType::ReturnRegister] != nullptr)
                    {
                        if (m_runtime_flag == RuntimeFlag::SequenceFunctionCall)
                        {
                            m_runtime_flag = RuntimeFlag::Null;
                            stack_frame->disposed_objects.push_back(m_registers[RegisterType::ReturnRegister]);
                        }
                        else
                            free_object(m_registers[RegisterType::ReturnRegister]);
                    }

                    // Move the return value into the return register
                    m_registers[RegisterType::ReturnRegister] = return_val;
                }
                else if (caller_object->has_member_runtime_function(fn_name))
                {
                    // Reverse the parameters for the runtime function case
                    std::reverse(param_pack->items.begin(), param_pack->items.end());

                    auto fn = caller_object->get_member_runtime_function(fn_name);

                    auto fn_stack_frame = fn.first->clone();
                    auto& fn_bytecode = fn.second;

                    // Adjust the param count to account for the "self" object
                    ++param_count;

                    // Setup function's parameters from
                    // the current function's parameter stack.
                    if (fn_stack_frame->params.size())
                    {
                        if (fn_stack_frame->params.size() != param_count)
                        {
                            auto ex_reason = "member function \"" + fn_name + "\" expected " +
                                std::to_string(fn_stack_frame->params.size()) +
                                " arguments, received " + std::to_string(param_count) +
                                " arguments";

                            // Destroy the stack frame since it's a clone
                            destroy_stack_frame(fn_stack_frame);

                            // Deallocate the parameter pack object
                            free_object(param_pack);

                            // Clear the parameter stack
                            parameter_stack.clear();

                            m_env->throw_exception(RuntimeException(ex_reason));
                            return 0;
                        }

                        // Assign the caller object as the first parameter in the param pack (self)
                        fn_stack_frame->params[0].second = allocate_object<YosenReference>(caller_object);

                        for (size_t i = 0; i < param_count - 1; ++i)
                            fn_stack_frame->params[i + 1].second = param_pack->items[param_count - 2 - i]->clone();

                        // Set the used parameter count, excluding the "self" parameter
                        param_pack->items_used = fn_stack_frame->params.size() - 1;
                    }

                    // Create an empty parameter stack to be used by the function for future functions
                    m_parameter_stacks.push({});

                    // Run the user function (return register will automatically be updated
                    execute_bytecode(fn_stack_frame, fn_bytecode);

                    // Deallocate the user function's stack frame
                    deallocate_stack_frame(fn_stack_frame);

                    // Destroy the stack frame since it's a clone
                    destroy_stack_frame(fn_stack_frame);

                    // Pop the functions's parameter stack
                    m_parameter_stacks.pop();

                    // Reverse the parameter pack again to get it back to normal
                    std::reverse(param_pack->items.begin(), param_pack->items.end());
                }
                else
                {
                    // Deallocate the parameter pack object
                    free_object(param_pack);

                    // Clear the parameter stack
                    parameter_stack.clear();

                    auto ex_reason = "Member function \"" + fn_name + "\" not found";
                    m_env->throw_exception(RuntimeException(ex_reason));
                    return 0;
                }
            }
            else
            {
                // Push the function name to the call stack
                m_call_stack.push_back(fn_name);

                // Check for a user-defined function
                if (m_env->is_static_runtime_function(fn_name))
                {
                    // Reverse the parameters for the runtime function case
                    std::reverse(param_pack->items.begin(), param_pack->items.end());

                    auto fn = m_env->get_static_runtime_function(fn_name);

                    auto fn_stack_frame = fn.first->clone();
                    auto& fn_bytecode = fn.second;

                    // Setup function's parameters from
                    // the current function's parameter stack.
                    if (fn_stack_frame->params.size())
                    {
                        if (fn_stack_frame->params.size() != param_count)
                        {
                            auto ex_reason = "function \"" + fn_name + "\" expected " +
                                std::to_string(fn_stack_frame->params.size()) +
                                " arguments, received " + std::to_string(param_count) +
                                " arguments";

                            // Destroy the stack frame since it's a clone
                            destroy_stack_frame(fn_stack_frame);

                            // Deallocate the parameter pack object
                            free_object(param_pack);

                            // Clear the parameter stack
                            parameter_stack.clear();

                            m_env->throw_exception(RuntimeException(ex_reason));
                            return 0;
                        }

                        for (size_t i = 0; i < param_count; ++i)
                            fn_stack_frame->params[i].second = param_pack->items[param_count - 1 - i]->clone();

                        // Set the used parameter count
                        param_pack->items_used = fn_stack_frame->params.size();
                    }

                    // Create an empty parameter stack to be used by the function for future functions
                    m_parameter_stacks.push({});

                    // Run the user function (return register will automatically be updated
                    execute_bytecode(fn_stack_frame, fn_bytecode);

                    // Deallocate the user function's stack frame
                    deallocate_stack_frame(fn_stack_frame);

                    // Destroy the stack frame since it's a clone
                    destroy_stack_frame(fn_stack_frame);

                    // Pop the functions's parameter stack
                    m_parameter_stacks.pop();

                    // Reverse the parameter pack again to get it back to normal
                    std::reverse(param_pack->items.begin(), param_pack->items.end());
                }

                // Check for a native function
                else if (m_env->is_static_native_function(fn_name))
                {
                    auto fn = m_env->get_static_native_function(fn_name);
                    return_val = fn(param_pack);

                    // If the return register is not empty, deallocate the existing object
                    if (m_registers[RegisterType::ReturnRegister] != nullptr)
                    {
                        if (m_runtime_flag == RuntimeFlag::SequenceFunctionCall)
                        {
                            m_runtime_flag = RuntimeFlag::Null;
                            stack_frame->disposed_objects.push_back(m_registers[RegisterType::ReturnRegister]);
                        }
                        else
                            free_object(m_registers[RegisterType::ReturnRegister]);
                    }

                    // Move the return value into the return register
                    m_registers[RegisterType::ReturnRegister] = return_val;
                }
                else
                {
                    // Deallocate the parameter pack object
                    free_object(param_pack);

                    // Clear the parameter stack
                    parameter_stack.clear();

                    auto ex_reason = "Static function \"" + fn_name + "\" not found";
                    m_env->throw_exception(RuntimeException(ex_reason));
                    return 0;
                }
            }

            size_t used_params = param_pack->items_used;
            size_t params_to_remove = param_pack->items.size() - used_params;

            // Deallocate the parameter pack object
            for (size_t i = 0; i < params_to_remove; ++i)
                param_pack->items.pop_back();

            free_object(param_pack);

            // Remove the used objects from the parameter stack
            for (size_t i = 0; i < used_params; ++i)
                parameter_stack.pop_back();

            // Pop the function name off the call stack
            m_call_stack.pop_back();

            break;
        }
        default: break;
        }

        return opcount;
	}

    void YosenInterpreter::execute_runtime_operator_instruction(RuntimeOperator op)
    {
        auto& lhs = m_operation_stack_objects.at(m_operation_stack_objects.size() - 2);
        auto& rhs = m_operation_stack_objects.at(m_operation_stack_objects.size() - 1);

        // Operation result
        auto result = lhs->call_runtime_operator_function(op, rhs);

        // Retrieve the original object from the allocated object register
        auto original_object = m_registers[RegisterType::AllocatedObjectRegister];

        // Copy the object into the register
        m_registers[RegisterType::AllocatedObjectRegister] = result;

        // Load the result into the LLOref as well
        LLOref = &m_registers[RegisterType::AllocatedObjectRegister];

        // Free the original object
        if (original_object)
            free_object(original_object);
    }

}
