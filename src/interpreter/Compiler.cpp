#include "Compiler.h"
#include "../parser/Parser.h"

namespace yosen
{
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

    static size_t get_function_index(const std::string& fn_name, StackFrame& stack_frame)
    {
        auto it = std::find(
            stack_frame.function_names.begin(),
            stack_frame.function_names.end(),
            fn_name
        );

        // If element was found
        if (it != stack_frame.function_names.end())
            return it - stack_frame.function_names.begin();
        
        return -1;
    }

    void Compiler::shutdown()
    {
        for (auto& stack_frame : allocated_stack_frames)
            destroy_stack_frame(*stack_frame);
    }

    void Compiler::debug_print_bytecode(bytecode_t& bytecode)
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
            default:
                printf("0x%x ", *it);
            }

            it++;
        }
    }

    bool Compiler::is_user_defined_function(const std::string& name)
    {
        return user_compiled_functions.find(name) != user_compiled_functions.end();
    }

    YosenFunction& Compiler::get_user_defined_function(const std::string& name)
    {
        return user_compiled_functions.at(name);
    }

    void Compiler::compile_source(std::string& source)
    {
        parser::Parser parser;
        parser::AST ast = parser.parse_source(source);

        // Compile all functions
        for (auto& fn : ast.functions)
            compile_function(&fn);
    }

    void Compiler::destroy_stack_frame(StackFrame& stack_frame)
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

    void Compiler::compile_function(json11::Json* node_ptr)
    {
        auto& node = *node_ptr;

        if (!node["type"].string_value()._Equal(parser::ASTNodeType_FunctionDeclaration))
            throw "Node is not a function declaration";

        StackFrame stack_frame;
        bytecode_t bytecode;

        // Registering the function name
        stack_frame.name = node["name"].string_value();
        printf("Compiling function \"%s\"...\n", stack_frame.name.c_str());

        // Allocating a Null object in the variables map (always first)
        stack_frame.var_keys.insert({ "null", 0 });
        stack_frame.vars.insert({ 0, YosenObject_Null->clone() });

        // Allocating parameters and their variable spaces
        uint32_t param_idx = 0;
        for (auto& param : node["params"].array_items())
        {
            auto param_name = param.string_value();
            printf("\tParameter: \"%s\"\n", param_name.c_str());

            // Registering the parameter on the stack frame
            stack_frame.params.push_back({ param_name, nullptr });
            
            auto param_key = param_idx + 1;

            // Reserving variable space on the stack frame
            stack_frame.var_keys.insert({ param_name, param_key });
            stack_frame.vars.insert({ param_key, YosenObject_Null->clone() });

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
            printf("%s\n", statement.dump().c_str());
            compile_statement(&statement, stack_frame, bytecode);
        }

        // If a user-defined return statement doesn't exist,
        // create an implicit "return null".
        if (node[parser::ASTNodeType_ReturnStatement] == json11::Json::NUL)
        {
            // Load YosenObject_Null
            bytecode.push_back(opcodes::LOAD);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x00));

            // Store it in the return register
            bytecode.push_back(opcodes::REG_STORE);
            bytecode.push_back(static_cast<opcodes::opcode_t>(0x02));
        }

        // Register the compiled function
        user_compiled_functions[stack_frame.name] = { stack_frame, bytecode };

        // Register the allocated stack frame
        allocated_stack_frames.push_back(&user_compiled_functions[stack_frame.name].stack_frame);
        
        printf("\n");
        debug_print_bytecode(bytecode);
        printf("\n");
    }

    void Compiler::compile_statement(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;
        auto type = node["type"].string_value();

        if (type._Equal(parser::ASTNodeType_VariableDeclaration))
            compile_variable_declaration(&node, stack_frame, bytecode);

        else if (type._Equal(parser::ASTNodeType_VariableAssignment))
            compile_variable_assignment(&node, stack_frame, bytecode);

        else if (type._Equal(parser::ASTNodeType_FunctionCall))
            compile_function_call(&node, stack_frame, bytecode);
    }

    void Compiler::compile_variable_declaration(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        auto& variable_name = node["name"].string_value();
        auto  value_node = node["value"];

        // Check if the variable exists
        if (stack_frame.var_keys.find(variable_name) != stack_frame.var_keys.end())
            throw "Variable already exists!";

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

    void Compiler::compile_variable_assignment(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        auto& variable_name = node["name"].string_value();
        auto  value_node = node["value"];

        // Make sure the variable exists
        if (stack_frame.var_keys.find(variable_name) == stack_frame.var_keys.end())
        {
            throw std::exception("Undefined variable used");
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

    void Compiler::compile_function_call(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
    {
        auto& node = *node_ptr;

        // Get function name
        auto& function_name = node["name"].string_value();

        // Get function index in the list of function names in a stack frame
        auto function_index = get_function_index(function_name, stack_frame);

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
        uint32_t caller_object = 0;
        if (!node["caller_object"].is_null())
        {
            auto caller_name = node["caller_object"].string_value();

            // Make sure the variable exists
            if (stack_frame.var_keys.find(caller_name) == stack_frame.var_keys.end())
                throw "Undefined variable used";

            // Get the key for the caller object variable in the stack frame
            caller_object = stack_frame.var_keys.at(caller_name);
        }

        // Create the bytecode for calling the function
        bytecode.push_back(opcodes::CALL);
        bytecode.push_back(static_cast<opcodes::opcode_t>(function_index));
        bytecode.push_back(static_cast<opcodes::opcode_t>(caller_object)); // Caller (this) object
    }

    void Compiler::compile_expression(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode)
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
    }

    uint32_t Compiler::get_constant_literal_key(json11::Json* node_ptr, StackFrame& stack_frame)
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

    uint32_t Compiler::get_variable_key(json11::Json* node_ptr, StackFrame& stack_frame)
    {
        auto& node = *node_ptr;

        // Get the variable name
        auto& identifier_value = node["value"].string_value();

        // Make sure the variable exists
        if (stack_frame.var_keys.find(identifier_value) == stack_frame.var_keys.end())
            throw "Undefined variable used";

        // Get the key for the variable in the stack frame
        auto var_key = stack_frame.var_keys.at(identifier_value);

        return var_key;
    }

    void Compiler::compile_function(const std::string& source)
	{
        if (source._Equal("main"))
        {
            StackFrame frame;
            bytecode_t bytecode;

            /* EXAMPLE CODE:

            func main(args) {
                var age = "22";
                var str = "Yosen";

                println(str);
                reverse(str);

                println(age);
                println(str);

                str = age;
                println(str);

                str = input();
                println(str);
            }

            */

            // For testing purposes
            using namespace opcodes;

            frame.name = "main";
            frame.params = {
                { "args", nullptr }
            };
            frame.var_keys = {
                { "args", 1 },
                { "age",  2 },
                { "str",  3 },
            };
            frame.vars = {
                { 0, YosenObject_Null->clone() }, // YosenObject_Null
                { 1, YosenObject_Null->clone() }, // reserved for parameter:args
                { 2, YosenObject_Null->clone() }, // reserved for var:age
                { 3, YosenObject_Null->clone() }, // reserved for var:str
            };
            frame.function_names = {
                "reverse",
                "println",
                "input"
            };
            frame.constant_keys = {
                { "22", 0 },
                { "Yosen", 1 },
            };
            frame.constants = {
                { 0, allocate_object<YosenString>("22") },
                { 1, allocate_object<YosenString>("Yosen") }
            };

            bytecode = {
                // Preamble: moving parameters into the appropriate variable slots
                LOAD_PARAM, 0x00,
                STORE, 0x01,

                // var age = 22;
                LOAD_CONST, 0x00,
                STORE, 0x02,

                // var str = "Yosen";
                LOAD_CONST, 0x01,
                STORE, 0x03,

                // println(str);
                LOAD, 0x03,
                PUSH,
                CALL, 0x01, 0x00,

                // reverse(str);
                LOAD, 0x03,
                PUSH,
                CALL, 0x00, 0x00,

                // println(age)
                LOAD, 0x02,
                PUSH,
                CALL, 0x01, 0x00,

                // println(str);
                LOAD, 0x03,
                PUSH,
                CALL, 0x01, 0x00,

                // str = age
                LOAD, 0x02,         // load "age"
                STORE, 0x03,        // stores the last loaded object into the variable "str" at index 3

                // println(str);
                LOAD, 0x03,
                PUSH,
                CALL, 0x01, 0x00,

                // str = input();
                CALL, 0x02, 0x00,   // calling the "input" function
                REG_LOAD, 0x02,     // loads and stores the object from the return register into the last loaded object
                STORE, 0x03,        // stores the last loaded object into the variable "str" at index 3

                // println(str);
                LOAD, 0x03,
                PUSH,
                CALL, 0x01, 0x00,

                // *Implicit* return YosenObject_Null
                LOAD, 0x00,
                RETURN
            };

            user_compiled_functions["main"] = { frame, bytecode };
        }
        else if (source._Equal("reverse"))
        {
            StackFrame frame;
            bytecode_t bytecode;

            // For testing purposes
            using namespace opcodes;

            frame.name = "reverse";
            frame.params = {
                { "str", nullptr }
            };
            frame.var_keys = {
                { "str", 1 },
            };
            frame.vars = {
                { 0, YosenObject_Null->clone() }, // YosenObject_Null
                { 1, YosenObject_Null->clone() }, // reserved for parameter:str
            };
            frame.function_names = {
                "reverse",
                "println"
            };
            frame.constant_keys = {
                { "", 0 },
            };
            frame.constants = {
                { 0, allocate_object<YosenString>("") },
            };

            bytecode = {
                // Preamble: moving parameters into the appropriate variable slots
                LOAD_PARAM, 0x00,
                STORE, 0x01,

                // println(str)
                LOAD, 0x01,
                PUSH,
                CALL, 0x00, 0x01,

                REG_LOAD, 0x02,
                PUSH,
                CALL, 0x01, 0x00,

                // println("");
                LOAD_CONST, 0x00,
                PUSH,
                CALL, 0x01, 0x00,

                // *Implicit* return YosenObject_Null
                LOAD, 0x00,
                RETURN
            };

            user_compiled_functions["reverse"] = { frame, bytecode };
        }
	}
}
