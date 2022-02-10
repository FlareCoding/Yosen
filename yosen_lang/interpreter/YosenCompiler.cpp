#include "YosenCompiler.h"
#include "parser/Parser.h"
#include <sstream>
#include <fstream>
#include <iostream>

namespace yosen
{
    static bool ends_with(const std::string& value, const std::string& ending)
    {
        if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }
    
    static ProgramSource* s_ProgramSourcePtr = nullptr;
    static std::string s_CurrentCompilingPath = "";

    void YosenCompiler::__ys_free_compiled_resources(StackFramePtr faulty_stack_frame)
    {
        // Free the compiled resources on all compiled stack frames
        if (s_ProgramSourcePtr)
        {
            for (auto& [stack_frame, bytecode] : s_ProgramSourcePtr->runtime_functions)
                destroy_stack_frame(stack_frame);

            s_ProgramSourcePtr->runtime_functions.clear();
        }

        // Destroy the faulty stack frame that hasn't compiled yet
        destroy_stack_frame(faulty_stack_frame);
    }

    static std::vector<std::string> split(const std::string& str, char delim)
    {
        std::stringstream ss(str);
        std::vector<std::string> result;

        while (ss.good())
        {
            std::string substr;
            std::getline(ss, substr, ',');
            result.push_back(substr);
        }

        return result;
    }

    static std::string deduce_literal_type(const std::string& value)
    {
        const auto is_integer = [](const std::string& str) -> bool
        {
            return std::all_of(str.begin(), str.end(), ::isdigit);
        };

        const auto is_float = [](const std::string& str) -> bool
        {
            std::istringstream iss(str);
            float f;
            iss >> std::noskipws >> f;
            return iss.eof() && !iss.fail();
        };

        const auto is_boolean = [](const std::string& str) -> bool
        {
            return (str == "true") || (str == "false");
        };

        if (value == "null") return "null";

        if (is_integer(value))  return "int";
        if (is_float(value))    return "float";
        if (is_boolean(value))  return "bool";

        return "string";
    }

    static YosenObject* allocate_literal_object(const std::string& type_str, const std::string& value)
    {
        parser::LiteralType type = parser::LiteralValueToken::type_from_string(type_str);

        switch (type)
        {
        case parser::LiteralType::Null:
        {
            return YosenObject_Null->clone();
        }
        case parser::LiteralType::Boolean:
        {
            bool bval = (value == "true");
            return allocate_object<YosenBoolean>(bval);
        }
        case parser::LiteralType::String:
        {
            return allocate_object<YosenString>(value);
        }
        case parser::LiteralType::Integer:
        {
            return allocate_object<YosenInteger>(static_cast<uint64_t>(std::stoi(value)));
        }
        case parser::LiteralType::Float:
        {
            return allocate_object<YosenFloat>(std::stod(value));
        }
        case parser::LiteralType::List:
        {
            auto element_values = split(value, ',');
            std::vector<YosenObject*> list_elems;

            try {
                for (auto& val : element_values)
                {
                    auto type = deduce_literal_type(val);
                    auto obj = allocate_literal_object(type, val);
                    list_elems.push_back(obj);
                }
            }
            catch (...) {
                // Free allocated list elements
                for (auto& allocated_elem : list_elems)
                    free_object(allocated_elem);

                auto ex_reason = "Failed to instantiate list, make sure all elements are literal values";
                YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
                break;
            }

            return allocate_object<YosenList>(list_elems);
        }
        default: throw "Unknown Literal Type";
        }

        return YosenObject_Null->clone();
    }

    static opcodes::opcode_t opcode_from_binary_operator(const std::string_view& op)
    {
        if (op == "+") return opcodes::ADD;
        if (op == "-") return opcodes::SUB;
        if (op == "*") return opcodes::MUL;
        if (op == "/") return opcodes::DIV;
        if (op == "%") return opcodes::MOD;

        return 0;
    }

    static opcodes::opcode_t opcode_from_boolean_operator(const std::string_view& op)
    {
        if (op == "==") return opcodes::EQU;
        if (op == "!=") return opcodes::NOTEQU;
        if (op == ">") return opcodes::GREATER;
        if (op == "<") return opcodes::LESS;
        if (op == "||") return opcodes::OR;
        if (op == "&&") return opcodes::AND;

        return 0;
    }

