#include "YosenCompiler.h"
#include "parser/Parser.h"

namespace yosen
{
    static std::string get_anonymous_stack_frame_name()
    {
        static uint64_t idx = 0;
        ++idx;
        
        return "__anonymous_frame_" + std::to_string(idx);
    }

    static YosenObject* allocate_literal_object(const std::string& type_str, const std::string& value)
    {
        parser::LiteralType type = parser::LiteralValueToken::type_from_string(type_str);

        switch (type)
        {
        case parser::LiteralType::String:
        {
            return allocate_object<YosenString>(value);
        }
        case parser::LiteralType::Integer:
        {
            return allocate_object<YosenInteger>(static_cast<uint64_t>(std::stoi(value)));
        }
        default: throw "Unknown type";
        }
    }

    void YosenCompiler::debug_print_bytecode(bytecode_t& bytecode)
    {
        auto it = bytecode.begin();

        while (it != bytecode.end())
        {
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
            case opcodes::CALL:
            {
                it++;
                auto first_op = *it;

                it++;
                auto second_op = *it;
                printf("CALL 0x%x 0x%x\n", first_op, second_op);
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
            default:
                printf("0x%x ", *it);
            }

            it++;
        }
    }

    uint32_t YosenCompiler::get_constant_literal_key(json11::Json* node_ptr, StackFrame& stack_frame)
    {
        auto& node = *node_ptr;

        auto& literal_type = node["value_type"].string_value();
        auto& value = node["value"].string_value();

        uint32_t constant_key = 0;

        // Check if the constant has not been found before
        if (stack_frame.constant_keys.find(value) == stack_frame.constant_keys.end())
        {
            // Allocate object for a constant
            auto obj = allocate_literal_object(literal_type, value);

            // Get the next available constant key
            constant_key = (uint32_t)stack_frame.constants.size();

            // Store the key in a constant keys map
            stack_frame.constant_keys.insert({ value, constant_key });

            // Store the object in a constants map
            stack_frame.constants.insert({ constant_key, obj });
        }
        else
            constant_key = stack_frame.constant_keys.at(value);

        return constant_key;
    }

    uint32_t YosenCompiler::get_variable_key(json11::Json* node_ptr, StackFrame& stack_frame)
    {
        auto& node = *node_ptr;

        // Get the variable name
        auto& identifier_value = node["value"].string_value();

        // Make sure the variable exists
        if (stack_frame.var_keys.find(identifier_value) == stack_frame.var_keys.end())
        {
            auto ex_reason = "Undefined variable \"" + identifier_value + "\" used";
            YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
            return 0;
        }

        // Get the key for the variable in the stack frame
        auto var_key = stack_frame.var_keys.at(identifier_value);

        return var_key;
    }

    void YosenCompiler::compile_statement(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto type = node["type"].string_value();

        if (type._Equal(parser::ASTNodeType_Import))
            compile_import_statement(node_ptr, stack_frame, bytecode);

        else if (type._Equal(parser::ASTNodeType_VariableDeclaration))
            compile_variable_declaration(node_ptr, stack_frame, bytecode);

        else if (type._Equal(parser::ASTNodeType_VariableAssignment))
            compile_variable_assignment(node_ptr, stack_frame, bytecode);

        if (type._Equal(parser::ASTNodeType_FunctionCall))
            compile_function_call(node_ptr, stack_frame, bytecode);
    }

    void YosenCompiler::compile_import_statement(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto library_name = node["library"].string_value();

        // Get library name index in the list of library names in a stack frame
        auto library_name_index = stack_frame.get_imported_lib_name_index(library_name);

        // Check if library name has not occured yet
        if (library_name_index == -1)
        {
            library_name_index = stack_frame.imported_library_names.size();
            stack_frame.imported_library_names.push_back(library_name);
        }

        // Create the bytecode for allocating the object
        bytecode.push_back(opcodes::IMPORT_LIB);
        bytecode.push_back(static_cast<opcodes::opcode_t>(library_name_index));
    }