    void YosenCompiler::debug_print_bytecode(bytecode_t& bytecode)
    {
        auto it = bytecode.begin();

        while (it != bytecode.end())
        {
            auto instruction_idx = (it - bytecode.begin());
            auto instruction_idx_str = "(" + std::to_string(instruction_idx) + ")";

            std::cout << std::setw(10) << std::left << instruction_idx_str;

            switch (*it)
            {
            case opcodes::LOAD:
            {
                it++;
                printf("LOAD 0x%x\n", *it);
                break;
            }
            case opcodes::LOAD_CONST:
            {
                it++;
                printf("LOAD_CONST 0x%x\n", *it);
                break;
            }
            case opcodes::LOAD_PARAM:
            {
                it++;
                printf("LOAD_PARAM 0x%x\n", *it);
                break;
            }
            case opcodes::STORE:
            {
                it++;
                printf("STORE 0x%x\n", *it);
                break;
            }
            case opcodes::LOAD_MEMBER:
            {
                it++;
                printf("LOAD_MEMBER 0x%x\n", *it);
                break;
            }
            case opcodes::STORE_MEMBER:
            {
                it++;
                printf("STORE_MEMBER 0x%x\n", *it);
                break;
            }
            case opcodes::REG_LOAD:
            {
                it++;
                printf("REG_LOAD 0x%x\n", *it);
                break;
            }
            case opcodes::REG_STORE:
            {
                it++;
                printf("REG_STORE 0x%x\n", *it);
                break;
            }
            case opcodes::PUSH:
            {
                printf("PUSH\n");
                break;
            }
            case opcodes::POP:
            {
                printf("POP\n");
                break;
            }
            case opcodes::PUSH_OP_NO_CLONE:
            {
                printf("PUSH_OP_NO_CLONE\n");
                break;
            }
            case opcodes::POP_OP_NO_FREE:
            {
                printf("POP_OP_NO_FREE\n");
                break;
            }
            case opcodes::PUSH_OP:
            {
                printf("PUSH_OP\n");
                break;
            }
            case opcodes::POP_OP:
            {
                printf("POP_OP\n");
                break;
            }
            case opcodes::ADD:
            {
                printf("ADD\n");
                break;
            }
            case opcodes::SUB:
            {
                printf("SUB\n");
                break;
            }
            case opcodes::MUL:
            {
                printf("MUL\n");
                break;
            }
            case opcodes::DIV:
            {
                printf("DIV\n");
                break;
            }
            case opcodes::MOD:
            {
                printf("MOD\n");
                break;
            }
            case opcodes::EQU:
            {
                printf("EQU\n");
                break;
            }
            case opcodes::NOTEQU:
            {
                printf("NOTEQU\n");
                break;
            }
            case opcodes::GREATER:
            {
                printf("GREATER\n");
                break;
            }
            case opcodes::LESS:
            {
                printf("LESS\n");
                break;
            }
            case opcodes::OR:
            {
                printf("OR\n");
                break;
            }
            case opcodes::AND:
            {
                printf("AND\n");
                break;
            }
            case opcodes::CALL:
            {
                it++;
                auto first_op = *it;

                it++;
                auto second_op = *it;
                printf("CALL 0x%x 0x%x\n", first_op, second_op);
                break;
            }
            case opcodes::RET:
            {
                printf("RET\n");
                break;
            }
            case opcodes::ALLOC_OBJECT:
            {
                it++;
                printf("ALLOC_OBJECT 0x%x\n", *it);
                break;
            }
            case opcodes::IMPORT_LIB:
            {
                it++;
                printf("IMPORT_LIB 0x%x\n", *it);
                break;
            }
            case opcodes::JMP:
            {
                it++;
                printf("JMP %i\n", *it);
                break;
            }
            case opcodes::JMP_IF_FALSE:
            {
                it++;
                printf("JMP_IF_FALSE %i\n", *it);
                break;
            }
            default:
                printf("0x%x ", *it);
            }

            it++;
        }
    }

    void YosenCompiler::shutdown()
    {
        for (auto& stack_frame : m_allocated_stack_frames)
        {
            // Deallocated created resources off the stack frame
            deallocate_stack_frame(stack_frame);

            // Completely destroy the last resources on the stack frame
            destroy_stack_frame(stack_frame);
        }
    }

    uint32_t YosenCompiler::get_constant_literal_key(json11::Json* node_ptr, StackFramePtr stack_frame)
    {
        auto& node = *node_ptr;

        auto& literal_type = node["value_type"].string_value();
        auto& value = node["value"].string_value();

        uint32_t constant_key = 0;

        // Check if the constant has not been found before
        if (stack_frame->constant_keys.find(value) == stack_frame->constant_keys.end())
        {
            // Allocate object for a constant
            auto obj = allocate_literal_object(literal_type, value);

            // Get the next available constant key
            constant_key = (uint32_t)stack_frame->constants.size();

            // Store the key in a constant keys map
            stack_frame->constant_keys.insert({ value, constant_key });

            // Store the object in a constants map
            stack_frame->constants.insert({ constant_key, obj });
        }
        else
            constant_key = stack_frame->constant_keys.at(value);

        return constant_key;
    }

    uint32_t YosenCompiler::get_variable_key(json11::Json* node_ptr, StackFramePtr stack_frame)
    {
        auto& node = *node_ptr;

        // Get the variable name
        const auto& identifier_value = node["value"].string_value();

        return get_variable_key(identifier_value, stack_frame);
    }

    uint32_t YosenCompiler::get_variable_key(const std::string& var, StackFramePtr stack_frame)
    {
        // Make sure the variable exists
        if (stack_frame->var_keys.find(var) == stack_frame->var_keys.end())
        {
            // Free all compiled resources
            __ys_free_compiled_resources(stack_frame);

            auto ex_reason = "Undefined variable \"" + var + "\" used";
            YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
            return 0;
        }

        // Get the key for the variable in the stack frame
        auto var_key = stack_frame->var_keys.at(var);

        return var_key;
    }

    void YosenCompiler::compile_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto type = node["type"].string_value();

        if (type == parser::ASTNodeType_Import)
            compile_import_statement(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_VariableDeclaration)
            compile_variable_declaration(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_VariableAssignment)
            compile_variable_assignment(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_FunctionCall)
            compile_function_call(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_ReturnStatement)
            compile_return_statement(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_Conditional)
            compile_conditional(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_WhileLoop)
            compile_while_loop(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_ForLoop)
            compile_for_loop(node_ptr, stack_frame, bytecode);