    void YosenCompiler::compile_expression(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto& value_node_type = node["type"].string_value();

        // Determining the value type, expression, literal, or another variable.
        if (value_node_type._Equal(parser::ASTNodeType_Literal))
        {
            // Get the key for the constant in the stack frame
            auto constant_key = get_constant_literal_key(&node, stack_frame);

            // Create bytecode for loading a constant
            bytecode.push_back(opcodes::LOAD_CONST);
            bytecode.push_back(static_cast<opcodes::opcode_t>(constant_key));
        }
        else if (value_node_type._Equal(parser::ASTNodeType_Identifier))
        {
            // If the argument is a variable
            //
            // Get the value variable key
            auto value_var_key = get_variable_key(&node, stack_frame);

            // Create bytecode for pushing the variable onto the stack
            bytecode.push_back(opcodes::LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(value_var_key));
        }
        else if (value_node_type._Equal(parser::ASTNodeType_FunctionCall))
        {
            // If the value is a function call
            //
            // Compile the function call
            compile_function_call(&node, stack_frame, bytecode);

            // Load the return value from the function
            bytecode.push_back(opcodes::REG_LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x02));
        }
        else if (value_node_type._Equal(parser::ASTNodeType_ClassInstantiation))
        {
            // If class instantiation is attempted
            compile_class_instantiation(&node, stack_frame, bytecode);

            // Load the allocated object
            bytecode.push_back(opcodes::REG_LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x01));
        }
    }

    void YosenCompiler::compile_function_call(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        // Get function name
        auto& function_name = node["name"].string_value();

        // Get function index in the list of function names in a stack frame
        auto function_index = stack_frame.get_function_index(function_name);

        // Check if function has not occured yet
        if (function_index == -1)
        {
            function_index = stack_frame.function_names.size();
            stack_frame.function_names.push_back(function_name);
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

        if (!node["caller"].is_null())
        {
            auto caller_name = node["caller"].string_value();

            // Make sure the variable exists
            if (stack_frame.var_keys.find(caller_name) == stack_frame.var_keys.end())
            {
                auto ex_reason = "Undefined variable \"" + caller_name + "\" used";
                YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
                return;
            }

            // Get the key for the caller object variable in the stack frame
            auto caller_object = stack_frame.var_keys.at(caller_name);

            // Set the caller flag
            has_caller_flag = 0x01;

            // Load the caller object
            bytecode.push_back(opcodes::LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(caller_object));
        }

        // Create the bytecode for calling the function
        bytecode.push_back(opcodes::CALL);
        bytecode.push_back(static_cast<opcodes::opcode_t>(function_index));
        bytecode.push_back(has_caller_flag); // Caller (this) object
    }

    void YosenCompiler::compile_variable_declaration(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        auto& variable_name = node["name"].string_value();
        auto  value_node = node["value"];

        // Check if the variable exists
        if (stack_frame.var_keys.find(variable_name) != stack_frame.var_keys.end())
        {
            auto ex_reason = "Variable \"" + variable_name + "\" already exists";
            YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
            return;
        }

        // Get the next available variable key
        uint32_t var_key = (uint32_t)stack_frame.var_keys.size();

        // Create a variable key entry for the variable name
        stack_frame.var_keys.insert({ variable_name, var_key });

        // Create an entry in the variables map
        stack_frame.vars.insert({ var_key, YosenObject_Null->clone() });

        // Compiling the expression and loading its value
        compile_expression(&value_node, stack_frame, bytecode);

        // At this point, the value object is loaded into LLOref,
        // now we need to create bytecode for storing the value into a variable.
        bytecode.push_back(opcodes::STORE);
        bytecode.push_back(static_cast<opcodes::opcode_t>(var_key));
    }

    void YosenCompiler::compile_variable_assignment(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        auto& variable_name = node["name"].string_value();
        auto  value_node = node["value"];

        // Make sure the variable exists
        if (stack_frame.var_keys.find(variable_name) == stack_frame.var_keys.end())
        {
            auto ex_reason = "Undefined variable \"" + variable_name + "\" used";
            YosenEnvironment::get().throw_exception(CompilerException(ex_reason));
            return;
        }

        // Get the variable key
        uint32_t var_key = stack_frame.var_keys.at(variable_name);

        // Compiling the expression and loading its value
        compile_expression(&value_node, stack_frame, bytecode);

        // At this point, the value object is loaded into LLOref,
        // now we need to create bytecode for storing the value into a variable.
        bytecode.push_back(opcodes::STORE);
        bytecode.push_back(static_cast<opcodes::opcode_t>(var_key));
    }

    void YosenCompiler::compile_class_instantiation(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        // Get function name
        auto& class_name = node["name"].string_value();

        // Get function index in the list of function names in a stack frame
        auto class_name_index = stack_frame.get_class_name_index(class_name);

        // Check if function has not occured yet
        if (class_name_index == -1)
        {
            class_name_index = stack_frame.class_names.size();
            stack_frame.class_names.push_back(class_name);
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

    std::pair<StackFrame, bytecode_t> YosenCompiler::compile_source(std::string& source)
    {
        return { StackFrame(), bytecode_t() };
    }
    
    bytecode_t YosenCompiler::compile_single_statement(std::string& source, StackFrame& stack_frame)
    {
        bytecode_t bytecode;

        parser::Parser parser;
        auto ast = parser.parse_single_statement(source);
        printf("\nAST:\n");
        printf("%s\n", ast.dump().c_str());
        printf("\n");

        compile_statement(&ast, stack_frame, bytecode);

        printf("Bytecode:\n");
        debug_print_bytecode(bytecode);

        printf("\n");
        return bytecode;
    }
}