        else if (type == parser::ASTNodeType_BreakStatement)
            compile_break_statement(node_ptr, stack_frame, bytecode);
    }

    void YosenCompiler::compile_import_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto import_name = node["name"].string_value();

        // Check if imported name is a module/library or another yosen file
        if (ends_with(import_name, ".ys"))
        {
            // Import another file
            compile_imported_yosen_source_file(import_name, s_CurrentCompilingPath);
        }
        else
        {
            // Get library name index in the list of library names in a stack frame
            auto library_name_index = stack_frame->get_imported_lib_name_index(import_name);

            // Check if library name has not occured yet
            if (library_name_index == -1)
            {
                library_name_index = stack_frame->imported_library_names.size();
                stack_frame->imported_library_names.push_back(import_name);
            }

            // Create the bytecode for allocating the object
            bytecode.push_back(opcodes::IMPORT_LIB);
            bytecode.push_back(static_cast<opcodes::opcode_t>(library_name_index));
        }
    }

    void YosenCompiler::compile_expression(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto& value_node_type = node["type"].string_value();

        // Determining the value type, expression, literal, or another variable.
        if (value_node_type == parser::ASTNodeType_Literal)
        {
            // Get the key for the constant in the stack frame
            auto constant_key = get_constant_literal_key(&node, stack_frame);

            // Create bytecode for loading a constant
            bytecode.push_back(opcodes::LOAD_CONST);
            bytecode.push_back(static_cast<opcodes::opcode_t>(constant_key));
        }
        else if (value_node_type == parser::ASTNodeType_Identifier)
        {
            if (!node["parent"].is_null())
            {
                compile_loading_parent_objects(&node, stack_frame, bytecode);

                auto var_name = node["value"].string_value();
                auto member_var_idx = stack_frame->get_member_variable_name_index(var_name);
                if (member_var_idx == -1)
                {
                    member_var_idx = stack_frame->member_variable_names.size();
                    stack_frame->member_variable_names.push_back(var_name);
                }

                // Load member variable
                bytecode.push_back(opcodes::LOAD_MEMBER);
                bytecode.push_back(static_cast<opcodes::opcode_t>(member_var_idx));

                // Pop the caller object off the operations stack
                bytecode.push_back(opcodes::POP_OP_NO_FREE);
            }
            else
            {
                // If the argument is a variable
                //
                // Get the value variable key
                auto value_var_key = get_variable_key(&node, stack_frame);

                // Create bytecode for loading the variable
                bytecode.push_back(opcodes::LOAD);
                bytecode.push_back(static_cast<opcodes::opcode_t>(value_var_key));
            }
        }
        else if (value_node_type == parser::ASTNodeType_FunctionCall)
        {
            // If the value is a function call
            //
            // Compile the function call
            compile_function_call(&node, stack_frame, bytecode);

            // Load the return value from the function
            bytecode.push_back(opcodes::REG_LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x02));
        }
        else if (value_node_type == parser::ASTNodeType_ClassInstantiation)
        {
            // If class instantiation is attempted
            compile_class_instantiation(&node, stack_frame, bytecode);

            // Load the allocated object
            bytecode.push_back(opcodes::REG_LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x01));
        }
        else if (value_node_type == parser::ASTNodeType_BinaryOperation)
        {
            //
            // If the expression is a binary operation (+, -, *, or /)
            //
            // 
            // First compile the left hand side and load it into LLOref object
            auto lhs_node = node["lhs"];
            compile_expression(&lhs_node, stack_frame, bytecode);

            // Push the loaded object onto the operations stack
            bytecode.push_back(opcodes::PUSH_OP);

            // Next compile the right hand side and load it into LLOref object
            auto rhs_node = node["rhs"];
            compile_expression(&rhs_node, stack_frame, bytecode);

            // Push the loaded object onto the operations stack
            bytecode.push_back(opcodes::PUSH_OP);

            // Call the appropriate binary operator
            auto operator_instruction = opcode_from_binary_operator(node["operator"].string_value());
            bytecode.push_back(operator_instruction);

            // Pop the last two objects off the operations stack
            bytecode.push_back(opcodes::POP_OP);
            bytecode.push_back(opcodes::POP_OP);
        }
        else if (value_node_type == parser::ASTNodeType_BooleanOperation)
        {
            //
            // If the expression is a boolean operation (||, &&, ==, etc.)
            //
            // 
            // First compile the left hand side and load it into LLOref object
            auto lhs_node = node["lhs"];
            compile_expression(&lhs_node, stack_frame, bytecode);

            // Push the loaded object onto the operations stack
            bytecode.push_back(opcodes::PUSH_OP);

            // Next compile the right hand side and load it into LLOref object
            auto rhs_node = node["rhs"];
            compile_expression(&rhs_node, stack_frame, bytecode);

            // Push the loaded object onto the operations stack
            bytecode.push_back(opcodes::PUSH_OP);

            // Call the appropriate binary operator
            auto operator_instruction = opcode_from_boolean_operator(node["operator"].string_value());
            bytecode.push_back(operator_instruction);

            // Pop the last two objects off the operations stack
            bytecode.push_back(opcodes::POP_OP);
            bytecode.push_back(opcodes::POP_OP);
        }
    }

    void YosenCompiler::compile_function_call(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        // Get function name
        auto& function_name = node["name"].string_value();

        // Get function index in the list of function names in a stack frame
        auto function_index = stack_frame->get_function_index(function_name);

        // Check if function has not occured yet
        if (function_index == -1)
        {
            function_index = stack_frame->function_names.size();
            stack_frame->function_names.push_back(function_name);
        }

        // Process function arguments
        for (auto arg_node : node["args"].array_items())
        {
            auto& arg_node_type = arg_node["type"].string_value();

            // Compiling the expression and loading its value
            compile_expression(&arg_node, stack_frame, bytecode);

            // Pushing the loaded object onto the parameter stack
            bytecode.push_back(opcodes::PUSH);
        }

        // Check if there is a caller object
        opcodes::opcode_t has_caller_flag = 0x00;

        if (!node["parent"].is_null())
        {
            // Set the caller flag
            has_caller_flag = 0x01;

            compile_loading_parent_objects(&node, stack_frame, bytecode);

            // Pop the last object from the operations stack
            bytecode.push_back(opcodes::POP_OP_NO_FREE);
        }

        // Create the bytecode for calling the function
        bytecode.push_back(opcodes::CALL);
        bytecode.push_back(static_cast<opcodes::opcode_t>(function_index));
        bytecode.push_back(has_caller_flag); // Caller (this) object
    }

    void YosenCompiler::compile_function_declaration(json11::Json* node_ptr, ProgramSource& program_source)
    {
        auto& node = *node_ptr;

        if (node["type"].string_value() != parser::ASTNodeType_FunctionDeclaration)
        {
            throw "Node is not a function declaration";
            return;
        }

        StackFramePtr stack_frame = allocate_stack_frame();
        bytecode_t bytecode;

        // Set the global program source object pointer
        s_ProgramSourcePtr = &program_source;

        // Registering the function name
        stack_frame->name = node["name"].string_value();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
        printf("Compiling function \"%s\"...\n", stack_frame->name.c_str());
#endif

        // Allocating a Null object in the variables map (always first)
        stack_frame->var_keys.insert({ "null", 0 });
        stack_frame->vars.insert({ 0, YosenObject_Null->clone() });

        // Allocating parameters and their variable spaces
        uint32_t param_idx = 0;
        for (auto& param : node["params"].array_items())
        {
            auto param_name = param.string_value();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
            printf("\tParameter: \"%s\"\n", param_name.c_str());
#endif

            // Registering the parameter on the stack frame
            stack_frame->params.push_back({ param_name, nullptr });

            auto param_key = param_idx + 1;

            // Reserving variable space on the stack frame
            stack_frame->var_keys.insert({ param_name, param_key });
            stack_frame->vars.insert({ param_key, YosenObject_Null->clone() });

            // Creating bytecode to store the parameter in a variable object
            bytecode.push_back(opcodes::LOAD_PARAM);
            bytecode.push_back(static_cast<opcodes::opcode_t>(param_idx));

            bytecode.push_back(opcodes::STORE);
            bytecode.push_back(static_cast<opcodes::opcode_t>(param_key));

            // Advacing the parameter index
            ++param_idx;
        }

        // Start compiling the body of the function
        auto& body = node["body"];

        for (auto statement : body.array_items())
        {
#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
            printf("%s\n", statement.dump().c_str());
#endif

            compile_statement(&statement, stack_frame, bytecode);
        }

        //
        // If a user-defined return statement doesn't exist,
        // create an implicit "return null".
        //
        // Load YosenObject_Null
        bytecode.push_back(opcodes::LOAD);
        bytecode.push_back(static_cast<opcodes::opcode_t>(0x00));

        // Store it in the return register
        bytecode.push_back(opcodes::REG_STORE);
        bytecode.push_back(static_cast<opcodes::opcode_t>(0x02));

        // Add the compiled function to the program source object
        program_source.runtime_functions.push_back({ stack_frame, bytecode });

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
        printf("\n");
        debug_print_bytecode(bytecode);
        printf("\n");
#endif
    }

    void YosenCompiler::compile_variable_declaration(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        auto& variable_name = node["name"].string_value();
        auto  value_node = node["value"];

        // Check if the variable exists
        if (stack_frame->var_keys.find(variable_name) != stack_frame->var_keys.end())
        {
            // Free all compiled resources
            __ys_free_compiled_resources(stack_frame);

            auto ex_reason = "Variable \"" + variable_name + "\" already exists";
            YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
            return;
        }

        // Get the next available variable key
        uint32_t var_key = (uint32_t)stack_frame->var_keys.size();

        // Create a variable key entry for the variable name
        stack_frame->var_keys.insert({ variable_name, var_key });

        // Create an entry in the variables map
        stack_frame->vars.insert({ var_key, YosenObject_Null->clone() });

        // Compiling the expression and loading its value
        compile_expression(&value_node, stack_frame, bytecode);

        // At this point, the value object is loaded into LLOref,
        // now we need to create bytecode for storing the value into a variable.
        bytecode.push_back(opcodes::STORE);
        bytecode.push_back(static_cast<opcodes::opcode_t>(var_key));
    }

    void YosenCompiler::compile_return_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        if (node["value"] == json11::Json::NUL)
        {
            // Load YosenObject_Null
            bytecode.push_back(opcodes::LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x00));
        }
        else
        {
            auto expression_node = node["value"];
            compile_expression(&expression_node, stack_frame, bytecode);
        }

        // Store the result in the return register
        bytecode.push_back(opcodes::REG_STORE);
        bytecode.push_back(static_cast<opcodes::opcode_t>(0x02));

        // Return from the function
        bytecode.push_back(opcodes::RET);
    }

    void YosenCompiler::compile_variable_assignment(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        auto& variable_name = node["name"].string_value();
        auto  value_node = node["value"];

        if (!node["parent"].is_null())
        {
            compile_loading_parent_objects(&node, stack_frame, bytecode);

            auto var_name = node["name"].string_value();
            auto member_var_idx = stack_frame->get_member_variable_name_index(var_name);
            if (member_var_idx == -1)
            {
                member_var_idx = stack_frame->member_variable_names.size();
                stack_frame->member_variable_names.push_back(var_name);
            }

            // Pop the parent object off the operations stack
            bytecode.push_back(opcodes::POP_OP_NO_FREE);

            // Pushes the member object onto the operations stack
            bytecode.push_back(opcodes::PUSH_OP_NO_CLONE);

            // Compiling the expression and loading its value
            compile_expression(&value_node, stack_frame, bytecode);

            // Store the loaded value into the member object on the operations stack
            bytecode.push_back(opcodes::STORE_MEMBER);
            bytecode.push_back(static_cast<opcodes::opcode_t>(member_var_idx));

            // Pop the member object off the operations stack
            bytecode.push_back(opcodes::POP_OP_NO_FREE);
        }
        else
        {
            // Compiling the expression and loading its value
            compile_expression(&value_node, stack_frame, bytecode);

            // Get the variable key
            uint32_t var_key = get_variable_key(variable_name, stack_frame);

            // At this point, the value object is loaded into LLOref,
            // now we need to create bytecode for storing the value into a variable.
            bytecode.push_back(opcodes::STORE);
            bytecode.push_back(static_cast<opcodes::opcode_t>(var_key));
        }
    }

    void YosenCompiler::compile_class_instantiation(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        // Get function name
        auto& class_name = node["name"].string_value();

        // Get function index in the list of function names in a stack frame
        auto class_name_index = stack_frame->get_class_name_index(class_name);

        // Check if function has not occured yet
        if (class_name_index == -1)
        {
            class_name_index = stack_frame->class_names.size();
            stack_frame->class_names.push_back(class_name);
        }

        // Process function arguments
        for (auto arg_node : node["args"].array_items())
        {
            auto& arg_node_type = arg_node["type"].string_value();

            // Compiling the expression and loading its value
            compile_expression(&arg_node, stack_frame, bytecode);

            // Pushing the loaded object onto the parameter stack
            bytecode.push_back(opcodes::PUSH);
        }

        // Create the bytecode for allocating the object
        bytecode.push_back(opcodes::ALLOC_OBJECT);
        bytecode.push_back(static_cast<opcodes::opcode_t>(class_name_index));
    }

    void YosenCompiler::compile_conditional(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto condition_expression = node["condition"];

        // Compile the condition
        compile_expression(&condition_expression, stack_frame, bytecode);

        auto if_false_jmp_instruction_index = bytecode.size();
        bytecode.push_back(opcodes::JMP_IF_FALSE);
        bytecode.push_back(0x0); // dummy address, will get replaced later

        // Compile the body of the if statement
        auto& if_body = node["if_body"];

        for (auto statement : if_body.array_items())
        {
            compile_statement(&statement, stack_frame, bytecode);
        }

        // At the end of the if statement, it should skip
        // over the else block if there is one and jump to the next instruction.
        auto final_if_jmp_instruction_index = bytecode.size();

        bytecode.push_back(opcodes::JMP);
        bytecode.push_back(0x0); // dummy address, will get replaced later

         // Get the instruction index at the end of the conditional
        auto next_instruction_index = bytecode.size();

        // Fix the jump instruction operands from earlier
        bytecode[if_false_jmp_instruction_index + 1] = static_cast<opcodes::opcode_t>(next_instruction_index);
        bytecode[final_if_jmp_instruction_index + 1] = static_cast<opcodes::opcode_t>(next_instruction_index);

        // Check if there is an "else" statement in the conditional
        auto else_body = node["else_body"];

        if (else_body != json11::Json::NUL)
        {
            // Compile the items in the else statement
            for (auto statement : else_body.array_items())
            {
                compile_statement(&statement, stack_frame, bytecode);
            }

            // Get the instruction index at the end of the conditional
            next_instruction_index = bytecode.size();

            // Fix the final jump instruction operand from
            // earlier to jump to the place after the else block.
            bytecode[final_if_jmp_instruction_index + 1] = static_cast<opcodes::opcode_t>(next_instruction_index);
        }
    }

    void YosenCompiler::compile_while_loop(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto condition_expression = node["condition"];

        // Get the starting instruction index
        auto condition_instruction_index = bytecode.size();

        // Push to the loop stack
        loop_break_jmp_operand_indices.push({});

        // Compile the condition
        compile_expression(&condition_expression, stack_frame, bytecode);

        // If the condition is false, then jump out of the loop
        bytecode.push_back(opcodes::JMP_IF_FALSE);

        loop_break_jmp_operand_indices.top().push_back(bytecode.size());
        bytecode.push_back(0x0); // dummy address, will get replaced later

        // Compile the loop body
        auto& loop_body = node["body"];

        for (auto statement : loop_body.array_items())
        {
            compile_statement(&statement, stack_frame, bytecode);
        }

        // After all body statements finish executing,
        // jump back to the condition expression.
        bytecode.push_back(opcodes::JMP);
        bytecode.push_back(static_cast<opcodes::opcode_t>(condition_instruction_index));

        // First instruction that is not part of the loop
        auto next_instruction_index = bytecode.size();

        // Fix all jmp instruction operands
        for (auto& op_idx : loop_break_jmp_operand_indices.top())
            bytecode[op_idx] = static_cast<opcodes::opcode_t>(next_instruction_index);

        // Pop the loop off the loop stack
        loop_break_jmp_operand_indices.pop();
    }

    void YosenCompiler::compile_for_loop(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto init_statement = node["init_statement"];
        auto condition_expression = node["condition"];
        auto post_iteration_statement = node["post_iteration"];

        // Compile the initial statement
        compile_statement(&init_statement, stack_frame, bytecode);

        // Get the instruction index of the condition (start of the loop)
        auto condition_instruction_index = bytecode.size();

        // Push to the loop stack
        loop_break_jmp_operand_indices.push({});

        // Compile the condition
        compile_expression(&condition_expression, stack_frame, bytecode);

        // If the condition is false, then jump out of the loop
        bytecode.push_back(opcodes::JMP_IF_FALSE);

        loop_break_jmp_operand_indices.top().push_back(bytecode.size());
        bytecode.push_back(0x0); // dummy address, will get replaced later

        // Compile the loop body
        auto& loop_body = node["body"];

        for (auto statement : loop_body.array_items())
        {
            compile_statement(&statement, stack_frame, bytecode);
        }

        // At the end of each iteration, the post iteration statement has to be called
        compile_statement(&post_iteration_statement, stack_frame, bytecode);

        // After all body statements finish executing,
        // jump back to the condition expression.
        bytecode.push_back(opcodes::JMP);
        bytecode.push_back(static_cast<opcodes::opcode_t>(condition_instruction_index));

        // First instruction that is not part of the loop
        auto next_instruction_index = bytecode.size();

        // Fix all jmp instruction operands
        for (auto& op_idx : loop_break_jmp_operand_indices.top())
            bytecode[op_idx] = static_cast<opcodes::opcode_t>(next_instruction_index);

        // Pop the loop off the loop stack
        loop_break_jmp_operand_indices.pop();
    }

    void YosenCompiler::compile_break_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        bytecode.push_back(opcodes::JMP);

        loop_break_jmp_operand_indices.top().push_back(bytecode.size());
        bytecode.push_back(0x0);
    }

    void YosenCompiler::compile_class_declaration(json11::Json* node_ptr, ProgramSource& program_source)
    {
        auto& class_node = *node_ptr;
        auto class_name = class_node["name"].string_value();

        auto class_builder = YosenEnvironment::get().create_runtime_class_builder(class_name);

        for (auto body_node : class_node["body"].array_items())
        {
            auto body_node_type = body_node["type"].string_value();

            if (body_node_type == parser::ASTNodeType_FunctionDeclaration)
            {
                auto& params = body_node["params"].array_items();
                if (params.size() && params[0].string_value() == "self")
                {
                    // Member function (first argument should always be "self")
                    ProgramSource source;
                    auto fn_name = body_node["name"].string_value();

                    // Compile the function into a separate program source object
                    compile_function_declaration(&body_node, source);

                    // Insert the runtime function into the class builder object
                    class_builder->runtime_functions[fn_name] = source.runtime_functions[0];
                }
                else
                {
                    // Static function
                    compile_function_declaration(&body_node, program_source);
                }
            }
            else if (body_node_type == parser::ASTNodeType_VariableDeclaration)
            {
                auto& variable_name = body_node["name"].string_value();
                auto  value_node = body_node["value"];

                if (value_node["type"].string_value() != parser::ASTNodeType_Literal)
                {
                    auto ex_reason = "Value of member variable \"" + variable_name + "\" for class \"" + class_name + "\" has to be a literal, not an expression";
                    YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
                    return;
                }

                // Check if the variable exists
                if (class_builder->member_variables.find(variable_name) != class_builder->member_variables.end())
                {
                    auto ex_reason = "Member variable \"" + variable_name + "\" for class \"" + class_name + "\" already exists";
                    YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
                    return;
                }

                auto& literal_type = value_node["value_type"].string_value();
                auto& value_node_literal_value = value_node["value"].string_value();

                // Allocate member variable object
                auto member_var_obj = allocate_literal_object(literal_type, value_node_literal_value);

                // Create a member variable entry
                class_builder->member_variables.insert({ variable_name, member_var_obj });
            }
        }

        // Register the class into the environment
        class_builder->create_runtime_class();
    }

    void YosenCompiler::compile_loading_parent_objects(json11::Json* member_node, StackFramePtr stack_frame, bytecode_t& bytecode)
    {
        // Get the first parent node
        auto node = (*member_node)["parent"];

        // ------------------------------------
        //  Find the outermost parent object.
        // 
        //  Example:
        //          tree.m_data.size;
        // 
        //  Outermost parent object is "tree".
        // ------------------------------------

        // Construct a list of all parent
        // objects from left to right order.
        std::vector<std::string> parent_nodes; // <Parent, IdentifierValue>

        while (!node.is_null())
        {
            auto value = node["value"].string_value();
            parent_nodes.insert(parent_nodes.begin(), value);

            node = node["parent"];
        }

        // Iterate over every parent from left to right and load them
        for (size_t i = 0; i < parent_nodes.size(); ++i)
        {
            auto& var_name = parent_nodes.at(i);

            // If it's the first node, then it doesn't have a parent
            // and must be a valid variable.
            if (i == 0)
            {
                auto var_key = get_variable_key(var_name, stack_frame);

                // Load variable object
                bytecode.push_back(opcodes::LOAD);
                bytecode.push_back(static_cast<opcodes::opcode_t>(var_key));

                // Push the parent object onto the operations stack
                bytecode.push_back(opcodes::PUSH_OP_NO_CLONE);
                continue;
            }

            // Get an existing or create a new member variable index
            auto member_var_name_idx = stack_frame->get_member_variable_name_index(var_name);
            if (member_var_name_idx == -1)
            {
                member_var_name_idx = stack_frame->member_variable_names.size();
                stack_frame->member_variable_names.push_back(var_name);
            }

            // Load member variable
            bytecode.push_back(opcodes::LOAD_MEMBER);
            bytecode.push_back(static_cast<opcodes::opcode_t>(member_var_name_idx));

            // Pop the parent object off the operations stack
            bytecode.push_back(opcodes::POP_OP_NO_FREE);

            // Push the newly loaded object onto the operations stack
            bytecode.push_back(opcodes::PUSH_OP_NO_CLONE);
        }
    }

    void YosenCompiler::deallocate_stack_frame(StackFramePtr stack_frame)
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
	}

    void YosenCompiler::destroy_stack_frame(StackFramePtr stack_frame)
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

    void YosenCompiler::compile_imported_yosen_source_file(const std::string& import_name, const std::string& current_path)
    {
        std::string file_path = current_path + "/" + import_name;

        if (import_name.empty() || !std::filesystem::is_regular_file(file_path))
        {
            auto ex_reason = "Can't find imported source file \"" + file_path + "\"";
            YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
            return;
        }

        std::ifstream stream(file_path);
        std::stringstream source_code_buffer;
        source_code_buffer << stream.rdbuf();

        auto source_code = source_code_buffer.str();

        auto program_source = compile_source(source_code, std::filesystem::path(file_path).parent_path().string());

        // Register all the functions in the yosen environment
        for (auto& fn : program_source.runtime_functions)
        {
            auto& stack_frame = fn.first;
            m_allocated_stack_frames.push_back(stack_frame);
            YosenEnvironment::get().register_static_runtime_function(stack_frame->name, fn);
        }

        // Reset the current compiling path
        s_CurrentCompilingPath = current_path;
    }

    ProgramSource YosenCompiler::compile_source(std::string& source, const std::string& source_path)
    {
        ProgramSource program_source;
        s_CurrentCompilingPath = source_path;

        parser::Parser parser;
        auto ast = parser.parse_source(source);
        for (auto node : ast.array_items())
        {
            if (node["type"].string_value() == parser::ASTNodeType_Import)
            {
                // Directly import the library
                auto import_name = node["name"].string_value();

                if (ends_with(import_name, ".ys"))
                {
                    // Load another yosen file
                    compile_imported_yosen_source_file(import_name, source_path);
                }
                else
                {
                    // Load a native library module
                    YosenEnvironment::get().load_yosen_module(import_name);   
                }
            }
            else if (node["type"].string_value() == parser::ASTNodeType_FunctionDeclaration)
            {
                compile_function_declaration(&node, program_source);
            }
            else if (node["type"].string_value() == parser::ASTNodeType_ClassDeclaration)
            {
                compile_class_declaration(&node, program_source);
            }
        }

        return program_source;
    }
    
    bytecode_t YosenCompiler::compile_single_statement(std::string& source, StackFramePtr stack_frame)
    {
        bytecode_t bytecode;

        parser::Parser parser;
        auto ast = parser.parse_single_statement(source);

        compile_statement(&ast, stack_frame, bytecode);

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
        printf("\nAST:\n");
        printf("%s\n", ast.dump().c_str());
        printf("\n");

        printf("Bytecode:\n");
        debug_print_bytecode(bytecode);

        printf("\n");
#endif

        return bytecode;
    }
}